#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <unistd.h>

namespace zhaw {
    namespace ipc {
        
        class Socket {
            
            private:
                int _fd;
                
            public:
                Socket(int fd);
                ~Socket();
                size_t write(void *buf, size_t count);
                size_t read(void *buf, size_t count);
                void close();
        };
        
        /*** HACK ***/
        size_t sw(int fd, void *buf, size_t count);
        size_t sr(int fd, void *buf, size_t count);
        void sc(int fd);
    };
};

#endif
