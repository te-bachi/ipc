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

void sendSignalHSDataTx();
void sendSignalHSDisplay();

using namespace zhaw::ipc;

Semaphore       *sem    = NULL;
SharedMemory    *shm    = NULL;
MessageQueue    *q      = NULL;
int              sensorCount = 0;
int              displayPid  = 0;

int main(int argc, char *argv[]) {
    
    Debug::setStream(fopen("HSMain.log", "a"));
    Debug::setLevel(INFO);
    
    // Nicht genug Argumente
    if (argc < 3) {
        usage(argv[0]);
    }
    
    // Argument ist keine Zahl
    if (!isnumber2(argv[1]) || !isnumber2(argv[2])) {
        printf("Argument is not a number!\n");
        usage(argv[0]);
    }
    
    sensorCount = atoi(argv[1]);
    displayPid = atoi(argv[2]);
    
    // Grösser als Maximum? 
    if (sensorCount > SENSOR_MAX_NUM) {
        printf("Number of sensors (%d) exceets maximum (%d)!\n", sensorCount, SENSOR_MAX_NUM);
        usage(argv[0]);
    }
    
    setupSignals();
    
    try {
        sem    = new Semaphore(SEM_KEY_FILE, PROJECT_ID);
        shm    = new SharedMemory(SHM_KEY_FILE, PROJECT_ID);
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID);
    } catch (Exception e) {
        Debug::log(FATAL, "Catched Exception!");
        exit(EX_SOFTWARE);
    }
    
    
    /**
     * SControl sendet alle Sekunden eine Message (Type 3333) an HSDisplay
     * und alle 2 Sekunden eine Message (Type 3334) an HSDataTx.
     */

    bool lastMessageHSDataTxSent = false;
    while (true) {
        sleep(1);

        SensorData * data = (SensorData *)shm->getMemory();

        sem->down(0);

        bool sensorError = false;
        for(int i = 0; i < sensorCount; i++) {
            if(data[i].status == -1) {
                sensorError = true;
            }
        }

        sem->up(0);

        if(sensorError) {
            kill(displayPid, SIGALRM);
        }


        if(lastMessageHSDataTxSent) {
            lastMessageHSDataTxSent = false;
        } else {
            lastMessageHSDataTxSent = true;

            // send 3334 HSDataTx
            Debug::log(DEBUG, "CONTROL: send message 3334 to HSData");
            sendSignalHSDataTx();
        }
        
        // send 3333 and HSDisplay
        Debug::log(DEBUG, "CONTROL: send message 3333 to HSDisplay");
        sendSignalHSDisplay();
    }
    
    return 0;
}

void usage(char *prog) {
    printf("%s <Number of sensors> <Display PID>\n", prog);
    exit(EX_USAGE);
}

void sendSignalHSDisplay() {
    Message msg;
    msg.msgType = MSG_TYPE;

    SensorData * data = (SensorData *)shm->getMemory();

    sem->down(0);

    float maxDiff = ABS(data[0].valIS - data[0].valREF);
    float diff;
    int maxId = 0;
    float sumDiff = 0;

    for(int i = 0; i < sensorCount; i++) {
        diff = ABS(data[i].valIS - data[i].valREF);
        if(diff > maxDiff) {
            maxDiff = diff;
            maxId = i;
        }

        sumDiff += diff;
    }

    if(sumDiff < -5) {
        strcpy(msg.mdata.statusText, "+++");
    } else if(sumDiff > 5) {
        strcpy(msg.mdata.statusText, "---");
    } else {
        strcpy(msg.mdata.statusText, "+/-");
    }


    msg.mdata.sequenceNr = data[maxId].sequenceNr;
    msg.mdata.deviceID = data[maxId].deviceID;
    msg.mdata.delta = diff;

    sem->up(0);

    q->send(&msg, MSG_LENGTH);
}

void sendSignalHSDataTx() {
    Msg msg;
    msg.msgType = MSG_TYPE1;
    msg.numOfSensors = sensorCount;


    SensorData * data = (SensorData *)shm->getMemory();

    sem->down(0);

    for(int i = 0; i < sensorCount; i++){
        msg.ctrl[i] = data[i].valIS;
    }

    sem->up(0);

    q->send(&msg, MSG_LENGTH1);
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
           Debug::log(INFO, "Control Shutdown (%d)", getpid());
           exit(0);
           break;
   }
}
