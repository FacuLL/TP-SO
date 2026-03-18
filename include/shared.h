#ifndef SHARED_H
#define SHARED_H

    #include <fcntl.h>
    #include <sys/stat.h>
    #include <stdio.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <sys/types.h>

    void * initializeShared(const char * name, unsigned long size);

#endif