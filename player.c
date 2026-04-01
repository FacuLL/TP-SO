#include "shared.h"
#include "semaphores.h"


int main(int argc, char const *argv[]){

    
    int fd_game_state = shm_open("/game_state", O_RDONLY, 0666);
    if (fd_game_state == -1) {
        perror("shm_open");
        return -1;
    }

    Game * gameState = (Game *)mmap(NULL, sizeof(Game),  PROT_READ, MAP_SHARED, fd_game_state, 0 );
    if (sync == MAP_FAILED){
		perror("mmap");
		return -1;
	}

    int fd_game_sync = shm_open("/game_sync", O_RDONLY, 0666);
    if (fd_game_sync == -1) {
        perror("shm_open");
        return -1;
    }

    SyncState * sync = (SyncState *)mmap(NULL, sizeof(SyncState), PROT_READ, MAP_SHARED, fd_game_sync, 0 );
	if (sync == MAP_FAILED){
		perror("mmap");
		return -1;
	}

    printf("Hijo Dice: %d",gameState->num_players);
    fflush(stdout);

    close(fd_game_sync);
    close(fd_game_state);

    sem_post(&(sync->has_to_print));
    sem_wait(&(sync->can_player_move[0]));
    
    munmap(gameState, sizeof(Game));
    munmap(sync, sizeof(SyncState));

    return 0;
}