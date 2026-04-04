#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defaultValues.h"
#include "structs.h"
#include "shared.h"
#include "semaphores.h"
#include "utils.h"

Arguments arguments = {
    .width = WIDTH,
    .height = HEIGHT,
    .delay = DELAY, 
    .timeout = TIMEOUT,
    .view_path = VIEW,
    .num_players = 0,
    .players_paths = {NULL}
}; 

void initializeGame(Game * game, Arguments * arguments);

int main(int argc, char *argv[])
{
    initializeArgs(argc, argv, &arguments);
    
    unsigned long gameSize = sizeof(Game) + arguments.width * arguments.height * sizeof(char) - sizeof(char);
    Game *game = initializeShared("/game_state", gameSize);    
    if (game == NULL) return 1;
    initializeGame(game, &arguments);

    SyncState * sync = initializeShared("/game_sync", sizeof(SyncState));
    if (sync == NULL) return 1;

    char * width = intToStr(game->width);
    char * height = intToStr(game->height);

    //Inicializamos los semaforos
    initializeSemaphores(sync, game);
    
    //Inicializamos la vista 
    int view_pid;
    if (arguments.view_path != NULL) {
        if ((view_pid = fork()) == 0) {
            char * argsv[3] = { width, height, NULL };
            char * argse[1] = { NULL };
            execve(arguments.view_path, argsv, argse);
        }
    }

    //Configuración nesesaria para los pipes
    int fd[game->num_players][2];

    printf("num players: %d \n",  game->num_players );

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
            char *args[] = {arguments.players_paths[i], NULL};

            //LLamamos al programa de la dirección correspondiente

            execvp(args[0], args);
            perror("Un jugador genera un error"); 
            exit(1);
        }
        else{

            close(fd[i][1]);

            game->players[i].pid = player_pid;
            
        }

    }

    // Logica en cada tick
    
    // Limpieza
    if(arguments.view_path != NULL) kill(view_pid, SIGKILL);

    for (int i = 0; i < game->num_players; i++) {
        kill(game->players[i].pid, SIGKILL);
    }

    free(width);
    free(height);
    
    munmap(game, gameSize);
    munmap(sync, sizeof(SyncState));
    shm_unlink("/game_state");
    shm_unlink("/game_sync");


    //Se cierran los pipes


    for(int i = 0 ; i < game->num_players; i++){
        close(fd[i][0]);
    }


    

    printf("Terminado");

    return 0;
}

void initializeGame(Game * game, Arguments * arguments) {
    // Inicializar data del juego
    *game = (Game) {
        .width = arguments->width,
        .height = arguments->height,
        .num_players = arguments->num_players,
        .game_over = false,
    };

    // Inicializar data de players
    for (int i = 0; i < game->num_players; i++) {
        game->players[i] = (Player) {
            .name = "Player",
            .score = 0,
            .invalid_moves = 0,
            .valid_moves = 0,
        };
    }
    
    // Inicializar board

}