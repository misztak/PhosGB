#include "Display.h"


Display::Display(std::array<uint8_t, TEXTURE_SIZE>& pixel) {
    textureHandler = 0;
    initTexture(pixel);
}

Display::~Display() {
    if (textureHandler != 0) {
        glDeleteTextures(1, &textureHandler);
    }
}

void Display::update(std::array<uint8_t, TEXTURE_SIZE>& pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void Display::render() {
    loadMainTexture();
}

void Display::processEvent(SDL_Event &event) {

}
