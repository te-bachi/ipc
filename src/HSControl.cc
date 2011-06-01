#include <stdio.h>
#include <unistd.h>         // sleep()
#include <signal.h>         // Signal-Funktionen und Signale selbst
#include <stdlib.h>         // exit()

#include <unistd.h>         // Linux standards
#include <sys/types.h>      // Linux Typendefinitionen
#include <sys/ipc.h>        // SVR4 IPC Mechanismen 
#include <sys/msg.h>        // SVR4 Message Queues 

#include "Utils.h"
#include "defs.h"

void setupSignals();
void signalHandler(int sig);

void sendSignalHSDataTx(int qid);
void sendSignalHSDisplay(int qid);

int main(int argc, char *argv[]) {
    
    setDebugLevel(INFO);
    
    setupSignals();
    debug(INFO, "Control Startup (%d)", getpid());
    
    close(0); //stdin

    int qidData = msgget(12340, 0777 | IPC_CREAT);
    int qidDisplay = msgget(12341, 0777 | IPC_CREAT);
    debug(INFO, "CONTROL: qidData: %i", qidData);

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
            debug(INFO, "CONTROL: send message 3334 to HSData");
            sendSignalHSDataTx(qidData);
        }
        
        // send 3333 and HSDisplay
        debug(INFO, "CONTROL: send message 3333 to HSDisplay");
        sendSignalHSDisplay(qidDisplay);
    }
    
    return 0;
}

void sendSignalHSDisplay(int qid) {
    Message msg;
    msg.msgType = MSG_TYPE;
    // msg.mdata = 0;

    msgsnd(qid, &msg, MSG_LENGTH, 0);
}

void sendSignalHSDataTx(int qid) {
    Msg msg;
    msg.msgType = MSG_TYPE1;
    msg.numOfSensors = 0;
    //msg.ctrl = //float ctrl[SENSOR_MAX_NUM]

    msgsnd(qid, &msg, MSG_LENGTH1, 0);
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
           debug(INFO, "Control Shutdown (%d)", getpid());
           exit(0);
           break;
   }
}
