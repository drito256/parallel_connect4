#include "../include/connect4.hpp"


void Connect4::play(){
   std::cout << "\033[3J\033[2J\033[H"; // clear screen
   cursor.show();
   grid.show();
   int16_t game_finished = 2; // 2 indicates that game is still being player, should later be turned to enum
   
   while(game_finished == 2){ 
    int button = 1;
    bool valid_move = false;
    // players turn
    while(!valid_move){
        int button = input.get();
        switch(button){
                case -1:
                    cursor.set_pos(std::max(cursor.get_pos() - 1, 0));
                    break;
                case 1:
                    cursor.set_pos(std::min(cursor.get_pos() + 1, 6));
                    break;
                case 0:
                    valid_move = grid.update(cursor.get_pos());
                    break;
        }

        std::cout << "\033[3J\033[2J\033[H"; // clear screen
        cursor.show();
        grid.show();
    }

    game_finished = grid.check_state(cursor.get_pos());
    if(game_finished != 2){ // don't let player have another turn
        break;
    }

    // computer turn
    //computer.make_move();
    //game_finished = grid.check_state(cursor.get_pos());
    }

    if(game_finished == 1){
        std::cout << "\n======================================================\n";
        std::cout << "Congrats! You won against my suuuuper smart AI brain!";
        std::cout << "\n======================================================\n";
    }
    else if(game_finished == -1){
        std::cout << "\n==========================\n";
        std::cout << "\nBetter luck next time bud!";
        std::cout << "\n==========================\n";
    }
    else{
        std::cout << "\n==========================================================\n";
        std::cout << "\nWho would have thought you are as smart as me! It's a draw!";
        std::cout << "\n==========================================================\n";
    }
}