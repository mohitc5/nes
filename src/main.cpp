#include "c6502.h"


int main(int argc, char** argv) {

    C6502 c6502;


    c6502.filepath = "../test_data/nestest.nes";
    c6502.tracelog.open("trace.log");
    c6502.reset();
    c6502.run();
    c6502.tracelog.close();
    std::cerr << std::hex << c6502.pc << '\n';
    std::cerr << std::hex << "ram " << c6502.RAM[0x0000] << '\n';
    

}