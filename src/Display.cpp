#include "Display.h"


Display::Display() {
    textureHandler = 0;
}

Display::~Display() {
    freeTexture();
}

bool Display::loadPixelArray(std::array<uint8_t, TEXTURE_SIZE>& pixels) {
    freeTexture();
    glGenTextures(1, &textureHandler);
    glBindTexture(GL_TEXTURE_2D, textureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u", error);
        return false;
    }

    return true;
}

bool Display::initGL() {
    glViewport(0, 0, SCALED_WIDTH, SCALED_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0,0,0,255);
    glEnable(GL_TEXTURE_2D);

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u", error);
        return false;
    }

    return true;
}

void Display::update() {

}

void Display::freeTexture() {
    if (textureHandler != 0) {
        glDeleteTextures(1, &textureHandler);
    }
}

void Display::render() {
    if (textureHandler != 0) {
        glLoadIdentity();
        glTranslatef(0.0f,0.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, textureHandler);

        glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
            glTexCoord2f(1.f, 0.f); glVertex2f(WIDTH, 0.f);
            glTexCoord2f(1.f, 1.f); glVertex2f(WIDTH, HEIGHT);
            glTexCoord2f(0.f, 1.f); glVertex2f(0.f, HEIGHT);
        glEnd();
    }
}
