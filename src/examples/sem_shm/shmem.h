#ifndef SHARED_MEMORY_INCLUDE
#define SHARED_MEMORY_INCLUDE

//**********************************************************************
// File:        shmem.h
// Author:	M. Thaler    18.01.2003
// 
// Shared Memory
//**********************************************************************

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <iostream>
using namespace std;

//**********************************************************************

#define DEFAULT_SHARED_MEMORY_SIZE 256

//**********************************************************************

class SharedMemory {
    public:
	SharedMemory(int size = DEFAULT_SHARED_MEMORY_SIZE);
	SharedMemory(int size, const char* keyFile, int projID);
	~SharedMemory();
	void* getSharedMemory();
	void  removeSharedMemory();

    private:
	int createSharedMemory();
	static int   shmSize;
	static int   shmID;
	static void* shmAddr;
	static char* keyFilename;
	static int   projectID;
};

//**********************************************************************

#endif

