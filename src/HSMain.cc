#include <stdio.h>          // printf()
#include <stdlib.h>         // exit(), atoi()
#include <sysexits.h>       // Return Codes an Eltern-Prozess
#include <signal.h>         // Signal-Funktionen und Signale selbst
#include <pthread.h>        // Pthreads
#include <unistd.h>         // sleep(), getpid()
#include <sys/types.h>      // waitpid(), getpid()
#include <sys/wait.h>       // waitpid()
#include <errno.h>          // errno
#include <string.h>         // strerror()

#include "Utils.h"

#define PROG_DISPLAY "./HSDisplay.e"
#define PROG_CONTROL "./HSControl.e"

// Main Funktionen
void setupSignals();
void signalHandler(int sigNo);
void shutdown();

// Threads
void* processThread(void* param);
void* socketThread(void* param);

// Funktionen für Process Thread
void createProcess(pid_t *pid, const char *path, char *const argv[]);
void createDisplayProcess(pid_t *displayPid, char *nSensors);
void createControlProcess(pid_t *controlPid, char *nSensors, pid_t displayPid);

// Funktionen für Socket Thread

// Globale Variablen
bool running;

int main(int argc, char *argv[]) {
    pthread_t procThread;
    pthread_t sockThread;
    
    setDebugLevel(INFO);
    
    // Nicht genug Argumente
    if (argc < 2) {
        printf("%s <Anzahl Sensoren>\n", argv[0]);
        exit(EX_USAGE);
    }
    
    if (!isnumber(argv[1])) {
        printf("Argument is not a number!\n");
        exit(EX_USAGE);
    }
    
    running = true;
    setupSignals();
    
    pthread_create(&procThread, NULL, processThread, argv[1]);
    pthread_create(&sockThread, NULL, socketThread, argv[1]);
    
    pthread_join(procThread, NULL);
    pthread_join(sockThread, NULL);
    
    return EX_OK;
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
           shutdown();
           break;
   }
}

void *processThread(void *param) {
    pid_t displayPid;
    pid_t oldDisplayPid;
    pid_t controlPid;
    pid_t returnPid;
    char *nSignals;
    int   status;
    
    nSignals = (char *) param;
    
    createDisplayProcess(&displayPid, nSignals);
    createControlProcess(&controlPid, nSignals, displayPid);
    
    while (running) {
        // Warten auf alle Kinder
        returnPid = waitpid(-1, &status, 0);
        if (status != 0) {
            debug(FATAL, "Status from child < 0: Exit!");
            shutdown();
        } else {
            if (returnPid == controlPid) {
                kill(displayPid, SIGUSR1);
                oldDisplayPid = displayPid;
                createDisplayProcess(&displayPid, nSignals);
                createControlProcess(&controlPid, nSignals, displayPid);
            } else if (returnPid == displayPid) {
                createDisplayProcess(&displayPid, nSignals);
            } else if (returnPid == oldDisplayPid) {
                debug(DEBUG, "Parent catched old Display PID (%d)\n", returnPid);
            } else {
                fprintf(stderr, "Unknow child with PID %d: Exit!\n", returnPid);
            }
        }
    }
    
    debug(DEBUG, "Kill PID %d\n", displayPid);
    kill(displayPid, SIGUSR1);
    debug(DEBUG, "Kill PID %d\n", controlPid);
    kill(controlPid, SIGUSR1);
    
    waitpid(displayPid, NULL, 0);
    waitpid(controlPid, NULL, 0);
    
    return NULL;
}

void *socketThread(void *param) {
    
    while (running) {
        
    }
    
    return NULL;
}

void shutdown() {
    debugNewLine();
    debug(INFO, "Shutdown");
    running = false;
}

void createDisplayProcess(pid_t *displayPid, char *nSensors) {
    char *path = (char *) PROG_DISPLAY;
    char *argv[3];
    
    argv[0] = path;
    argv[1] = nSensors;
    argv[2] = NULL;
    
    createProcess(displayPid, path, argv);
}

void createControlProcess(pid_t *controlPid, char *nSensors, pid_t displayPid) {
    char *path = (char *) PROG_CONTROL;
    char  pidStr[255];
    char *argv[4];
    
    snprintf(pidStr, sizeof(pidStr), "%d", displayPid);
    
    argv[0] = path;
    argv[1] = nSensors;
    argv[2] = pidStr;
    argv[3] = NULL;
    
    createProcess(controlPid, path, argv);
}

/**
 * Erstellt einen Prozess und führt den Pfad aus
 *
 * @param pid Gibt PID zurück
 * @param path Führt den Pfad aus
 * @param argv Argumente Liste für die Ausführung
 */
void createProcess(pid_t *pid, const char *path, char *const argv[]) {
    
#if DEBUG_MORE
    int i;
    
    printf("PATH=\"%s\" ", path);
    for (i = 0; argv[i] != NULL; i++) {
        printf("ARG%d=\"%s\" ", i, argv[i]);
    }
    printf("\n");
#endif
    
    *pid = fork();
    
    switch (*pid) {
        // Fehler
        case -1:
            debug(FATAL, "Can't fork: %s\n", strerror(errno));
            shutdown();
            break;
            
        // Kind
        case 0:
            if (execv(path, argv) < 0) {
                debug(FATAL, "Can't change image: %s", strerror(errno));
                exit(EX_OSERR);
            }
            break;
            
        // Eltern
        default:
            break;
    }
}

