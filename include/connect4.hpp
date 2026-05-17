#include "cursor.hpp"
#include "input.hpp"
#include "parallel_computer.hpp"


#include <mpi.h>
#include <iostream>

class Connect4{
    public:
        void init(int argc, char** argv);
        void play();
        void refresh();
    private:
        Grid grid;
        Cursor cursor;
        Input input;
        ParallelComputer computer;
};