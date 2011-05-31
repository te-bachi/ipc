#include <ctype.h>          // isdigit()
#include <stdio.h>          // vprintf()
#include <stdarg.h>         // va_list
#include <unistd.h>         // sleep(), getpid()
#include <sys/types.h>      // waitpid(), getpid()
#include <time.h>

#include "Utils.h"

DebugLevel currentLevel = NONE;
FILE *debugStream = stderr;

bool isnumber(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i]) == false) {
            return false;
        }
    }
    return true;
}

void setDebugStream(FILE *stream) {
    debugStream = stream;
}

void setDebugLevel(DebugLevel level) {
    currentLevel = level;
}

void debugNewLine() {
    fprintf(debugStream, "\n");
}

int debug(DebugLevel level, const char *format, ...) {
    int       retVal = 0;
    time_t    unixtime;
    struct tm now;
    va_list   args;
    
    if (level <= currentLevel) {
        va_start(args, format);
        unixtime = time(NULL);
        localtime_r(&unixtime, &now);
        fprintf(debugStream, "[%02d.%02d.%02d %02d:%02d:%02d] ", now.tm_mday,
                                                                 now.tm_mon + 1,
                                                                 now.tm_year + 1900,
                                                                 now.tm_hour,
                                                                 now.tm_min,
                                                                 now.tm_sec);
        if      (level == FATAL) fprintf(debugStream, "[FATAL] ");
        else if (level == ERROR) fprintf(debugStream, "[ERROR] ");
        else if (level == WARN)  fprintf(debugStream, "[WARN ] ");
        else if (level == INFO)  fprintf(debugStream, "[INFO ] ");
        else if (level == DEBUG) fprintf(debugStream, "[DEBUG] ");
        fprintf(debugStream, "[%5d] ", getpid());
        retVal = vfprintf(debugStream, format, args);
        fprintf(debugStream, "\n");
        fflush(debugStream);
        va_end(args);
    }
    
    return retVal;
}

