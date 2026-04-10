#include "semaphores.h"

void initSemaphore(sem_t *sem, unsigned int value);

void initializeSemaphores(SyncState * state, Game * game){
    initSemaphore(&(state->has_to_print), 1);
    initSemaphore(&(state->view_finished), 1);
    initSemaphore(&(state->master_priority), 1);
    initSemaphore(&(state->can_access_game_state), 1);
    initSemaphore(&(state->can_access_readers_count), 1);
    state->readers_count=0;
    for(int i = 0 ; i < game->num_players ; i++){
        initSemaphore(&(state->can_player_move[i]), 0);
    }
}

void initSemaphore(sem_t *sem, unsigned int value) {
    if (sem_init(sem, SHARED_MEMORY, value) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
}