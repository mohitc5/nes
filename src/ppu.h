#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

class PPU {
    public:

        static constexpr unsigned VRAM_SIZE    = 0x800; // 2KB nametable RAM
        static constexpr unsigned OAM_SIZE     = 0x100; // 256B sprite RAM
        static constexpr unsigned PALETTE_SIZE = 0x20;  // 32B palette RAM
        static constexpr unsigned CHRROM_SIZE = 0x2000;

        // Internal memory
        std::array<uint8_t, VRAM_SIZE>    VRAM;
        std::array<uint8_t, OAM_SIZE>     OAM;
        std::array<uint8_t, PALETTE_SIZE> palette;
        std::array<uint8_t, CHRROM_SIZE> CHRROM;

        // Pattern-table debug view: 256x128, two 128x128 tables side-by-side.
        // One byte per pixel, each holding a 2-bit color index (0-3).
        static constexpr unsigned PATTERN_W = 256;
        static constexpr unsigned PATTERN_H = 128;
        std::array<uint8_t, PATTERN_W * PATTERN_H> patternBuffer;

        // Memory-mapped registers ($2000-$2007)
        uint8_t ppuctrl;   // $2000
        uint8_t ppumask;   // $2001
        uint8_t ppustatus; // $2002
        uint8_t oamaddr;   // $2003
        uint8_t oamdata;   // $2004
        uint8_t ppuscroll; // $2005
        uint8_t ppuaddr;   // $2006
        uint8_t ppudata;   // $2007

        // Internal state
        uint16_t v;          // current VRAM address (15 bits)
        uint16_t t;          // temp VRAM address (15 bits)
        uint8_t  fineX;      // fine X scroll (3 bits)
        bool     writeToggle;// $2005/$2006 latch
        uint8_t  readBuffer; // $2007 read buffer

        int  scanline;
        int  cycle;
        bool nmiPending;

        std::string filepath;

        void reset();
        void tick();              // advance one PPU cycle
        void patternTable(std::string filepath);

        // Called by CPU on access to $2000-$3FFF (reg = addr & 0x7)
        uint8_t cpuRead(uint16_t reg);
        void    cpuWrite(uint16_t reg, uint8_t val);
};