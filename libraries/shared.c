#include "shared.h"

void * initializeShared(const char * name, unsigned long size) {

    int fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        fprintf(stderr, "Error al iniciar shared memory %s\n", name);
        return NULL;
    }

    if (ftruncate(fd, size) == -1) {
        fprintf(stderr, "Error al establecer el tamaño shared memory %s\n", name);
        return NULL;
    }

    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    return ptr;
}