#include <iostream>
#include <array>

class Grid{
    public:
        Grid();
        void show();
        bool update(u_int16_t cursor_pos, int16_t turn = 1); // turn 1 = player, turn -1 = computer

    private:
        static constexpr u_int16_t grid_width = 7;
        static constexpr u_int16_t grid_height = 6;
        std::array<std::array<int16_t, grid_width>, grid_height> grid;

        void draw_horizontal_line();
};