#ifndef SEMAPHOR_INCLUDE
#define SEMAPHOR_INCLUDE

//**********************************************************************
// File:        semaphore.h
// Author:	M. Thaler    15.01.2003
// 
// Semphor operations 
//**********************************************************************

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
using namespace std;

//**********************************************************************

#define DEFAULT_NUM_SEM 8

//**********************************************************************
//* declare semun by ourselfes (see "man semctl")

#ifdef _SEM_SEMUN_UNDEFINED
#undef _SEM_SEMUN_UNDEFINED
    /* Union used for argument for `semctl'.  */
    union semun {
        int val;                   /* value  for SETVAL              */
        struct semid_ds *buf;      /* buffer for IPC_STAT & IPC_SET  */
        unsigned short int *array; /* array  for GETALL & SETALL     */
        struct seminfo *__buf;     /* buffer for IPC_INFO            */
    };
#endif

//**********************************************************************

class Semaphore {
  public:
	Semaphore(int num = DEFAULT_NUM_SEM);
	Semaphore(int num, const char* keyFile, int projID);
	~Semaphore();
	int up(int semaphor);
	int down(int semaphor);
	int getValue(int semaphor);
	int setValue(int semaphor, int value);
	void removeSemaphore(void);

  private:
	int createSemaphorArray(void);
	static int   numOfSems;
	static int   semID;
	static char* keyFilename;
	static int   projectID;
};

//**********************************************************************

#endif
