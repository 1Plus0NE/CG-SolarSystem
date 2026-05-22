#include "shader_manager.h"
#include "config.h"
#include "texture.h"
#include "application_state.h"
#include "math_helpers.h"
#include "log_system.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <GL/glew.h>

using namespace std;

// ============================================================================
// PATH HELPERS
// ============================================================================

static string directoryOf(const string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos) return string();
    return path.substr(0, pos + 1);
}

static string resolveTexturePath(const string& textureFile) {
    if (textureFile.empty()) return textureFile;
    if (textureFile.find('/') != string::npos || textureFile.find('\\') != string::npos)
        return directoryOf(currentConfigFile) + textureFile;
    return directoryOf(currentConfigFile) + "textures/" + textureFile;
}

// ============================================================================
// ShaderManager
// ============================================================================

ShaderManager& ShaderManager::instance() {
    static ShaderManager sm;
    return sm;
}

// Opacity excluded: it is a per-frame uniform, not a structural variation.
string ShaderManager::makeKey(const vector<TextureLayer>& layers) const {
    string key;
    for (const auto& l : layers) {
        if (!key.empty()) key += "|";
        key += l.blend + ":" + l.mixFactor + ":" + l.useChannel;
    }
    return key;
}

string ShaderManager::buildFragmentSource(const vector<TextureLayer>& layers) const {
    stringstream ss;
    int n = (int)layers.size();

    ss << "#version 120\n";
    for (int i = 0; i < n; i++)
        ss << "uniform sampler2D u_tex_" << i << ";\n";
    ss << "uniform vec4  uLightPosEye;\n";
    ss << "uniform vec3  uGlobalAmbient;\n";
    for (int i = 0; i < n; i++)
        ss << "uniform float u_opacity_" << i << ";\n";

    ss << R"GLSL(
varying vec3 vNormalEye;
varying vec3 vPosEye;
varying vec2 vTexCoord;

void main() {
    vec3 N = normalize(vNormalEye);
    vec3 L = (uLightPosEye.w < 0.5)
           ? normalize(uLightPosEye.xyz)
           : normalize(uLightPosEye.xyz - vPosEye);
    vec3 V = normalize(-vPosEye);
    vec3 H = normalize(L + V);
    float ndl        = max(dot(N, L), 0.0);
    float rawSpec    = pow(max(dot(N, H), 0.0), max(gl_FrontMaterial.shininess, 1.0));
    float specFactor = rawSpec * sign(ndl);

    vec3 color = gl_FrontMaterial.ambient.rgb  * uGlobalAmbient
               + gl_FrontMaterial.diffuse.rgb  * ndl
               + gl_FrontMaterial.specular.rgb * specFactor
               + gl_FrontMaterial.emission.rgb;
)GLSL";

    for (int i = 0; i < n; i++) {
        const auto& l = layers[i];
        ss << "\n    // --- layer " << i << " role=\"" << l.role
           << "\" blend=" << l.blend << " mixFactor=" << l.mixFactor
           << " useChannel=" << l.useChannel << "\n";

        ss << "    vec4 raw" << i << " = texture2D(u_tex_" << i << ", vTexCoord);\n";

        string ch = l.useChannel.empty() ? "rgb" : l.useChannel;
        if      (ch == "r") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".r);\n";
        else if (ch == "g") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".g);\n";
        else if (ch == "b") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".b);\n";
        else if (ch == "a") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".a);\n";
        else                ss << "    vec3 samp" << i << " = raw" << i << ".rgb;\n";

        string mf = l.mixFactor.empty() ? "1.0" : l.mixFactor;
        if      (mf == "ndl")      ss << "    float fac" << i << " = ndl;\n";
        else if (mf == "1-ndl")    ss << "    float fac" << i << " = 1.0 - ndl;\n";
        else if (mf == "specular") ss << "    float fac" << i << " = specFactor;\n";
        else                       ss << "    float fac" << i << " = " << mf << ";\n";

        string w  = "fac" + to_string(i) + " * u_opacity_" + to_string(i);
        string bm = l.blend.empty() ? "mix" : l.blend;
        if (bm == "multiply") {
            ss << "    color = mix(color, color * samp" << i << ", " << w << ");\n";
        } else if (bm == "add") {
            ss << "    color += samp" << i << " * " << w << ";\n";
        } else if (bm == "overlay") {
            ss << "    vec3 ov" << i << " = mix(2.0*color*samp" << i
               << ", 1.0-2.0*(1.0-color)*(1.0-samp" << i << "), step(0.5, color));\n";
            ss << "    color = mix(color, ov" << i << ", " << w << ");\n";
        } else { // mix / replace / default
            ss << "    color = mix(color, samp" << i << ", " << w << ");\n";
        }
    }

    ss << "\n    gl_FragColor = vec4(clamp(color, 0.0, 1.0), gl_FrontMaterial.diffuse.a);\n}\n";
    return ss.str();
}

GLuint ShaderManager::compileStage(GLenum type, const char* src) const {
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        vector<char> log(max(1, logLen));
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        LOG_ERROR("Shader compile error: " << log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint ShaderManager::linkProgram(GLuint vs, GLuint fs) const {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        vector<char> log(max(1, logLen));
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        LOG_ERROR("Shader link error: " << log.data());
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint ShaderManager::getOrCompile(const vector<TextureLayer>& layers) {
    string key = makeKey(layers);

    auto it = cache_.find(key);
    if (it != cache_.end())
        return it->second.programId;

    static const char* vertexSrc = R"GLSL(
#version 120
varying vec3 vNormalEye;
varying vec3 vPosEye;
varying vec2 vTexCoord;

void main() {
    vNormalEye = normalize(gl_NormalMatrix * gl_Normal);
    vec4 posEye = gl_ModelViewMatrix * gl_Vertex;
    vPosEye = posEye.xyz;
    vTexCoord = gl_MultiTexCoord0.st;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)GLSL";

    string fragmentSrc = buildFragmentSource(layers);
    GLuint vs = compileStage(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileStage(GL_FRAGMENT_SHADER, fragmentSrc.c_str());

    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    GLuint program = linkProgram(vs, fs);
    if (!program) return 0;

    CachedShader cs;
    cs.programId = program;
    cs.layers    = layers;
    cache_[key]  = cs;
    return program;
}

bool ShaderManager::bind(const Model& m) {
    if (m.textureLayers.empty()) return false;

    GLuint program = getOrCompile(m.textureLayers);
    if (!program) return false;

    glUseProgram(program);

    GLint lightLoc = glGetUniformLocation(program, "uLightPosEye");
    GLint ambLoc   = glGetUniformLocation(program, "uGlobalAmbient");
    glUniform4fv(lightLoc, 1, lightPosEye_);
    glUniform3f(ambLoc, sceneGlobalAmbient[0], sceneGlobalAmbient[1], sceneGlobalAmbient[2]);

    for (int i = 0; i < (int)m.textureLayers.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint texId = loadTextureFromFile(resolveTexturePath(m.textureLayers[i].file));
        glBindTexture(GL_TEXTURE_2D, texId);

        string sampName = "u_tex_"     + to_string(i);
        string opName   = "u_opacity_" + to_string(i);
        glUniform1i(glGetUniformLocation(program, sampName.c_str()), i);
        glUniform1f(glGetUniformLocation(program, opName.c_str()),   m.textureLayers[i].opacity);
    }

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    return true;
}

void ShaderManager::unbind(int layerCount) {
    glUseProgram(0);
    for (int i = 0; i < layerCount; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
}

void ShaderManager::updateLightPosition() {
    lightPosEye_[0] = 0.0f;
    lightPosEye_[1] = 0.0f;
    lightPosEye_[2] = 0.0f;
    lightPosEye_[3] = 1.0f;

    const Light* light = nullptr;
    for (const auto& candidate : sceneLights) {
        if (candidate.type == Light::Type::LT_POINT) {
            light = &candidate;
            break;
        }
    }
    if (!light) return;

    GLfloat modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    GLfloat lightWorld[4] = {light->x, light->y, light->z, 1.0f};
    mat4_transform_point(modelView, lightWorld, lightPosEye_);
}

void ShaderManager::cleanup() {
    for (auto& entry : cache_)
        glDeleteProgram(entry.second.programId);
    cache_.clear();
}
