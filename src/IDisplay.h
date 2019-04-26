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
    virtual void update(u8* data) = 0;
    virtual void render() = 0;
    virtual void processEvent(SDL_Event& event) = 0;

    bool initTexture(GLuint* textureHandler, u32 width, u32 height, u8* data);
    bool hasGLError();
    bool loadTexture(GLuint textureHandler, u32 width, u32 height);
public:
    GLuint mainTextureHandler;
};

#endif //PHOS_IDISPLAY_H
