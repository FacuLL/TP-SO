#ifndef SHARED_H
#define SHARED_H

    #include <fcntl.h>
    #include <sys/stat.h>
    #include <stdio.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <fcntl.h>

    #include "structs.h"

    #define SHARED_GAME "/game_state"
    #define SHARED_SYNC "/game_sync"

    void * initializeShared(const char * name, unsigned long size);
    void * attachShared(const char * name, unsigned long size);

#endif