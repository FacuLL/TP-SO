#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER 100

int main(int argc, char const *argv[])
{
    int fd = open("/tmp/ganador", O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el pipe compartido");
        exit(1);
    }

    char buffer[BUFFER];
    int length = 0;
    while ((length = read(fd, buffer, BUFFER)) != 0) {
        for (size_t i = 0; i < length; i++) {
            putc(buffer[i], STDOUT_FILENO);
        }
    }

    close(fd);
    return 0;
}
