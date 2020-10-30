#include <iostream>
#include <cstdint>

extern "C" {
    uint8_t regex(const char*);
}

int main(int argc, char ** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " test_string" << std::endl;
        return 1;
    }
    uint8_t matched = regex(argv[1]);
    if (matched) {
        std::cout << "Matched." << std::endl;
    } else {
        std::cout << "Did not match." << std::endl;
    }
    return 0;
}
