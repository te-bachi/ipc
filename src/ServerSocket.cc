#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <string.h>

#include "ServerSocket.h"
#include "Debug.h"
#include "Exception.h"

using namespace zhaw::ipc;

ServerSocket::ServerSocket(int port, int maxListen) {
    struct sockaddr_in serverAddr;
    
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family      = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port        = htons(port);
        
        if (bind(_fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) == 0) {
            if (listen(_fd, maxListen) == 0) {
                return;
            } else {
                Debug::log(FATAL, "Can't mark socket as passive (listen-mode): %s", strerror(errno));
            }
        } else {
            Debug::log(FATAL, "Can't bind socket: %s", strerror(errno));
        }
    } else {
        Debug::log(FATAL, "Can't create socket: %s", strerror(errno));
    }
    
    throw Exception();
}

ServerSocket::~ServerSocket() {
    
}

Socket *ServerSocket::accept() {
    int                 connectfd;
    struct sockaddr_in  clientAddr;
    socklen_t           clientAddrLen;
    char                clientIp[INET_ADDRSTRLEN];
    unsigned short      clientPort;
    
    clientAddrLen = sizeof(clientAddr);
    if ((connectfd = ssa(_fd, (struct sockaddr *) &clientAddr, &clientAddrLen)) >= 0) {
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
        clientPort = ntohs(clientAddr.sin_port);
        Debug::log(INFO, "Accept client %s from port %u", clientIp, clientPort);
        
        return new Socket(connectfd);
    } else {
        Debug::log(ERROR, "Can't accept connection: %s", errno, strerror(errno));
    }
    
    throw Exception();
}

void ServerSocket::close() {
    ssc(_fd);
}

/*** HACK ***/
int zhaw::ipc::ssa(int fd, struct sockaddr *addr, socklen_t *addrlen) {
    return accept(fd, addr, addrlen);
}

void zhaw::ipc::ssc(int fd) {
    close(fd);
}

