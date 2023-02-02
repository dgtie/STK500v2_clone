#include "uart.h"

bool stop_selection(int i) {
    if (i == NUM_STOP_BITS_1) return true;
    if (i == NUM_STOP_BITS_2) return true;
    return false;
}

bool parity_data_selection(int p, int d) {
    if ((p == PARITY_NONE) && (d == 9)) return true;
    if (d == 8) {
        if (p == PARITY_NONE) return true;
        if (p == PARITY_EVEN) return true;
        if (p == PARITY_ODD) return true;
    }
    return false;
}

bool baud_selection(unsigned b) {
    unsigned baud = (2500000 + (b>>1))/b - 1;
    return (baud && (baud < 65536));
}
