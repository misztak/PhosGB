#include "IDisplay.h"

bool IDisplay::initTexture(std::array<uint8_t, TEXTURE_SIZE> &pixel) {
    glGenTextures(1, &textureHandler);
    glBindTexture(GL_TEXTURE_2D, textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return !hasGLError();
}

bool IDisplay::loadMainTexture() {
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
    return !hasGLError();
}

bool IDisplay::hasGLError() {
    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u", error);
        return true;
    }
    return false;
}

