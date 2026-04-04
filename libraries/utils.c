#include "utils.h"

void exitError(const char * error) {
    fprintf(stderr, error);
    exit(1);
}

char * intToStr(int num) {
    char * str = malloc((int)((ceil(log10(num))+1)*sizeof(char)));
    sprintf(str, "%d", num);
    return str;
}