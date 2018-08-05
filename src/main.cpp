#include <iostream>
#include "MultimediaSystem.h"
#include "Chip8.h"

int main(int argc, char* argv[])
{
    // Check if the name of the game was sent as an argument
    if (argc != 2)
    {
        std::cout << "No game loaded. Usage: CHIP8_Emulator.exe <game> \n";
        std::system("pause");
        return 1;
    } 
    
    // Initialize Systems
    MultimediaSystem& multimediaSystem = MultimediaSystem::getInstance();

    multimediaSystem.initializeGraphics("CHIP-8 Emulator", 640, 320, Chip8::c_displayWidth, Chip8::c_displayHeight);
    multimediaSystem.initializeSound("beep.wav");
    multimediaSystem.initializeInput(Chip8::c_numKeys);

    Chip8 chip8;
    chip8.initialize();

    // Load game
    const std::string& gamePath(argv[1]);

    if (!chip8.loadGame(gamePath))
    {
        std::cout << "Failed to load game. Check that the game name is spelled correctly or try to load a different game. \n";
        std::system("pause");
        return 1;
    }

    // CHIP-8 Loop
    double currentTime = multimediaSystem.getTicks();
    double accumTime   = 0.0;

    const double timeStep = 16.6666;                // 60 Hz in ms.
    const unsigned int chip8CycleFrequency = 10;    // 600 Hz

    bool quit = false;

    while (!quit)
    {
        const double newTime = multimediaSystem.getTicks();
        accumTime  += newTime - currentTime;
        currentTime = newTime;

        while (accumTime >= timeStep)
        {
            accumTime -= timeStep;

            multimediaSystem.handleInputEvents(quit);

            if (quit)
                break;

            chip8.setKeys(multimediaSystem.getUpdatedKeys());

            for (unsigned int i = 0; i < chip8CycleFrequency; ++i)
                chip8.emulateCycle();

            bool playSoundNeeded = false;
            chip8.updateTimers(playSoundNeeded);

            if (playSoundNeeded)
                multimediaSystem.playSound();

            if (chip8.getDrawFlag())
            {
                multimediaSystem.renderDisplay(chip8.getDisplay());
                chip8.setDrawFlagFalse();
            }
        }
    }

    multimediaSystem.uninitialize();

    return 0;
}
