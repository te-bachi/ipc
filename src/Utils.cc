#include <ctype.h>          // isdigit()
#include <stdio.h>          // vprintf()
#include <stdarg.h>         // va_list
#include <unistd.h>         // sleep(), getpid()
#include <sys/types.h>      // waitpid(), getpid()

#include "Utils.h"

bool isnumber(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i]) == false) {
            return false;
        }
    }
    return true;
}
       
int debug(const char *format, ...) {
#if DEBUG
    int     retVal;
    va_list args;

    va_start(args, format);
    printf("[DEBUG] [%5d] ", getpid());
    retVal = vprintf(format, args);
    va_end(args);
    
    return retVal;
#endif
}

