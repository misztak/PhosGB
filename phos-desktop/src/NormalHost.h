#ifndef PHOS_NORMALHOST_H
#define PHOS_NORMALHOST_H

#include "Host.h"

class NormalHost : public Host {
public:
    NormalHost(SDL_Window* window, Emulator* emu, SDL_AudioDeviceID deviceId);
    ~NormalHost();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(u8* data) override;
};


#endif //PHOS_NORMALHOST_H
