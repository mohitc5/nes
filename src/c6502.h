#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <random>



class C6502 {
    public:

        static constexpr unsigned RAM_SIZE = 0x800;   // ram addr from 0x0000 to 0x07FF
        static constexpr unsigned ROM_SIZE = 0x8000;  // rom addr from 0x8000 to 0xFFFF
        static constexpr unsigned HEADER_SIZE = 0x10; // iNES header

        uint16_t pc;// program counter
        uint8_t A; // alu reg
        uint8_t X; // xy for indexing and counting
        uint8_t Y;
        bool cpuHalted = false;

        std::array<uint8_t, RAM_SIZE> RAM;
        std::array<uint8_t, ROM_SIZE> ROM;
        std::array<uint8_t, HEADER_SIZE> Header;

        std::string filepath;

        // Flags
        bool flagCarry;
        bool flagZero;
        bool flagInterruptDisable;
        bool flagDecimal;
        bool flagOverflow;
        bool flagNegative;


        uint8_t read(uint16_t addr);
        void reset();
        void run();
        void write(uint16_t addr, uint8_t val);
        void emulateCPU();
};