#include "Display.h"

Display::Display(u8* pixel) {
    mainTextureHandler = 0;
    initTexture(&mainTextureHandler, WIDTH, HEIGHT, pixel);
}

Display::~Display() {
    if (mainTextureHandler != 0) {
        glDeleteTextures(1, &mainTextureHandler);
    }
}

void Display::update(u8* pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void Display::render() {
    loadTexture(mainTextureHandler, WIDTH, HEIGHT);
}

void Display::processEvent(SDL_Event &event) {

}
