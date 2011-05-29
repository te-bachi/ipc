//***************************************************************************
// ip_server.cc	TCP/IP socket server
// Author: 	M. Thaler
// Date:	7.7.99
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
#include <sys/wait.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>

#include <iostream>
using namespace std;

//***************************************************************************
// local includes
//***************************************************************************

#include "lsocks.h"				// PORT_NUMBER, BUF_SIZE

//***************************************************************************
// Function: main(), parameter: none
//***************************************************************************

int  main(void) {

    int  sfd, cfd, sysRet, j, addrlen, anz;
    char stringPort[8], buf[BUF_SIZE];;
    struct addrinfo hints, *aiList, *aiPtr = NULL;
	struct sockaddr addr;

	cout << endl << "IP server" << endl;

    sprintf(stringPort, "%d", PORT_NUMBER);     // portnumber to string
    memset(&hints, '\0', sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sysRet = getaddrinfo(NULL, stringPort, &hints, &aiList);
    if (sysRet != 0) {
        printf("error getting network address %s\n", gai_strerror(sysRet));
        return(-1);
    }

    aiPtr = aiList;                             // search through list
    while (aiPtr != 0) {
        sfd = socket(aiPtr->ai_family, aiPtr->ai_socktype, aiPtr->ai_protocol);
        if (sfd >= 0) {
	        j = 1;
	        sysRet = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(j));
	        if (sysRet < 0)
		        perror("cannot set socket options");

	        if (bind(sfd, aiPtr->ai_addr, aiPtr->ai_addrlen) < 0) {
		        perror("bind failed ");
		        close(sfd);
		        exit(-1);
	        }
	        cout << "Binding successful" << endl;
	        cout << "Connected to port #" << stringPort << endl;

            if (listen(sfd, 5) < 0) {
                close(sfd);
                perror("listen failed ");
                exit(-1);
            }
            else          
                break;
         }
         aiPtr = aiPtr->ai_next;
    }
    freeaddrinfo(aiList);
    if (aiPtr == NULL) {
        printf("could not set up a socket server %s\n");
        exit(-1);
    }

	cout << "Listen successful" << endl;
	cout << "Wainting for client" << endl << endl;

	while ((cfd = accept(sfd, &addr,(unsigned *) &addrlen)) >= 0)  {
		cout << "Contacted by client" << endl;
		// get data from client and dispaly on stdout
		cout << "-> ";
		while ( (anz = read(cfd, buf, BUF_SIZE)) > 0) {
			if (buf[0] == '!') break;	                    // terminate on '!'
			if (buf[0] == '@') break;                       // terminate on '@'
			for (j = 0; j < anz; j++) {
				cout << buf[j];
				if (buf[j] == '\n')
					cout << "-> ";
			}
			fflush(stdout);
		}
		cout << "Client disconnects" << endl;
		close(cfd);
		if (buf[0] == '@')
            break;		                                    // exit on '@'
		cout << "Waiting for next client" << endl << endl;
	}
	cout << "Received '@' from client: good bye" << endl;
	close(sfd);
}
