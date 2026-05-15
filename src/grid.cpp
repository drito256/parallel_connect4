#include "../include/grid.hpp"

Grid::Grid(){

    for(int i = 0; i < this->grid_height; i++){
        for(int j = 0; j < this->grid_width; j++){
            this->grid[i][j] = 0;
        }
        grid[5][5] = 1;
    }
}

void Grid::draw_horizontal_line(){
    for(int i = 0; i < this->grid_width * 2 + 1; i++){
        std::cout << "--";
    }
    std::cout << "\n";
}

// draw grid
// lower left coordinates: (0,0)
// uuper right coordinates: (6,5)
void Grid::show(){

    this->draw_horizontal_line();

    for(int i = 0; i < this->grid_height; i++){
        for(int j = 0; j < this->grid_width; j++){
            int reverse_gh = this->grid_height - i - 1;

            if(j == 0)
                std::cout << "|";
            
            if(this->grid[reverse_gh][j] == 0){
                std::cout << "   " << "|";
            }
            else if(this->grid[reverse_gh][j] == 1){
                std::cout << " " << "o" << " " << "|";
            }
            else{
                std::cout <<  " " << "x" << " " << "|";
            }
        }
        std::cout << "\n";
        this->draw_horizontal_line();
    }
}

// return boolean which indicates whether column was already full
bool Grid::update(uint16_t cursor_pos, int16_t turn){
    for(int i = 0; i < 6; i++){
        if(grid[i][cursor_pos] == 0){
            grid[i][cursor_pos] = turn;
            return true;
        }
    }
    return false;
}

