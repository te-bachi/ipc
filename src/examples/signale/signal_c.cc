/******************************************************************************
*
* Signale                                       Filename: "signal_c.cc"
*
* Programmbeschreibung:
*
* Es wird ein Vater- und ein Kindprozess erzeugt. Der Vaterprozess steuert
* mit verschiedenen Signalen den Ablauf des Kindprozesses. 
*
*******************************************************************************
*
* Projekt      : Linux IPC Praktikum 
* 
* Datum/Name   : 25-Mai-98  durch M. Rueesch und D. Eisenegger 
*
* Aenderungen  : 6-11-00 Markus Thaler: Signale mit sigaction() 
*                   
******************************************************************************/

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>                             // Signalfunktionen
#include <unistd.h> 

//-----------------------------------------------------------------------------
// Lokale Funktionen
//-----------------------------------------------------------------------------
void sig_handler (int sig);

//-----------------------------------------------------------------------------
// Globale Variabeln
//-----------------------------------------------------------------------------
char exit_flag = 1;


//*****************************************************************************
// FUNKTION: main ()                            Kindprozess
//*****************************************************************************

int main (void){
   
   int i = 0;
   struct sigaction neuAction;

   neuAction.sa_handler = sig_handler;
   neuAction.sa_flags = SA_RESTART;
   sigemptyset(&neuAction.sa_mask);
   sigaction (SIGINT, &neuAction, NULL);
   sigaction (SIGUSR1, &neuAction, NULL);

   printf (" Kind geboren (PID: %d, PPID: %d)\n", getpid(), getppid());
   
   while (exit_flag){                           // bleibt hier bis SIGUSR1 oder  
      printf (" KIND: erste Schlaufe %d\n", i); // SIGINT empfangen wurde.
      i++;
      sleep(1);
   }

   i = 0;
   while (1){                                   // bleibt hier bis SIGINT
      printf (" KIND: zweite Schlaufe %d\n", i);// empfangen wurde.
      i++;
      sleep(1);
   }
   printf (" Kind normal terminiert\n");
   exit (0);  
}

//*****************************************************************************
// HANDLER handler ()
//*****************************************************************************

void sig_handler (int sig){
   
   switch (sig){
      case SIGINT:{                             // SIGINT terminiert das Kind
         printf ("KIND: Signal SIGINT empfangen\n");
         sleep(2);
         printf ("*** Kind terminiert. ***\n");
         exit(5);                               // exit mit einem Statuswert                             
         break;
      }
      case SIGUSR1:{                            // SIGUSR1 bringt das Kind in
         printf ("KIND: Verlasse erste Schlaufe\n");  // die zweite Schlaufe
         exit_flag = 0; 
         break;
      }
      default:
         break;
   }
}
