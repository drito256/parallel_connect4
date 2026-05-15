#include <iostream>
#include <array>

class Grid{
    public:
        Grid();
        void show();
        bool update(u_int16_t cursor_pos, int16_t turn = 1); // turn 1 = player, turn -1 = computer
        int16_t check_state(uint16_t last_move_x_pos); // 1 = player won, -1 = computer won, 0 = draw, 2 = still playing

    private:
        static constexpr u_int16_t grid_width = 7;
        static constexpr u_int16_t grid_height = 6;
        std::array<std::array<int16_t, grid_width>, grid_height> grid;

        void draw_horizontal_line();
        int16_t check_state(uint16_t last_move_x_pos, uint16_t last_move_y_pos);
};