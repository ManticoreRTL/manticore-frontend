#include <sstream>
#include <iostream>
#include <limits>
int main() {


    int max = std::numeric_limits<uint32_t>::max();

    std::stringstream builder;

    builder << std::hex <<  max << std::dec;
    builder << std::hex << 0 << std::dec;

    std::cout << builder.str() << std::endl;
}