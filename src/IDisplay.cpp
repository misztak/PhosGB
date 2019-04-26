#include "IDisplay.h"

bool IDisplay::initTexture(GLuint* textureHandler, u32 width, u32 height, u8* data) {
    glGenTextures(1, textureHandler);
    glBindTexture(GL_TEXTURE_2D, *textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return !hasGLError();
}

bool IDisplay::loadTexture(GLuint textureHandler, u32 width, u32 height) {
    if (textureHandler != 0) {
        glLoadIdentity();
        glTranslatef(0.0f,0.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, textureHandler);

        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
        glTexCoord2f(1.f, 0.f); glVertex2f(width, 0.f);
        glTexCoord2f(1.f, 1.f); glVertex2f(width, height);
        glTexCoord2f(0.f, 1.f); glVertex2f(0.f, height);
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

