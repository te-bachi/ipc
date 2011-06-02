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

#include "defs.h"
#include "Utils.h"
#include "Debug.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "Semaphore.h"
#include "SharedMemory.h"
#include "MessageQueue.h"
#include "Exception.h"

#define PROG_DISPLAY "./HSDisplay.e"
#define PROG_CONTROL "./HSControl.e"

using namespace zhaw::ipc;

/******************************************************************************
 * Funktions Deklaration
 *****************************************************************************/

// Main Funktionen
void usage(char *prog);
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
void *socketRequest(void *param);

/******************************************************************************
 * Globale Variablen
 *****************************************************************************/

// Main Variablen
pthread_t       procThread;
pthread_t       sockThread;

// Process Thread Variablen
pid_t           mainPid;
pid_t           displayPid;
pid_t           controlPid;
bool            running;
pthread_mutex_t runningMutex;

// Setup IPC Variablen
int             anzSensors;
SensorData*     sensors;

// Anzeigen:            ipcs
// Lösche Semaphore:    ipcrm -s <semid>
// Lösche SharedMemory: ipcrm -m <shmid>
// Lösche MessageQueue: ipcrm -q <qid>
ServerSocket    *server = NULL;
Semaphore       *sem    = NULL;
SharedMemory    *shm    = NULL;
MessageQueue    *q      = NULL;

/******************************************************************************
 * Main Thread
 *****************************************************************************/

int main(int argc, char *argv[]) {
    
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
    
    anzSensors = atoi(argv[1]);
    
    if (anzSensors > SENSOR_MAX_NUM) {
        printf("Number of sensors (%d) exceets maximum (%d)!\n", anzSensors, SENSOR_MAX_NUM);
        usage(argv[0]);
    }
    
    running = true;
    pthread_mutex_init(&runningMutex, NULL);
    setupSignals();
    
    try {
        int shmLen = 2 * anzSensors * sizeof(SensorData);
        
        server = new ServerSocket(COMM_PORT, SENSOR_MAX_NUM);
        sem    = new Semaphore(SEM_KEY_FILE, PROJECT_ID, 1);
        shm    = new SharedMemory(SHM_KEY_FILE, PROJECT_ID, shmLen);
        q      = new MessageQueue(MBOX_KEY_FILE, PROJECT_ID, true);
            
        pthread_create(&procThread, NULL, processThread, argv[1]);
        pthread_create(&sockThread, NULL, socketThread, NULL);
        
        pthread_join(procThread, NULL);
        pthread_join(sockThread, NULL);
        
        Debug::log(INFO, "Exit!");
        
    } catch (Exception e) {
        Debug::log(FATAL, "Catcht Exception!");
        exit(EX_SOFTWARE);
    }
    return EX_OK;
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
    sigaction(SIGALRM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
}

void signalHandler(int sigNo) {
    switch (sigNo) {
        case SIGINT:
            Debug::newLine();
            Debug::log(INFO, "Received interrupt signal (%d)", sigNo);
            shutdown();
            break;
        
        case SIGTERM:
            Debug::log(INFO, "Received termination signal (%d)", sigNo);
            shutdown();
            break;
        
        case SIGALRM:
            Debug::log(INFO, "Received alarm signal (%d)", sigNo);
            break;
    }
}

void shutdown() {
    
    Debug::log(INFO, "Shutdown");
    
    pthread_mutex_lock(&runningMutex);
    running = false;
    pthread_mutex_unlock(&runningMutex);
    
    pthread_cancel(sockThread);
    
    if (server != NULL) {
        Debug::log(INFO, "Close SocketServer and delete it");
        server->close();
        delete(server);
    }
    
    if (sem != NULL) {
        Debug::log(INFO, "Remove Semaphore and delete it");
        sem->remove();
        delete(sem);
    }
    
    if (shm != NULL) {
        Debug::log(INFO, "Remove Shared Memory and delete it");
        shm->remove();
        delete(shm);
    }
    
    if (q != NULL) {
        Debug::log(INFO, "Remove Message Queue and delete it");
        q->remove();
        delete(q);
    }
    
    if (displayPid != 0) {
        if (kill(displayPid, SIGUSR1) == -1) {
            Debug::log(FATAL, "Can't kill Display PID %d: %s", displayPid, strerror(errno));
        }
    }
    
    if (controlPid != 0) {
        if (kill(controlPid, SIGUSR1) == -1) {
            Debug::log(FATAL, "Can't kill Control PID %d: %s", controlPid, strerror(errno));
        }
    }
}

/******************************************************************************
 * Process Thread
 *****************************************************************************/

void *processThread(void *param) {
    pid_t returnPid;
    pid_t oldDisplayPid;
    char *nSignals;
    int   status;
    
    nSignals = (char *) param;
    
    createDisplayProcess(&displayPid, nSignals);
    createControlProcess(&controlPid, nSignals, displayPid);
    
    // Warten auf alle Kinder
    returnPid = waitpid(-1, &status, 0);
    
    while (returnPid > 0) {
        pthread_mutex_lock(&runningMutex);
        if (running) {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                Debug::log(FATAL, "Status from child %d: Exit!", WEXITSTATUS(status));
                shutdown();
            } else {
                if (returnPid == controlPid) {
                    Debug::log(DEBUG, "Restart HSControl");
                    if (kill(displayPid, SIGUSR1) == -1) {
                        Debug::log(FATAL, "Can't kill PID %d", displayPid, strerror(errno));
                    }
                    oldDisplayPid = displayPid;
                    createDisplayProcess(&displayPid, nSignals);
                    createControlProcess(&controlPid, nSignals, displayPid);
                } else if (returnPid == displayPid) {
                    Debug::log(DEBUG, "Restart HSDisplay");
                    createDisplayProcess(&displayPid, nSignals);
                } else if (returnPid == oldDisplayPid) {
                    Debug::log(DEBUG, "Parent catched old Display PID (%d)\n", returnPid);
                } else {
                    Debug::log(ERROR, "Unknow child with PID %d\n", returnPid);
                }
            }
        }
        pthread_mutex_unlock(&runningMutex);
        
        // Warten auf alle Kinder
        returnPid = waitpid(-1, &status, 0);
    }
    
    Debug::log(INFO, "Exit Process Thread");
    return NULL;
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
    char  pidStr[16];
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
            Debug::log(FATAL, "Can't fork: %s\n", strerror(errno));
            shutdown();
            break;
            
        // Kind
        case 0:
            if (execv(path, argv) < 0) {
                Debug::log(FATAL, "Can't change image: %s", strerror(errno));
                exit(EX_OSERR);
            }
            break;
            
        // Eltern
        default:
            break;
    }
}

/******************************************************************************
 * Socket Thread
 *****************************************************************************/

typedef struct {
    int i;
} IntStruct;

void *socketThread(void *param) {
    Socket    *client;
    pthread_t  tid;
    
    while (running) {
        client = server->accept();
        pthread_create(&tid, NULL, socketRequest, (void *) client);
    }
    
    return NULL;
}

/*
typedef struct {                // sensor data
    unsigned deviceID;          // sensor ID
    unsigned sequenceNr;        // sequence number of data
    float    valIS;             // temperature: measured
    float    valREF;            // temperature: reference
    int      status;            // status
} SensorData, *SensorDataPtr;
*/
void *socketRequest(void *param) {
    Socket     *client;
    SensorData  sensor;
    ssize_t     len;
    
    client = (Socket *) param;
    
    len = client->read((char *)&sensor, sizeof(SensorData));
    if (len >= 0) {
        if (len == sizeof(SensorData)) {
            Debug::log(INFO, "deviceID=%u sequenceNr=%u valIS=%f valREF=%f status=%d",
                sensor.deviceID, sensor.sequenceNr, sensor.valIS, sensor.valREF,
                sensor.status);
            
            // Ist shared memory initialisiert?
            if (sensors >= 0) {
                
                // TODO: wechsel, wenn gleiche devId
                sensors[sensor.deviceID] = sensor;
            } else {
                Debug::log(INFO, "Can't access sensor memory");
            }
        } else {
            Debug::log(ERROR, "Size of data read don't match! size=%u", len);
        }
    } else {
        Debug::log(ERROR, "Can't read: %s", strerror(errno));
    }
                                            
    client->close();
    delete(client);
    return NULL;
}





