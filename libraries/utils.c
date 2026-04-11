#include "utils.h"

void exitError(const char * error) {
    fputs(error, stderr);
    exit(1);
}

char * intToStr(int num) {
    char * str = malloc((int)((ceil(log10(num))+1)*sizeof(char)));
    sprintf(str, "%d", num);
    return str;
}

int randInt(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

void initializeArgs(int argc, char *argv[], Arguments * arguments, char * argsRequired) {
    arguments->seed = SEED;

    int opt;

    while ((opt = getopt(argc, argv, argsRequired)) != -1) {
        switch (opt) {
            case 'w':
                arguments->width = atoi(optarg);
                if (arguments->width < 10) exitError("El width debe ser mayor o igual a 10");
                break;
            case 'h':
                arguments->height = atoi(optarg);
                if (arguments->height < 10) exitError("El height debe ser mayor o igual a 10");
                break;
            case 'd':
                arguments->delay = atoi(optarg);
                break;
            case 't':
                arguments->timeout = atoi(optarg);
                break;
            case 's':
                arguments->seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                arguments->view_path = optarg;
                break;
            case 'p': {
                int num_players = 0;

                if (optarg != NULL && optarg[0] != '-') {
                    if (num_players >= MAX_PLAYERS)
                        exitError("No deben haber más de 9 jugadores\n");
                    arguments->players_paths[num_players++] = optarg;
                }

                // Collect any additional players that follow
                while (optind < argc && argv[optind][0] != '-') {
                    if (num_players >= MAX_PLAYERS)
                        exitError("No deben haber más de 9 jugadores\n");
                    arguments->players_paths[num_players++] = argv[optind++];
                }

                if (num_players < MIN_PLAYERS)
                    exitError("Debe haber al menos un jugador después de -p\n");

                if(arguments->timeout < arguments->delay)
                    exitError("El timeout debe ser mayor al delay\n");

                arguments->num_players = (unsigned char)num_players;
                break;
            } default:
                fprintf(stderr, "Uso: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v view] -p player1 [player2...]\n", argv[0]);
                exit(1);
        }
    }
}

bool isPlayerBlocked(Game * game, int player_id) {
    char (*board)[game->width] = (char (*)[game->width])game->board;
    int px = game->players[player_id].x;  // columna
    int py = game->players[player_id].y;  // fila
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;
            int nx = px + dx;
            int ny = py + dy;
            if (nx < 0 || nx >= game->width || ny < 0 || ny >= game->height) continue;
            int value = board[ny][nx];
            if (value >= 1 && value <= 9) return false;
        }
    }
    return true;
}