#include "Display.h"

Display::Display(u8* pixel) {
    textureHandler = 0;
    initTexture(pixel);
}

Display::~Display() {
    if (textureHandler != 0) {
        glDeleteTextures(1, &textureHandler);
    }
}

void Display::update(u8* pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void Display::render() {
    loadMainTexture();
}

void Display::processEvent(SDL_Event &event) {

}
