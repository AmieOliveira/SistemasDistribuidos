/*
 *  Header for the 'valentao' code
 *  Author: Amanda, Vinicius
 *
 */

#include <vector>


//LOCKS
std::mutex electionVectorMutex;
std::mutex mtx_isCheckingOnLeader;
std::mutex mtx_isOperational;
std::mutex mtx_leaderIdx;
std::mutex mtx_isSilenced;
std::mutex mtx_outElection;
std::mutex mtx_leaderAnswered;
std::mutex mtx_msgCount;
std::mutex mtx_outMsgCount;
std::mutex mtx_inMsgCount;
std::mutex mtx_electionNumber;
std::mutex mtx_totalElectionMsgs;
std::mutex mtx_alreadyOnThreadleader;
std::mutex mtx_vivo_ok;

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
int electionNumber=0;
int totalElectionMsgs=0;
int caseNumber=0;
int number_vivo_ok=0;

char* delimiter;

int msgCount[5] = {0, 0, 0, 0, 0};
int outMsgCount = 0;
int inMsgCount = 0;


std::vector<int> ongoingElections;

std::vector<ProcessClient> processes;
int N_PROC;     // Number of processes operating

// std::chrono::high_resolution_clock::time_point start_time;
// std::chrono::high_resolution_clock::time_point actual_time;


// Global flags
bool isOperational;
bool leaderAnswered;
bool isSilenced;
bool isCheckingOnLeader;
bool alreadyOnThreadleader=false;
bool outElection=false;
// bool resetTimer=false;


// Message types
enum MessageTyp {
    m_eleicao = 0,
    m_ok = 1,
    m_lider = 2,
    m_vivo = 3,
    m_vivo_ok = 4
};
//Lock function;
int get_electionSize();
int erase_ongoingElections();
bool get_isOperational();
void set_isOperational(bool temp);
bool get_isCheckingOnLeader();
void set_isCheckingOnLeader(bool temp);
int get_leaderIdx();
void set_leaderIdx(int temp);
bool get_isSilenced();
void set_isSilenced(bool temp);
int get_inMsgCount();
void increment_inMsgCount();
int get_outMsgCount();
void increment_outMsgCount();
bool get_leaderAnswered();
void set_leaderAnswered(bool temp);
bool get_outElection();
void set_outElection(bool temp);
void increasing_msgCount(int temp);
int get_electionNumber();
void set_electionNumber();
int get_totalElectionMsgs();
void set_totalElectionMsgs(int temp);
int get_alreadyOnThreadleader();
void set_alreadyOnThreadleader(int temp);


// Main funtions
int main(int argc, char* argv[]);

// Auxiliary funtions
int setupServerSocket(int port);
void* electionFinish(void*);
void* caseResolver(void*);
void* interface(void*);
void* communication(void*);
void* leader(void*);