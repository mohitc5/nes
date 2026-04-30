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

void C6502::readAbsolute(){
    addressBus = read(pc);
    pc++;
    addressBus = read(pc << 8 | addressBus);
    pc++;
}

void C6502::readZeroPage(){
    addressBus = read(pc);
    pc++;
}

void C6502::readAbsoluteXIndexed(){
    addressBus = read(pc);
    pc++;
    addressBus = read(pc << 8 | addressBus);
    pc++;
    addressBus += X;
}
void C6502::readAbsoluteYIndexed(){
    addressBus = read(pc);
    pc++;
    addressBus = read(pc << 8 | addressBus);
    pc++;
    addressBus += Y;
}

void C6502::readZeroPageXIndexed(){
    addressBus = read(pc);
    pc++;
    addressBus = (addressBus + X) % 256;
}
void C6502::readZeroPageYIndexed(){
    addressBus = read(pc);
    pc++;
    addressBus = (addressBus + Y) % 256;

}

void C6502::opASL(uint16_t addr, uint8_t input){
    flagCarry = input >= 0x80;
    input <<= 1;
    write(addr,input);
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}

void C6502::opROL(uint16_t addr, uint8_t input){
    bool futureFlagCarry = input >= 0x80;
    input <<= 1;
    if (flagCarry) input |= 1;
    write(addr,input);
    flagCarry = futureFlagCarry;
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}
void C6502::opLSR(uint16_t addr, uint8_t input){
    flagCarry = (input & 1) != 0;
    input >>= 1;
    write(addr,input);
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}

void C6502::opROR(uint16_t addr, uint8_t input){
    bool futureFlagCarry = (input & 1) != 0;
    input >>= 1;
    if (flagCarry) input |= 0x80;
    write(addr,input);
    flagCarry = futureFlagCarry;
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}

void C6502::opINC(uint16_t addr, uint8_t input){
    input ++;
    write(addr,input);
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}

void C6502::opDEC(uint16_t addr, uint8_t input){
    input --;
    write(addr,input);
    flagNegative = input >= 0x80;
    flagZero = input == 0;
}
void C6502::opORA( uint8_t input){
    A |= input;
    flagNegative = A >= 0x80;
    flagZero = A == 0;
}
void C6502::opAND( uint8_t input){
    A &= input;
    flagNegative = A >= 0x80;
    flagZero = A == 0;
}
void C6502::opEOR( uint8_t input){
    A ^= input;
    flagNegative = A >= 0x80;
    flagZero = A == 0;
}
void C6502::opADC( uint8_t input){
    int sum = input + A + (flagCarry ? 1 :0);
    flagOverflow = (~(A ^ input) & (A ^ sum) & 0x80) != 0;
    flagCarry = sum > 0xFF;
    A = uint8_t(sum);
    flagNegative = A >= 0x80;
    flagZero = A == 0;
}
void C6502::opSBC( uint8_t input){
    int sum = A - input - (flagCarry ? 0 :1);
    flagOverflow = ((A ^ input) & (A ^ sum) & 0x80) != 0;
    flagCarry = sum >= 0;
    A = uint8_t(sum);
    flagNegative = A >= 0x80;
    flagZero = A == 0;
}
void C6502::opCMP( uint8_t input){
    
    flagNegative = uint8_t(A - input) >= 0x80;
    flagZero = A == input;
    flagCarry = A >= input;
}
void C6502::opCPX( uint8_t input){
    
    flagNegative = uint8_t(X - input) >= 0x80;
    flagZero = X == input;
    flagCarry = X >= input;
}
void C6502::opCPY( uint8_t input){
    
    flagNegative = uint8_t(Y - input) >= 0x80;
    flagZero = Y == input;
    flagCarry = Y >= input;
}
void C6502::opBIT( uint8_t input){
    
    flagNegative = (input & 0x80) != 0;
    flagZero = (A & input) == 0;
    flagOverflow = (input & 0x40) != 0;
}

void C6502::reset(){
    flagInterruptDisable = true;
    flagCarry = false;
    flagDecimal = false;
    flagNegative = false;
    flagOverflow = false;
    flagZero = false;
    sp = 0xFD;
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

void C6502::push(uint8_t val){
    write(0x100 + sp, val);
    sp --;
}

uint8_t C6502::pull(){
    sp++;
    return read(0x100 + sp);
}

void C6502::traceLogger(uint16_t logPc, uint8_t opcode){
    if(!logging) return;

    std::ostringstream line;
    line << std::uppercase << std::hex << std::setfill('0');
    line << "$" << std::setw(4) << logPc
         << "\t" << std::setw(2) << static_cast<int>(opcode)
         << "\t" << opcodeNames[opcode]
         << "\t\tA: " << std::setw(2) << static_cast<int>(A)
         << "\tX: " << std::setw(2) << static_cast<int>(X)
         << "\tY: " << std::setw(2) << static_cast<int>(Y)
         << "\tSP: " << std::setw(2) << static_cast<int>(sp)
         << "\tProcessor Flags: "
         << (flagNegative ? 'N' : 'n')
         << (flagOverflow ? 'V' : 'v')
         << "--"
         << (flagDecimal ? 'D' : 'd')
         << (flagInterruptDisable ? 'I' : 'i')
         << (flagZero ? 'Z' : 'z')
         << (flagCarry ? 'C' : 'c');

    tracelog << line.str() << '\n';
}

void C6502::emulateCPU(){
    uint16_t logPc = pc;
    uint8_t opcode = read(pc);
    traceLogger(logPc, opcode);
    //std::cerr << std::hex << pc << '\n';
    pc++;
    
    // need to keep track of cycles for ppu
    int cycles = 0;

    switch(opcode)
    {
        case 0x00: // BRK
        {
            pc ++;
            push(uint8_t(pc >> 8));
            push(uint8_t(pc));
            uint8_t temp = 0;
            temp += flagCarry ? 1 : 0; // b0
            temp += flagZero ? 2 : 0; // b1
            temp += flagInterruptDisable ? 4 : 0; // b2
            temp += flagDecimal ? 8 : 0;
            temp += 16;
            temp += 32;
            temp += flagOverflow ? 64 : 0;
            temp += flagNegative ? 128 : 0;
            push(temp);
            uint8_t tempLow = read(0xFFFE);
            uint8_t tempHigh = read(0xFFFF);
            pc = uint16_t((tempHigh * 0x100) + tempLow);
            cycles = 7;
            break;
        }
        case 0x40: // RTI
        {
            uint8_t temp = pull();
            flagCarry = (temp & 1) != 0;
            flagZero = (temp & 2) != 0;
            flagInterruptDisable = (temp & 4) != 0;
            flagDecimal = (temp & 8) != 0;
            flagOverflow = (temp & 64) != 0;
            flagNegative = (temp & 128) != 0;
            uint8_t tempLow = pull();
            uint8_t tempHigh = pull();
            pc = uint16_t((tempHigh * 256) + tempLow);
            cycles = 6;
            break;
        }
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
        case 0xA9: // LDA Immediate
        {
            A = read(pc);
            flagZero = A == 0;
            flagNegative = A > 127;
            pc++;
            cycles = 2;
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
        case 0xB5: // LDA Zero Page, X
        {
            readZeroPageXIndexed();
            A = read(addressBus);
            flagZero = A == 0;
            flagNegative = A > 127;
            cycles = 4;

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
        case 0xBD: // LDA Absolute, X
        {
            readAbsoluteXIndexed();
            A = read(addressBus);
            flagZero = A == 0;
            flagNegative = A > 127;
            cycles = 4; 
            break;
        }
        case 0xB9: // LDA Absolute, Y
        {
            readAbsoluteYIndexed();
            A = read(addressBus);
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
        case 0x48: // PHA
        {
            push(A);
            cycles = 3;
            break;
        }
        case 0x68: // PLA
        {
            A = pull();
            flagZero = A == 0;
            flagNegative = A > 127;
            cycles = 4;
            break;
        }
        case 0x4C: // JMP
        {
            uint8_t tempLow = read(pc);
            pc++;
            uint8_t tempHigh = read(pc);
            pc = uint16_t((tempHigh * 256) + tempLow);
            cycles = 3;
            break;

        }
        case 0x6C: // JMP, Indirect
        {
            uint8_t tempLow = read(pc);
            pc++;
            uint8_t tempHigh = read(pc);
            uint16_t tempAddr = uint16_t((tempHigh * 256) + tempLow);
            pc = uint16_t((read(tempAddr) * 256) + read(tempAddr + 1));
            cycles = 5;
            break;

        }
        case 0x20: // JSR
        {
            uint8_t tempLow = read(pc);
            pc++;
            uint8_t tempHigh = read(pc);
            // push return addr
            push(uint8_t(pc/256));
            push(uint8_t(pc));
            pc = uint16_t((tempHigh * 256) + tempLow);
            cycles = 6;
            break;
        }
        case 0x60: // RTS
        {
            uint8_t tempLow = pull();
            uint8_t tempHigh = pull();
            pc = uint16_t((tempHigh * 256) + tempLow);
            pc ++;
            cycles = 6;
            break;
        }
        case 0xE8: // INX
        {
            X += 1;
            flagZero = X == 0;
            flagNegative = X > 127;
            cycles = 2;
            break;
        }
        case 0xCA: // DEX
        {
            X -= 1;
            flagZero = X == 0;
            flagNegative = X > 127;
            cycles = 2;
            break;
        }
        case 0xC8: // INY
        {
            Y += 1;
            flagZero = Y == 0;
            flagNegative = Y > 127;
            cycles = 2;
            break;
        }
        case 0x88: // DEX
        {
            Y -= 1;
            flagZero = Y == 0;
            flagNegative = Y > 127;
            cycles = 2;
            break;
        }
        case 0xAA: // TAX
        {
            X = A;
            flagZero = X == 0;
            flagNegative = X > 127;
            cycles = 2;
            break;
        }
        case 0x8A: // TXA
        {
            A = X;
            flagZero = X == 0;
            flagNegative = X > 127;
            cycles = 2;
            break;
        }
        case 0xA8: // TAY
        {
            Y = A;
            flagZero = Y == 0;
            flagNegative = Y > 127;
            cycles = 2;
            break;
        }
        case 0x98: // TYA
        {
            A = Y;
            flagZero = Y == 0;
            flagNegative = Y > 127;
            cycles = 2;
            break;
        }
        case 0xBA: // TSX
        {
            X = sp;
            flagZero = X == 0;
            flagNegative = X > 127;
            cycles = 2;
            break;
        }
        case 0x9A: // TXS
        {
            sp = X;
            cycles = 2;
            break;
        }
        case 0x38: // SEC
        {
            flagCarry = true;
            cycles = 2;
            break;
        }
        case 0x18: // CLC
        {
            flagCarry = false;
            cycles = 2;
            break;
        }
        case 0xB8: // CLV
        {
            flagOverflow = false;
            cycles = 2;
            break;
        }
        case 0x78: // SEI
        {
            flagInterruptDisable = true;
            cycles = 2;
            break;
        }
        case 0x58: // CLI
        {
            flagInterruptDisable = false;
            cycles = 2;
            break;
        }
        case 0xF8: // SED
        {
            flagDecimal = true;
            cycles = 2;
            break;
        }
        case 0xD8: // CLD
        {
            flagDecimal = false;
            cycles = 2;
            break;
        }
        case 0xEA: // NOP
        {
            cycles = 2;
            break;
        }
        case 0x08: // PHP
        {
            uint8_t temp = 0;
            temp += flagCarry ? 1 : 0; // b0
            temp += flagZero ? 2 : 0; // b1
            temp += flagInterruptDisable ? 4 : 0; // b2
            temp += flagDecimal ? 8 : 0;
            temp += 16;
            temp += 32;
            temp += flagOverflow ? 64 : 0;
            temp += flagNegative ? 128 : 0;
            push(temp);
            cycles = 3;
            break;
        }
        case 0x28: // PLP
        {
            uint8_t temp = pull();
            flagCarry = (temp & 1) != 0;
            flagZero = (temp & 2) != 0;
            flagInterruptDisable = (temp & 4) != 0;
            flagDecimal = (temp & 8) != 0;
            flagOverflow = (temp & 0x40) != 0;
            flagNegative = (temp & 0x80) != 0;
            cycles = 3;
            break;
        }
        case 0x0A: // ASL Implied
        {
            flagCarry = A > 127;
            A = A << 1;
            flagNegative = A > 127;
            flagZero = A == 0;
            cycles = 2;
            break;
        }
        case 0x0E: // ASL Absolute
        {
            readAbsolute();
            opASL(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0x06: // ASL Zero Page
        {
            readZeroPage();
            opASL(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0x2A: // ROL Implied
        {
            bool futureFlagCarry = A >= 0x80;
            A <<= 1;
            if (flagCarry) A |= 1;
            flagCarry = futureFlagCarry;
            flagNegative = A >= 0x80;
            flagZero = A == 0;
            cycles = 2;
            break;
        }
        case 0x2E: // ROL Absolute
        {
            readAbsolute();
            opROL(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0x26: // ROL Zero Page
        {
            readZeroPage();
            opROL(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0x4A: // LSR Implied
        {
            flagCarry = (A & 1) != 0;
            A = A >> 1;
            flagNegative = A > 127;
            flagZero = A == 0;
            cycles = 2;
            break;
        }
        case 0x4E: // LSR Absolute
        {
            readAbsolute();
            opLSR(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0x46: // LSR Zero Page
        {
            readZeroPage();
            opLSR(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0x6A: // ROR Implied
        {
            bool futureFlagCarry = (A & 1) != 0;
            A >>= 1;
            if (flagCarry) A |= 0x80;
            flagCarry = futureFlagCarry;
            flagNegative = A >= 0x80;
            flagZero = A == 0;
            cycles = 2;
            break;
        }
        case 0x6E: // ROR Absolute
        {
            readAbsolute();
            opROR(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0x66: // ROR Zero Page
        {
            readZeroPage();
            opROR(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0xEE: // INC Absolute
        {
            readAbsolute();
            opINC(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0xE6: // INC Zero Page
        {
            readZeroPage();
            opINC(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0xCE: // DEC Absolute
        {
            readAbsolute();
            opDEC(addressBus,read(addressBus));
            cycles = 6;
            break;
        }
        case 0xC6: // DEC Zero Page
        {
            readZeroPage();
            opDEC(addressBus,read(addressBus));
            cycles = 5;
            break;
        }
        case 0x09: // ORA Immediate
        {
            opORA(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0x05: // ORA Zero Page
        {
            readZeroPage();
            opORA(read(addressBus));
            cycles = 3;
            break;
        }

        case 0x0D: // ORA Absolute ( 2 bytes)
        {
            readAbsolute();
            opORA(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0x29: // AND Immediate
        {
            opAND(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0x25: // AND Zero Page
        {
            readZeroPage();
            opAND(read(addressBus));
            cycles = 3;
            break;
        }

        case 0x2D: // AND Absolute ( 2 bytes)
        {
            readAbsolute();
            opAND(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0x49: // EOR Immediate
        {
            opEOR(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0x45: // EOR Zero Page
        {
            readZeroPage();
            opEOR(read(addressBus));
            cycles = 3;
            break;
        }

        case 0x4D: // EOR Absolute ( 2 bytes)
        {
            readAbsolute();
            opEOR(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0x69: // ADC Immediate
        {
            opADC(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0x65: // ADC Zero Page
        {
            readZeroPage();
            opADC(read(addressBus));
            cycles = 3;
            break;
        }

        case 0x6D: // ADC Absolute ( 2 bytes)
        {
            readAbsolute();
            opADC(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0xE9: // SBC Immediate
        {
            opSBC(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0xE5: // SBC Zero Page
        {
            readZeroPage();
            opSBC(read(addressBus));
            cycles = 3;
            break;
        }

        case 0xED: // SBC Absolute ( 2 bytes)
        {
            readAbsolute();
            opSBC(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0xC9: // CMP Immediate
        {
            opCMP(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0xC5: // CMP Zero Page
        {
            readZeroPage();
            opCMP(read(addressBus));
            cycles = 3;
            break;
        }

        case 0xCD: // CMP Absolute ( 2 bytes)
        {
            readAbsolute();
            opCMP(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0xE0: // CPX Immediate
        {
            opCPX(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0xE4: // CPX Zero Page
        {
            readZeroPage();
            opCPX(read(addressBus));
            cycles = 3;
            break;
        }

        case 0xEC: // CPX Absolute ( 2 bytes)
        {
            readAbsolute();
            opCPX(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0xC0: // CPY Immediate
        {
            opCPY(read(pc));
            pc++;
            cycles = 2;
            break;
        }

        case 0xC4: // CPY Zero Page
        {
            readZeroPage();
            opCPY(read(addressBus));
            cycles = 3;
            break;
        }

        case 0xCC: // CPY Absolute ( 2 bytes)
        {
            readAbsolute();
            opCPY(read(addressBus));
            cycles = 4; 
            break;
        }
        case 0x24: // CPY Zero Page
        {
            readZeroPage();
            opBIT(read(addressBus));
            cycles = 3;
            break;
        }

        case 0x2C: // CPY Absolute ( 2 bytes)
        {
            readAbsolute();
            opBIT(read(addressBus));
            cycles = 4; 
            break;
        }
        


        default:
        {
            //std::cerr << std::hex << opcode<< '\n';
            break;
        }


    }
}