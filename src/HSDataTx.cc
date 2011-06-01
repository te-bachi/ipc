#include <stdio.h>

#include <unistd.h>         // Linux standards
#include <sys/types.h>      // Linux Typendefinitionen
#include <sys/ipc.h>        // SVR4 IPC Mechanismen 
#include <sys/msg.h>        // SVR4 Message Queues 

#include "defs.h"

int main(int argc, char* argv[]) {
    Msg msg;
    msg.msgType = MSG_TYPE1;

    int qid = msgget(12340, 0777 | IPC_CREAT);


    printf("DataTX: Warte auf Anfrage...\n");

    msgrcv(qid, &msg, MSG_LENGTH1, MSG_TYPE1, 0); // 1. Meldung lesen
    while(true)   {
        printf ("SERVER: Verarbeite Anfrage...\n");
        // TODO: impelementieren
    }

    printf ("SERVER: *** Ende ***\n");
    return 0;
}
