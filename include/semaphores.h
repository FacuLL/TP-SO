#ifndef SEMAPHORES_H
#define SEMAPHORES_H

    #include "structs.h"
    #include "defaultValues.h"
    #include <stdlib.h>
    #include <stdio.h>

    #define SHARED_MEMORY 1
    #define SEMAPHORE_EXIT_FAILURE -1

    void initializeSemaphores(SyncState * state, Game * game);

#endif