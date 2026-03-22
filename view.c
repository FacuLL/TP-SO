#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ncurses.h>
#include "include/structs.h"

// Nombres de las memorias compartidas, definidos por el máster
#define GAME_STATE_SHM "/game_state"
#define GAME_SYNC_SHM  "/game_sync"

// IDs de los pares de colores para ncurses
// Las celdas libres usan el par 1; cada jugador usa el par 2+índice
#define COLOR_CELDA_LIBRE  1
#define COLOR_BASE_JUGADOR 2

// Dibuja el tablero celda por celda.
// Cada celda contiene un valor entre -8 y 9:
//   - Si es positivo (1-9): la celda está libre y el valor es su recompensa
//   - Si es negativo o cero: la celda fue capturada por el jugador con ese índice (-valor)
static void dibujar_tablero(Game *game, int ancho, int alto) {
    char *tablero = BOARD_START(game); // viene definido en structs.h junto al struct Game

    for (int fila = 0; fila < alto; fila++) {
        move(fila + 2, 0);  // dejamos 2 filas arriba para el título
        for (int col = 0; col < ancho; col++) {
            signed char celda = (signed char)tablero[fila * ancho + col];

            if (celda > 0) {
                // Celda libre: mostramos su recompensa en blanco
                attron(COLOR_PAIR(COLOR_CELDA_LIBRE));
                printw("%d ", celda);
                attroff(COLOR_PAIR(COLOR_CELDA_LIBRE));
            } else {
                // Celda capturada: mostramos la letra del jugador (A, B, C, ...)
                int idx_jugador = (int)(-celda);
                attron(COLOR_PAIR(COLOR_BASE_JUGADOR + idx_jugador) | A_BOLD);
                printw("%c ", 'A' + idx_jugador);
                attroff(COLOR_PAIR(COLOR_BASE_JUGADOR + idx_jugador) | A_BOLD);
            }
        }
    }
}

// Dibuja la tabla de estadísticas de cada jugador debajo del tablero.
// Muestra: letra asignada, nombre, puntaje, movimientos válidos e inválidos,
// posición actual en el tablero y si está bloqueado o no.
static void dibujar_jugadores(Game *game, int alto_tablero) {
    int fila = alto_tablero + 3;  // dejamos espacio entre el tablero y la tabla

    // Encabezado de la tabla
    mvprintw(fila,     0, " #  %-15s  %6s  %5s  %5s  %6s  %s",
             "Name", "Score", "Valid", "Invld", "Pos", "Status");
    mvprintw(fila + 1, 0, "---  %-15s  %6s  %5s  %5s  %6s  %s",
             "---------------", "------", "-----", "-----", "------", "------");

    // Una fila por jugador, con el color que le corresponde
    for (int i = 0; i < game->num_players; i++) {
        Player *p = &game->players[i];

        // Si el jugador está bloqueado lo mostramos tenue, si no, en negrita
        attron(COLOR_PAIR(COLOR_BASE_JUGADOR + i) | (p->blocked ? A_DIM : A_BOLD));
        mvprintw(fila + 2 + i, 0,
                 "[%c] %-15s  %6u  %5u  %5u  %2u,%-2u  %s",
                 'A' + i, p->name, p->score,
                 p->valid_moves, p->invalid_moves,
                 p->x, p->y,
                 p->blocked ? "BLOCKED" : "active");
        attroff(COLOR_PAIR(COLOR_BASE_JUGADOR + i) | (p->blocked ? A_DIM : A_BOLD));
    }
}

// Limpia la pantalla y redibuja el estado completo del juego:
// título, tablero, estadísticas de jugadores y estado de la partida.
static void mostrar_estado(Game *game, int ancho, int alto) {
    clear();

    mvprintw(0, 0, "=== ChompChamps ===  Board: %dx%d  Players: %d",
             ancho, alto, game->num_players);

    dibujar_tablero(game, ancho, alto);
    dibujar_jugadores(game, alto);

    // Línea de estado al final de todo
    int fila_estado = alto + 3 + 2 + game->num_players + 1;
    if (game->game_over) {
        attron(A_BOLD);
        mvprintw(fila_estado, 0, "=== GAME OVER ===");
        attroff(A_BOLD);
    } else {
        mvprintw(fila_estado, 0, "Running...");
    }

    refresh();  // volcamos el buffer a la pantalla
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <ancho> <alto>\n", argv[0]);
        return 1;
    }

    int ancho = atoi(argv[1]);
    int alto  = atoi(argv[2]);

    // El tamaño de la memoria compartida del estado es el struct Game
    // (sin contar el puntero al tablero, que no ocupa espacio real) más las celdas del tablero
    size_t tam_estado = sizeof(Game) - sizeof(char *) + (size_t)ancho * alto;

    // Abrimos la memoria compartida del estado del juego (solo lectura)
    int fd_estado = shm_open(GAME_STATE_SHM, O_RDONLY, 0);
    if (fd_estado == -1) { perror("shm_open estado"); return 1; }
    Game *game = mmap(NULL, tam_estado, PROT_READ, MAP_SHARED, fd_estado, 0);
    close(fd_estado);
    if (game == MAP_FAILED) { perror("mmap estado"); return 1; }

    // Abrimos la memoria compartida de sincronización (lectura y escritura para los semáforos)
    int fd_sync = shm_open(GAME_SYNC_SHM, O_RDWR, 0);
    if (fd_sync == -1) {
        perror("shm_open sincronizacion");
        munmap(game, tam_estado);
        return 1;
    }
    SyncState *sync = mmap(NULL, sizeof(SyncState), PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd_sync, 0);
    close(fd_sync);
    if (sync == MAP_FAILED) {
        perror("mmap sincronizacion");
        munmap(game, tam_estado);
        return 1;
    }

    // Abrimos el terminal directamente con /dev/tty.
    // El máster puede haber redirigido stdin/stdout para comunicarse con los jugadores,
    // así que no podemos asumir que apuntan al terminal real.
    FILE *tty = fopen("/dev/tty", "r+");
    if (tty == NULL) { perror("fopen /dev/tty"); return 1; }

    // Iniciamos ncurses sobre ese terminal
    SCREEN *pantalla = newterm(getenv("TERM") ? getenv("TERM") : "xterm-256color", tty, tty);
    if (pantalla == NULL) { fprintf(stderr, "newterm falló\n"); return 1; }
    set_term(pantalla);
    noecho();    // no mostramos las teclas que se presionen
    cbreak();    // entregamos los caracteres de a uno sin esperar Enter
    curs_set(0); // ocultamos el cursor

    // Configuramos un color distinto para celdas libres y para cada jugador (hasta 9)
    if (has_colors()) {
        start_color();
        init_pair(COLOR_CELDA_LIBRE, COLOR_WHITE, COLOR_BLACK);
        short colores_jugadores[] = {
            COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
            COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE, COLOR_RED, COLOR_GREEN
        };
        for (int i = 0; i < 9; i++)
            init_pair(COLOR_BASE_JUGADOR + i, colores_jugadores[i], COLOR_BLACK);
    }

    // Ciclo principal de la vista:
    // 1. Esperamos a que el máster nos avise que hay cambios para mostrar
    // 2. Dibujamos el estado actual
    // 3. Le avisamos al máster que terminamos de dibujar
    // 4. Repetimos hasta que el juego termine
    while (1) {
        sem_wait(&sync->has_to_print);   // esperamos señal del máster
        bool termino = game->game_over;
        mostrar_estado(game, ancho, alto);
        sem_post(&sync->view_finished);  // avisamos que terminamos de imprimir
        if (termino) break;
    }

    napms(2000);  // dejamos el estado final visible 2 segundos antes de cerrar
    endwin();
    delscreen(pantalla);
    fclose(tty);

    munmap(game, tam_estado);
    munmap(sync, sizeof(SyncState));
    return 0;
}
