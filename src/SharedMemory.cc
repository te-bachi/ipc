#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "SharedMemory.h"
#include "Debug.h"
#include "Exception.h"

using namespace zhaw::ipc;

SharedMemory::SharedMemory(const char *keyname, int projectId) {
    _keyname   = keyname;
    _projectId = projectId;
    _size      = 0;
    _shmId     = 0;
    _shmAddr   = NULL;
    
    init(O_RDWR, 0, 0);
}

SharedMemory::SharedMemory(const char *keyname, int projectId, int size) {
    _keyname   = keyname;
    _projectId = projectId;
    _size      = size;
    _size      = size;
    _shmId     = 0;
    _shmAddr   = NULL;
    
    init(O_RDWR | O_CREAT, 0660, 0660 | IPC_CREAT | IPC_EXCL);
}

SharedMemory::~SharedMemory() {
    
}

void SharedMemory::init(int fileFlags, mode_t fileMode, int shmFlags) {
    int   fd;
    key_t key;
    
    // Try to open and close file
    if ((fd = open(_keyname, fileFlags, fileMode)) >= 0) {
        close(fd);
        
        // Generate key
        if ((key = ftok(_keyname, _projectId)) != 0) {
            
            // Create Shared Memory
            if ((_shmId = shmget(key, _size, shmFlags)) >= 0) {
                
                // Map Address
                if ((_shmAddr = shmat(_shmId, NULL, 0)) != (void *) -1) {
                    return;
                } else {
                    Debug::log(FATAL, "Setup shared memory: Can't map shared mempory: %s", strerror(errno));
                }
            } else {
                Debug::log(FATAL, "Setup shared memory: Can't create semaphore: %s", strerror(errno));
                Debug::log(FATAL, "Try 'ipcs' to find out why!");
            }
        } else {
            Debug::log(FATAL, "Setup shared memory: Can't convert key: %s", strerror(errno));
        }
    } else {
        Debug::log(FATAL, "Setup shared memory: Can't open file: %s", strerror(errno));
    }
    
    throw Exception();
}

void SharedMemory::remove() {
    if (_shmId > 0) {
        shmctl(_shmId, IPC_RMID, NULL);
    }
    
    if (_keyname != NULL) {
        unlink(_keyname);
    }
}

void *SharedMemory::getMemory() {
    return _shmAddr;
}

