#include <stdio.h>
#include <unistd.h>         // sleep()
#include <signal.h>         // Signal-Funktionen und Signale selbst
#include <stdlib.h>         // exit()
#include <sysexits.h>

#include "defs.h"
#include "Utils.h"
#include "Debug.h"
#include "Semaphore.h"
#include "SharedMemory.h"
#include "MessageQueue.h"
#include "Exception.h"

void setupSignals();
void signalHandler(int sig);

void sendSignalHSDataTx();
void sendSignalHSDisplay();

using namespace zhaw::ipc;

Semaphore       *sem    = NULL;
SharedMemory    *shm    = NULL;
MessageQueue    *q      = NULL;

int main(int argc, char *argv[]) {
    
    Debug::setStream(fopen("HSControl.log", "a"));
    Debug::setLevel(DEBUG);
    
    setupSignals();
    Debug::log(INFO, "Control Startup (%d)", getpid());
    
    try {
        sem    = new Semaphore(SEM_KEY_FILE, PROJECT_ID);
        shm    = new SharedMemory(SHM_KEY_FILE, PROJECT_ID);
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID);
    } catch (Exception e) {
        Debug::log(FATAL, "Catcht Exception!");
        exit(EX_SOFTWARE);
    }

    /**
     * SControl sendet alle Sekunden eine Message (Type 3333) an HSDisplay
     * und alle 2 Sekunden eine Message (Type 3334) an HSDataTx.
     */

    bool lastMessageHSDataTxSent = false;
    while (true) {
        sleep(1);
        if(lastMessageHSDataTxSent) {
            lastMessageHSDataTxSent = false;
        } else {
            lastMessageHSDataTxSent = true;

            // send 3334 HSDataTx
            Debug::log(INFO, "CONTROL: send message 3334 to HSData");
            sendSignalHSDataTx();
        }
        
        // send 3333 and HSDisplay
        Debug::log(INFO, "CONTROL: send message 3333 to HSDisplay");
        sendSignalHSDisplay();
    }
    
    return 0;
}

void sendSignalHSDisplay() {
    Message msg;
    msg.msgType = MSG_TYPE;
    // msg.mdata = 0;

    q->send(&msg, MSG_LENGTH);
}

void sendSignalHSDataTx() {
    Msg msg;
    msg.msgType = MSG_TYPE1;
    msg.numOfSensors = 0;
    //msg.ctrl = //float ctrl[SENSOR_MAX_NUM]

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
