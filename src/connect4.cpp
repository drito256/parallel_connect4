#include "../include/connect4.hpp"


void Connect4::play(){
   std::cout << "\033[3J\033[2J\033[H"; // clear screen
   cursor.show();
   grid.show();
    
   int button = 1;
   bool valid_move = false;
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
}