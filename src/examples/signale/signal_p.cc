/******************************************************************************
*
* Signale                                       Filename: "signal_p.cc"
*
* Programmbeschreibung:
*
* Es wird ein Vater und ein Kindprozess erzeugt. Der Vaterprozess steuert
* mit verschiedenen Signalen den Ablauf des Kindprozesses. 
*
*******************************************************************************
*
* Projekt      : Linux IPC Praktikum 
* 
* Datum/Name   : 25-Mai-98  durch M. Rueesch und D. Eisenegger 
*
* Aenderungen  : 6-11-00 Markus Thaler: Signale mit sigemtyset()
*                   
******************************************************************************/

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------
#include <stdio.h>                              // Standard IO Funktionen
#include <stdlib.h>                             // Standardbibliothek
#include <unistd.h>                             // Linux Standard 
#include <errno.h>                              // Fehlerbehandlung     
#include <sys/types.h>                          // Linux Typendefinitionen
#include <sys/wait.h>                           // fuer wait(), waitpid() 
#include <signal.h>                             // Signalhandling Funktionen

//-----------------------------------------------------------------------------
// Lokale Funktionen
//-----------------------------------------------------------------------------
void vater_prozess (pid_t kind_pid);		// Prototyp Vaterprozess 
void signal_handler (int sig);

//*****************************************************************************
// FUNKTION: main ()
//*****************************************************************************

int main (void){

   pid_t pid_c;                                 // Variable fuer Process ID
   
   printf("Hauptprogramm startet mit (PID: %d, PPID: %d)\n",getpid(),getppid());
   pid_c = fork();                              // Kindprozess erzeugen
   switch (pid_c){
      case -1: {                                // Fehlerfall 
         perror ("Fork :");
         exit (-1);
      }
      case 0: {                                 // wir sind im Kindprozess 
         execl ("./signal_c.e", "signal_c.e", NULL); 
         perror ("execl error:  ");             // externes Programm starten
         exit (-1);
         break;
      }
      default: {
         vater_prozess (pid_c);                 // Vaterprozess starten
         break;
      }
   }
   return 0;
}

//*****************************************************************************
// FUNKTION: vater_prozess ()                   Vaterprozess
//*****************************************************************************

void vater_prozess (pid_t kind_pid){
 
   int		i;
   int		status, stat;
   struct sigaction neu;                       
   
      
   printf ("Vater nimmt Arbeit auf (PID: %d, PPID: %d)\n", getpid(), getppid());
   sleep(2);

   neu.sa_handler = signal_handler;             // Signalhandler fuer das Signal
   neu.sa_flags = SA_RESTART;                   // SIGCLD einrichten
   sigemptyset(&neu.sa_mask);			// Zuslassen anderer Signale
   sigaction (SIGCLD, &neu, NULL);              // Das Flag SA_SIGINFO existiert
                                                // nicht unter Linux!
                                                
   signal (SIGINT, SIG_IGN);                    // Control C ignorieren !!
 
   for (i = 0 ; i <= 4 ; i++){
      printf ("VATER: Schlaufe %u \n", i);
      sleep (2);
   }

   printf ("VATER: Kind gestoppt\n");           // Kind stoppen
   kill (kind_pid, SIGSTOP);
   
   for (i = 5 ; i <= 8 ; i++){
      printf ("VATER: Schlaufe %u \n", i);
      sleep (2);
   }

   printf ("VATER: Kind laeuft weiter\n");
   kill (kind_pid, SIGCONT);                    // Kind wieder starten
   sleep (2);

   printf ("VATER: Sende SIGUSR1 an Kind...\n");
   kill (kind_pid, SIGUSR1);
   for (i = 8 ; i <= 10 ; i++){
      printf ("VATER: Schlaufe %u \n", i);
      sleep (2);
   }      
   printf ("VATER: Jetzt bringen wir den Kindprozess um...\n");
   kill (kind_pid, SIGINT);
   
   printf ("VATER: Warten, bis Kind abgeschlossen hat...\n");
   waitpid (kind_pid, &status, 0);
   stat = WEXITSTATUS(status);                  // Status auswerten und in 
                                                // int wandeln
   printf ("VATER: Exitstatus des Kindes: %d\n", stat);
   printf ("*** Vater terminiert ***\n");
}

//*****************************************************************************
// HANDLER: signal_handler ()
//*****************************************************************************

void signal_handler (int sig){
   
   printf ("------------------------------------------------------\n");
   printf ("Signalhandler Vater hat Signal Nr: %d empfangen\n", sig);
   printf ("------------------------------------------------------\n");
}

