#include "shared.h"


int main(int argc, char const *argv[]){
    
    Game * game = initializeShared("/game_state", sizeof(Game));
    if (game == NULL) return 1;

    SyncState * sync = initializeShared("/game_sync", sizeof(SyncState));
    if (sync == NULL) return 1;

    printf("Hola");
    fflush(stdout);

    return 0;
}
