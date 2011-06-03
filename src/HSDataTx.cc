#include <stdio.h>

#include <unistd.h>         // Linux standards
#include <sys/types.h>      // Linux Typendefinitionen
#include <sys/ipc.h>        // SVR4 IPC Mechanismen 
#include <sys/msg.h>        // SVR4 Message Queues 
#include <stdlib.h>         // exit()

#include "Utils.h"
#include "defs.h"
#include "Debug.h"
#include "MessageQueue.h"
#include "Exception.h"

using namespace zhaw::ipc;

MessageQueue    *q      = NULL;

int main(int argc, char* argv[]) {
    Msg msg;

    try {
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID);
    } catch (Exception e) {
        Debug::log(FATAL, "Catcht Exception!");
        exit(-1);
    }

    printf("DATA: Warte auf Anfrage...\n");

    q->receive(&msg, MSG_LENGTH1, MSG_TYPE1); // 1. Meldung lesen
    while(true) {
        if (q->receive(&msg, MSG_LENGTH1, MSG_TYPE1) < 0) { // naechste Meldung lesen.
            perror("Queue lesen");
            return 1;
        }

	    ClearScreen();
	    HomeScreen();
	    for(int i = 0; i < msg.numOfSensors; i++){
		    printf("Sensor %d: %f\n", i,  msg.ctrl[i]);
	    }

    }

    printf("DATA: *** Ende ***\n");
    return 0;
}
