#include <iostream>

bool debug = false;

void debugPrint(std::string text) {
    if(debug) {
        std::cout << text << std::endl;
    }
}

void Error() {
    std::cout << "Syntax Error" << std::endl;
    exit(1);
}