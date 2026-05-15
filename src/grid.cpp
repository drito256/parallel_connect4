#include "../include/grid.hpp"

Grid::Grid(){

    for(int i = 0; i < this->grid_height; i++){
        for(int j = 0; j < this->grid_width; j++){
            this->grid[i][j] = 0;
        }
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
int16_t Grid::check_state(uint16_t last_move_x_pos, uint16_t last_move_y_pos){
    uint16_t consecutive_counter = 0;
    int16_t turn = grid[last_move_y_pos][last_move_x_pos];

    // vertical check
    for(int i = 5; i >=0; i--){

        if(grid[i][last_move_x_pos] == turn){
            consecutive_counter++;
            if(consecutive_counter == 4){
                return turn;
            }
        }
        else{
            consecutive_counter = 0;
        }
    }
    if(consecutive_counter == 4){
        return turn;
    }
    consecutive_counter = 0;

    // horizontal check
    for(int i = 6; i >=0; i--){

        if(grid[last_move_y_pos][i] == turn){
            consecutive_counter++;
            if(consecutive_counter == 4){
                return turn;
            }
        }
        else{
            consecutive_counter = 0;
        }
    }
    if(consecutive_counter == 4){
        return turn;
    }
    consecutive_counter = 0;

    // diagonal 1
    int start_x = last_move_x_pos;
    int start_y = last_move_y_pos;
    while(start_x > 0 && start_y > 0){
        start_x--;
        start_y--;
    }
    while(start_x < grid_width && start_y < grid_height){
        if(grid[start_y][start_x] == turn){
            consecutive_counter++;
        }
        else{
            consecutive_counter = 0;
        }
        start_x++;
        start_y++;
    }
    if(consecutive_counter == 4){
        return turn;
    }
    consecutive_counter = 0;


    // diagonal 2
    start_x = last_move_x_pos;
    start_y = last_move_y_pos;
    while(start_x < grid_width && start_y > 0){
        start_x++;
        start_y--;
    }
    while(start_x > 0  && start_y < grid_height){
        if(grid[start_y][start_x] == turn){
            consecutive_counter++;
        }
        else{
            consecutive_counter = 0;
        }
        start_x--;
        start_y++;
    }

    if(consecutive_counter == 4){
        return turn;
    }
    consecutive_counter = 0;
    
    // check if grid is full
    uint16_t last_row_coins = 0;
    for(int j = 0; j < grid_width; j++){ // its enough to check the last row
        if(grid[5][j] != 0){
            last_row_coins++;
        }
        else{
            break;
        }
    }
    if(last_row_coins == 7){ // if all the coins have been placed, its a draw
        return 0;
    }

    return 2; // keep playing

}

int16_t Grid::check_state(uint16_t last_move_x_pos){
    uint16_t last_move_y_pos = 0;

    for(int i = 0; i < 6; i++){
        if(grid[i][last_move_x_pos] != 0){
            last_move_y_pos = i;
        }
    }
    int16_t status = check_state(last_move_x_pos, last_move_y_pos);
    return status;
}

