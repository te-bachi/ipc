//***************************************************************************
// ip_client.cc	TCP/IP socket client
// Author:      M. Thaler
// Date:        7.7.99
// Changes:     tha, 5/2008
//              using getaddrinfo -> IPV4 and IPV6 compatible
//***************************************************************************

//***************************************************************************
// system includes
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
using namespace std;

//***************************************************************************
// local includes
//***************************************************************************

#include "lsocks.h"				// PORT_NUMBER, BUF_SIZE

//***************************************************************************
// Function: main(), parameter: hostname or IP address in dot format
//**************************************************************************

int main(int argc, char *argv[]) {

	int    sfd, cfd, addrlen, j, sysRet;
	char   stringPort[8], buf[BUF_SIZE];
    struct addrinfo hints, *aiList, *aiPtr = NULL;

	if (argc < 2)  {
		cout << "Need hostname or IP address" << endl;
		exit(-1);
	}

	cout << endl << "IP client" << endl;

    sprintf(stringPort, "%d", PORT_NUMBER);

    memset(&hints, '\0', sizeof(hints));
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sysRet = getaddrinfo(argv[1], stringPort, &hints, &aiList);
    if (sysRet != 0) {
        printf("error getting network address %s\n", gai_strerror(sysRet));
        return(-1);
    }

    aiPtr = aiList;                             // search through list
    while (aiPtr != 0) {
        sfd = socket(aiPtr->ai_family, aiPtr->ai_socktype, aiPtr->ai_protocol);
        if (sfd >= 0) {
            cout << "Socket created" << endl;
            cout << "Connected to port #" << stringPort << endl;
            cout << "Connecting to server ..." << endl;
            sysRet = connect(sfd, aiPtr->ai_addr, aiPtr->ai_addrlen);
            if (sysRet == 0)         
                break;                          // connect successful
            else 
                close(sfd);
        }
        aiPtr = aiPtr->ai_next;
    }
    freeaddrinfo(aiList);
    if (aiPtr == NULL) {
        printf("could not connect to %s\n", argv[1]);
        exit(-1);
    }

    cout << "... connection established" << endl;

	cout << "? " ;	
	while (fgets(buf, BUF_SIZE, stdin) > 0) {
		write(sfd, buf, strlen(buf));
		if (buf[0] == '!') break;		    // terminate on '!'
		if (buf[0] == '@') break;		    // terminate on '@'
		cout << "? " ;	
	}
	close(sfd);
	cout << "Connection terminted" << endl;
}
