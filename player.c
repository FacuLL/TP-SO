#include "shared.h"


int main(int argc, char const *argv[]){
    
    Game * game = initializeShared("/game_state", sizeof(Game));
    
    if (game == NULL) return 1;

    printf("%d \n",game->num_players);

    return 0;
}
