#include <iostream>

class Cursor{
    public:
        Cursor();
        void show();
        void set_pos(uint16_t pos);
        uint16_t get_pos();

    private:
        uint16_t pos; // [0, 6]
};