#include <time.h>           // time()
#include <stdarg.h>         // va_list
#include <unistd.h>         // getpid()
#include <sys/types.h>      // getpid()

#include "Debug.h"

using namespace zhaw::ipc;

FILE *Debug::_stream = stderr;
Level Debug::_level = NONE;

void Debug::setStream(FILE *stream) {
    _stream = stream;
}

void Debug::setLevel(Level level) {
    _level = level;
}

void Debug::newLine() {
    fprintf(_stream, "\n");
}

void Debug::log(Level level, const char *format, ...) {
    time_t    unixtime;
    struct tm now;
    va_list   args;
    
    if (level <= _level) {
        va_start(args, format);
        unixtime = time(NULL);
        localtime_r(&unixtime, &now);
        fprintf(_stream, "[%02d.%02d.%02d %02d:%02d:%02d] ", now.tm_mday,
                                                             now.tm_mon + 1,
                                                             now.tm_year + 1900,
                                                             now.tm_hour,
                                                             now.tm_min,
                                                             now.tm_sec);
        if      (level == FATAL) fprintf(_stream, "[FATAL] ");
        else if (level == ERROR) fprintf(_stream, "[ERROR] ");
        else if (level == WARN)  fprintf(_stream, "[WARN ] ");
        else if (level == INFO)  fprintf(_stream, "[INFO ] ");
        else if (level == DEBUG) fprintf(_stream, "[DEBUG] ");
        fprintf(_stream, "[%5d] ", getpid());
        vfprintf(_stream, format, args);
        fprintf(_stream, "\n");
        fflush(_stream);
        va_end(args);
    }
}

