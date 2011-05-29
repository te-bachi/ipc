//******************************************************************************
// defs.h   global definitions for IPC project
// Author:  M. Thaler
// Date:    3/2011
//******************************************************************************

#ifndef THE_GLOBAL_DEFINITIONS
#define THE_GLOBAL_DEFINITIONS

//------------------------------------------------------------------------------

#define PGROUP		"MyName"
#define PGROUPNR	0

// fflush requires stdio.h
#include <stdio.h>

#define ClearScreen() printf("\033[H\033[J"); fflush(stdout)
#define HomeScreen()  printf("\033[H");       fflush(stdout)

#define mSLEEP(delay) usleep((int)(delay*1000))

#define COMPANY "Green Systems\n\n\n"

#define BUF_SIZE 256            // max size of buffers

#define SENSOR_MAX_NUM 8        // max number of devices

#define COMM_PORT 44444         // sensor ports

//------------------------------------------------------------------------------

typedef struct {                // sensor data
	unsigned deviceID;          // sensor ID
	unsigned sequenceNr;        // sequence number of data
    float    valIS;             // temperature: measured
    float    valREF;            // temperature: reference
	int		 status;			// status
} SensorData, *SensorDataPtr;

//------------------------------------------------------------------------------

typedef struct {                                    // message data
    int   sequenceNr;
    int   deviceID;
    float delta;
    char  statusText[64];  
} MessageData, *MessageDataPtr;

#define MSG_TYPE 3333L                              // message type

typedef struct {                                    // message 
    long        msgType;                            //     type
    MessageData mdata;                              //     data
} Message, *MessagePtr;

#define MSG_LENGTH (sizeof(Message) - sizeof(long)) // length of message

//------------------------------------------------------------------------------

#define MSG_TYPE1 3334L                             // message type

typedef struct {                                    // message type 1
    long  msgType;                                  //     type
	int	  numOfSensors;								//	   number of sensors
    float ctrl[SENSOR_MAX_NUM];                     //     data
} Msg, *MsgPtr;

#define MSG_LENGTH1 (sizeof(Msg) - sizeof(long))    // length of message

//------------------------------------------------------------------------------

#define PROJECT_ID    201
#define SEM_KEY_FILE  (const char *)("/tmp/my_sem_key.tmp")
#define SHM_KEY_FILE  (const char *)("/tmp/my_shm_key.tmp")
#define MBOX_KEY_FILE (const char *)("/tmp/my_box_key.tmp")

//------------------------------------------------------------------------------

#endif
