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
#include <strings.h>        // bzero()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "Utils.h"

#define PROG_DISPLAY "./HSDisplay.e"
#define PROG_CONTROL "./HSControl.e"

// Main Funktionen
void usage(char *prog);
void setupSignals();
void setupSharedMemory();
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

// Globale Variablen
pthread_t       procThread;
pthread_t       sockThread;
pid_t           mainPid;
pid_t           displayPid;
pid_t           controlPid;
bool            running;
pthread_mutex_t runningMutex;
int             shmId;
SensorData*     sensors;
int             anzSensors;

/******************************************************************************
 * Main Thread
 *****************************************************************************/

int main(int argc, char *argv[]) {
    
    setDebugLevel(INFO);
    
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
    setupSharedMemory();
    
    pthread_create(&procThread, NULL, processThread, argv[1]);
    pthread_create(&sockThread, NULL, socketThread, argv[1]);
    
    pthread_join(procThread, NULL);
    pthread_join(sockThread, NULL);
    
    debug(INFO, "Exit!");
    
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

/**
 * Anzeigen: ipcs -m
 * Löschen: ipcrm -m <shmid>
 */
void setupSharedMemory() {
    int   fd;
    key_t key;
    void *shmAddr;
    size_t size;
    
    size = 2 * anzSensors * sizeof(SensorData);
    
    if ((fd = open(SHM_KEY_FILE, O_RDWR | O_CREAT, 0770)) >= 0) {
        close(fd);
        
        if ((key = ftok(SHM_KEY_FILE, PROJECT_ID)) != 0) {
            if ((shmId = shmget(key, size, 0770 | IPC_CREAT)) >= 0) {
                if ((shmAddr = shmat(shmId, NULL, 0)) != (void *) -1) {
                    sensors = (SensorData *) shmAddr;
                } else {
                    debug(FATAL, "Can't map shared mempory: %s", strerror(errno));
                }
            } else {
                debug(FATAL, "Can't create shared memory: %s", strerror(errno));
            }
        } else {
            debug(FATAL, "Can't convert key: %s", strerror(errno));
        }
    } else {
        debug(FATAL, "Can't open file: %s", strerror(errno));
    }
}

void signalHandler(int sigNo) {
    switch (sigNo) {
        case SIGINT:
            debugNewLine();
            debug(INFO, "Received interrupt signal (%d)", sigNo);
            shutdown();
            break;
        
        case SIGTERM:
            debug(INFO, "Received termination signal (%d)", sigNo);
            shutdown();
            break;
        
        case SIGALRM:
            debug(INFO, "Received alarm signal (%d)", sigNo);
            break;
    }
}

void shutdown() {
    
    debug(INFO, "Shutdown");
    
    pthread_mutex_lock(&runningMutex);
    running = false;
    
    if (shmctl(shmId, IPC_RMID, NULL) == -1) {
        debug(FATAL, "Can't remove shared memory %u: %s", shmId, strerror(errno));
    }
    
    pthread_mutex_unlock(&runningMutex);
    
    pthread_cancel(sockThread);
    
    if (kill(displayPid, SIGUSR1) == -1) {
        debug(FATAL, "Can't kill Display PID %d: %s", displayPid, strerror(errno));
    }
        
    if (kill(controlPid, SIGUSR1) == -1) {
        debug(FATAL, "Can't kill Control PID %d: %s", controlPid, strerror(errno));
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
                debug(FATAL, "Status from child %d: Exit!", WEXITSTATUS(status));
                shutdown();
            } else {
                if (returnPid == controlPid) {
                    debug(DEBUG, "Restart HSControl");
                    if (kill(displayPid, SIGUSR1) == -1) {
                        debug(FATAL, "Can't kill PID %d", displayPid, strerror(errno));
                    }
                    oldDisplayPid = displayPid;
                    createDisplayProcess(&displayPid, nSignals);
                    createControlProcess(&controlPid, nSignals, displayPid);
                } else if (returnPid == displayPid) {
                    debug(DEBUG, "Restart HSDisplay");
                    createDisplayProcess(&displayPid, nSignals);
                } else if (returnPid == oldDisplayPid) {
                    debug(DEBUG, "Parent catched old Display PID (%d)\n", returnPid);
                } else {
                    debug(ERROR, "Unknow child with PID %d\n", returnPid);
                }
            }
        }
        pthread_mutex_unlock(&runningMutex);
        
        // Warten auf alle Kinder
        returnPid = waitpid(-1, &status, 0);
    }
    
    debug(INFO, "Exit Process Thread");
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

/******************************************************************************
 * Socket Thread
 *****************************************************************************/

typedef struct {
    int i;
} IntStruct;

void *socketThread(void *param) {
    int                 listenfd;
    int                 connectfd;
    IntStruct           threadConnectFd;
    struct sockaddr_in  serverAddr;
    struct sockaddr_in  clientAddr;
    socklen_t           clientAddrLen;
    pthread_t           tid;
    char                clientIp[INET_ADDRSTRLEN];
    unsigned short      clientPort;
    
    struct sigaction action;
    
    action.sa_handler = signalHandler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGALRM, &action, NULL);
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family      = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port        = htons(COMM_PORT);
        
        if (bind(listenfd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) == 0) {
            if (listen(listenfd, SENSOR_MAX_NUM) == 0) {
                clientAddrLen = sizeof(clientAddr);
                while (running) {
                    if ((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &clientAddrLen)) >= 0) {
                        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
                        clientPort = ntohs(clientAddr.sin_port);
                        debug(INFO, "Accept client %s from port %u", clientIp, clientPort);

                        threadConnectFd.i = connectfd;
                        pthread_create(&tid, NULL, socketRequest, (void *) &threadConnectFd);
                    } else if (errno == EWOULDBLOCK) {
                        
                    } else {
                        debug(ERROR, "Can't accept connection: %s", errno, strerror(errno));
                    }
                }
                
            } else {
                debug(FATAL, "Can't mark socket as passive (listen-mode): %s", strerror(errno));
                shutdown();
            }
        } else {
            debug(FATAL, "Can't bind socket: %s", strerror(errno));
            shutdown();
        }
        
        close(listenfd);
        
    } else {
        debug(FATAL, "Can't create socket: %s", strerror(errno));
        shutdown();
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
    SensorData sensor;
    int        connectfd;
    ssize_t    len;
    
    connectfd = ((IntStruct *) param)->i;
    
    len = read(connectfd, (char *)&sensor, sizeof(SensorData));
    if (len >= 0) {
        if (len == sizeof(SensorData)) {
            debug(INFO, "deviceID=%u sequenceNr=%u valIS=%f valREF=%f status=%d",
                sensor.deviceID, sensor.sequenceNr, sensor.valIS, sensor.valREF,
                sensor.status);
            
            if (sensors >= 0) {
                sensors[sensor.deviceID] = sensor;
            } else {
                debug(INFO, "Can't access sensor memory");
            }
        } else {
            debug(ERROR, "Size of data read don't match! size=%u", len);
        }
    } else {
        debug(ERROR, "Can't read: %s", strerror(errno));
    }
                                            
    close(connectfd);
    return NULL;
}





