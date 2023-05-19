#include "systick.hpp"

unsigned int systick = 0;

void tick_systick() {
    systick++;
}

unsigned int get_systick() {
    return systick;
}