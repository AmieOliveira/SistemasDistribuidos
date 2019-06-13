#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <algorithm>


#include "valentao.h"


using namespace std;



// User interface management
void interface(){
    /*
     *  Thread responsible for the user interface
     *
     */
    cout << "In interface thread" << endl;

    // Setup to read keys
    struct termios oldSettings, newSettings;

    tcgetattr( fileno( stdin ), &oldSettings );
    newSettings = oldSettings;
    newSettings.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr( fileno( stdin ), TCSANOW, &newSettings );

    while(1){
        // Read letter
        char l;
        char* l_p = &l;

        fd_set set;
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO( &set );
        FD_SET( fileno( stdin ), &set );
        int res = select( fileno( stdin )+1, &set, NULL, NULL, &tv );
        if ( res > 0 ){
            read( fileno( stdin ), l_p, 1 );
            // cout << "Pressed key: " << l << endl;
            l = tolower(l);
        } else if ( res < 0 ){
            perror( "select error" );
            break;
        }

        // Check current leader
        if ( strncmp(l_p, "l", 1) == 0 ){
            cout << "Pressed L" << endl;
            cout << "\t Checking on leader" << endl;
            // checkLeader();
            // TODO: Initiate leader check process.
            // Might be better to take leader check thread out of sleep...
        }

        // Emulate process failure
        else if ( strncmp(l_p, "f", 1) == 0 ){
            cout << "Pressed F" << endl;
            cout << "\t Process will emulate failure" << endl;
            // TODO: Make process non-responsive to messages
        }

        // Recuperate process from (fake) failure
        else if ( strncmp(l_p, "r", 1) == 0 ){
            cout << "Pressed R" << endl;
            cout << "\t Process will start answering again" << endl;
            // TODO: Make process non-responsive to messages
        }

        // Print Statistics
        else if ( strncmp(l_p, "s", 1) == 0 ){
            cout << "Pressed S" << endl;
            cout << "\t Printing statistcs" << endl;
            // TODO: Make process non-responsive to messages
        }
    }
}


void communication(){
    /*
     *  Thread responsible for receiving messages from other processes and
     *
     */
    cout << "In communication thread" << endl;

    struct sockaddr_in client_address;
    socklen_t addrlen = sizeof(client_address);
    int totCount = 0;

    while (true){
        char buffer[messageLength];
        // read content into buffer from an incoming client
        int recvlen = recvfrom(server_socket, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&client_address,
                             &addrlen);
        // inet_ntoa prints user friendly representation of the
        // ip address
        if (recvlen>0){
          buffer[recvlen] = 0;
          printf("received: '%s' from client %s , Count:%d\n", buffer,
                 inet_ntoa(client_address.sin_addr),totCount);
        }
        totCount++;

        // Interpreting message
        char* msgPart;
        int msgT;
        int msgSender;

        try {
            msgPart = strtok(buffer, delimiter);
            msgT = atoi(msgPart);   // Message type number

            // TODO: Put lock!!!
            msgCount[msgT]++;

            msgPart = strtok(NULL, delimiter);
            msgSender = atoi(msgPart);
        } catch(...) {
            cout << "ERROR: Message reading error" << endl;
            continue;
        }

        if (msgT == m_eleicao){
            msgPart = strtok(NULL, delimiter);
            int elecValueReceived = atoi(msgPart);

            // If ID is bigger than the received, should initiate election
            if (selfElecValue > elecValueReceived){
                // TODO: Sleep shortly to avoid sending too many elections

                // Checking if has sent this election message already
                bool messageSent = false;
                for (int i; i<ongoingElections.size(); i++){
                    if (ongoingElections[i] == msgSender) messageSent = true;
                }

                if (!messageSent){ // Hasn't sent messages for this election
                    char outMsg[messageLength];
                    sprintf( outMsg, "%i%s%i%s%i%s",
                             m_eleicao, delimiter,
                             msgSender, delimiter,
                             selfElecValue, delimiter );

                    isSilenced = false;

                    // Broadcast of election message
                    for (int i=0; i<N_PROC-1; i++){
                        processes[i].sendMessage(outMsg);
                    }

                    ongoingElections.push_back(msgSender);
                    // TODO: Start time counting to see if will be leader
                    // NOTE: WHERE IS THIS TIMER HANDLED??
                    // (Possibly use a thread)
                }
            }
            // If ID is smaller than received it makes no sense to do anything else
        } else if (msgT == m_ok){
            // TODO: Check if this erasing should't be done after some thread notices the election is over or something
            isSilenced = true;
            ongoingElections.erase(std::remove(ongoingElections.begin(),
                                                ongoingElections.end(), msgSender),
                                    ongoingElections.end() ); // Taken out of elections

        } else if (msgT == m_lider){
            // Update leader ID
            for (int i=0; i<N_PROC-1; i++){
                if (processes[i].myport == msgSender){
                    leaderIdx = i;
                    break;
                }
            }
            // TODO: Check!!
            // TODO: Implement locks!!

        } else if (msgT == m_vivo){
            // In this case I am the leader myself, so I should answer the inquiry

            cout << "Received VIVO message" << endl;
            if (leaderIdx == -1){ // NOTE: If I am the leader, leaderIdx is -1
                char outMsg[messageLength];
                sprintf( outMsg, "%i%s%i%s",
                         m_vivo_ok, delimiter, selfID, delimiter );
                // cout << "Sending VIVO_OK message back (" << outMsg << ")" << endl;

                leaderAnswered = false;
                for (int i=0; i<N_PROC-1; i++){
                    if (processes[i].myport == msgSender){ // NOTE: Change if I change selfID configuration!!
                        processes[i].sendMessage(outMsg);
                        break;
                    }
                }
            } else {
                cout << "ERROR: Received leader check message when I am not the leader" << endl;
            }
            // TODO: Check this

        }   else if (msgT == m_vivo_ok){
            leaderAnswered = true;
            // TODO: Check if there's something else
        }
    }
}


void leader(){
    /*
     *  Thread responsible for checking current leader
     *
     */
    cout << "In leader check thread" << endl;

    while (1){
        // TODO: If I am not leader, send messafe to leader. Check if message was received
        if (leaderIdx == -1){ // NOTE: If I am the leader, leaderIdx is -1
            sleep(100);

        } else {
            char buffer[messageLength];
            sprintf(buffer, "%i%s%i%s", m_vivo, delimiter,
                                        selfID, delimiter );

            cout << "Messaging process. (" << buffer << ")" << endl;

            processes[leaderIdx].sendMessage(buffer);

            sleep(3); // Wait to see if there'll be an answer

            // TODO: locks!!
            if (!leaderAnswered){ // No answer from leader
                // TODO: Put timer to delay beginning
                cout << "No leader! Start election!" << endl;

                // Initiating election
                char outMsg[messageLength];
                sprintf( outMsg, "%i%s%i%s%i%s",
                         m_eleicao, delimiter,
                         selfID, delimiter,
                         selfElecValue, delimiter );

                isSilenced = false;

                // Broadcast of election message
                for (int i=0; i<N_PROC-1; i++){
                    processes[i].sendMessage(outMsg);
                }

                ongoingElections.push_back(selfID);
                // TODO: Start time counting to see if will be leader
                // NOTE: WHERE IS THIS TIMER HANDLED??
                // (Possibly use a thread)

                // TODO: End of elections!!

            } // else { cout << "Leader ok" << endl; }

            sleep(100);
        }
    }
}


int main(int argc, char* argv[]){
    // int myServerPort = 8081;
    int sendPorts[N_PROC] = {8080, 8081, 8082, 8083, 8084};
    messageLength = 1024;
    delimiter = new char[2];
    sprintf(delimiter, "\\");
    // ----

    cout << "Delimiter = " << delimiter << endl;

    // Function arguments
    if (argc < 2){
        cout << "ERROR: Enter a port number." << endl;
        return 1;
    }

    int myServerPort = atoi(argv[1]);
    if ( (myServerPort < 8080) || (myServerPort > 8084) ){
        cout << "ERROR: Enter a valid port number. Options are 8080, 8081, 8082, 8083 or 8084." << endl;
        return 1;
    }

    selfID = myServerPort;
    selfElecValue = myServerPort; // TODO: Change this (PID, for example)
    // ----

    // for (int i = 0; i < N_PROC-1; i++){
    //     std::stringstream procName;
    //     procName << "Processo" << i;
    //     ProcessClient Proc(sendPorts[i], procName.str());
    //     processes.push_back(Proc);
    //     cout<<"Criando processos"<<endl;
    // }

    if ( setupServerSocket(myServerPort) == -1 ) {
        cout << "ERROR: Could not setup socket" << endl;
        exit(1);
    }


    int idx = 0;
    for (int i = 0; i < N_PROC; i++) {
        if (sendPorts[i] == myServerPort){
            // Set initial leader
            if (myServerPort == 8080){
                leaderIdx = -1;
                cout << "Am initial leader!" << endl;
            }
            continue;
        }

        std::stringstream procName;
        procName << "Processo" << idx;
        ProcessClient Proc(sendPorts[i], procName.str());
        Proc.setupClientSocket();
        cout<<"Processo Criado"<<endl;

        processes.push_back(Proc);

        // Set initial leader
        if (sendPorts[i] == 8080){
            leaderIdx = idx;
            cout << "Initial leader: " << sendPorts[i] << endl;
        }

        idx++;
        // char client_buffer[messageLength];
        // sprintf(client_buffer, "0\\1");
        // Proc.sendMessage(client_buffer);
    }

    // interface();
    // communication();
    leader();
}






// Auxiliary Functions ----------------------------------------------------------

int setupServerSocket(int port){
    /*
     *  Thread responsible for create a server socket
     *
     */
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if ( bind( server_socket, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
        cout << "ERROR: Unable to bind socket" << endl;
        perror("bind failed");
        return -1;
    }
    return 0;
}


// ProcessClient class
ProcessClient::ProcessClient(int port, string name){
    myPid = getpid();
    processName = name;
    myport = port;
}

int ProcessClient::getPid(){ // NOTE: What is the usage of this?
    return (int)myPid;
}

int ProcessClient::sendMessage(char client_buffer[]){
    socklen_t addrlen = sizeof(remaddr);
    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(myport);
    char server[] = "127.0.0.1";
    int recvlen;
    if (inet_aton(server, &remaddr.sin_addr)==0){
        fprintf(stderr, "inet_aton() failed\n");
        return 1;
	  }

    if (sendto(client_socket_ID, client_buffer, strlen(client_buffer), 0, (struct sockaddr *)&remaddr, addrlen)==-1) {
        perror("sendto");
        return 1;
	  }

    return 0; // If everything went fine
}

int ProcessClient::setupClientSocket(){
    /*
     *  Thread responsible for create a client socket
     *
     */
    //struct sockaddr_in myaddr;
    int port = myport;
    int client_socket;
    int recvlen;		/* # bytes in acknowledgement message */
    char server[] = "127.0.0.1";	/* change this to use a different server */
    if ((client_socket=socket(AF_INET, SOCK_DGRAM, 0))==-1){
        printf("socket created\n");
    }
    memset( (char *)&myaddr, 0, sizeof(myaddr) );
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    if ( bind(client_socket, (struct sockaddr *)&myaddr,
              sizeof(myaddr)) < 0 ){
        perror("bind failed");
        exit(0);
    }
    //client_sockets.push_back(client_socket);
    client_socket_ID=client_socket;
    return 1;
}
