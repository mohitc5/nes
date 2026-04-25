#include "c6502.h"


int main(int argc, char** argv) {

    C6502 c6502;


    c6502.filepath = "../test_data/2_ReadWrite.nes";
    c6502.reset();
    std::cerr << std::hex << c6502.pc << '\n';
    c6502.run();

}