#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <semaphore.h>

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
        Player players[9]; // Lista de jugadores
        bool game_over; // Indica si el juego se ha terminado
        char *board; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
    } Game;

// El tablero se almacena en la memoria compartida justo después del campo game_over.
// No se usa offsetof(Game, board) porque el compilador agrega padding antes del puntero
// (alineación a 8 bytes en sistemas de 64 bits), lo que correría la lectura 7 bytes.
// Todos los procesos deben usar este macro para acceder al tablero.
#define BOARD_START(game) ((char *)(game) + offsetof(Game, game_over) + sizeof(bool))

    typedef struct {
        sem_t has_to_print; // El máster le indica a la vista que hay cambios por imprimir
        sem_t view_finished; // La vista le indica al máster que terminó de imprimir
        sem_t master_mutex; // Mutex para evitar inanición del máster al acceder al estado
        sem_t game_state_mutex; // Mutex para el estado del juego
        sem_t next_variable_mutex; // Mutex para la siguiente variable
        unsigned int readers_count; // Cantidad de jugadores leyendo el estado
        sem_t can_player_move[9]; // Le indican a cada jugador que puede enviar 1 movimiento
    } SyncState;

#endif