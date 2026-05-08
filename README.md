# 6502 CPU — NES Emulator

A cycle-correct emulation of the MOS 6502 (NES variant) in modern C++.
This is the CPU core of an in-progress NES emulator; it currently passes all official opcodes
in [`nestest`](https://www.nesdev.org/wiki/Emulator_tests#nestest), which is the standard golden-log
test ROM used to validate 6502 implementations and serves as the foundation for the PPU and
bus integration that follow.

> **Status:** CPU core complete and verified against `nestest`. PPU in progress, targeting NROM (Mapper 0) titles.

---

## About the 6502

The NES's 6502 is an 8-bit CPU running at ~1.79 MHz with a 16-bit address bus, three GP
registers (`A`, `X`, `Y`), a stack pointer, a program counter, and a 7-flag status register.
Every instruction encodes both an *operation* and an *addressing mode* — and for the emulator
to be useful downstream, both have to be **flag-correct and cycle-correct**, because real
games depend on subtle side effects that are easy to get wrong.

What this implementation handles:

- **All 151 official opcodes** across **13 addressing modes** — immediate, zero page,
  zero page,X/Y, absolute, absolute,X/Y, indirect, indexed indirect (`(zp,X)`),
  indirect indexed (`(zp),Y`), relative, accumulator, and implied.
- **Per-instruction cycle counting**, including the +1 page-cross penalty on indexed
  addressing modes and the +1 / +2 penalties on taken branches.
- **All seven status flags** (`N V B D I Z C`) with correct semantics — including the
  `V` (overflow) on `ADC`/`SBC` and the `B`-flag distinction between
  `BRK`/`PHP` pushes and hardware interrupts.
- **Stack and interrupt model** — `BRK`, `RTI`, `JSR`/`RTS`, and the
  NMI / IRQ / RESET vectors at `$FFFA`–`$FFFF`.
- **Documented hardware quirks** — the indirect `JMP` page-boundary bug and the
  NES' lack of working decimal mode (the `D` flag is encoded but ignored
  by the ALU, which `nestest` exercises).

## Verification
The CPU was validated throughout developement using test ROMs created by 100th_Coin from his NES
Emulator guide, passing all CPU related tests. To validate behaviour with these tests, the VS Code
debugger with gdb was used to ensure correct values were loaded into memory and registers. A custom
tracelogger was also developed to get cycle accurate information of every instruction performed
by the CPU. The tracelogger output was compared against reference logs to ensure correct behaviour
for all tests. below shows an example output of the tracelogger, with the columns in this order: 

Program Counter, Instruction opcode, Instruction, A reg, X reg, Y reg, Stack Pointer, Flag Statuses (lowercase  = 0, uppercase = 1)
<img width="1534" height="560" alt="image" src="https://github.com/user-attachments/assets/aacbdd32-ee91-47f4-91a5-4cf6cfbb6d9e" />

The CPU is validated against `nestest`, the canonical 6502 test ROM. Run with `PC` initialized
to `$C000` (automated mode), the emulator's trace output is byte-comparable to `nestest.log`
across the official-opcode portion of the ROM:

- ✅ Register state (`A`, `X`, `Y`, `P`, `SP`)
- ✅ Cycle count after every instruction
- ✅ All flag transitions

This is the standard correctness bar for a 6502 implementation, and it covers the full
instruction set that every commercial NROM game depends on.

## Design

The CPU lives in a single self-contained class (`c6502`) exposing a tick-driven step
interface. Addressing-mode logic and opcode logic are decoupled, so each instruction is
written against an effective address rather than duplicated per addressing mode — `LDA`
is one function, not eight. Memory access goes through a bus abstraction so the same CPU
can later be wired to the PPU, APU, RAM, and cartridge mappers without changes to the core.

## Build

```bash
git clone https://github.com/mohitc5/nes.git
cd nes
cmake -S . -B build
cmake --build build
./build/nes <path-to-rom.nes>
```

Requirements: C++ compiler and CMake ≥ 3.16. Builds clean with `-Wall -Wextra`.

## Repository layout

```
src/
  c6502.cpp     — 6502 CPU implementation
  main.cpp      — entry point / test driver
test_data/      — nestest ROM and reference log
CMakeLists.txt
```

## Roadmap

- [x] 6502 CPU core — all official opcodes, `nestest` passing
- [ ] PPU — background rendering, sprite evaluation, NMI on VBlank
- [ ] NROM (Mapper 0) cartridge + memory bus
- [ ] SDL2 frontend — framebuffer output, controller input
- [ ] **Goal:** boot a commercial NROM title (e.g. *Donkey Kong*)

## References

- [NESdev Wiki — CPU](https://www.nesdev.org/wiki/CPU)
- [6502.org instruction reference](http://www.6502.org/tutorials/6502opcodes.html)
- [`nestest` test ROM](https://www.nesdev.org/wiki/Emulator_tests#nestest)
- [100th_Coins NES Emulator Guide](https://www.patreon.com/posts/137873901?collection=1707806)
