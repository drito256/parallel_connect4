#include <iostream>

class Cursor{
    public:
        Cursor();
        void show();
        void set_pos(u_int16_t pos);
        u_int16_t get_pos();

    private:
        u_int16_t pos; // [0, 6]
};