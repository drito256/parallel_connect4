#include "parallel_computer.hpp"
#include <cstdint>
#include <mpi.h>
#include <queue>
#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
// MPI tags
// ---------------------------------------------------------------------------
/*static constexpr int TAG_TASK      = 1;
static constexpr int TAG_RESULT    = 2;
static constexpr int TAG_TERMINATE = 3;*/

// ---------------------------------------------------------------------------
// Task descriptor sent from manager to worker
// ---------------------------------------------------------------------------
/*struct Task {
    Grid  grid;
    int   task_id;
    int   remaining_depth;  // -1 = terminal node, no recursion needed
    bool  ai_turn;
};

// Result sent back from worker to manager
struct Result {
    int   task_id;
    float value;
};*/

// ---------------------------------------------------------------------------
// ParallelComputer::simulate_game
// ---------------------------------------------------------------------------
/*std::array<float, 7> ParallelComputer::simulate_game(Grid& grid, int depth, bool ai_turn) {
    std::array<float, 7> state_values;
    int16_t turn = ai_turn ? -1 : 1;

    for (int col = 0; col < 7; col++) {
        Grid temp = grid;

        if (!temp.update(col, turn)) {
            state_values[col] = NAN;   // full column — skip
            continue;
        }

        int16_t result = temp.check_state(col);

        if (result == -1) {
            state_values[col] = 1.0f;  // AI wins
        } else if (result == 1) {
            state_values[col] = -1.0f; // AI loses
        } else if (result == 2) {
            state_values[col] = 0.0f;  // draw — neutral
        } else if (depth == 0) {
            state_values[col] = 0.0f;  // depth limit — neutral
        } else {
            std::array<float, 7> sub = simulate_game(temp, depth - 1, !ai_turn);
            float sum = 0.0f;
            int count = 0;
            for (int j = 0; j < 7; j++) {
                if (!std::isnan(sub[j])) { sum += sub[j]; count++; }
            }
            state_values[col] = (count > 0) ? sum / count : 0.0f;
        }
    }
    return state_values;
}
    */

// ---------------------------------------------------------------------------
// ParallelComputer::worker_loop
// ---------------------------------------------------------------------------
/*void ParallelComputer::worker_loop() {
    MPI_Status status;

    while (true) {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_TERMINATE) {
            MPI_Recv(nullptr, 0, MPI_BYTE, 0, TAG_TERMINATE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            return;
        }

        Task task;
        MPI_Recv(&task, sizeof(Task), MPI_BYTE, 0, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Result res;
        res.task_id = task.task_id;

        if (task.remaining_depth == -1) {
            // Terminal node identified during pre-expansion
            res.value = 0.0f;
        } else {
            std::array<float, 7> vals = simulate_game(task.grid, task.remaining_depth, task.ai_turn);
            float sum = 0.0f;
            int count = 0;
            for (int j = 0; j < 7; j++) {
                if (!std::isnan(vals[j])) { sum += vals[j]; count++; }
            }
            res.value = (count > 0) ? sum / count : 0.0f;
        }

        MPI_Send(&res, sizeof(Result), MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
    }
}

// ---------------------------------------------------------------------------
// ParallelComputer::manager_loop
// ---------------------------------------------------------------------------
uint16_t ParallelComputer::manager_loop(const Grid& grid, int split_depth, int total_depth) {
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int num_workers = world_size - 1;

    // --- Pre-expand tree and build task queue ---
    std::queue<Task> task_queue;
    int num_tasks = 0;

    // Recursive pre-expansion via helper struct
    struct Expander {
        int split_depth;
        int total_depth;
        std::queue<Task>& q;
        int& num_tasks;

        void expand(const Grid& grid, int level, int task_id_so_far, bool ai_turn) {
            int16_t turn = ai_turn ? -1 : 1;
            for (int col = 0; col < 7; col++) {
                Grid temp = grid;
                if (!temp.update(col, turn)) continue;

                int16_t result  = temp.check_state(col);
                int     new_id  = task_id_so_far * 7 + col;

                if (result != 0) {
                    // Terminal during pre-expansion — send as sentinel task
                    Task t;
                    t.grid            = temp;
                    t.task_id         = new_id;
                    t.remaining_depth = -1;
                    t.ai_turn         = !ai_turn;
                    q.push(t);
                    num_tasks++;
                } else if (level < split_depth) {
                    expand(temp, level + 1, new_id, !ai_turn);
                } else {
                    Task t;
                    t.grid            = temp;
                    t.task_id         = new_id;
                    t.remaining_depth = total_depth - split_depth - 1;
                    t.ai_turn         = !ai_turn;
                    q.push(t);
                    num_tasks++;
                }
            }
        }
    };

    Expander e{split_depth, total_depth, task_queue, num_tasks};
    e.expand(grid, 0, 0, true);

    // Flat result array sized for worst case at this split_depth
    int result_size = 1;
    for (int i = 0; i <= split_depth + 1; i++) result_size *= 7;
    std::vector<float> flat_results(result_size, NAN);

    // --- Send initial tasks to all workers ---
    int active_workers = 0;
    for (int rank = 1; rank <= num_workers && !task_queue.empty(); rank++) {
        Task t = task_queue.front(); task_queue.pop();
        MPI_Send(&t, sizeof(Task), MPI_BYTE, rank, TAG_TASK, MPI_COMM_WORLD);
        active_workers++;
    }

    // --- Dynamic scheduling loop ---
    while (active_workers > 0) {
        Result res;
        MPI_Status status;
        MPI_Recv(&res, sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
        active_workers--;

        flat_results[res.task_id] = res.value;

        if (!task_queue.empty()) {
            Task t = task_queue.front(); task_queue.pop();
            MPI_Send(&t, sizeof(Task), MPI_BYTE, status.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);
            active_workers++;
        }
    }

    // --- Terminate all workers ---
    for (int rank = 1; rank <= num_workers; rank++) {
        MPI_Send(nullptr, 0, MPI_BYTE, rank, TAG_TERMINATE, MPI_COMM_WORLD);
    }

    // --- Aggregate results up to 7 root values ---
    // split 2: 343 -> 49 -> 7
    // split 1:  49 ->  7
    // split 0:   7 (nothing to do)
    std::vector<float> current = flat_results;
    int current_size = result_size;

    while (current_size > 7) {
        int parent_size = current_size / 7;
        std::vector<float> parent(parent_size, NAN);
        for (int p = 0; p < parent_size; p++) {
            float sum = 0.0f; int count = 0;
            for (int c = 0; c < 7; c++) {
                float v = current[p * 7 + c];
                if (!std::isnan(v)) { sum += v; count++; }
            }
            parent[p] = (count > 0) ? sum / count : NAN;
        }
        current      = parent;
        current_size = parent_size;
    }

    return choose_max(current, 7);
}

// ---------------------------------------------------------------------------
// ParallelComputer::choose_move  — public entry point
// ---------------------------------------------------------------------------
uint16_t ParallelComputer::choose_move(const Grid& grid, int rank, int split_depth, int total_depth) {
    if (rank == 0) {
        return manager_loop(grid, split_depth, total_depth);
    } else {
        worker_loop();
        return 0;
    }
}*/

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

        /*
            Generate 7 × 7 tasks:
            first AI move, first human response.
        */
        for (int ai_col = 0; ai_col < 7; ai_col++) {
            Grid after_ai = grid;

            if (!after_ai.update(ai_col, -1)) {
                continue;
            }

            int16_t ai_result = after_ai.check_state(ai_col);

            /*
                If AI wins immediately, no need to generate human responses.
                Treat this as a direct result.
            */
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

        /*
            Send initial work to workers.
        */
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

        /*
            Receive results and keep assigning new work.
        */
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

        /*
            Tell all ranks the chosen move.
        */
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
