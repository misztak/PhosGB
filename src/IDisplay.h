#ifndef PHOS_IDISPLAY_H
#define PHOS_IDISPLAY_H

#include <GL/glew.h>
#include <SDL.h>

#include "Common.h"

constexpr int SCALE = 2;
constexpr int SCALED_WIDTH = WIDTH * SCALE;
constexpr int SCALED_HEIGHT = HEIGHT * SCALE;

class IDisplay {
public:
    virtual void update(u8* pixel) = 0;
    virtual void render() = 0;
    virtual void processEvent(SDL_Event& event) = 0;

    bool initTexture(u8* pixel);
    bool hasGLError();
    bool loadMainTexture();
public:
    GLuint textureHandler;
};

#endif //PHOS_IDISPLAY_H
