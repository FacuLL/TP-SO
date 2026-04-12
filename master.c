// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>
#include <math.h>
#include "defaultValues.h"
#include "structs.h"
#include "shared.h"
#include "semaphores.h"
#include "utils.h"
#define _USE_MATH_DEFINES

#define FOR_EACH_PLAYER(game, idx) for (int idx = 0; idx < (game)->num_players; idx++)
#define ACCESS_GAME_STATE(code) \
    do { \
        sem_wait(&sync->master_priority); \
        sem_wait(&sync->can_access_game_state); \
        sem_post(&sync->master_priority); \
        code; \
        sem_post(&sync->can_access_game_state); \
    } while (0); 

int movements[8][2] = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

Arguments arguments = {
    .width = WIDTH,
    .height = HEIGHT,
    .delay = DELAY, 
    .timeout = TIMEOUT,
    .view_path = VIEW,
    .num_players = 0,
    .players_paths = {NULL}
}; 

void printWelcome(Arguments arguments, Game * game);

void initializeGame(Game * game, Arguments * arguments);

void freeFds(Game * game, int fds[][2]);

void freeAll(Game * game, SyncState * sync, char * height, char * width, int fds[][2]);

int main(int argc, char *argv[])
{

    initializeArgs(argc, argv, &arguments, "+w:h:d:t:s:v:p");
    if (arguments.num_players < 1) {
        exitError("Debe seleccionarse al menos 1 jugador con -p\n");
    }

    srand(arguments.seed);
    
    unsigned long gameSize = sizeof(Game) + arguments.width * arguments.height * sizeof(char);
    Game *game = initializeShared(SHARED_GAME, gameSize);
    if (game == NULL) {
        freeAll(game, NULL, NULL, NULL, NULL);
        perror("Error al inicializar memoria compartida del estado del juego");
        exit(EXIT_FAILURE);
    }
    initializeGame(game, &arguments);
    char (*board)[game->width] = (char (*)[game->width])game->board;

    SyncState * sync = initializeShared(SHARED_SYNC, sizeof(SyncState));
    if (sync == NULL) {
        freeAll(game, sync, NULL, NULL, NULL);
        perror("Error al inicializar memoria compartida de sincronización");
        exit(EXIT_FAILURE);
    }

    char * width = intToStr(game->width);
    char * height = intToStr(game->height);

    //Inicializamos los semaforos
    initializeSemaphores(sync, game);

    printWelcome(arguments, game);

    //Configuración nesesaria para los pipes
    int fd[game->num_players][2];
    for (int i = 0; i < game->num_players; i++) {
        fd[i][0] = -1;
        fd[i][1] = -1;
    }

    for(int i = 0 ; i < game->num_players ; i++){
        //Inicialización de pipes
        if(pipe(fd[i]) == -1) {
            freeAll(game, sync, height, width, fd);
            perror("Error al inicializar pipes");
            exit(EXIT_FAILURE);
        }
        
        //Creo nuevo proceso hijo
        int player_pid = fork();

        if(player_pid == -1){
            perror("Error en fork");
            freeAll(game, sync, height, width, fd);
            exit(EXIT_FAILURE);
        } else if (player_pid == 0){
            //Se redirige el pipe 
            dup2(fd[i][1], STDOUT_FILENO);

            freeFds(game, fd);

            char *args[] = {arguments.players_paths[i], width, height, NULL};

            //Llamamos al programa de la dirección correspondiente solo cuando ya esta listo el estado (sobre todo los pids)
            sem_wait(&sync->can_player_move[i]);
            sem_post(&sync->can_player_move[i]);

            execvp(args[0], args);
            perror("Un jugador genera un error");
            exit(EXIT_FAILURE);
        } else{
            game->players[i].pid = player_pid;

            close(fd[i][1]);
            fd[i][1] = -1;
        }
    }

    //Inicializamos la vista 
    int view_pid;
    if (arguments.view_path != NULL) {
        if ((view_pid = fork()) == 0) {
            freeFds(game, fd);
            char * argsv[] = { arguments.view_path, width, height, NULL };
            execvp(arguments.view_path, argsv);
            perror("La vista genera error");
            exit(EXIT_FAILURE);
        }
    }
    
    FOR_EACH_PLAYER(game, i) {
        sem_post(&sync->can_player_move[i]);
    }

    fd_set set;

    int last_player_served = -1;

    // Logica en cada tick
    while (!game->game_over) {
        // Escuchamos acciones
        int max_fd = -1;
        FD_ZERO(&set);
        FOR_EACH_PLAYER(game, i) {
            if (!game->players[i].blocked) {
                FD_SET(fd[i][0], &set);
                if (fd[i][0] > max_fd) {
                    max_fd = fd[i][0];
                }
            }
        }
        struct timeval timeout = {.tv_sec = arguments.timeout};
        int ret = select(max_fd + 1, &set, NULL, NULL, &timeout);
        if (ret == 0) {
            sem_wait(&sync->master_priority);
            sem_wait(&sync->can_access_game_state);
            sem_post(&sync->master_priority);
                game->game_over = true;
            sem_post(&sync->can_access_game_state);
        } else if (ret < 0) {
            if (errno != EINTR) {
                perror("Error en select");
                sem_wait(&sync->master_priority);
                sem_wait(&sync->can_access_game_state);
                sem_post(&sync->master_priority);
                    game->game_over = true;
                sem_post(&sync->can_access_game_state);
            }
        } else {
            int start_index = (last_player_served + 1) % game->num_players;

            for (int i = 0; i < game->num_players; i++) {
                // Calculamos el índice actual de forma circular
                int player = (start_index + i) % game->num_players;
                if (FD_ISSET(fd[player][0], &set)) {
                    unsigned char move;
                    ssize_t bytes = read(fd[player][0], &move, sizeof(move));
                    if (bytes > 0) {
                        // 1. Validar:
                        if (move < 8) {
                            sem_wait(&sync->master_priority);
                            sem_wait(&sync->can_access_game_state);
                            sem_post(&sync->master_priority);

                            int x = game->players[player].x;
                            int y = game->players[player].y;
                            int nextX = x + movements[move][0];
                            int nextY = y + movements[move][1];

                            bool inBounds = (nextX >= 0 && nextX < game->width && nextY >= 0 && nextY < game->height);

                            if (inBounds && board[nextY][nextX] >= 1 && board[nextY][nextX] <= 9) {
                                int value = board[nextY][nextX];

                                // 2. Procesar movimiento
                                game->players[player].score += value;
                                game->players[player].valid_moves++;
                                game->players[player].x = nextX;
                                game->players[player].y = nextY;

                                // Marcamos la celda con el ID del jugador (valor negativo)
                                board[nextY][nextX] = (char)(-player);

                                FOR_EACH_PLAYER(game, j) {
                                    if (!game->players[j].blocked) {
                                        game->players[j].blocked = isPlayerBlocked(game, j);
                                    }
                                }

                                sem_post(&sync->can_access_game_state);
                                

                                sem_post(&sync->can_player_move[player]);
                                
                                // 3. Notificar y imprimir
                                if (arguments.view_path != NULL) {
                                    sem_post(&sync->has_to_print);
                                    sem_wait(&sync->view_finished);
                                    struct timespec delay = {.tv_nsec = arguments.delay * 1000000};
                                    nanosleep(&delay, &delay);
                                }
                                
                            } else {
                                game->players[player].invalid_moves++;
                                
                                sem_post(&sync->can_access_game_state);
                                
                                sem_post(&sync->can_player_move[player]); 
                            }
                        } else {
                            sem_wait(&sync->master_priority);
                            sem_wait(&sync->can_access_game_state);
                            sem_post(&sync->master_priority);
                                game->players[player].invalid_moves++;
                            sem_post(&sync->can_access_game_state);
                            
                            sem_post(&sync->can_player_move[player]); 
                        }
                    } else if (bytes == 0) {
                        // EOF: El jugador se cerró o bloqueó
                        sem_wait(&sync->master_priority);
                        sem_wait(&sync->can_access_game_state);
                        sem_post(&sync->master_priority);
                            game->players[player].blocked = true;
                        sem_post(&sync->can_access_game_state);
                    }
                }
                last_player_served++;
            }
        }

        // Checkea si ya debe terminar el juego
        bool all_blocked = true;
        FOR_EACH_PLAYER(game, i) {
            if (!game->players[i].blocked) all_blocked = false;
        }
        if (all_blocked) {
            sem_wait(&sync->master_priority);
            sem_wait(&sync->can_access_game_state);
            sem_post(&sync->master_priority);
                game->game_over = true;
            sem_post(&sync->can_access_game_state);
        }
    }

    // Limpieza
    int status;

    if (arguments.view_path != NULL) {
        sem_post(&sync->has_to_print);
        sem_wait(&sync->view_finished);
        waitpid(view_pid, &status, 0);
        printf("View exited (%d)\n", status);
    }

    FOR_EACH_PLAYER(game, i) {
        waitpid(game->players[i].pid, &status, 0);
        printf("Player %s (%d) exited (%d) with a score of %u / %u / %u\n", arguments.players_paths[i], i, status, game->players[i].score, game->players[i].valid_moves, game->players[i].invalid_moves);
    }

    freeAll(game, sync, height, width, fd);

    printf("Terminado");

    return 0;
}

void printWelcome(Arguments arguments, Game * game) {
    system("clear");

    printf(
        "width: %d\n"
        "height: %d\n"
        "delay: %d ms\n"
        "timeout: %d s\n"
        "seed: %d\n"
        "view: %s\n"
        "num_players: %d\n",
        arguments.width, 
        arguments.height, 
        arguments.delay, 
        arguments.timeout,
        arguments.seed,
        arguments.view_path == NULL ? "-" : arguments.view_path,
        arguments.num_players
    );
    FOR_EACH_PLAYER(game, i) {
        printf("\t%s\n", arguments.players_paths[i]);
    }

    sleep(2);
    system("clear");
}

void initializeGame(Game * game, Arguments * arguments) {
    // Inicializar data del juego
    *game = (Game) {
        .width = arguments->width,
        .height = arguments->height,
        .num_players = arguments->num_players,
        .game_over = false,
    };

    char (*board)[game->width] = (char (*)[game->width])game->board;

    // Llenar el tablero con recompensas primero
    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            board[i][j] = randInt(1, 9);
        }
    }

    double centerX = game->width / 2.0;
    double centerY = game->height / 2.0;

    double radiusX = (game->width / 2.0) * 0.8;
    double radiusY = (game->height / 2.0) * 0.8;

    // Inicializar data de players y colocarlos en el tablero encima de las recompensas
    for (int i = 0; i < game->num_players; i++) {
        game->players[i] = (Player) {
            .name = "Player",
            .score = 0,
            .invalid_moves = 0,
            .valid_moves = 0,
        };

        // Definir posicion con simetria central
        double angle = (2.0 * M_PI / game->num_players) * i - (M_PI / 2.0);
        int x = (int)(centerX + radiusX * cos(angle));
        int y = (int)(centerY + radiusY * sin(angle));

        // Corrección por si el redondeo los saca del tablero (índices 0 a W-1)
        if (x >= game->width) x = game->width - 1;
        if (y >= game->height) y = game->height - 1;
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        game->players[i].x = x;
        game->players[i].y = y;

        // Marcar celda con el ID del jugador (valores <= 0 indican jugador, -i para jugador i)
        board[y][x] = (char)(-i);
    }
}

void freeFds(Game * game, int fds[][2]) {
    if (fds != NULL && game != NULL) {
        for (int i = 0; i < game->num_players; i++) {
            for (int j = 0; j <= 1; j++) {
                if (fds[i][j] != -1) {
                    close(fds[i][j]);
                    fds[i][j] = -1;
                }
            }
        }
    }
}

void freeAll(Game * game, SyncState * sync, char * height, char * width, int fds[][2]){
    freeFds(game, fds);

    if (width != NULL) {
        free(width);
    }

    if (height != NULL) {
        free(height);
    }

    if (game != NULL) {
        unsigned long gameSize = sizeof(Game) + arguments.width * arguments.height * sizeof(char);
        munmap(game, gameSize);
        shm_unlink(SHARED_GAME);
    }

    if (sync != NULL) {
        munmap(sync, sizeof(SyncState));
        shm_unlink(SHARED_SYNC);
    }
}