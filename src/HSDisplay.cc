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

void usage(char *prog);
void setupSignals();
void signalHandler(int sig);
void mainLoop();
void printSensors(SensorData * sharedMemory);
void printSensor(unsigned device, unsigned sequence, int status, float valIS, float valREF);

using namespace zhaw::ipc;

Semaphore       *sem    = NULL;
SharedMemory    *shm    = NULL;
MessageQueue    *q      = NULL;
int              sensorCount = 0;

// Max. alarmCount = (2 sec) / (0.5 1/sec) = 4
#define          ALARM_COUNT_MAX 4
int              alarmCount = 0;
bool             isAlarm    = false;

int main(int argc, char *argv[]) {
    
    Debug::setStream(fopen("HSMain.log", "a"));
    Debug::setLevel(INFO);
   
    
    // Nicht genug Argumente
    if (argc < 2) {
        usage(argv[0]);
    }
    
    // Argument ist keine Zahl
    if (!isnumber2(argv[1])) {
        printf("Argument is not a number!\n");
        usage(argv[0]);
    }
    
    sensorCount = atoi(argv[1]);
    
    // Grösser als Maximum? 
    if (sensorCount > SENSOR_MAX_NUM) {
        printf("Number of sensors (%d) exceets maximum (%d)!\n", sensorCount, SENSOR_MAX_NUM);
        usage(argv[0]);
    }

    setupSignals();
    Debug::log(INFO, "Display Startup (%d)", getpid());
    
    try {
        sem    = new Semaphore(SEM_KEY_FILE, PROJECT_ID);
        shm    = new SharedMemory(SHM_KEY_FILE, PROJECT_ID);
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID);
    } catch (Exception e) {
        Debug::log(FATAL, "Catched Exception!");
        exit(EX_SOFTWARE);
    }
    
    mainLoop();
    
    return 0;
}

void usage(char *prog) {
    printf("%s <Number of sensors>\n", prog);
    exit(EX_USAGE);
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
       case SIGTERM:
       case SIGUSR1:
           Debug::log(INFO, "Display Shutdown (%d)", getpid());
           exit(0);
           break;
       
       case SIGALRM:
           Debug::log(INFO, "Got alarm!", getpid());
           isAlarm    = true;
           alarmCount = 0;
           break;
   }
}

void mainLoop() {
    SensorData tmpData[sensorCount];
    size_t     tmpDataLen;
    
    tmpDataLen = sensorCount * sizeof(SensorData);
    
    while(true) {
        mSLEEP(500);
        sem->down(0);
        memcpy(tmpData, shm->getMemory(), tmpDataLen);
        sem->up(0);

        printSensors(tmpData);

    }
}

void printSensors(SensorData *data) {
    int i;
    
    ClearScreen();
    HomeScreen();
    
    printf("%d) %s - %s", PGROUPNR, PGROUP, COMPANY);
    
    if (isAlarm) {
        printf("--- Control Alarm ---\n");
        alarmCount++;
        if (alarmCount > ALARM_COUNT_MAX) {
            isAlarm = false;
        }
    } else {
        printf("\n");
    }
    
    for(i = 0; i < sensorCount; i++) {
        printSensor(data[i].deviceID, data[i].sequenceNr, data[i].status, data[i].valIS, data[i].valREF);
    }
}

void printSensor(unsigned device, unsigned sequence, int status, float valIS, float valREF) {
    printf("Device %i @ %3i: %i V act  ", device, sequence, status);

    if(valIS < 0) {
        printf("-");
    }

    for(int i = 0; i < valIS; i++) {
        printf(".");
    }

    printf("\n");

    printf("                  V ref  ");
    for (int i = 0; i < valREF; i++){
        printf("-");
    }
    printf("\n");
}

