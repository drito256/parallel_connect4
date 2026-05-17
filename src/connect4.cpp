#include "../include/connect4.hpp"

#include <mpi.h>
#include <type_traits>

namespace {
    constexpr int CMD_AI_TURN = 0;
    constexpr int CMD_STOP    = 1;

    void broadcast_grid(Grid& grid) {
        static_assert(
            std::is_trivially_copyable<Grid>::value,
            "Grid must be trivially copyable to broadcast with MPI_BYTE. "
            "If this fails, implement Grid::serialize / Grid::deserialize instead."
        );

        MPI_Bcast(
            &grid,
            sizeof(Grid),
            MPI_BYTE,
            0,
            MPI_COMM_WORLD
        );
    }
}

void Connect4::play() {
    int rank = 0;
    int size = 1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /*
        Worker ranks do not run the interactive game.
        They only wait for rank 0 to announce AI turns.
    */
    if (rank != 0) {
        while (true) {
            int command = CMD_STOP;

            MPI_Bcast(
                &command,
                1,
                MPI_INT,
                0,
                MPI_COMM_WORLD
            );

            if (command == CMD_STOP) {
                return;
            }

            /*
                Receive the current board from rank 0.
            */
            broadcast_grid(grid);

            /*
                All ranks must enter this function together.
                Rank 0 acts as master inside choose_move_parallel_dynamic().
                Worker ranks receive tasks and compute.
            */
            uint16_t computer_choice = computer.choose_move_parallel_dynamic(grid);

            /*
                Optional, but keeps worker-local grids consistent.
                Rank 0 will broadcast the full grid again next AI turn anyway.
            */
            grid.update(computer_choice, -1);
        }
    }

    /*
        Only rank 0 runs the actual game loop and handles input/output.
    */
    int16_t game_finished = 0;

    while (game_finished == 0) {
        bool valid_move = false;

        while (!valid_move) {
            int button = input.get();

            switch (button) {
                case -1:
                    cursor.set_pos(std::max(cursor.get_pos() - 1, 0));
                    break;

                case 1:
                    cursor.set_pos(std::min(cursor.get_pos() + 1, 6));
                    break;

                case 0:
                    valid_move = grid.update(cursor.get_pos(), 1);
                    break;
            }

            refresh();
        }

        game_finished = grid.check_state(cursor.get_pos());

        if (game_finished != 0) {
            break;
        }

        uint16_t computer_choice = 0;

        if (size == 1) {
            /*
                No MPI workers available.
                Use the normal single-process AI.
            */
            computer_choice = computer.choose_move(grid);
        } else {
            /*
                Tell workers that an AI turn is starting.
            */
            int command = CMD_AI_TURN;

            MPI_Bcast(
                &command,
                1,
                MPI_INT,
                0,
                MPI_COMM_WORLD
            );

            /*
                Send the current board to all workers.
            */
            broadcast_grid(grid);

            /*
                Rank 0 enters the same parallel function as the workers.
            */
            computer_choice = computer.choose_move_parallel_dynamic(grid);
        }

        grid.update(computer_choice, -1);
        game_finished = grid.check_state(computer_choice);

        refresh();
    }

    /*
        Tell workers to exit.
    */
    if (size > 1) {
        int command = CMD_STOP;

        MPI_Bcast(
            &command,
            1,
            MPI_INT,
            0,
            MPI_COMM_WORLD
        );
    }

    if (game_finished == 1) {
        std::cout << "\n  ======================================================\n";
        std::cout << "  Congrats! you won against my suuuuper smart ai brain!";
        std::cout << "\n  ======================================================\n";
    } else if (game_finished == -1) {
        std::cout << "\n  ==========================\n";
        std::cout << "  Better luck next time bud!";
        std::cout << "\n  ==========================\n";
    } else {
        std::cout << "\n  ==========================================================\n";
        std::cout << "  Who would have thought you are as smart as me! It's a draw!";
        std::cout << "\n  ==========================================================\n";
    }
}

void Connect4::refresh(){

        std::cout << "\033[3J\033[2J\033[H"; // clear screen
        cursor.show();
        grid.show();
}