#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "MessageQueue.h"
#include "Debug.h"
#include "Exception.h"

using namespace zhaw::ipc;

MessageQueue::MessageQueue(const char *keyname, int projectId) {
    _keyname   = keyname;
    _projectId = projectId;
    _qId       = 0;
    
    init(O_RDWR, 0, 0);
}

MessageQueue::MessageQueue(const char *keyname, int projectId, bool create) {
    _keyname   = keyname;
    _projectId = projectId;
    _qId       = 0;
    
    if (create) {
        init(O_RDWR | O_CREAT, 0660, 0660 | IPC_CREAT | IPC_EXCL);
    } else {
        init(O_RDWR, 0, 0);
    }
}

MessageQueue::~MessageQueue() {
    
}

void MessageQueue::init(int fileFlags, mode_t fileMode, int shmFlags) {
    int   fd;
    key_t key;
    
    // Try to open and close file
    if ((fd = open(_keyname, fileFlags, fileMode)) >= 0) {
        close(fd);
        
        // Generate key
        if ((key = ftok(_keyname, _projectId)) != 0) {
            
            // Create Shared Memory
            if ((_qId = msgget(key, shmFlags)) >= 0) {
                return;
            } else {
                Debug::log(FATAL, "Setup message queue: Can't create message queue: %s", strerror(errno));
                Debug::log(FATAL, "Try 'ipcs' to find out why, 'ipcrm -q %d' to remove it!", _qId);
            }
        } else {
            Debug::log(FATAL, "Setup message queue: Can't convert key: %s", strerror(errno));
        }
    } else {
        Debug::log(FATAL, "Setup message queue: Can't open file: %s", strerror(errno));
    }
    
    throw Exception();
}

void MessageQueue::remove() {
    if (_qId >= 0) {
        msgctl(_qId, IPC_RMID, NULL);
    }
    
    if (_keyname != NULL) {
        unlink(_keyname);
    }
}

void MessageQueue::send(void *msg, size_t count) {
    msgsnd(_qId, msg, count, 0);
}

ssize_t MessageQueue::receive(void *msg, size_t count, long type) {
    msgrcv(_qId, msg, count, type, 0);
}

