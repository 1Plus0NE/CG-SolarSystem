#include "texture.h"
#include <iostream>
#include <fstream>
#include <string>
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

    std::string actualPath = path;
    std::ifstream test(actualPath);
    if (!test.is_open()) {
        actualPath = "../../figures/" + path;
        test.open(actualPath);
        if (!test.is_open()) {
            std::cerr << "Texture: cannot find '" << path << "' or '" << actualPath << "'" << std::endl;
            return 0;
        }
        test.close();
    } else {
        test.close();
    }

    ILuint ilImg = 0;
    ilGenImages(1, &ilImg);
    ilBindImage(ilImg);

    if (!ilLoadImage((ILstring)actualPath.c_str())) {
        std::cerr << "Texture: failed to load JPG '" << actualPath << "' (IL error: " << iluErrorString(ilGetError()) << ")" << std::endl;
        ilDeleteImages(1, &ilImg);
        return 0;
    }

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glBindTexture(GL_TEXTURE_2D, 0);

    ilDeleteImages(1, &ilImg);
    g_textureCache[path] = tex;
    return tex;
}
