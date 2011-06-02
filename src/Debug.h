#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

namespace zhaw {
    namespace ipc {
        
        enum Level {
            NONE,
            FATAL,
            ERROR,
            WARN,
            INFO,
            DEBUG
        };
        
        class Debug {
            private:
                static FILE *_stream;
                static Level _level;
            
            public:
                static void setStream(FILE *stream);
                static void setLevel(Level level);
                static void newLine();
                static void log(Level level, const char *format, ...);
        };
    };
};

#endif
