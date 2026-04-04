#include "shared.h"
#include "semaphores.h"
#include "utils.h"

int main(int argc, char *argv[]){

    if (argc != 3) {
        exitError("Uso: ./view [width] [height]");
    } 

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    
    //Referencio las memorias compartidas
    
    unsigned long gameSize = sizeof(Game) + width * height * sizeof(char) - sizeof(char);
    Game *game = attachShared(SHARED_GAME, gameSize);    
    if (game == NULL) return 1;

    SyncState * sync = attachShared(SHARED_SYNC, sizeof(SyncState));
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