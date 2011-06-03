/******************************************************************************
*
* Message Queues                                Filename: "msg_bsp.cc"
*
* Programmbeschreibung:
*
* Der Vaterprozess (Client) sendet dem Kindprozess (Server) Rechnungsaufgaben.
* Der Meldungstyp bestimmt, ob die Meldung fuer den Server oder den Client ist.
* Je nach Operation (op) fuehrt der Server die Rechnung aus und sendet das 
* Resultat dem Client zurueck.
*
*******************************************************************************
*
* Projekt      : Linux IPC Praktikum
* 
* Datum/Name   : 25-Mai-98  durch M. Rueesch und D. Eisenegger 
*
* Aenderungen  :  
*                   
******************************************************************************/

//-----------------------------------------------------------------------------
//  Include-Dateien:
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>                             	// Standard IO Funktionen
#include <sys/wait.h>                           // feur wait, waitpid,...
#include <unistd.h>                             // Linux standards
#include <sys/types.h>                          // Linux Typendefinitionen
#include <sys/ipc.h>                            // SVR4 IPC Mechanismen 
#include <sys/msg.h>                            // SVR4 Message Queues 

//-----------------------------------------------------------------------------
//  Lokale Symbole, Makros und Typendefinitionen:
//-----------------------------------------------------------------------------
#define PLUS  '+'                               // Definitionen der Operationen. 
#define MINUS '-'
#define MULT  '*'
#define DIV   '/'
#define ENDE  0x7F

#define REQUEST 100L                            // Meldungstypen
#define ANSWER  101L

typedef struct {
	long	mtype;
	char	op;
	int	arg1;
	int	arg2;
} msg_request_type;

typedef struct {
	long	mtype;
	long	result;
} msg_answer_type;

#define LEN_REQ sizeof (msg_request_type) - sizeof (long)  
#define LEN_ANS sizeof (msg_answer_type) - sizeof (long)
                                                // Laenge der Message definieren
                                                // wobei die Laenge des Grundtyps ab-
                                                // gezogen wird.
//-----------------------------------------------------------------------------
//  Lokale Funktionen:
//-----------------------------------------------------------------------------

void vater_prozess (pid_t kind_pid, int qid);
void calculate (int qid, int arg1, int arg2, char op, int lauf);
void kind_prozess (int qid);

//*****************************************************************************
// FUNKTION: main ()
//*****************************************************************************

int main (void){

   pid_t pid_c;
   int qid;

   qid = msgget (IPC_PRIVATE, 0777 | IPC_CREAT | IPC_EXCL); 
                                              	// Queue erzeugen ohne Key
   pid_c = fork ();                             // Kindprozess erzeugen	
   switch (pid_c)
   {
      case -1:{                                 // Fehlerfall
         perror ("Fork :");
         exit (-1);
      }
      case 0:{                                  // Kindprozess starten
         kind_prozess (qid);
         break;
      }
      default:{
         vater_prozess (pid_c, qid);            // Vaterprozess starten
         msgctl (qid, IPC_RMID, NULL);          // Queue wieder zerstoeren
         break;
      }
   }
   return 0;
}

//*****************************************************************************
// FUNKTION: vater_prozess ()
//*****************************************************************************

void vater_prozess (pid_t kind_pid, int qid){

   msg_request_type sendbuf;                    // Buffer um Meldungen zu senden

   printf ("Start der Verarbeitung\n");
   
   calculate (qid, 12, 3, PLUS, 1);             // Rechnungsaufgaben senden
   sleep(1);

   calculate (qid, 12, 3, MINUS, 2);
   sleep(1);

   calculate (qid, 12, 3, MULT, 3);
   sleep(1);

   calculate (qid, 12, 3, DIV, 4);
   sleep(1);

   printf ("CLIENT: Server abschalten\n");
   
   sendbuf.mtype = REQUEST;                     // Dem Client das Ende der Be-
   sendbuf.op = ENDE;                           // rechnungen signalisieren.
   msgsnd (qid, &sendbuf, LEN_REQ, 0);
   wait (NULL);                                 // Warten bis der Kindprozess 
}                                               // terminiert hat.

//*****************************************************************************
// FUNKTION: calculate ()
//*****************************************************************************

void calculate (int qid, int arg1, int arg2, char op, int lauf){

   msg_request_type sendbuf;                    // Buffer um Meldungen zu senden. 
   msg_answer_type  recvbuf;                    // Buffer fuer emfangene Meldungen. 

   sendbuf.mtype = REQUEST;                     // Sendebuffer einfuellen.
   sendbuf.op = op;
   sendbuf.arg1 = arg1;
   sendbuf.arg2 = arg2;
   
   printf("CLIENT: gebe %d. Rechnung (%d %c %d) in Auftrag.\n",lauf,arg1,op,arg2);
   
   msgsnd (qid, &sendbuf, LEN_REQ, 0);          // Rechnung als Meldung senden
    
   printf ("CLIENT: warte auf Antwort...\n");
   
   msgrcv (qid, &recvbuf, LEN_ANS, 666, 0);  // Auf Resultat als Meldung warten
   
   printf ("CLIENT: Das Ergebnis lautet: %ld\n", recvbuf.result); 
}                                               // Resultat ausgeben.

//*****************************************************************************
// FUNKTION: kind_prozess ()
//*****************************************************************************

void kind_prozess (int qid){

   msg_request_type recvbuf;                    // Variablen fuer Sende- und  
   msg_answer_type  sendbuf;                    // Empfangsmeldungen.

   printf ("SERVER: Warte auf Anfrage...\n");
   
   msgrcv (qid, &recvbuf, LEN_REQ, 999, 0); // 1. Meldung lesen
   while (recvbuf.op != ENDE)   {
      printf ("SERVER: Verarbeite Anfrage...\n");
      sendbuf.result = 0; 
      switch (recvbuf.op){                      // Je nach op Resultat berechnen 
         case PLUS:{
            sendbuf.result = recvbuf.arg1 + recvbuf.arg2;
            break;
         } 
         case MINUS:{
            sendbuf.result = recvbuf.arg1 - recvbuf.arg2;
            break;
         } 
         case MULT:{
            sendbuf.result = recvbuf.arg1 * recvbuf.arg2;
            break;
         } 
         case DIV:{
            sendbuf.result = recvbuf.arg1 / recvbuf.arg2;
            break;
         } 
         default:{
            break;
         }
      }
      printf ("SERVER: Sende Antwort...\n");  
      sendbuf.mtype = ANSWER;
        
      if (msgsnd(qid, &sendbuf, LEN_ANS, 0) < 0){
                                                // Meldung (Resultat) senden. 
         perror ("error beim Queue beschreiben");
         exit (1);
      }
      sleep (5);
      if (msgrcv (qid, &recvbuf, LEN_REQ, REQUEST, 0) < 0){
                                              	// naechste Meldung lesen.
         perror ("Queue lesen");
         exit (1);
      }
   }
   printf ("SERVER: *** Ende ***\n");
}
