#include "texture.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

static std::unordered_map<std::string, GLuint> g_textureCache;
static bool g_ilInitialized = false;

GLuint loadTextureFromFile(const std::string& path) {
    if (g_textureCache.find(path) != g_textureCache.end()) return g_textureCache[path];

    if (!g_ilInitialized) {
        ilInit();
        iluInit();
        g_ilInitialized = true;
    }

    std::ifstream test(path);
    if (!test.is_open()) {
        std::cerr << "Texture: cannot find '" << path << "'" << std::endl;
        return 0;
    }
    test.close();

    const std::string actualPath = path;

    ILuint ilImg = 0;
    ilGenImages(1, &ilImg);
    ilBindImage(ilImg);

    if (!ilLoadImage((ILstring)actualPath.c_str())) {
        std::cerr << "Texture: failed to load JPG '" << actualPath << "' (IL error: " << iluErrorString(ilGetError()) << ")" << std::endl;
        ilDeleteImages(1, &ilImg);
        return 0;
    }
    iluFlipImage();
    ILint tw = ilGetInteger(IL_IMAGE_WIDTH);
    ILint th = ilGetInteger(IL_IMAGE_HEIGHT);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    ILubyte* texData = ilGetData();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Use trilinear filtering: linear mipmap interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

    // Generate mipmaps for the texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // Enable anisotropic filtering if available to improve quality at glancing angles
#ifdef GL_EXT_texture_filter_anisotropic
    if (GLEW_EXT_texture_filter_anisotropic) {
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        // Use a reasonable value (clamp to max supported)
        GLfloat aniso = std::min(4.0f, maxAniso);
        if (aniso > 1.0f) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
        }
    }
#endif

    glBindTexture(GL_TEXTURE_2D, 0);

    ilDeleteImages(1, &ilImg);
    g_textureCache[path] = tex;
    return tex;
}
