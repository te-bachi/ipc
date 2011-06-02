//******************************************************************************
// SensorDevices.cc Messtatioen
// Author:          M. Thaler
// Date:            3/2011
//******************************************************************************

//******************************************************************************
// system includes

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

//******************************************************************************
// local includes

#include "defs.h"   


//******************************************************************************
// local procedures

int connToServer(char *hostname, int port);

//**************************************************************************
// Function: main(), parameter: hostname or IP address in dot format
//**************************************************************************

int main(int argc, char *argv[]) {
    int         sfd;
    int         anzSensors;
    SensorData  sensor;
    float       deltaT;
    int         i;

    float       tempPreset[8]  = {20, 45, 30, 20, 15, 10, 15, 20};
    float       startup[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
    int         sequenceNr[8]  = {0, 0, 0, 0, 0, 0, 0, 0};

    //*** check for hostname ... a kind of hack
    
    char       anzSensoren[] = "6";
    char       ip[]          = "127.0.0.1";
    char       port[]        = "44444";
    
    anzSensors = atoi(anzSensoren);
    
    for (i = 0; i <  anzSensors; i++) {
            deltaT            = i;
            sensor.deviceID   = i;
            sensor.sequenceNr = sequenceNr[sensor.deviceID];
            sensor.valIS      = deltaT + startup[sensor.deviceID];
            sensor.valREF     = tempPreset[sensor.deviceID];
            sensor.status     = 0;

            sfd = connToServer(ip, atoi(port));
            write(sfd, (char *)&sensor,sizeof(SensorData));
            close(sfd);           
    }
    exit(0);

}

//******************************************************************************
// socket client

int connToServer(char *hostname, int port) {
    int  sfd, sysRet;
    char stringPort[8];
    struct addrinfo hints, *aiList, *aiPtr = NULL;

    sprintf(stringPort, "%d", port);

    memset(&hints, '\0', sizeof(hints));
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sysRet = getaddrinfo(hostname, stringPort, &hints, &aiList);
    if (sysRet != 0) {
        printf("error getting network address %s\n", gai_strerror(sysRet));
        return(-1);
    }

    aiPtr = aiList;
    while (aiPtr != 0) {
        sfd = socket(aiPtr->ai_family, aiPtr->ai_socktype, aiPtr->ai_protocol);
        if (sfd >= 0) {
            sysRet = connect(sfd, aiPtr->ai_addr, aiPtr->ai_addrlen);
            if (sysRet == 0)
                break;
            else 
                close(sfd);
        }
        aiPtr = aiPtr->ai_next;
    }
    if (aiPtr == NULL) {
        return(-1);
    }
    else
        return(sfd);
}

//******************************************************************************
