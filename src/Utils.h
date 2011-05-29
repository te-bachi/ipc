#ifndef __UTILS_H__
#define __UTILS_H__

enum DebugLevel {
    NONE,
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG
};


bool isnumber(char *str);
void setDebugStream(FILE *stream);
void setDebugLevel(DebugLevel level);
void debugNewLine();
int debug(DebugLevel level, const char *format, ...);

#endif

