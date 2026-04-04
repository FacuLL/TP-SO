#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defaultValues.h"
#include "structs.h"
#include "shared.h"
#include "semaphores.h"
#include "utils.h"


int delay = DELAY;
int timeout = TIMEOUT;
int seed;
char * view_path = VIEW;
pid_t view_pid;
char * players_paths[9] = {};

void initializeArgs(int argc, char *argv[], Game *game);
void exitError(const char * error);

int main(int argc, char *argv[])
{

    // Incializamos las memorias compartidas

    Game * game = initializeShared("/game_state", sizeof(Game));
    if (game == NULL) return 1;

    SyncState * sync = initializeShared("/game_sync", sizeof(SyncState));
    if (sync == NULL) return 1;
    
    initializeArgs(argc, argv, &game);

    // El tamaño es el struct + El tamaño del tablero y como estan solapados en el char *, resto su tamaño.
    unsigned long newSize = sizeof(Game) * game->width * game->height * sizeof(char) - sizeof(char *);
    munmap(game, sizeof(Game));
    game = initializeShared("/game_state", newSize);

    char * width = intToStr(game->width);
    char * height = intToStr(game->height);

    //Inicializamos los semaforos

    initializeSemaphores(sync, game);


    
    //Inicializamos la vista 

    if (view_path != NULL) {
        if ((view_pid = fork()) == 0) {
            char * argsv[3] = { width, height, NULL };
            char * argse[1] = { NULL };
            execve(view_path, argsv, argse);
        }
    }

    //Configuración nesesaria para los pipes

    int fd[game->num_players][2];


    for(int i = 0 ; i < game->num_players ; i++){
        
        //Inicialización de pipes
        if(pipe(fd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        
        //Creo nuevo proceso hijo
        int player_pid = fork();

        if(player_pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }

        //Proceso hijo

        if(player_pid == 0){

            //Se redirige el pipe 

            close(fd[i][0]);
            dup2(fd[i][1], STDOUT_FILENO);
            close(fd[i][1]);

            //NO SACAR EL NULL, *args debe terminar en el. Se elimino momentaneamente y causo MUCHOS problemas
            char *args[] = {players_paths[i], NULL};

            //LLamamos al programa de la dirección correspondiente

            execvp(args[0], args);
            perror("Un jugador genera un error"); 
            exit(1);
        }
        else{

            close(fd[i][1]);

            game->players[i].pid = player_pid;

            char buffer[100];
            int n = read(fd[i][0], buffer, sizeof(buffer) - 1);
            
            buffer[n] = '\0';
            
            printf("Padre recibió: %s de longitud %d del hijo %d de %d \n", buffer , n, i, game->num_players);

        }
    }

    // Logica en cada tick

    
    // Limpieza

    kill(view_pid, SIGKILL);
    for (int i = 0; i < game->num_players; i++) {
        kill(game->players[i].pid, SIGKILL);
    }

    free(width);
    free(height);
    
    munmap(game, newSize);
    munmap(sync, sizeof(SyncState));


    //Se cierran los pipes


    for(int i = 0 ; i < game->num_players; i++){
        close(fd[i][0]);
    }

    shm_unlink("/game_state");
    shm_unlink("/game_sync");


    return 0;
}

void initializeArgs(int argc, char *argv[], Game **game){
    (*game)->width = WIDTH;
    (*game)->height = HEIGHT;
    (*game)->game_over = false;
    seed = SEED;   

    int opt;

    while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p")) != -1) {
        switch (opt) {
            case 'w':
                game->width = atoi(optarg);
                if (game->width < 10) exitError("El width debe ser mayor o igual a 10");
                break;
            case 'h':
                game->height = atoi(optarg);
                if (game->height < 10) exitError("El height debe ser mayor o igual a 10");
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 's':
                seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                view_path = optarg;
                break;
            case 'p':
                int num_players = 0;
                while (optind < argc && argv[optind][0] != '-') {
                    if (num_players >= MAX_PLAYERS) exitError("No deben haber más de 9 jugadores");
                    players_paths[num_players++] = argv[optind++];
                }
                if (num_players < MIN_PLAYERS ) exitError("Debe haber al menos un jugador");
                (*game)->num_players = num_players;
                break;
            default:
                fprintf(stderr, "Uso: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v view] -p player1 [player2...]\n", argv[0]);
                exit(1);
        }
    }
}