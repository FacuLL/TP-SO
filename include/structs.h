#ifndef STRUCTS_H
#define STRUCTS_H

    #include <stdbool.h>
    #include <sys/types.h>
    #include <semaphore.h>
    #include "defaultValues.h"

    typedef struct {
        char name[16]; // Nombre del jugador
        unsigned int score; // Puntaje
        unsigned int invalid_moves; // Cantidad de solicitudes de movimientos inválidas realizadas
        unsigned int valid_moves; // Cantidad de solicitudes de movimientos válidas realizadas
        unsigned short x, y; // Coordenadas x e y en el tablero
        pid_t pid; // Identificador de proceso
        bool blocked; // Indica si el jugador está bloqueado
    } Player;

    typedef struct {
        unsigned short width; // Ancho del tablero
        unsigned short height; // Alto del tablero
        unsigned char num_players; // Cantidad de jugadores
        Player players[MAX_PLAYERS]; // Lista de jugadores
        bool game_over; // Indica si el juego se ha terminado
        char board[]; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
    } Game;

    typedef struct {
        sem_t has_to_print; // El máster le indica a la vista que hay cambios por imprimir
        sem_t view_finished; // La vista le indica al máster que terminó de imprimir
        sem_t master_priority; // Mutex para evitar inanición del máster al acceder al estado
        sem_t can_access_game_state; // Mutex para el estado del juego
        sem_t can_access_readers_count; // Mutex para la siguiente variable 
        unsigned int readers_count; // Cantidad de jugadores leyendo el estado
        sem_t can_player_move[9]; // Le indican a cada jugador que puede enviar 1 movimiento
    } SyncState;

    typedef struct{
        int width;
        int height;
        int delay; 
        int timeout;
        int seed;
        char* view_path;
        int num_players;
        char * players_paths[9]; 
    } Arguments;

#endif