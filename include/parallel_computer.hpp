#pragma once

#include "computer.hpp"

#include <iostream>
#include <array>
#include <vector>
#include <cstdint>

class ParallelComputer : Computer {
    public:
        using Computer::choose_move; // import choose_move from Computer

        uint16_t choose_move(const Grid& grid, int rank, int split_depth = 2, int total_depth = 7);
        uint16_t choose_move_parallel_dynamic(const Grid& grid);
};
