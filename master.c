#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
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
    initializeArgs(argc, argv, &arguments, "+w:h:d:t:s:v:p:");

    srand(arguments.seed);
    
    unsigned long gameSize = sizeof(Game) + arguments.width * arguments.height * sizeof(char) - sizeof(char);
    Game *game = initializeShared(SHARED_GAME, gameSize);    
    if (game == NULL) return 1;
    initializeGame(game, &arguments);
    char (*board)[game->width] = (char (*)[game->width])game->board;

    SyncState * sync = initializeShared(SHARED_SYNC, sizeof(SyncState));
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
    shm_unlink(SHARED_GAME);
    shm_unlink(SHARED_SYNC);

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

    // Limpieza board
    char (*board)[game->width] = (char (*)[game->width])game->board;
    for (int i = 0; i < game->width; i++) {
        for (int j = 0; j < game->height; j++) {
            board[i][j] = 0;
        }
    }

    double centerX = game->width / 2.0;
    double centerY = game->height / 2.0;
    
    double radiusX = (game->width / 2.0) * 0.8;
    double radiusY = (game->height / 2.0) * 0.8;

    // Inicializar data de players
    for (int i = 0; i < game->num_players; i++) {
        game->players[i] = (Player) {
            .name = "Player",
            .score = 0,
            .invalid_moves = 0,
            .valid_moves = 0,
        };

        // Definir posicion con simetria central
        double angle = (2.0 * PI / game->num_players) * i - (PI / 2.0);
        int x = (int)(centerX + radiusX * cos(angle));
        int y = (int)(centerY + radiusY * sin(angle));
        
        // Corrección por si el redondeo los saca del tablero (índices 0 a W-1)
        if (x >= game->width) x = game->width - 1;
        if (y >= game->height) y = game->height - 1;
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        game->players[i].x = x;
        game->players[i].y = y;

        // Actualizar board con la posicion del jugador
        board[x][y] = i + '0';
    }
    
    // Inicializar board con valores
    for (int i = 0; i < game->width; i++) {
        for (int j = 0; j < game->height; j++) {
            if (board[i][j] == 0) {
                board[i][j] = randInt(1, 9);
            }
        }
    }
}