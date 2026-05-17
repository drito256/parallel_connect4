#include <iostream>
#include <array>

class Grid{
    public:
        Grid();
        void show() const;
        bool update(uint16_t cursor_pos, int16_t turn = 1); // turn 1 = player, turn -1 = computer
        int16_t check_state(uint16_t last_move_x_pos) const; // 1 = player won, -1 = computer won, 0 = draw, 2 = still playing
        uint16_t get_width() const;

    private:
        static constexpr uint16_t width = 7;
        static constexpr uint16_t height = 6;
        std::array<std::array<int16_t, width>,height> grid;

        void draw_horizontal_line() const;
        int16_t check_state(uint16_t last_move_x_pos, uint16_t last_move_y_pos) const;
};