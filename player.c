#include "shared.h"
#include "semaphores.h"
#include "utils.h"

Arguments arguments = {
    .width = WIDTH,
    .height = HEIGHT
};

int main(int argc, char *argv[]){

    initializeArgs(argc, argv, &arguments);
    
    //Referencio las memorias compartidas
    
    unsigned long gameSize = sizeof(Game) + arguments.width * arguments.height * sizeof(char) - sizeof(char);
    Game *game = initializeShared(SHARED_GAME, gameSize);    
    if (game == NULL) return 1;

    SyncState * sync = initializeShared(SHARED_SYNC, sizeof(SyncState));
    if (sync == NULL) return 1;
    
    
    printf("Hijo Dice: %d",game->num_players);
    fflush(stdout);

    
    sem_post(&(sync->has_to_print));
    sem_wait(&(sync->can_player_move[0]));
    
    munmap(game, gameSize);
    munmap(sync, sizeof(SyncState));
    shm_unlink(SHARED_GAME);
    shm_unlink(SHARED_SYNC);

    return 0;
}