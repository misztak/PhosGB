#ifndef PHOS_IDISPLAY_H
#define PHOS_IDISPLAY_H

#include <GL/glew.h>
#include <SDL.h>

#include <array>

#define WIDTH 160
#define HEIGHT 144
#define SCALE 2
#define SCALED_WIDTH (WIDTH * SCALE)
#define SCALED_HEIGHT (HEIGHT * SCALE)
#define TEXTURE_SIZE (WIDTH * HEIGHT * 4)

class IDisplay {
public:
    virtual void update(std::array<uint8_t, TEXTURE_SIZE>& pixel) = 0;
    virtual void render() = 0;
    virtual void processEvent(SDL_Event& event) = 0;

    bool initTexture(std::array<uint8_t, TEXTURE_SIZE>& pixel);
    bool hasGLError();
    bool loadMainTexture();
public:
    GLuint textureHandler;
};

#endif //PHOS_IDISPLAY_H
