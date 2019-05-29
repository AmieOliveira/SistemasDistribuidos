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


int main(int argc, char* argv[]){
    interface();
}