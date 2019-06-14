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
    int setupClientSocket();
};


// Global variables
int server_socket;         // This process' socket
// TODO: Should it be protected by lock?
int selfID;
int leaderIdx;   // leaderIdx should be the position in the process vector, or -1 if the leader is the process itslef
int selfElecValue;

int messageLength;

char* delimiter;

int msgCount[5] = {0, 0, 0, 0, 0};
int outMsgCount = 0;

std::vector<int> ongoingElections;

std::vector<ProcessClient> processes;
#define N_PROC 5      // Number of processes operating

// std::chrono::high_resolution_clock::time_point start_time;
// std::chrono::high_resolution_clock::time_point actual_time;


// Global flags
bool isOperational;
bool leaderAnswered;
bool isSilenced;
bool isCheckingOnLeader;
// bool resetTimer=false;


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

void* interface(void*);
void* communication(void*);
void* leader(void*);


// Auxiliary funtions
int setupServerSocket(int port);
void* electionFinish(void*);
