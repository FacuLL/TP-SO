#include "semaphores.h"

void initializeSemaphores(SyncState * state, Game * game){
        // sem_init(sem_t *sem, int pshared, unsigned int value)
        //*sem: Puntero a semaforo ; 
        //pshared: 0 si no comparte con otro proceso, != 0 si lo va a hacer, en ese caso debe ser colocado en una region de memoria compartida; 
        //value: Valor inicial 
    if (sem_init(&(state->has_to_print), SHARED_MEMORY, 1) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
    if (sem_init(&(state->view_finished), SHARED_MEMORY, 1) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
    if (sem_init(&(state->master_mutex), SHARED_MEMORY, 1) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
    if (sem_init(&(state->game_state_mutex), SHARED_MEMORY, 1) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
    if (sem_init(&(state->readers_count_mutex), SHARED_MEMORY, 1) == -1) {
        perror("sem_init failed");
        exit(SEMAPHORE_EXIT_FAILURE);
    }
    state->readers_count=0;
    for(int i = 0 ; i < game->num_players ; i++){
        if (sem_init(&(state->can_player_move[i]), SHARED_MEMORY, 1) == -1){
            perror("sem_init failed");
            exit(SEMAPHORE_EXIT_FAILURE);
        }
    }
}