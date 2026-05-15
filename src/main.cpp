#include "../include/connect4.hpp"

int main(){
    std::cout << "\033[3J\033[2J\033[H"; // clear screen

    Connect4 game;
    while(true){
        game.play();
    }

    return 0;
}