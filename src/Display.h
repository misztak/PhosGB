#ifndef PHOS_DISPLAY_H
#define PHOS_DISPLAY_H

#include <array>

#include "IDisplay.h"

class Display : public IDisplay {
public:
    Display(std::array<uint8_t, TEXTURE_SIZE>& pixel);
    ~Display();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(std::array<uint8_t, TEXTURE_SIZE>& pixels) override;
};


#endif //PHOS_DISPLAY_H
