#include <fstream>
#include "Chip8.h"

Chip8::Chip8()
    : randomGenerator(std::random_device()())
    , randomDistribution(0, 0xFF)
{
    
}

/////////////////////////////////////////////////////////////////////////////

bool Chip8::loadGame(const std::string& name)
{
    // Open file in bynary mode
    std::ifstream inputFileStream(name, std::ios::binary);

    if (inputFileStream.fail())
        return false;

    // Load the game in memory from location: 0x200
    unsigned int i = 0x200;
    char currentFileByte;

    while (inputFileStream.read(&currentFileByte, sizeof(currentFileByte)))
    {
        memory[i] = currentFileByte;
        ++i;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::initialize()
{
    // Clear stack, V registers, memory and display
    stack = std::stack<twoByte>();
    V.fill(0);
    memory.fill(0);
    display.resize(c_displaySize, 0);

    // Initialize opcode accessors  
    opCode = 0;
    NNN    = 0;
    NN     = 0;
    N      = 0;
    X      = 0;
    Y      = 0;

    // Set Program Counter at 0x200 and reset Index register
    PC = 0x200;
    I  = 0;

    // Reset timers
    delayTimer = 0;
    soundTimer = 0;

    // Load Font Set into memory
    // Fontset examples:
    //  HEX     BIN          RESULT      HEX     BIN         RESULT
    //  0xF0    1111 0000    ****       0xF0    1111 0000    ****
    //  0x90    1001 0000    *  *       0x10    0001 0000       *
    //  0x90    1001 0000    *  *       0x20    0010 0000      *
    //  0x90    1001 0000    *  *       0x40    0100 0000     *
    //  0xF0    1111 0000    ****       0x40    0100 0000     *
    static constexpr std::array<byte, c_fontSetSize> fontSet =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    std::copy(fontSet.begin(), fontSet.end(), memory.begin());

    // Reset Draw Flag
    drawFlag = true;
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::emulateCycle()
{
    fetchOpcode();
    decodeAndExecuteOpcode();
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::fetchOpcode()
{
    // Op-Code structure example:   |   Shift memory[PC] to the left 8 bits:   |    Bitwise OR with memory[PC + 1]:
    // Opcode = 0xA2F0              |   0xA2       0xA2 << 8 = 0xA200   HEX    |    1010001000000000 |  0xA200
    // memory[PC]     == 0xA2       |   10100010   1010001000000000     BIN    |            11110000 =  0x00F0
    // memory[PC + 1] == 0xF0       |                                          |    ------------------
    //                              |                                          |    1010001011110000    0xA2F0
    opCode = memory[PC] << 8 | memory[PC + 1];

    NNN = opCode & 0x0FFF;
    NN  = opCode & 0x00FF;
    N   = opCode & 0x000F;
    X   = (opCode & 0x0F00) >> 8;
    Y   = (opCode & 0x00F0) >> 4;
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::decodeAndExecuteOpcode()
{
    const twoByte instructionIndex = opCode & 0xF000;
    opCodesTable[instructionIndex]();
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::updateTimers(bool& playSound)
{
    if (delayTimer > 0)
        --delayTimer;

    if (soundTimer > 0)
    {
        if (soundTimer == 1)
            playSound = true;

        --soundTimer;
    }
}

/////////////////////////////////////////////////////////////////////////////

void Chip8::draw()
{
    V[0xF] = 0;

    for (unsigned int yPos = 0; yPos < N; ++yPos)
    {
        const byte rowDrawStart = memory[I + yPos];
        const unsigned int spritePos1D = V[X] + (V[Y] + yPos) * c_displayWidth; // 2D to 1D indexing

        for (unsigned int xPos = 0; xPos < 8; ++xPos)
        {
            // Check if the current evaluated pixel is set to 1 (check bit from MSB to LSB each iteration)
            if ((rowDrawStart & (MSB >> xPos)) != 0)
            {
                const unsigned int pixelPos = (spritePos1D + xPos) % c_displaySize;

                // Register collision
                if (display[pixelPos] == 0xFF)
                    V[0xF] = 1;

                // Set pixel (XOR)
                display[pixelPos] ^= 0xFF;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
