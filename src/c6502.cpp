#include "c6502.h"


uint8_t C6502::read(uint16_t addr){

    if(addr < 0x800){
        return RAM[addr]; // ram read
    } else if (addr >= 0x800 && addr <= 0x1FFF) {
        return RAM[addr & 0x7FF]; // ram mirroring read
    } else if (addr >= 0x8000){
        return ROM[addr - 0x8000]; // rom read
    }
    return 0;
}


void C6502::write(uint16_t addr, uint8_t val){

    if(addr < 0x800){
        RAM[addr] = val; // ram write
    } else if (addr >= 0x800 && addr <= 0x1FFF) {
        RAM[addr & 0x7FF] = val; // ram mirroring write
    } 
}

void C6502::reset(){
    flagInterruptDisable = true;
    std::ifstream file(filepath, std::ios::binary);
    std::vector<uint8_t> headeredROM(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

    std::copy(headeredROM.begin() + 0x10, // start
              headeredROM.begin() + 0x10 + 0x8000, // end
              ROM.begin()); // store in ROM

    // Copy header
    std::copy(headeredROM.begin(), // start
              headeredROM.begin() + 0x10, // end
              Header.begin()); // store header

    uint8_t PCL = read(0xFFFC); // PC low byte
    uint8_t PCH = read(0xFFFD); // PC high byte
    pc = static_cast<uint16_t>((PCH *0x100 )+ PCL);
}


void C6502::run(){
    while(!cpuHalted){
        emulateCPU();
    }
}

void C6502::emulateCPU(){
    uint8_t opcode = read(pc);
    std::cerr << std::hex << pc << '\n';
    pc++;
    
    // need to keep track of cycles for ppu
    int cycles = 0;

    switch(opcode)
    {
        case 0x02: // HLT
        {
            cpuHalted = true;
            break;
        }
        case 0xA0: // LDY
        {
            Y = read(pc);
            flagZero = Y == 0;
            flagNegative = Y > 127;
            pc++;
            cycles = 2;
            break;
        }
        case 0xA2: // LDX
        {
            X = read(pc);
            flagZero = X == 0;
            flagNegative = X > 127;
            pc++;
            cycles = 2;
            break;
        }
        case 0xA9: // LDA Immediate
        {
            A = read(pc);
            flagZero = A == 0;
            flagNegative = A > 127;
            pc++;
            cycles = 2;
            break;
        }

        case 0x85: // STA Zero Page
        {
            uint8_t temp = read(pc);
            pc++;
            write(temp,A);
            cycles = 3;

            break;
        }
        case 0x8D: // STA Absolute (2 bytes)
        {
            uint8_t low = read(pc);
            pc++;
            uint8_t high = read(pc);
            pc ++;
            write( (high * 256) + low, A);
            cycles = 4; 
            break;
        }

        case 0x86: // STX Zero Page
        {
            uint8_t temp = read(pc);
            pc++;
            write(temp,X);
            cycles = 3;

            break;
        }
        case 0x8E: // STX Absolute (2 bytes)
        {
            uint8_t low = read(pc);
            pc++;
            uint8_t high = read(pc);
            pc ++;
            write( (high * 256) + low, X);
            cycles = 4; 
            break;
        }

        case 0x84: // STY Zero Page
        {
            uint8_t temp = read(pc);
            pc++;
            write(temp,Y);
            cycles = 3;

            break;
        }
        case 0x8C: // STY Absolute (2 bytes)
        {
            uint8_t low = read(pc);
            pc++;
            uint8_t high = read(pc);
            pc ++;
            write( (high * 256) + low, Y);
            cycles = 4; 
            break;
        }

        case 0xA5: // LDA Zero Page
        {
            uint8_t temp = read(pc);
            pc++;
            A = read(temp);
            flagZero = A == 0;
            flagNegative = A > 127;
            cycles = 3;

            break;
        }

        case 0xAD: // LDA Absolute ( 2 bytes)
        {
            uint8_t low = read(pc);
            pc++;
            uint8_t high = read(pc);
            pc ++;
            A = read((high * 256) + low);
            flagZero = A == 0;
            flagNegative = A > 127;
            cycles = 4; 
            break;
        }

        case 0xD0: // BNE
        {
            uint8_t temp = read(pc);
            pc ++;
            if(!flagZero){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0x10: // BPL
        {
            uint8_t temp = read(pc);
            pc ++;
            if(!flagNegative){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0x30: // BMI
        {
            uint8_t temp = read(pc);
            pc ++;
            if(flagNegative){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0x50: // BVC
        {
            uint8_t temp = read(pc);
            pc ++;
            if(!flagOverflow){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0x70: // BVS
        {
            uint8_t temp = read(pc);
            pc ++;
            if(flagOverflow){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0x90: // BCC
        {
            uint8_t temp = read(pc);
            pc ++;
            if(!flagCarry){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0xB0: // BCS
        {
            uint8_t temp = read(pc);
            pc ++;
            if(flagCarry){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        case 0xF0: // BEQ
        {
            uint8_t temp = read(pc);
            pc ++;
            if(flagZero){
                signed int signedVal = temp;
                if (signedVal > 127){
                    signedVal -= 256;
                }
                pc = uint16_t(pc + signedVal);
                cycles = 3;
            }else{
                cycles = 2;
            }
            
            break;
        }
        default:
        {
            break;
        }


    }
}