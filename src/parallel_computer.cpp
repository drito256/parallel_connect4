#include "parallel_computer.hpp"
#include <cstdint>
#include <mpi.h>
#include <cmath>


uint16_t ParallelComputer::choose_move_parallel_dynamic(const Grid& grid) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    constexpr int WORK_TAG = 1;
    constexpr int STOP_TAG = 2;
    constexpr int RESULT_TAG = 3;

    const int total_depth = 8;
    const int remaining_depth = total_depth - 2;

    if (rank == 0) {
        std::vector<std::array<int, 2>> tasks;

        // generate 7 × 7 tasks: first ai_move -> human -> ai_move -> human and so on
        for (int ai_col = 0; ai_col < 7; ai_col++) {
            Grid after_ai = grid;

            if (!after_ai.update(ai_col, -1)) {
                continue;
            }

            int16_t ai_result = after_ai.check_state(ai_col);

            // if ai wins, no need to generate human responses
            if (ai_result == -1) {
                tasks.push_back({ai_col, -1});
                continue;
            }

            for (int human_col = 0; human_col < 7; human_col++) {
                Grid after_human = after_ai;

                if (!after_human.update(human_col, 1)) {
                    continue;
                }

                tasks.push_back({ai_col, human_col});
            }
        }

        std::array<float, 7> sums;
        std::array<int, 7> counts;

        sums.fill(0.0f);
        counts.fill(0);

        int next_task = 0;
        int active_workers = 0;

        // send initial work to workers
        for (int worker = 1; worker < size && next_task < static_cast<int>(tasks.size()); worker++) {
            MPI_Send(
                tasks[next_task].data(),
                2,
                MPI_INT,
                worker,
                WORK_TAG,
                MPI_COMM_WORLD
            );

            next_task++;
            active_workers++;
        }

        // receive results and assignt new work
        while (active_workers > 0) {
            float result_buffer[2];

            MPI_Status status;

            MPI_Recv(
                result_buffer,
                2,
                MPI_FLOAT,
                MPI_ANY_SOURCE,
                RESULT_TAG,
                MPI_COMM_WORLD,
                &status
            );

            int worker = status.MPI_SOURCE;
            int ai_col = static_cast<int>(result_buffer[0]);
            float value = result_buffer[1];

            if (!std::isnan(value)) {
                sums[ai_col] += value;
                counts[ai_col]++;
            }

            if (next_task < static_cast<int>(tasks.size())) {
                MPI_Send(
                    tasks[next_task].data(),
                    2,
                    MPI_INT,
                    worker,
                    WORK_TAG,
                    MPI_COMM_WORLD
                );

                next_task++;
            } else {
                MPI_Send(
                    nullptr,
                    0,
                    MPI_INT,
                    worker,
                    STOP_TAG,
                    MPI_COMM_WORLD
                );

                active_workers--;
            }
        }

        std::array<float, 7> state_values;
        state_values.fill(NAN);

        for (int col = 0; col < 7; col++) {
            if (counts[col] > 0) {
                state_values[col] = sums[col] / counts[col];
            }
        }

        uint16_t chosen_move = choose_max(state_values);

        // broadcast chosen move to all
        MPI_Bcast(&chosen_move, 1, MPI_UNSIGNED_SHORT, 0, MPI_COMM_WORLD);

        return chosen_move;
    }

    else {
        while (true) {
            int task_buffer[2];
            MPI_Status status;

            MPI_Recv(
                task_buffer,
                2,
                MPI_INT,
                0,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status
            );

            if (status.MPI_TAG == STOP_TAG) {
                break;
            }

            int ai_col = task_buffer[0];
            int human_col = task_buffer[1];

            float value = 0.0f;

            Grid temp = grid;

            bool valid = true;

            if (!temp.update(ai_col, -1)) {
                valid = false;
                value = NAN;
            }

            if (valid) {
                int16_t ai_result = temp.check_state(ai_col);

                if (ai_result == -1) {
                    value = 1.0f;
                } else if (human_col == -1) {
                    value = 1.0f;
                } else {
                    if (!temp.update(human_col, 1)) {
                        valid = false;
                        value = NAN;
                    }

                    if (valid) {
                        int16_t human_result = temp.check_state(human_col);

                        if (human_result == 1) {
                            value = -1.0f;
                        } else if (human_result == 2) {
                            value = 0.0f;
                        } else {
                            std::array<float, 7> sub_values =
                                simulate_game(temp, remaining_depth, true);

                            float sum = 0.0f;
                            int count = 0;

                            for (float v : sub_values) {
                                if (!std::isnan(v)) {
                                    sum += v;
                                    count++;
                                }
                            }

                            value = count > 0 ? sum / count : 0.0f;
                        }
                    }
                }
            }

            float result_buffer[2];
            result_buffer[0] = static_cast<float>(ai_col);
            result_buffer[1] = value;

            MPI_Send(
                result_buffer,
                2,
                MPI_FLOAT,
                0,
                RESULT_TAG,
                MPI_COMM_WORLD
            );
        }

        uint16_t chosen_move = 0;
        MPI_Bcast(&chosen_move, 1, MPI_UNSIGNED_SHORT, 0, MPI_COMM_WORLD);

        return chosen_move;
    }
}
