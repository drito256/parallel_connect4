#include "../include/computer.hpp"
#include <cmath>

uint16_t choose_max(std::array<float, 7> state_values) {
    float max_value = -std::numeric_limits<float>::infinity();
    int max_index = 0;

    for (int i = 0; i < 7; i++) {
        if (!std::isnan(state_values[i]) && state_values[i] > max_value) {
            max_value = state_values[i];
            max_index = i;
        }
    }
    return max_index;
}

uint16_t Computer::choose_move(const Grid& grid){
    Grid temp = grid;
    std::array<float, 7> state_values = simulate_game(temp, 9);

    return choose_max(state_values);
}

std::array<float, 7> Computer::simulate_game(Grid& grid, uint16_t depth) {
    return simulate_game(grid, depth, true);
}

std::array<float, 7> Computer::simulate_game(Grid& grid, uint16_t depth, bool ai_turn) {
    std::array<float, 7> state_values;
    int16_t turn = ai_turn ? -1 : 1;

    for (int col = 0; col < 7; col++) {
        Grid temp = grid;

        if (!temp.update(col, turn)) {
            state_values[col] = NAN; // full column, skip
            continue;
        }

        int16_t result = temp.check_state(col);

        if (result == -1) {
            state_values[col] = 1.0f;   // ai wins
        } else if (result == 1) {
            state_values[col] = -1.0f;  // ai loses
        } else if (result == 2) {
            state_values[col] = 0.0f;   // draw, neutral
        } else if (depth == 0) {
            state_values[col] = 0.0f;   // depth exhausted, neutral
        } else {
            std::array<float, 7> sub_values = simulate_game(temp, depth - 1, !ai_turn);

            float sum = 0.0f;
            int count = 0;
            for (int j = 0; j < 7; j++) {
                if (!std::isnan(sub_values[j])) {
                    sum += sub_values[j];
                    count++;
                }
            }
            state_values[col] = (count > 0) ? sum / count : 0.0f;
        }
    }
    return state_values;
}