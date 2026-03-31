#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defaultValues.h"
#include "structs.h"
#include "shared.h"
#include "semaphores.h"


int delay = DELAY;
int timeout = TIMEOUT;
int seed;
char * view_path = VIEW;
char * players_paths[9] = {};

void initializeArgs(int argc, char *argv[], Game **game);
void exitError(const char * error);

int main(int argc, char *argv[])
{

    // Incializamos las memorias compartidas

    printf("Inizialización de memoria \n");

    Game * game = initializeShared("/game_state", sizeof(Game));
    if (game == NULL) return 1;

    SyncState * sync = initializeShared("/game_sync", sizeof(SyncState));
    if (sync == NULL) return 1;

    //Colocamos los datos iniciales
    
    initializeArgs(argc, argv, &game);

    printf("Memoria inizializada\n");

    //Inicializamos los semaforos

    initializeSemaphores(sync, game);

    // Inicializamos los players

    printf("%d\n", game->num_players);

    for(int i = 0 ; i < game->num_players ; i++){
        int player_pid = fork();
        if(player_pid == 0){

            printf("%s \n", players_paths[i]);
            char *args[] = {players_paths[i], NULL}; // Argumentos del programa
            execvp(args[0], args);

            perror("Un jugador genera un error");
            exit(1);
        }
    }

    wait(NULL);

    // Incializamos la vista, si hay

    // Logica en cada tick

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
                (*game)->width = atoi(optarg);
                if ((*game)->width < 10) exitError("El width debe ser mayor o igual a 10");
                break;
            case 'h':
                (*game)->height = atoi(optarg);
                if ((*game)->height < 10) exitError("El height debe ser mayor o igual a 10");
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
                    if (num_players > 9) exitError("No deben haber más de 9 jugadores");
                    players_paths[num_players++] = argv[optind++];
                }
                if (num_players == 0) exitError("Debe haber al menos un jugador");
                (*game)->num_players = num_players;
                break;
            default:
                fprintf(stderr, "Uso: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v view] -p player1 [player2...]\n", argv[0]);
                exit(1);
        }
    }

    // El tamaño es el struct + El tamaño del tablero y como estan solapados en el char *, resto su tamaño.
    unsigned long newSize = sizeof(Game) * (*game)->width * (*game)->height * sizeof(char) - sizeof(char *);
    munmap(*game, sizeof(Game));
    *game = initializeShared("/game_state", newSize);
}

void exitError(const char * error) {
    fprintf(stderr, error);
    exit(1);
}