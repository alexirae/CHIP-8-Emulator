#pragma once
#include <array>
#include <functional>
#include <map>
#include <random>
#include <stack>
#include <vector>

typedef unsigned char  byte;
typedef unsigned short twoByte;

// Resources:
// https://en.wikipedia.org/wiki/CHIP-8
// http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
// http://mattmik.com/files/chip8/mastering/chip8.html
// https://www.youtube.com/watch?v=rpLoS7B6T94

class Chip8
{

public:
    Chip8();

    // System Specifications:
    static constexpr unsigned int c_stackLevels   = 16;                                  // The stack is only used to store return addresses when subroutines are called.
    static constexpr unsigned int c_numRegisters  = 16;                                  // 16 8-bit data registers named from V0 to VF.
    static constexpr unsigned int c_memorySize    = 4096;                                // 4096 memory locations (4K) of 8 bits (a byte) each. 0x000 - 0x1FF is system reserved, 0x200 - 0xFFF is for program ROM and work RAM.

    static constexpr unsigned int c_displayWidth  = 64;
    static constexpr unsigned int c_displayHeight = 32;
    static constexpr unsigned int c_displaySize   = c_displayWidth * c_displayHeight;    // Display resolution is 64x32 pixels, color is monochrome.

    static constexpr unsigned int c_numKeys       = 16;                                  // Input is done with a hex keyboard that has 16 keys which range from 0 to F.
    static constexpr unsigned int c_fontSetSize   = 16 * 5;                              // 4x5 pixel font set(0 - F).
    
    static constexpr byte MSB = 0x80;
    static constexpr byte LSB = 0x01;

    bool loadGame(const std::string& name);
    void initialize();
    void emulateCycle();

    void fetchOpcode();
    void decodeAndExecuteOpcode();
    void updateTimers(bool& playSound);

    void draw();

    constexpr void setDrawFlagFalse()  { drawFlag = false; }
    constexpr bool getDrawFlag() const { return drawFlag;  }

    const std::vector<byte>& getDisplay() const { return display; }

    void setKeys(const std::vector<bool>& updatedKeys) { std::copy(updatedKeys.begin(), updatedKeys.end(), keys.begin()); }

private:
    twoByte randomNext() { return randomDistribution(randomGenerator); }

    std::mt19937 randomGenerator;
    std::uniform_int_distribution<twoByte> randomDistribution;

    std::stack<twoByte> stack;

    std::array<byte, c_numRegisters> V;
    std::array<byte, c_memorySize>   memory;
    std::array<bool, c_numKeys>      keys;

    std::vector<byte>  display;

    twoByte opCode;     // The instruction to execute by the interpreter

    twoByte NNN;        // NNN - A 12 bit value, the lowest 12 bits of the instruction
    byte    NN;         // NN  - An 8 bit value, the lowest 8  bits of the instruction
    byte    N;          // N   - A 4 bit value,  any of the last three 4 bits of the instruction
    byte    X;          // X   - A 4 bit value,  the lower   4 bits of the high byte of the instruction
    byte    Y;          // Y   - A 4 bit value,  the upper   4 bits of the low  byte of the instruction

    twoByte PC;         // PC - Program Counter
    twoByte I;          // I  - 16bit register (For memory address) (Similar to void pointer)

    byte delayTimer;    // Delay timer: This timer is intended to be used for timing the events of games. Its value can be set and read. Count down at 60 hertz, until they reach 0.
    byte soundTimer;    // Sound timer: This timer is used for sound effects. When its value is nonzero, a beeping sound is made. Count down at 60 hertz, until they reach 0.

    bool drawFlag;      // Since the system doesn't draw every cycle, we need to set a draw flag to update the screen.


    // 35 opcodes, all two bytes long.
    std::map<twoByte, std::function<void()>> opCodesTable
    {
        { 0x0000, [this]() { opCode0Table[NN]();           PC += 2; } }, // Go to Op-Code table 0 (System Operations) (0x00E0, 0x00EE)
        { 0x1000, [this]() { PC = NNN;                              } }, // JMP:  1NNN - Jumps to address NNN.
        { 0x2000, [this]() { stack.push(PC); PC = NNN;              } }, // CALL: 2NNN - Calls subroutine at NNN.
        { 0x3000, [this]() { PC += (V[X] == NN)   ? 4 : 2;          } }, // SE:   3XNN - Skip next instruction if Vx == NN.
        { 0x4000, [this]() { PC += (V[X] != NN)   ? 4 : 2;          } }, // SNE:  4XNN - Skip next instruction if Vx != NN.
        { 0x5000, [this]() { PC += (V[X] == V[Y]) ? 4 : 2;          } }, // SE:   5XY0 - Skip next instruction if Vx == Vy.
        { 0x6000, [this]() { V[X] = NN;                    PC += 2; } }, // LD:   6XNN - Set Vx = NN
        { 0x7000, [this]() { V[X] += NN;                   PC += 2; } }, // ADD:  7XNN - Set Vx = Vx + NN. (Carry flag is not changed)
        { 0x8000, [this]() { opCode8Table[N]();            PC += 2; } }, // Go to Op-Code table 8 (Arithmetic Operations)
        { 0x9000, [this]() { PC += (V[X] != V[Y]) ? 4 : 2;          } }, // SNE:  9XY0 - Skip next instruction if Vx != Vy.
        { 0xA000, [this]() { I = NNN;                      PC += 2; } }, // LD:   ANNN - Set I = NNN.
        { 0xB000, [this]() { PC = NNN + V[0];                       } }, // JMP:  BNNN - PC = NNN + V0.
        { 0xC000, [this]() { V[X] = randomNext() & NN;     PC += 2; } }, // RND:  CXNN - Set Vx = random() & NN.
        { 0xD000, [this]() { draw(); drawFlag = true;      PC += 2; } }, // DRW:  DXYN - Draws a sprite at memory location I at coordinate (Vx, Vy) that has a width of 8 pixels and a height of N pixels.
        { 0xE000, [this]() { opCodeETable[NN]();                    } }, // Go to Op-Code table E (Input Operations)
        { 0xF000, [this]() { opCodeFTable[NN]();                    } }, // Go to Op-Code table F (System Operations)
    };

    std::map<twoByte, std::function<void()>> opCode0Table
    {
        { 0x00E0, [this]() { std::fill(display.begin(), display.end(), 0); drawFlag = true; } }, // CLS: Clear the display.             
        { 0x00EE, [this]() { PC = stack.top(); stack.pop();                                 } }, // RET: Return from a subroutine.
    };

    std::map<byte, std::function<void()>> opCode8Table
    {
        { 0x0000, [this]() { V[X] = V[Y];                                                 } }, // LD:   8XY0 - Set Vx = Vy.
        { 0x0001, [this]() { V[X] |= V[Y];                                                } }, // OR:   8XY1 - Set Vx = Vx | Vy.
        { 0x0002, [this]() { V[X] &= V[Y];                                                } }, // AND:  8XY2 - Set Vx = Vx & Vy.
        { 0x0003, [this]() { V[X] ^= V[Y];                                                } }, // XOR:  8XY3 - Set Vx = Vx ^ Vy.
        { 0x0004, [this]() { V[0xF] = ((V[X] + V[Y]) > 0xFF) ? 1 : 0; V[X] += V[Y];       } }, // ADD:  8XY4 - Set Vx = Vx + Vy, set VF = carry.
        { 0x0005, [this]() { V[0xF] = (V[X] > V[Y]) ? 1 : 0;          V[X] -= V[Y];       } }, // SUB:  8XY5 - Set Vx = Vx - Vy, set VF = NOT borrow.
        { 0x0006, [this]() { V[0xF] = (V[X] & LSB)  ? 1 : 0;          V[X] >>= 1;         } }, // SHR:  8XY6 - Set Vx = Vx >> 1 (Vx is divided by 2). If the least significant bit of Vx is 1, then VF is set to 1, otherwise 0.
        { 0x0007, [this]() { V[0xF] = (V[Y] > V[X]) ? 1 : 0;          V[X] = V[Y] - V[X]; } }, // SUBN: 8XY7 - Set Vx = Vy - Vx, set VF = NOT borrow.
        { 0x000E, [this]() { V[0xF] = (V[X] & MSB)  ? 1 : 0;          V[X] <<= 1;         } }, // SHR:  8XYE - Set Vx = Vx << 1 (Vx is multiplied by 2). If the most significant bit of Vx is 1, then VF is set to 1, otherwise 0.
    };

    std::map<twoByte, std::function<void()>> opCodeETable
    {
        { 0x009E, [this]() { PC += (keys[V[X]])  ? 4 : 2; } }, // SKP:  EX9E - Skip the next instruction if the key stored in Vx is pressed.
        { 0x00A1, [this]() { PC += (!keys[V[X]]) ? 4 : 2; } }, // SKNP: EXA1 - Skip next instruction if key stored in Vx is not pressed.
    };

    std::map<twoByte, std::function<void()>> opCodeFTable
    {
        { 0x0007, [this]() { V[X] = delayTimer;                                                                             PC += 2; } }, // LD: FX07 - Set Vx = delay timer value.
        { 0x000A, [this]() { auto it = std::find(keys.begin(), keys.end(), true);                                                         
                             if (it != keys.end()) { V[X] = static_cast<byte>(std::distance(keys.begin(), it)); PC += 2; }           } }, // LD:  FX0A - If a key is pressed, store the value of the key in Vx.
        { 0x0015, [this]() { delayTimer = V[X];                                                                             PC += 2; } }, // LD:  FX15 - Set delay timer = Vx.
        { 0x0018, [this]() { soundTimer = V[X];                                                                             PC += 2; } }, // LD:  FX18 - Set sound timer = Vx.
        { 0x001E, [this]() { const twoByte result = I + V[X]; V[0xF] = (result > 0xFFF) ? 1 : 0; I += V[X];                 PC += 2; } }, // ADD: FX1E - Set I = I + Vx.
        { 0x0029, [this]() { I = V[X] * 5;                                                                                  PC += 2; } }, // LD:  FX29 - Sets I = location of the sprite for the character in Vx. Characters 0-F (in hex) are represented by a 4x5 font
        { 0x0033, [this]() { memory[I] = V[X] / 100; memory[I + 1] = (V[X] / 10) % 10; memory[I + 2] = (V[X] % 100) % 10;   PC += 2; } }, // LD:  FX33 - Store the BCD representation (https://en.wikipedia.org/wiki/Binary-coded_decimal) of Vx in memory locations I, I+1, and I+2.
        { 0x0055, [this]() { std::copy_n(V.begin(),          X + 1, memory.begin() + I); I += X + 1;                        PC += 2; } }, // LD:  FX55 - Store registers V0 through Vx in memory starting at location I.
        { 0x0065, [this]() { std::copy_n(memory.begin() + I, X + 1, V.begin());          I += X + 1;                        PC += 2; } }, // LD:  FX65 - Fill registers V0 through Vx from memory starting at location I.
    };
};
