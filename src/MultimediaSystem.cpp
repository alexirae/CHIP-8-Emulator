#include "MultimediaSystem.h"

MultimediaSystem::MultimediaSystem()
    : window(nullptr)
    , renderer(nullptr)
    , sdlTexture(nullptr)
    , windowWidth(0)
    , windowHeight(0)
    , renderTextureWidth(0)
    , renderTextureHeight(0)
    , wavBuffer(nullptr)
    , wavLength(0)
    , audioDeviceId(0)
    , numberOfKeys(0)
{
    SDL_Init(SDL_INIT_EVERYTHING);
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::initializeGraphics(const std::string& windowsName, const Uint32 windowW, const Uint32 windowH, const Uint32 renderTextureW, const Uint32 renderTextureH)
{
    windowWidth  = windowW;
    windowHeight = windowH;

    renderTextureWidth  = renderTextureW;
    renderTextureHeight = renderTextureH;

    window   = SDL_CreateWindow(windowsName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, renderTextureWidth, renderTextureHeight);
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::initializeSound(const std::string& soundPath)
{
    SDL_AudioSpec wavSpec;
    SDL_LoadWAV(soundPath.c_str(), &wavSpec, &wavBuffer, &wavLength);

    audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, 0);
    SDL_PauseAudioDevice(audioDeviceId, 0);
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::initializeInput(const Uint32 numKeys)
{
    numberOfKeys = numKeys;
    keys.resize(numKeys, false);
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::renderDisplay(const std::vector<Uint8>& display)
{
    SDL_UpdateTexture(sdlTexture, nullptr, &display.front(), renderTextureWidth * sizeof(Uint8));
    SDL_RenderCopy(renderer, sdlTexture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::handleInputEvents(bool& quit)
{
    SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent))
    {
        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

        if (currentKeyStates[SDL_SCANCODE_ESCAPE] || sdlEvent.type == SDL_QUIT)
        {
            quit = true;
            return;
        }

        keys[0]  = currentKeyStates[SDL_SCANCODE_1];
        keys[1]  = currentKeyStates[SDL_SCANCODE_2];
        keys[2]  = currentKeyStates[SDL_SCANCODE_3];
        keys[3]  = currentKeyStates[SDL_SCANCODE_4];
                
        keys[4]  = currentKeyStates[SDL_SCANCODE_Q];
        keys[5]  = currentKeyStates[SDL_SCANCODE_W];
        keys[6]  = currentKeyStates[SDL_SCANCODE_E];
        keys[7]  = currentKeyStates[SDL_SCANCODE_R];

        keys[8]  = currentKeyStates[SDL_SCANCODE_A];
        keys[9]  = currentKeyStates[SDL_SCANCODE_S];
        keys[10] = currentKeyStates[SDL_SCANCODE_D];
        keys[11] = currentKeyStates[SDL_SCANCODE_F];

        keys[12] = currentKeyStates[SDL_SCANCODE_Z];
        keys[13] = currentKeyStates[SDL_SCANCODE_X];
        keys[14] = currentKeyStates[SDL_SCANCODE_C];
        keys[15] = currentKeyStates[SDL_SCANCODE_V];
    }
}

/////////////////////////////////////////////////////////////////////////////

void MultimediaSystem::uninitialize()
{
    SDL_CloseAudioDevice(audioDeviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();
}

/////////////////////////////////////////////////////////////////////////////
