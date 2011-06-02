#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

namespace zhaw {
    namespace ipc {
        
        class MessageQueue {
            
            private:
                const char *_keyname;
                int         _projectId;
                int         _qId;
            
            private:
                void init(int fileFlags, mode_t fileMode, int semFlags);
            
            public:
                MessageQueue(const char *keyname, int projectId);
                MessageQueue(const char *keyname, int projectId, bool create);
                ~MessageQueue();
                void remove();
                void send(void *msg, size_t count);
                void receive(void *msg, size_t count, long type);
        };
    };
};

#endif
