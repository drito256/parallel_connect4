#include "../include/cursor.hpp"


Cursor::Cursor(){
    this->pos = 3;
}

void Cursor::show(){
    for(int i = 0; i < this->pos; i++){
        std::cout << "    ";
    }

    std::cout << "  |  ";
    std::cout << "\n";
}

void Cursor::set_pos(u_int16_t pos){
    this->pos = pos;
}

u_int16_t Cursor::get_pos(){
    return pos;
}