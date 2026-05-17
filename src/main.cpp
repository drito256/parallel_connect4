#include "../include/connect4.hpp"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    Connect4 game;

    // only rank 0 clears the screen and shows the board
    if (world_rank == 0) {
        std::cout << "\033[3J\033[2J\033[H";
        game.refresh();
    }

    game.play();

    MPI_Finalize();
    return 0;
}