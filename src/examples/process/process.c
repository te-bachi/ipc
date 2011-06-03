#include <stdio.h>          // printf()
#include <stdlib.h>         // exit(), atoi()
#include <unistd.h>         // sleep(), getpid()
#include <sys/types.h>      // waitpid(), getpid()
#include <sys/wait.h>       // waitpid()
#include <errno.h>          // errno
#include <string.h>         // strerror()

int main(int argc, char *argv[]) {
    pid_t child1;
    pid_t child2;
    pid_t child3;
    pid_t child4;
    pid_t returnPid;
    int status;
    
    child1 = fork();
    
    if (child1 == 0) {
        printf("Child 1\n");
        sleep(3);
        exit(1);
    }
    
    child2 = fork();
    
    if (child2 == 0) {
        printf("Child 2\n");
        sleep(4);
        exit(2);
    }
    
    
    child3 = fork();
    
    if (child3 == 0) {
        printf("Child 3\n");
        sleep(5);
        exit(3);
    }
    
    child4 = fork();
    
    if (child4 == 0) {
        printf("Child 4\n");
        sleep(6);
        exit(4);
    }
    
    returnPid = waitpid(-1, &status, 0);
    do {
        printf("PID %d returned with status %d\n", returnPid, WEXITSTATUS(status));
        returnPid = waitpid(-1, &status, 0);
    } while (returnPid > 0);
    
    return 0;
}
