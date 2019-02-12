#ifndef PHOS_DISPLAY_H
#define PHOS_DISPLAY_H

#include <GL/glew.h>
#include <array>
#include <iostream>

#define WIDTH 160
#define HEIGHT 144
#define SCALE 2
#define SCALED_WIDTH (WIDTH * SCALE)
#define SCALED_HEIGHT (HEIGHT * SCALE)
#define TEXTURE_SIZE (WIDTH * HEIGHT * 4)

class Display {
public:
    Display();
    ~Display();
    bool loadPixelArray(std::array<uint8_t, TEXTURE_SIZE> pixels);
    bool initGL();
    void update();
    void freeTexture();
    void render();
public:
    GLuint textureHandler;
};


#endif //PHOS_DISPLAY_H
