#include <ctype.h>          // isdigit()

#include "Utils.h"


bool isnumber2(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i]) == false) {
            return false;
        }
    }
    return true;
}

