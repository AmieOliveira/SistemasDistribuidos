/*
 *  Header for the 'valentao' code
 *  Author: Amanda
 *
 */


// Global variables
int sockfd;         // This process' socket
// TODO: Should it be protected by lock?

char* messageBuffer;
int messageLength;

char* delimiter;


// Message types
enum MessageTyp {
    m_eleicao = 1,
    m_ok = 2,
    m_lider = 3,
    m_vivo = 4,
    m_vivo_ok = 5
};



// Main funtions
int main(int argc, char* argv[]);

void interface();
void communication();
//void leader();


// Auxiliary funtions
int setupSocket(int port);

//bool checkLeader();