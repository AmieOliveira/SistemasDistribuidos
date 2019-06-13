/*
 *  Header for the 'valentao' code
 *  Author: Amanda, Vinicius
 *
 */

#include <vector>


// Processes class
class ProcessClient {
    pid_t myPid;
    std::string processName;
public:
    struct sockaddr_in myaddr,remaddr;
    int client_socket_ID;
    int myport;

    ProcessClient(int port, std::string name);
    int getPid();
    int sendMessage(char* msg);
};


// Global variables
int server_socket;         // This process' socket
// TODO: Should it be protected by lock?
int selfID;
int leaderID;

char* messageBuffer;
int messageLength;

char* delimiter;

int msgCount[5] = {0, 0, 0, 0, 0};
int outMsgCount = 0;

std::vector<int> ongoingElections;

std::vector<ProcessClient> processes;
#define N_PROC 5      // Number of processes operating

// Message types
enum MessageTyp {
    m_eleicao = 0,
    m_ok = 1,
    m_lider = 2,
    m_vivo = 3,
    m_vivo_ok = 4
};

// Main funtions
int main(int argc, char* argv[]);

void interface();
void communication();
//void leader();


// Auxiliary funtions
//void receiveMsgFromClients(int flag);
int setupServerSocket(int port);
int setupClientSocket(ProcessClient& clientProcess);
void sendMsgToClient(int flag,ProcessClient& clientProcess);

//bool checkLeader();
