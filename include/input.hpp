#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

class Input{
    public:
        Input();
        int16_t get();


    private:
        termios oldt, newt;
};