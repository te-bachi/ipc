#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

namespace zhaw {
    namespace ipc {
        
        union semun {
            int val;                   /* value  for SETVAL              */
            struct semid_ds *buf;      /* buffer for IPC_STAT & IPC_SET  */
            unsigned short int *array; /* array  for GETALL & SETALL     */
            struct seminfo *__buf;     /* buffer for IPC_INFO            */
        };
        
        class Semaphore {
            
            private:
                const char *_keyname;
                int         _projectId;
                int         _size;
                int         _semId;
                
            private:
                void init(int fileFlags, mode_t fileMode, int semFlags);
                
            public:
                Semaphore(const char *keyname, int projectId);
                Semaphore(const char *keyname, int projectId, int size);
                ~Semaphore();
                void remove();
                int up(int semaphor);
                int down(int semaphor);
                int getValue(int semaphor);
                int setValue(int semaphor, int value);
        };
    };
};

#endif
