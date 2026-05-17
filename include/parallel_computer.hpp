#pragma once

#include "computer.hpp"

#include <iostream>
#include <array>
#include <vector>
#include <cstdint>

class ParallelComputer : Computer {
    public:
    // All ranks must call this together.
    // split_depth: 0 -> 7 tasks, 1 -> 49 tasks, 2 -> 343 tasks
    // Rank 0 returns best column (0-6), others return 0.
    
        using Computer::choose_move;
        uint16_t choose_move(const Grid& grid, int rank, int split_depth = 2, int total_depth = 7);
        uint16_t choose_move_parallel_dynamic(const Grid& grid);


    private:
        // Recursive simulate — same logic as Computer, reused by workers
        //std::array<float, 7> simulate_game(Grid& grid, int depth, bool ai_turn);

        // Manager: pre-expands tree, distributes tasks, aggregates results
        //uint16_t manager_loop(const Grid& grid, int split_depth, int total_depth);

        // Worker: receives and processes tasks until termination signal
        //void worker_loop();
};
