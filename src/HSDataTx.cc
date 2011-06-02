#include <stdio.h>

#include <unistd.h>         // Linux standards
#include <sys/types.h>      // Linux Typendefinitionen
#include <sys/ipc.h>        // SVR4 IPC Mechanismen 
#include <sys/msg.h>        // SVR4 Message Queues 

#include "Utils.h"
#include "defs.h"
#include "Debug.h"

using namespace zhaw::ipc;

int main(int argc, char* argv[]) {
    Msg msg;

    int qid = msgget(12340, 0777);
    printf("DATA: qid: %i\n", qid);


    printf("DATA: Warte auf Anfrage...\n");

    msgrcv(qid, &msg, MSG_LENGTH1, MSG_TYPE1, 0); // 1. Meldung lesen
    while(true) {
        Debug::log(INFO, "DATA: Verarbeite Anfrage...");
        // TODO: impelementieren

        if (msgrcv(qid, &msg, MSG_LENGTH1, MSG_TYPE1, 0) < 0) { // naechste Meldung lesen.
            perror("Queue lesen");
            return 1;
        }
    }

    printf("DATA: *** Ende ***\n");
    return 0;
}
