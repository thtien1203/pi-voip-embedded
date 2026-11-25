#include "hardware.hpp"
#include <iostream>

bool hardwareInit() {
    std::cout << "[HW] fake hardware ready (type 'b' to press the button)\n";
    return true;
}
void hardwareCleanup() {
    std::cout << "[HW] cleanup\n";
}
void setLed(bool on) {
    if (on) {
        std::cout << "[LED] ON\n";
    }
    else {
        std::cout << "[LED] OFF\n";
    }
}