/******************************************************************/
/* Example shows how to set alarms for blocking socket APIs       */
/******************************************************************/

/******************************************************************/
/* Include files                                                  */
/******************************************************************/
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

/******************************************************************/
/* Signal catcher routine.  This routine will be called when the  */
/* signal occurs.                                                 */
/******************************************************************/
void catcher(int sig)
{
   printf("   Signal catcher called for signal %d\n", sig);
}

/******************************************************************/
/* Main program                                                   */
/******************************************************************/
int main(int argc, char *argv[])
{
   struct sigaction sact;
   struct sockaddr_in6 addr;
   time_t t;
   int sd, rc;

/******************************************************************/
/* Create an AF_INET6, SOCK_STREAM socket                         */
/******************************************************************/
   printf("Create a TCP socket\n");
   sd = socket(AF_INET6, SOCK_STREAM, 0);
   if (sd == -1)
   {
      perror("   socket failed");
      return(-1);
   }

/******************************************************************/
/* Bind the socket.  A port number was not specified because      */
/* we are not going to ever connect to this socket.               */
/******************************************************************/
   memset(&addr, 0, sizeof(addr));
   addr.sin6_family = AF_INET6;
   printf("Bind the socket\n");
   rc = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
   if (rc != 0)
   {
      perror("   bind failed");
      close(sd);
      return(-2);
   }

/******************************************************************/
/* Perform a listen on the socket.                                */
/******************************************************************/
   printf("Set the listen backlog\n");
   rc = listen(sd, 5);
   if (rc != 0)
   {
      perror("   listen failed");
      close(sd);
      return(-3);
   }

/******************************************************************/
/* Set up an alarm that will go off in 5 seconds.                 */
/******************************************************************/
   printf("\nSet an alarm to go off in 5 seconds. This alarm will cause the\n");
   printf("blocked accept() to return a -1 and an errno value of EINTR.\n\n");
   sigemptyset(&sact.sa_mask);
   sact.sa_flags = 0;
   sact.sa_handler = catcher;
   sigaction(SIGALRM, &sact, NULL);
   alarm(5);

/******************************************************************/
/* Display the current time when the alarm was set                */
/******************************************************************/
   time(&t);
   printf("Before accept(), time is %s", ctime(&t));

/******************************************************************/
/* Call accept.  This call will normally block indefinitely,      */
/* but because we have an alarm set, it will only block for       */
/* 5 seconds.  When the alarm goes off, the accept call will      */
/* complete with -1 and an errno value of EINTR.                  */
/******************************************************************/
   errno = 0;
   printf("   Wait for an incoming connection to arrive\n");
   rc = accept(sd, NULL, NULL);
   printf("   accept() completed.  rc = %d, errno = %d\n", rc, errno);
   if (rc >= 0)
   {
      printf("   Incoming connection was received\n");
      close(rc);
   }
   else
   {
      perror("   errno string");
   }

/******************************************************************/
/* Show what time it was when the alarm went off                  */
/******************************************************************/
   time(&t);
   printf("After accept(), time is %s\n", ctime(&t));
   close(sd);
   return(0);
}