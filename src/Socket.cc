#include "Socket.h"

using namespace zhaw::ipc;

Socket::Socket(int fd) {
    _fd = fd;
}

Socket::~Socket() {
    
}

size_t Socket::write(void *buf, size_t count) {
    return sw(_fd, buf, count);
}

size_t Socket::read(void *buf, size_t count) {
    return sr(_fd, buf, count);
}

void Socket::close() {
    sc(_fd);
}

/*** HACK ***/
size_t zhaw::ipc::sw(int fd, void *buf, size_t count) {
    return write(fd, buf, count);
}

size_t zhaw::ipc::sr(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

void zhaw::ipc::sc(int fd) {
    close(fd);
}

