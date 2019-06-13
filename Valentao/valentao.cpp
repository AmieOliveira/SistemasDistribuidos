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
     *  Flag 1 -> Send ack for confirmation
     */
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
        try
        {
        char* msgPart = strtok(buffer, delimiter);
        int msgT = atoi(msgPart);   // Message type number

        // TODO: Put lock!!!
        msgCount[msgT]++;

        msgPart = strtok(NULL, delimiter);
        int msgSender = atoi(msgPart);
        }
        catch(...)
        {
            std::cout<<"Error"<<std::endl;
        }

        // if (msgT == m_eleicao){
        //     msgPart = strtok(NULL, delimiter);
        //     int elecValueReceived = atoi(msgPart);

        //     // If ID is bigger than the received, should initiate election
        //     if (selfID > elecValueReceived){
        //         // TODO: Sleep shortly to avoid sending too many elections

        //         // Checking if has sent this election message already
        //         bool messageSent = false;
        //         for (int i; i<ongoingElections.size(); i++){
        //             if (ongoingElections[i] == msgSender) messageSent = true;
        //         }

        //         if (!messageSent){ // Hasn't sent messages for this election
        //             char outMsg[messageLength];
        //             sprintf( outMsg, "%i%s%i%s%i%s",
        //                      m_eleicao, delimiter,
        //                      msgSender, delimiter,
        //                      selfID, delimiter );
        //             //sendto(); BROADCAST
        //             ongoingElections.push_back(msgSender);
        //             // TODO: Start time counting to see if will be leader
        //             // NOTE: WHERE IS THIS TIMER HANDLED??
        //         }
        //     }
        //     // If ID is smaller than received it makes no sense to do anything else

        // } else if (msgT == m_ok){
        //     // TODO: Stop election time counting
        //     //       Or turn on OK flag!
        //     // TODO: Check if this erasing should't be done after some thread notices the election is over or something
        //     ongoingElections.erase(std::remove(ongoingElections.begin(),
        //                                         ongoingElections.end(), msgSender),
        //                             ongoingElections.end() ); // Taken out of elections

        // } else if (msgT == m_lider){
        //     // Update leader ID
        //     //leaderID = msgSender;   IDEALLY THE IDENTIFICATION SHOULD BE AN EASY WAY TO GET THE LEADER FROM THE PROCESSES ARRAY
        //     // TODO: Implement locks!!

        // } else if (msgT == m_vivo){
        //     // In this case I am the leader myself, so I should answer the inquiry
        //     char outMsg[messageLength];
        //     sprintf( outMsg, "%i%s%i%s",
        //              m_vivo, delimiter, selfID, delimiter );
        //     // TODO: Send message.........

        // }   else if (msgT == m_vivo_ok){
        //     // TODO: Deal with this case

        // }
    }
}


int main(int argc, char* argv[]){
    // TODO: Function arguments
    int myServerPort = 8080;
    int sendPorts[N_PROC-1] = {8080, 8082, 8083, 8084};
    messageLength = 1024;
    messageBuffer = new char[messageLength];
    //delimiter = '\n';
    // ----

    selfID = myServerPort; // TODO: Change this

    //if (argc < 2){
    //    printf("Enter a valid integer.");
    //    return 0;
    //}
    int numTimes = atoi(argv[1]);
    //if(argc > 2){
    //    sendPort = atoi(argv[2]);
    //}
    // ----

    // for (int i = 0; i < N_PROC-1; i++){
    //     std::stringstream procName;
    //     procName << "Processo" << i;
    //     ProcessClient Proc(sendPorts[i], procName.str());
    //     processes.push_back(Proc);
    //     cout<<"Criando processos"<<endl;
    // }
    std::cout<<numTimes<<endl;
    if (numTimes==1)
    {
       if ( setupServerSocket(myServerPort) == -1 )
       {
           cout << "ERROR: Could not setup socket" << endl;
           exit(1);
       }
       communication();
    }
    else
    {
        for (int i = 0; i < N_PROC-1; i++)
        {
            std::stringstream procName;
            procName << "Processo" << i;
            ProcessClient Proc(sendPorts[i], procName.str());
            cout<<"Processo Criado"<<endl;
            char client_buffer[messageLength];
            Proc.setupClientSocket();
            sprintf(client_buffer, "0\\1");
            Proc.sendMessage(client_buffer);
            processes.push_back(Proc);
        }
    }
    // interface();
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


int setupClientSocket(ProcessClient& clientProcess){
    /*
     *  Thread responsible for create a client socket
     *
     */
    //struct sockaddr_in myaddr;
    int port = clientProcess.myport;
    int client_socket;
    int recvlen;		/* # bytes in acknowledgement message */
    char server[] = "127.0.0.1";	/* change this to use a different server */
    if ((client_socket=socket(AF_INET, SOCK_DGRAM, 0))==-1){
        printf("socket created\n");
    }
    memset( (char *)&clientProcess.myaddr, 0, sizeof(clientProcess.myaddr) );
	clientProcess.myaddr.sin_family = AF_INET;
	clientProcess.myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientProcess.myaddr.sin_port = htons(0);
    if ( bind(client_socket, (struct sockaddr *)&clientProcess.myaddr,
              sizeof(clientProcess.myaddr)) < 0 ){
        perror("bind failed");
        exit(0);
    }
    //client_sockets.push_back(client_socket);
    clientProcess.client_socket_ID=client_socket;
    return 1;
}


void sendMsgToClient(int flag, ProcessClient& clientProcess){
    /*
     *  Thread responsible for sending msg in a client socket
     *
     */
    //struct sockaddr_in remaddr;
    int port=clientProcess.myport;
    socklen_t addrlen = sizeof(clientProcess.remaddr);
    memset((char *) &clientProcess.remaddr, 0, sizeof(clientProcess.remaddr));
    char client_buffer[messageLength];	/* message buffer */
	  clientProcess.remaddr.sin_family = AF_INET;
	  clientProcess.remaddr.sin_port = htons(port);
    char server[] = "127.0.0.1";
    int recvlen;
    //int count=0;
    //int temp_client_socket = client_sockets.back();
    if (inet_aton(server, &clientProcess.remaddr.sin_addr)==0){
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
	  }
    printf("Sending packet %d to %s on port %d\n", outMsgCount, server, port);
    sprintf(client_buffer, "This is packet %d", outMsgCount);
    if (sendto(clientProcess.client_socket_ID, client_buffer, strlen(client_buffer), 0, (struct sockaddr *)&clientProcess.remaddr, addrlen)==-1) {
        perror("sendto");
        exit(1);
	  }
    outMsgCount++;
    /* now receive an acknowledgement from the server */
    if(flag==1){
	      recvlen = recvfrom(clientProcess.client_socket_ID, client_buffer, messageLength, 0, (struct sockaddr *)&clientProcess.remaddr, &addrlen);
        if (recvlen >= 0){
            client_buffer[recvlen] = 0;	/* expect a printable string - terminate it */
            printf("received message: \"%s\"\n", client_buffer);
        }
    }
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
