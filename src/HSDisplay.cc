#include <stdio.h>
#include <unistd.h>         // sleep()
#include <signal.h>         // Signal-Funktionen und Signale selbst
#include <stdlib.h>         // exit()
#include <sysexits.h>
#include <string.h>         // memcpy

#include "defs.h"
#include "Utils.h"
#include "Debug.h"
#include "Semaphore.h"
#include "SharedMemory.h"
#include "MessageQueue.h"
#include "Exception.h"

void setupSignals();
void signalHandler(int sig);
void printSensors(SensorData * sharedMemory);
void printSensor(unsigned device, unsigned sequence, int status, float valIS, float valREF);

using namespace zhaw::ipc;

Semaphore       *sem    = NULL;
SharedMemory    *shm    = NULL;
MessageQueue    *q      = NULL;

int main(int argc, char *argv[]) {
    
    Debug::setStream(fopen("HSDisplay.log", "a"));
    Debug::setLevel(INFO);
    
    setupSignals();
    Debug::log(INFO, "Display Startup (%d)", getpid());
    
    try {
        sem    = new Semaphore(SEM_KEY_FILE, PROJECT_ID);
        shm    = new SharedMemory(SHM_KEY_FILE, PROJECT_ID);
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID);
    } catch (Exception e) {
        Debug::log(FATAL, "Catcht Exception!");
        exit(EX_SOFTWARE);
    }


    SensorData tmpData[SENSOR_MAX_NUM];
    while(true) {
        mSLEEP(500);
		sem->down(0);
		memcpy(tmpData, shm->getMemory(), sizeof(SensorData) * SENSOR_MAX_NUM);
		sem->up(0);

		printSensors(tmpData);
    }
    
    return 0;
}

void printSensors(SensorData * data) {
	ClearScreen();
	HomeScreen();

	for(int i = 0; i < SENSOR_MAX_NUM; i++) {
		printSensor(data[i].deviceID, data[i].sequenceNr, data[i].status, data[i].valIS, data[i].valREF);
	}
}

void printSensor(unsigned device, unsigned sequence, int status, float valIS, float valREF) {
    printf("Device %i @ %i: %i V act  ", device, sequence, status);

    if(valIS < 0) {
        printf("-");
	}

    for(int i = 0; i < valIS; i++) {
        printf(".");
    }

	printf("\n");

	printf("                V ref  ");
	for (int i = 0; i < valREF; i++){
	    printf("-");
	}
	printf("\n");
}

void setupSignals() {
   struct sigaction action;

   action.sa_handler = signalHandler;
   action.sa_flags = SA_RESTART;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGUSR1, &action, NULL);
}

void signalHandler(int sigNo) {
   switch (sigNo) {
       case SIGINT:
           break;
           
       case SIGTERM:
           break;
           
       case SIGUSR1:
           Debug::log(INFO, "Display Shutdown (%d)", getpid());
           exit(0);
           break;
   }
}

