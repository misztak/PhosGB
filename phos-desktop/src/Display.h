#ifndef PHOS_DISPLAY_H
#define PHOS_DISPLAY_H

#include <array>

#include "IDisplay.h"

class Display : public IDisplay {
public:
    Display(SDL_Window* window, Emulator* emu);
    ~Display();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(u8* data) override;
private:
    void showMenuPopup();
};


#endif //PHOS_DISPLAY_H
