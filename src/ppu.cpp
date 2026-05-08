#include "ppu.h"


void PPU::reset() {
    VRAM.fill(0);
    OAM.fill(0);
    palette.fill(0);

    ppuctrl = 0;
    ppumask = 0;
    ppustatus = 0;
    oamaddr = 0;
    oamdata = 0;
    ppuscroll = 0;
    ppuaddr = 0;
    ppudata = 0;

    v = 0;
    t = 0;
    fineX = 0;
    writeToggle = false;
    readBuffer = 0;

    scanline = 0;
    cycle = 0;
    nmiPending = false;
}

void PPU::tick() {
    // TODO: scanline/cycle timing, rendering, VBlank/NMI
}

void PPU::patternTable(std::string filepath){
    std::ifstream file(filepath, std::ios::binary);
    file.seekg(0x8010);
    file.read(reinterpret_cast<char*>(CHRROM.data()), 0x2000);

    for (int table = 0; table < 2; table++){
        for (int row = 0; row < 16; row++){
            for(int col = 0; col < 16; col++){
                for(int y = 0; y < 8; y++){
                    uint8_t low = CHRROM[y + col*16 + row*256 + table*4096];
                    uint8_t high = CHRROM[8 + y + col*16 + row*256 + table*4096];
                    for (int x = 0; x < 8; x++)
                    {
                        int twoBit = ((low>>(7-x) & 1)) == 1 ? 1 : 0;
                        twoBit += ((high>>(7-x) & 1)) == 1 ? 2 : 0;

                        int px = table * 128 + col * 8 + x;
                        int py = row * 8 + y;
                        patternBuffer[py * PATTERN_W + px] = (uint8_t)twoBit;
                    }
                }
            }
        }
    }
}

uint8_t PPU::cpuRead(uint16_t reg) {
    // TODO: implement register read side effects
    (void)reg;
    return 0;
}

void PPU::cpuWrite(uint16_t reg, uint8_t val) {
    // TODO: implement register write side effects
    (void)reg;
    (void)val;
}