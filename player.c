// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shared.h"
#include "semaphores.h"
#include "utils.h"

int getPlayerId(Game *game);

int playerMovement(int id, int height , int width, Game * game);

void rotateVector();

int movements[8][2] = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

int vector = 0;

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

        //write en vez de print porque el print no envía hasta llenar el buffer y los players se quedan colgados
        unsigned char dir = (unsigned char)playerMovement(id, height, width, game);
        write(STDOUT_FILENO, &dir, 1);

        if (game->players[id].blocked || game->game_over) hasFinished = true;

        sem_wait(&sync->can_access_readers_count);
            if (sync->readers_count-- == 1) sem_post(&sync->can_access_game_state);
        sem_post(&sync->can_access_readers_count);

    }

    close(STDOUT_FILENO);

    munmap(game, gameSize);
    munmap(sync, sizeof(SyncState));

    return 0;
}

int playerMovement(int id, int height , int width, Game *game){
    char (*board)[game->width] = (char (*)[game->width])game->board;

    while(
        (width <= game->players[id].x + movements[vector][0] ||
        0 >  game->players[id].x + movements[vector][0] ||
        height <= game->players[id].y + movements[vector][1] ||
        0 > game->players[id].y + movements[vector][1] ||
        board[game->players[id].y + movements[vector][1]][game->players[id].x + movements[vector][0]] <= 0) &&
        (!game->players[id].blocked)
    )
    {
        rotateVector();
    }

    return vector;
}

void rotateVector(){
    ++vector;
    if(vector > 7) vector = 0;
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
