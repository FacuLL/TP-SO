#include "shared.h"
#include "semaphores.h"


int main(int argc, char const *argv[]){

    Game * game = initializeShared("/game_state", sizeof(Game));
    if (game == NULL) return 1;

    SyncState * sync = initializeShared("/game_sync", sizeof(SyncState));
    if (sync == NULL) return 1;

    write(STDOUT_FILENO, "Hola, hijos!\n", 13);

    return 0;
}
