#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>



class C6502 {
    public:

        static constexpr unsigned RAM_SIZE = 0x800;   // ram addr from 0x0000 to 0x07FF
        static constexpr unsigned ROM_SIZE = 0x8000;  // rom addr from 0x8000 to 0xFFFF
        static constexpr unsigned HEADER_SIZE = 0x10; // iNES header

        uint16_t pc;// program counter
        uint16_t addressBus; // temp address for reusability
        uint8_t A; // alu reg
        uint8_t X; // xy for indexing and counting
        uint8_t Y;
        uint8_t sp; // stack pointer
        bool cpuHalted = false;

        std::array<uint8_t, RAM_SIZE> RAM;
        std::array<uint8_t, ROM_SIZE> ROM;
        std::array<uint8_t, HEADER_SIZE> Header;

        std::string filepath;

        bool logging = true;
        std::ofstream tracelog;

        // Flags
        bool flagCarry;
        bool flagZero;
        bool flagInterruptDisable;
        bool flagDecimal;
        bool flagOverflow;
        bool flagNegative;


        static constexpr const char* opcodeNames[256] = {
            // 0x_0    0x_1    0x_2    0x_3    0x_4    0x_5    0x_6    0x_7    0x_8    0x_9    0x_A    0x_B    0x_C    0x_D    0x_E    0x_F
            "BRK",  "ORA",  "HLT",  "SLO",  "NOP",  "ORA",  "ASL",  "SLO",  "PHP",  "ORA",  "ASL",  "ANC",  "NOP",  "ORA",  "ASL",  "SLO",  // 0x0_
            "BPL",  "ORA",  "HLT",  "SLO",  "NOP",  "ORA",  "ASL",  "SLO",  "CLC",  "ORA",  "NOP",  "SLO",  "NOP",  "ORA",  "ASL",  "SLO",  // 0x1_
            "JSR",  "AND",  "HLT",  "RLA",  "BIT",  "AND",  "ROL",  "RLA",  "PLP",  "AND",  "ROL",  "ANC",  "BIT",  "AND",  "ROL",  "RLA",  // 0x2_
            "BMI",  "AND",  "HLT",  "RLA",  "NOP",  "AND",  "ROL",  "RLA",  "SEC",  "AND",  "NOP",  "RLA",  "NOP",  "AND",  "ROL",  "RLA",  // 0x3_
            "RTI",  "EOR",  "HLT",  "SRE",  "NOP",  "EOR",  "LSR",  "SRE",  "PHA",  "EOR",  "LSR",  "ALR",  "JMP",  "EOR",  "LSR",  "SRE",  // 0x4_
            "BVC",  "EOR",  "HLT",  "SRE",  "NOP",  "EOR",  "LSR",  "SRE",  "CLI",  "EOR",  "NOP",  "SRE",  "NOP",  "EOR",  "LSR",  "SRE",  // 0x5_
            "RTS",  "ADC",  "HLT",  "RRA",  "NOP",  "ADC",  "ROR",  "RRA",  "PLA",  "ADC",  "ROR",  "ARR",  "JMP",  "ADC",  "ROR",  "RRA",  // 0x6_
            "BVS",  "ADC",  "HLT",  "RRA",  "NOP",  "ADC",  "ROR",  "RRA",  "SEI",  "ADC",  "NOP",  "RRA",  "NOP",  "ADC",  "ROR",  "RRA",  // 0x7_
            "NOP",  "STA",  "NOP",  "SAX",  "STY",  "STA",  "STX",  "SAX",  "DEY",  "NOP",  "TXA",  "XAA",  "STY",  "STA",  "STX",  "SAX",  // 0x8_
            "BCC",  "STA",  "HLT",  "AHX",  "STY",  "STA",  "STX",  "SAX",  "TYA",  "STA",  "TXS",  "TAS",  "SHY",  "STA",  "SHX",  "AHX",  // 0x9_
            "LDY",  "LDA",  "LDX",  "LAX",  "LDY",  "LDA",  "LDX",  "LAX",  "TAY",  "LDA",  "TAX",  "LAX",  "LDY",  "LDA",  "LDX",  "LAX",  // 0xA_
            "BCS",  "LDA",  "HLT",  "LAX",  "LDY",  "LDA",  "LDX",  "LAX",  "CLV",  "LDA",  "TSX",  "LAS",  "LDY",  "LDA",  "LDX",  "LAX",  // 0xB_
            "CPY",  "CMP",  "NOP",  "DCP",  "CPY",  "CMP",  "DEC",  "DCP",  "INY",  "CMP",  "DEX",  "AXS",  "CPY",  "CMP",  "DEC",  "DCP",  // 0xC_
            "BNE",  "CMP",  "HLT",  "DCP",  "NOP",  "CMP",  "DEC",  "DCP",  "CLD",  "CMP",  "NOP",  "DCP",  "NOP",  "CMP",  "DEC",  "DCP",  // 0xD_
            "CPX",  "SBC",  "NOP",  "ISC",  "CPX",  "SBC",  "INC",  "ISC",  "INX",  "SBC",  "NOP",  "SBC",  "CPX",  "SBC",  "INC",  "ISC",  // 0xE_
            "BEQ",  "SBC",  "HLT",  "ISC",  "NOP",  "SBC",  "INC",  "ISC",  "SED",  "SBC",  "NOP",  "ISC",  "NOP",  "SBC",  "INC",  "ISC",  // 0xF_
        };

        uint8_t read(uint16_t addr);
        void reset();
        void run();
        void write(uint16_t addr, uint8_t val);
        void push(uint8_t val); // stack operations
        uint8_t pull(); // stack pop
        void readAbsolute();
        void readZeroPage();
        void readAbsoluteXIndexed();
        void readAbsoluteYIndexed();
        void readZeroPageXIndexed();
        void readZeroPageYIndexed();
        void opASL(uint16_t addr, uint8_t input);
        void opROL(uint16_t addr, uint8_t input);
        void opLSR(uint16_t addr, uint8_t input);
        void opROR(uint16_t addr, uint8_t input);
        void opINC(uint16_t addr, uint8_t input);
        void opDEC(uint16_t addr, uint8_t input);
        void opORA(uint8_t input);
        void opAND(uint8_t input);
        void opEOR(uint8_t input);
        void opADC(uint8_t input);
        void opSBC(uint8_t input);
        void opCMP(uint8_t input);
        void opCPX(uint8_t input);
        void opCPY(uint8_t input);
        void opBIT(uint8_t input);
        void emulateCPU();
        void traceLogger(uint16_t logPc, uint8_t opcode);
};