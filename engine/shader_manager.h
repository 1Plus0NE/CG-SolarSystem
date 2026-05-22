#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <GL/glew.h>
#include "geometry.h"

// Singleton that owns the lifecycle of all dynamically-generated GLSL programs.
// One program is compiled per unique (blend:mixFactor:useChannel) key and cached
// for reuse across frames. Opacity is a per-frame uniform, not part of the key.
class ShaderManager {
public:
    static ShaderManager& instance();

    // Compile/cache and activate the shader for this model's texture layers.
    // Returns false if the model has no texture layers or compilation fails.
    bool bind(const Model& m);

    // Deactivate shader and unbind all texture units used by bind().
    void unbind(int layerCount);

    // Snapshot the first point-light position in eye space (call once per frame).
    void updateLightPosition();

    // Delete all cached GL programs; call on config reload or shutdown.
    void cleanup();

private:
    ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    struct CachedShader {
        GLuint programId;
        std::vector<TextureLayer> layers;
    };

    std::map<std::string, CachedShader> cache_;
    GLfloat lightPosEye_[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    std::string makeKey(const std::vector<TextureLayer>& layers) const;
    std::string buildFragmentSource(const std::vector<TextureLayer>& layers) const;
    GLuint compileStage(GLenum type, const char* src) const;
    GLuint linkProgram(GLuint vs, GLuint fs) const;
    GLuint getOrCompile(const std::vector<TextureLayer>& layers);
};

#endif // SHADER_MANAGER_H
