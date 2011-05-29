#include <stdio.h>
#include <unistd.h>         // sleep()
#include <signal.h>         // Signal-Funktionen und Signale selbst
#include <stdlib.h>         // exit(), atoi()

#include "Utils.h"

void setupSignals();
void signalHandler(int sig);

int main(int argc, char *argv[]) {
    
    setDebugLevel(INFO);
    
    setupSignals();
    debug(INFO, "Display (%d)", getpid());
    sleep(2);
    
    return 0;
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
           debug(DEBUG, "Display Shutdown (%d)", getpid());
           exit(0);
           break;
   }
}

