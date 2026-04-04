#include "utils.h"

void exitError(const char * error) {
    fprintf(stderr, error);
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
    for (int i = -1; i < 1; i++) {
        for (int j = -1; j < 1; j++) {
            if (i != 0 || j != 0) {
                int x = game->players[player_id].x;
                int y = game->players[player_id].y;
                int value = board[x+i][y+j];
                if (value > 0 && value < 10) return false;
            }
        }
    }
    return true;
}