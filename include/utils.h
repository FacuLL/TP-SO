#ifndef UTILS_H
#define UTILS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>
    #include <getopt.h>
    #include <unistd.h>
    #include "structs.h"

    #define PI 3.14159

    void exitError(const char * error);
    char * intToStr(int num);
    int randInt(int min, int max);
    void initializeArgs(int argc, char *argv[], Arguments * arguments);

#endif