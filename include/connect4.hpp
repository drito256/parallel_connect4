#include "grid.hpp"
#include "cursor.hpp"
#include "input.hpp"

#include <iostream>

class Connect4{
    public:
        void play();

    private:
        Grid grid;
        Cursor cursor;
        Input input;
};