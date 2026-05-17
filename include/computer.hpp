#pragma once

#include "grid.hpp"

#include <iostream>

class Computer{
    public:
        uint16_t choose_move(const Grid& grid);
    protected:
        std::array<float, 7> simulate_game(Grid& grid, uint16_t depth);
        std::array<float, 7> simulate_game(Grid& grid, uint16_t depth, bool ai_turn);
};

uint16_t choose_max(std::array<float, 7> state_values);