#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

//#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "Semaphore.h"
#include "Debug.h"
#include "Exception.h"

using namespace zhaw::ipc;

Semaphore::Semaphore(const char *keyname, int projectId) {
    _keyname   = keyname;
    _projectId = projectId;
    _size      = 0;
    _semId     = 0;
    
    init(O_RDWR, 0, 0);
}

Semaphore::Semaphore(const char *keyname, int projectId, int size) {
    _keyname   = keyname;
    _projectId = projectId;
    _size      = size;
    _size      = size;
    _semId     = 0;
    
    init(O_RDWR | O_CREAT, 0660, 0660 | IPC_CREAT | IPC_EXCL);
}

Semaphore::~Semaphore() {
    
}

void Semaphore::init(int fileFlags, mode_t fileMode, int semFlags) {
    int   fd;
    key_t key;
    
    // Try to open and close file
    if ((fd = open(_keyname, fileFlags, fileMode)) >= 0) {
        close(fd);
        
        // Generate key
        if ((key = ftok(_keyname, _projectId)) != 0) {
            
            // Create Message Queue
            if ((_semId = semget(key, _size, semFlags)) >= 0) {
                return;
            } else {
                Debug::log(FATAL, "Setup semaphore: Can't create semaphore: %s", strerror(errno));
                Debug::log(FATAL, "Try 'ipcs' to find out why!");
            }
        } else {
            Debug::log(FATAL, "Setup semaphore: Can't convert key: %s", strerror(errno));
        }
    } else {
        Debug::log(FATAL, "Setup semaphore: Can't open file: %s", strerror(errno));
    }
    
    throw Exception();
}

void Semaphore::remove() {
    if (_semId >= 0) {
        semctl(_semId, 0, IPC_RMID);
    }
    
    if (_keyname != NULL) {
        unlink(_keyname);
    }
}

int Semaphore::up(int semaphor) {
    struct sembuf buf;
    buf.sem_num = semaphor;
    buf.sem_op  = 1;
    buf.sem_flg = 0;
    return semop(_semId, &buf, 1);
}

int Semaphore::down(int semaphor) {
    struct sembuf buf;
    buf.sem_num = semaphor;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    return semop(_semId, &buf, 1);
}

int Semaphore::getValue(int semaphor) {
    semun sem_union;
    return semctl(_semId, semaphor, GETVAL, sem_union);
}

int Semaphore::setValue(int semaphor, int value) {
    semun sem_union;
    sem_union.val = value;
    return semctl(_semId, semaphor, SETVAL, sem_union);
}

