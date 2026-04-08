#include "shared.h"
#include "semaphores.h"
#include "utils.h"

int getPlayerId(Game *game);

int main(int argc, char *argv[]){

    if (argc != 3) {
        exitError("Uso: ./view [width] [height]");
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    //Referencio las memorias compartidas

    unsigned long gameSize = sizeof(Game) + width * height * sizeof(char) - sizeof(char);
    Game *game = attachShared(SHARED_GAME, gameSize, false);
    if (game == NULL) return 1;

    SyncState * sync = attachShared(SHARED_SYNC, sizeof(SyncState), true);
    if (sync == NULL) return 1;

    // Reconocer que player es
    int id = getPlayerId(game);
    if (id == -1) return 1;

    bool hasFinished = false;
    while(!hasFinished) {
        sem_wait(&sync->can_player_move[id]);

        sem_wait(&sync->master_priority);
        sem_post(&sync->master_priority);

        sem_wait(&sync->can_access_readers_count);
            if (sync->readers_count++ == 0) sem_wait(&sync->can_access_game_state);
        sem_post(&sync->can_access_readers_count);

        // Zona de acceso al game state

        printf("%d", randInt(0, 7));

        if (game->players[id].blocked) hasFinished = true;

        sem_wait(&sync->can_access_readers_count);
            if (sync->readers_count-- == 1) sem_post(&sync->can_access_game_state);
        sem_post(&sync->can_access_readers_count);

    }

    munmap(game, gameSize);
    munmap(sync, sizeof(SyncState));

    return 0;
}

int getPlayerId(Game *game) {
    pid_t pid = getpid();
    for (size_t i = 0; i < game->num_players; i++) {
        if (game->players[i].pid == pid) {
            return i;
        }
    }
    return -1;
}
