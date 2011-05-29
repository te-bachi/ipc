/****************************************************************************
*
* "Semaphoren und Shared-Memory"                Filename: "shm.cc"
*
* Programmbeschreibung:
*
* Das vorliegende Programm demonstriert eine Moeglichkeit um von mehreren
* Prozessen auf einen gemeinsamen Speicherbereich zuzugreiffen. 
*
* Zu Beginn des Programms wird eine Semaphore und ein gemeinsamer Memory-
* Bereich definiert und initialisiert. Ein Reader-Prozess (Kind) liest in 
* regelmaessigen Abstaenden die Daten aus dem Shared-Memory und zeigt
* diese auf dem Bildschirm an waehrend ein Writer-Prozess (Vater) in
* zufaelligen Abstaenden ins Shared-Memory schreibt. Der Zugriff auf
* das Shared-Memory wird durch eine Semaphore geschuetzt.
*
*****************************************************************************
*
* Projekt      : Linux IPC Praktikum
* 
* Datum/Name   : 25-Mai-98  durch M. Rueesch und D. Eisenegger 
*
* Aenderungen  : 31.5.2007, M. Thaler: semaphore.cc und shm.cc
*                   
****************************************************************************/

#include <stdio.h>                              // Standard IO Funktionen 
#include <errno.h>                              // Fehlerbehandlung 
#include <stdlib.h>                             // Standard Funktionen
#include <signal.h>                             // Signalbehandlung 
#include <termio.h>                             // Terminal IO Funktionen 
#include <unistd.h>	                        // Fuer "sleep"	

#include <sys/wait.h>	                        // Fuer "wait"		
#include <sys/types.h>                          // UNIX Typendefinitionen 
#include <sys/ipc.h>                            // SVR4 IPC Mechanismen 
#include <sys/sem.h>                            // SVR4 Semaphoren 
#include <sys/shm.h>                            // SVR4 Shared Memory 

#include "semaphore.h"
#include "shmem.h"

//***************************************************************************
//  Lokale Symbole und Typendefinitionen
//***************************************************************************

struct T_Shared_Memory{                         // *** Shared Memory Typ ***
   int field[10];				
   char msg[20+1];                              // Wird in diesem Beispiel
};                                              // nicht benutzt

typedef T_Shared_Memory *shared_memory_ptr;

#define SHM_LEN sizeof (T_Shared_Memory)
#define keyFnameShm "/tmp/sem_key_file.shm.uebung" 

#define keyFnameSem "/tmp/sem_key_file.sem.uebung"
 
#define PROJECT_ID 199


//***************************************************************************
// Lokale Funktionen
//***************************************************************************
void writer_prozess (pid_t kind_pid);
void reader_prozess ();
void handler        (int sig);
void cleanup        (int sig);  
 
//***************************************************************************
// Lokale Variablen
//***************************************************************************
int reader_flag = 1;                            // Wenn =0: Kind terminiert
int shm_id;                                     // Shared Memory Identifikation
int sem_id;                                     // Semaphore Identifikation

//***************************************************************************
// FUNKTION: main ()
//***************************************************************************
int main (void)
{
   pid_t pid_c;
   int i;

   struct sigaction sig;
   sigemptyset(&sig.sa_mask);
   sig.sa_handler = handler;
   sig.sa_flags = 0;
   sigaction(SIGUSR1, &sig, NULL);

   shared_memory_ptr shared;        // Pointer auf eine Shared Memory Region
    
   // ***** Definieren eines Shared Memory Bereichs *****

   SharedMemory shm = SharedMemory(SHM_LEN,  keyFnameShm, PROJECT_ID); 
   shared =  (shared_memory_ptr)(shm.getSharedMemory());

   Semaphore sem = Semaphore(1, keyFnameSem, PROJECT_ID);
   sem.up(0);
   
   
   for (i=0; i<10; i++) shared->field[i]=i;     // Memorysegment initialisieren 
   
   pid_c = fork ();                             // Neuer Prozess erzeugen 
   switch (pid_c)
   {
      case -1:                                  // Fehlerfall 
      {
         perror ("Fork :");
         exit (-1);
      }
      case 0:                                   // Kindprozess 
      {
         reader_prozess ();
         break;
      }
      default:                                  // Vaterprozess
      { 
         writer_prozess (pid_c);       
         kill(getpid(),SIGINT);			// Aufruf von cleanup
         break;                                
      }
   }
   return 0;
}

//***************************************************************************
// FUNKTION: writer_prozess ()         "VATER"
//***************************************************************************
//void writer_prozess (pid_t pid_c, int sem_id, shared_memory_ptr shared)
void writer_prozess (pid_t pid_c)
{ 

   SharedMemory shm = SharedMemory(0,  keyFnameShm, PROJECT_ID);
   shared_memory_ptr shared =  (shared_memory_ptr)(shm.getSharedMemory());

   Semaphore sem = Semaphore(0, keyFnameSem, PROJECT_ID);
   signal (SIGINT, cleanup);
      
   for (int i = 9 ; i >= 0; i--) 
   {
      printf ("WRITER: Entering Critical Section\n"); 
      sem.down(0);
      printf ("WRITER: Inside Critical Section\n");             
      shared->field[i] = 10 - i;
      sleep (1);
      printf ("WRITER: Leave Critical Section\n"); 
      sem.up(0);
      sleep (rand() % 10);                   // Schreiber 0..10 Sekunden warten 
   }
   kill (pid_c, SIGUSR1);                    // Signal zum Beenden an Kind senden
   wait (NULL);	                             // Warten bis Kind terminiert hat
}

//***************************************************************************
// FUNKTION: reader_prozess ()        "KIND"
//***************************************************************************
void reader_prozess ()
{ 
   SharedMemory shm = SharedMemory(0, keyFnameShm, PROJECT_ID);
   shared_memory_ptr shared =  (shared_memory_ptr)(shm.getSharedMemory());
   Semaphore sem = Semaphore(0, keyFnameSem, PROJECT_ID);
   
   
   while (reader_flag)
   {      
      printf ("READER: Entering Critical Section\n"); 
      sem.down(0);
      printf ("READER: Inside Critical Section\n"); 
      printf ("READER: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
               shared->field[0], shared->field[1], shared->field[2], shared->field[3], 
               shared->field[4], shared->field[5], shared->field[6], shared->field[7],
               shared->field[8], shared->field[9]);
      fflush (stdout);                          // Buffer an stdout ausgeben
      sleep (2); 
      printf ("READER: Leave Critical Section\n");
      sem.up(0); 
      sleep (1);
   }
   exit (0);
}

//***************************************************************************
// FUNKTION: handler () (Signalhandler)
//***************************************************************************
void handler (int sig)
{
   reader_flag = 0;
}

//***************************************************************************
// FUNKTION: cleanup () (Signalhandler)
//***************************************************************************
void cleanup (int sig)
{
   semun sem_union;
   
   printf("CLEANUP: Ressourcen werden freigegeben \n");
   SharedMemory shm = SharedMemory(0, keyFnameShm, PROJECT_ID);
   shm.removeSharedMemory();
   Semaphore sem = Semaphore(0, keyFnameSem, PROJECT_ID); 
   sem.removeSemaphore();
   exit (0);
}

//***************************************************************************
