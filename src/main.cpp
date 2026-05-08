#include "c6502.h"
#include "ppu.h"
#include "platform.h"


int main(int argc, char** argv) {
    (void)argc; (void)argv;

    C6502 c6502;
    PPU ppu;
    c6502.ppu = &ppu;

    c6502.filepath = "../test_data/SuperMarioBros.nes";
    c6502.tracelog.open("trace.log");
    ppu.reset();

    ppu.patternTable(c6502.filepath);

    Platform platform("NES - Pattern Tables", PPU::PATTERN_W, PPU::PATTERN_H, 4);
    while (platform.processInput()) {
        platform.render(ppu.patternBuffer.data());
        SDL_Delay(16);
    }

    c6502.reset();
    c6502.run();
    c6502.tracelog.close();

    
}