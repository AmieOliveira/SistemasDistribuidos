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
#include <pthread.h>
#include <chrono>


#include "valentao.h"


using namespace std;



// User interface management
void* interface(void*){
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
            cout << "Pressed L: Checking on leader" << endl;

            if (leaderIdx == -1) {
                // TODO: lock in cout!!
                cout << "\t Am already leader. Can't check on myself." << endl;
            }
            else if (ongoingElections.size() != 0){
                cout << "\t Already in leader election. No leader to check on." << endl;
            } else if (isCheckingOnLeader) {
                cout << "\t Already checking on leader. Will wait for the results." << endl;
            } else if (isOperational) {
                cout << "\t Am emulating failure. Can't send messages "
                     << "unless you press recovery button (R)." << endl;
            } else {
                isCheckingOnLeader = true;

                char buffer[messageLength];
                sprintf(buffer, "%i%s%i%s", m_vivo, delimiter,
                                            selfID, delimiter );

                cout << "Messaging process. (" << buffer << ")" << endl;

                leaderAnswered = false;
                processes[leaderIdx].sendMessage(buffer);
                outMsgCount++;

                sleep(3); // Wait to see if there'll be an answer

                isCheckingOnLeader = false;
            }
        }

        // Emulate process failure
        else if ( strncmp(l_p, "f", 1) == 0 ){
            // TODO: locks! (both on flag and cout)
            cout << "Pressed F: Process will emulate failure" << endl;

            isOperational = false;

        }

        // Recuperate process from (fake) failure
        else if ( strncmp(l_p, "r", 1) == 0 ){
            // TODO: locks! (both on flag and cout)
            cout << "Pressed R: Process will start answering again" << endl;

            isOperational = false;
        }

        // Print Statistics
        else if ( strncmp(l_p, "s", 1) == 0 ){
            cout << "Pressed S: Printing statistcs" << endl;
            // TODO: Make process non-responsive to messages
            cout << "---------------------------" << endl;
            if (leaderIdx == -1) {
                cout << "| leader          |  " << selfID << " |" << endl;
            } else {
                cout << "| leader          |  " << processes[leaderIdx].myport << " |" << endl;
            }
            cout << "---------------------------" << endl;
            cout << " Message counting: " << endl;
            cout << "     - ELEICAO -> " << msgCount[m_eleicao] << endl;
            cout << "     - OK -> " << msgCount[m_ok] << endl;
            cout << "     - LIDER -> " << msgCount[m_lider] << endl;
            cout << "     - VIVO -> " << msgCount[m_vivo] << endl;
            cout << "     - VIVO_OK -> " << msgCount[m_vivo_ok] << endl;
        }
    }
}


void* communication(void*){
    /*
     *  Thread responsible for receiving messages from other processes and
     *
     */
    cout << "In communication thread" << endl;

    struct sockaddr_in client_address;
    socklen_t addrlen = sizeof(client_address);

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
                   inet_ntoa(client_address.sin_addr), inMsgCount);
        }
        inMsgCount++;

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

            if (isOperational){
                // If ID is bigger than the received, should initiate election
                if (selfElecValue > elecValueReceived){
                    // TODO: Sleep shortly to avoid sending too many elections

                    // Checking if has sent this election message already
                    bool messageSent = false;
                    for (int i; i<ongoingElections.size(); i++){
                        if (ongoingElections[i] == msgSender){
                            messageSent = true;
                            break;
                        }
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
                        outMsgCount++;

                        ongoingElections.push_back(msgSender);

                        // Creating auxiliar thread
                        pthread_t threadsAux;
                        pthread_attr_t attrAux;

                        pthread_attr_init(&attrAux);
                        pthread_attr_setdetachstate(&attrAux, PTHREAD_CREATE_DETACHED);

                        pthread_create(&threadsAux, &attrAux, electionFinish, NULL );
                    }
                }
            }
            // If ID is smaller than received it makes no sense to do anything else
        } else if (msgT == m_ok){
            // TODO: Check if this erasing should't be done after some thread notices the election is over or something
            isSilenced = true;
            for (int i = 0; i < ongoingElections.size(); i++){
              if (ongoingElections[i] == msgSender){
                  // Take out of elections
                  ongoingElections.erase( ongoingElections.begin()+i );
                  break;
              }
            }

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
            if (isOperational){
                if (leaderIdx == -1){ // NOTE: If I am the leader, leaderIdx is -1
                    char outMsg[messageLength];
                    sprintf( outMsg, "%i%s%i%s",
                             m_vivo_ok, delimiter, selfID, delimiter );

                    // Sending VIVO_OK message back
                    for (int i=0; i<N_PROC-1; i++){
                        if (processes[i].myport == msgSender){ // NOTE: Change if I change selfID configuration!!
                            processes[i].sendMessage(outMsg);
                            outMsgCount++;
                            break;
                        }
                    }
                } else {
                    cout << "ERROR: Received leader check message when I am not the leader" << endl;
                }
            }
            // TODO: Check this

        }   else if (msgT == m_vivo_ok){
            if (msgSender == processes[leaderIdx].myport)
                leaderAnswered = true;
            else {
                // TODO: Put locks into every cout!!
                cout << "ERROR: Received leader aliveness confirmation from process that isn't the leader" << endl;
            }
            // TODO: Check if there's something else
        }
    }
}


void* leader(void*){
    /*
     *  Thread responsible for checking current leader
     *
     */
    cout << "In leader check thread" << endl;

    // TODO: lock!!
    isCheckingOnLeader = false;

    while (1){
        // TODO: If I am not leader, send messafe to leader. Check if message was received
        if (leaderIdx == -1 || !isOperational){ // NOTE: If I am the leader, leaderIdx is -1
            sleep(20);

        } else if ( (!isCheckingOnLeader) && (ongoingElections.size() == 0) ) {
            cout << "Will check on leader" << endl;

            // If I am not checking leader nor running an election, I should check the leader!
            isCheckingOnLeader = true;

            char buffer[messageLength];
            sprintf(buffer, "%i%s%i%s", m_vivo, delimiter,
                                        selfID, delimiter );

            cout << "Messaging leader. (" << buffer << ")" << endl;

            leaderAnswered = false;
            processes[leaderIdx].sendMessage(buffer);
            outMsgCount++;

            sleep(3); // Wait to see if there'll be an answer

            isCheckingOnLeader = false;

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
                outMsgCount++;

                ongoingElections.push_back(selfID);

                // Creating auxiliar thread
                pthread_t threadsAux;
                pthread_attr_t attrAux;

                pthread_attr_init(&attrAux);
                pthread_attr_setdetachstate(&attrAux, PTHREAD_CREATE_DETACHED);

                pthread_create(&threadsAux, &attrAux, electionFinish, NULL );
                // TODO: Check

            } // else { cout << "Leader ok" << endl; }
            sleep(20);
        } else {
            sleep(20);
        }
    }
}


int main(int argc, char* argv[]){
    // int myServerPort = 8081;
    int sendPorts[N_PROC] = {8080, 8081, 8082, 8083, 8084};
    messageLength = 1024;
    delimiter = new char[2];
    sprintf(delimiter, "\\");
    isOperational = true; // TODO: locks!
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
    }

    int threadResponse[3];
    int threadStatus;
    pthread_t threads[3];
    pthread_attr_t attr;
    void* status;

    // Initialize and set thread joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    cout << "Creating threads" << endl;

    threadResponse[0] = pthread_create(&threads[0], &attr, interface, NULL );
    threadResponse[1] = pthread_create(&threads[0], &attr, communication, NULL );
    threadResponse[2] = pthread_create(&threads[0], &attr, leader, NULL );

    for( int i = 0; i < 3; i++ ) {
        if (threadResponse[i]){
        cout << "Error:unable to create thread," << threadResponse[i] << endl;
        exit(-1);
        }
    }

    // free attribute and wait for the other threads
    pthread_attr_destroy(&attr);
    for( int i = 0; i < 3; i++ ) {
       threadStatus = pthread_join(threads[i], &status);
       if (threadStatus) {
          cout << "Error:unable to join," << threadStatus << endl;
          exit(-1);
       }
       cout << "Main: completed thread " << i ;
       cout << "  Exiting with status :" << status << endl;
    }

    cout << "Main: program exiting." << endl;
}




// Auxiliary Functions ----------------------------------------------------------

int setupServerSocket(int port){
    /* Funtion to create a server socket
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


void* electionFinish(void*){
    /* Thread to wait out for election results and execute end of election procedures
     */
    sleep(5);

    if (!isSilenced){
        // No OK message was received. Therefore I am the current leader
        leaderIdx = -1;

        char outMsg[messageLength];
        sprintf(outMsg, "%i%s%i%s", m_lider, delimiter,
                                    selfID, delimiter  );

        // Broadcast new leader message
        for (int i=0; i<N_PROC-1; i++){
            processes[i].sendMessage(outMsg);
        }
        outMsgCount++;
    }

    pthread_exit(NULL);
}


// ProcessClient class ----------------------------------------------------------
ProcessClient::ProcessClient(int port, string name){
    myPid = getpid();
    processName = name;
    myport = port;
}

int ProcessClient::getPid(){ // NOTE: What is the usage of this?
    return (int)myPid;
}

int ProcessClient::sendMessage(char client_buffer[]){
    // TODO: Comment this out
    cout << "Sending message '" << client_buffer << "' to process " << myport << endl;

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
