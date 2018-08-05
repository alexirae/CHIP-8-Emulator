#pragma once
#include <SDL.h>
#include <vector>

class MultimediaSystem
{
public:
    void initializeGraphics(const std::string& windowsName, const Uint32 windowW, const Uint32 windowH, const Uint32 renderTextureW, const Uint32 renderTextureH);
    void initializeSound(const std::string& soundPath);
    void initializeInput(const Uint32 numKeys);

    void renderDisplay(const std::vector<Uint8>& display);
    void handleInputEvents(bool& quit);
    void uninitialize();

    Uint32 getTicks() const { return SDL_GetTicks(); }
    void playSound()  const { SDL_QueueAudio(audioDeviceId, wavBuffer, wavLength); }
    constexpr std::vector<bool>& getUpdatedKeys() { return keys; }

    static MultimediaSystem& getInstance() { static MultimediaSystem singleton; return singleton; }

private:
    MultimediaSystem();
    MultimediaSystem(const MultimediaSystem&)            = delete;
    MultimediaSystem& operator=(const MultimediaSystem&) = delete;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* sdlTexture;

    Uint32 windowWidth;
    Uint32 windowHeight;
    Uint32 renderTextureWidth;
    Uint32 renderTextureHeight;

    Uint8* wavBuffer;
    Uint32 wavLength;
    SDL_AudioDeviceID audioDeviceId;

    std::vector<bool> keys;
    Uint32 numberOfKeys;
};
