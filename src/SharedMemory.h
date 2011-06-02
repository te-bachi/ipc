#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

namespace zhaw {
    namespace ipc {
        
        class SharedMemory {
            
            private:
                const char *_keyname;
                int         _projectId;
                int         _size;
                int         _shmId;
                void       *_shmAddr;
            
            private:
                void init(int fileFlags, mode_t fileMode, int shmFlags);
            
            public:
                SharedMemory(const char *keyname, int projectId);
                SharedMemory(const char *keyname, int projectId, int size);
                ~SharedMemory();
                void remove();
                void* getMemory();
        };
    };
};

#endif
