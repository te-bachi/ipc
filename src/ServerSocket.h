#ifndef __SERVER_SOCKET_H__
#define __SERVER_SOCKET_H__

#include "Socket.h"

namespace zhaw {
    namespace ipc {
        
        class ServerSocket {
            
            private:
                int _fd;
                
            public:
                ServerSocket(int port, int maxListen);
                ~ServerSocket();
                Socket *accept();
                void close();
        };
        
        /*** HACK ***/
        int ssa(int fd, struct sockaddr *addr, socklen_t *addrlen);
        void ssc(int fd);
        
    };
};

#endif
