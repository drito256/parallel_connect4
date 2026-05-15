#include "../include/input.hpp"

Input::Input(){
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

int16_t Input::get(){
    while (true) {
        char c;

        if (read(STDIN_FILENO, &c, 1) > 0) {
            char seq[2];

            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'B':
                        return 0; // drop the coin
                    case 'C':
                        return 1; // move cursor right
                    case 'D':
                        return -1; // move cursor left
                }
            }
        }
        usleep(10000);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}