#include "viewer/framework/gl/renderer.h"
#include <glad/glad.h>
#include <cstdio>
#include <cmath>
#include <cstring>

namespace cagd {

// Helper: upload a Matrix4d as float uniform (OpenGL doesn't support double uniforms in core)
static void setUniformMat4(GLint loc, const Matrix4d& m) {
    Eigen::Matrix4f mf = m.cast<float>();
    glUniformMatrix4fv(loc, 1, GL_FALSE, mf.data());
}

// ============================================================================
// Vertex shader with Phong lighting and vertex colors
// ============================================================================
static const char* kVertShader = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in uint aColor;

uniform mat4 uViewProj;
uniform mat4 uModel;

out vec3 vWorldPos;
out vec3 vNormal;
flat out uint vColor;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(uModel) * aNormal;
    vColor = aColor;
    gl_Position = uViewProj * worldPos;
}
)";

static const char* kFragShader = R"(
#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
flat in uint vColor;

uniform vec3 uLightDir;
uniform vec3 uAmbientColor;
uniform vec3 uDiffuseColor;
uniform bool uUseUniformColor;
uniform vec3 uUniformColor;
uniform bool uUseVertexColor;

out vec4 FragColor;

vec3 unpackColor(uint c) {
    float r = float((c >> 0) & 0xFFu) / 255.0;
    float g = float((c >> 8) & 0xFFu) / 255.0;
    float b = float((c >> 16) & 0xFFu) / 255.0;
    return vec3(r, g, b);
}

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lighting = uAmbientColor + uDiffuseColor * diff;

    vec3 baseColor;
    if (uUseUniformColor) {
        baseColor = uUniformColor;
    } else {
        baseColor = unpackColor(vColor);
    }

    FragColor = vec4(baseColor * lighting, 1.0);
}
)";

// ============================================================================
// Solid shader (no lighting, for wireframes and UI elements)
// ============================================================================
static const char* kSolidVertShader = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in uint aColor;

uniform mat4 uViewProj;
uniform mat4 uModel;

flat out uint vColor;

void main() {
    vColor = aColor;
    gl_Position = uViewProj * uModel * vec4(aPos, 1.0);
}
)";

static const char* kSolidFragShader = R"(
#version 330 core

flat in uint vColor;

uniform bool uUseUniformColor;
uniform vec3 uUniformColor;
uniform bool uUseVertexColor;

out vec4 FragColor;

vec3 unpackColor(uint c) {
    float r = float((c >> 0) & 0xFFu) / 255.0;
    float g = float((c >> 8) & 0xFFu) / 255.0;
    float b = float((c >> 16) & 0xFFu) / 255.0;
    return vec3(r, g, b);
}

void main() {
    if (uUseUniformColor) {
        FragColor = vec4(uUniformColor, 1.0);
    } else {
        FragColor = vec4(unpackColor(vColor), 1.0);
    }
}
)";

GLRenderer::GLRenderer()
    : shaderProgram_(0)
    , solidShaderProgram_(0)
    , tempVAO_(0)
    , tempVBO_(0)
    , tempEBO_(0)
    , viewportWidth_(0)
    , viewportHeight_(0) {}

GLRenderer::~GLRenderer() {
    if (tempVAO_) glDeleteVertexArrays(1, &tempVAO_);
    if (tempVBO_) glDeleteBuffers(1, &tempVBO_);
    if (tempEBO_) glDeleteBuffers(1, &tempEBO_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
    if (solidShaderProgram_) glDeleteProgram(solidShaderProgram_);
}

bool GLRenderer::initialize() {
    GLuint vert = compileShader(GL_VERTEX_SHADER, kVertShader);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, kFragShader);
    if (!vert || !frag) return false;
    shaderProgram_ = linkProgram(vert, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
    if (!shaderProgram_) return false;

    GLuint svert = compileShader(GL_VERTEX_SHADER, kSolidVertShader);
    GLuint sfrag = compileShader(GL_FRAGMENT_SHADER, kSolidFragShader);
    if (!svert || !sfrag) return false;
    solidShaderProgram_ = linkProgram(svert, sfrag);
    glDeleteShader(svert);
    glDeleteShader(sfrag);
    if (!solidShaderProgram_) return false;

    // Lit shader uniforms
    uniforms_.viewProj = glGetUniformLocation(shaderProgram_, "uViewProj");
    uniforms_.model = glGetUniformLocation(shaderProgram_, "uModel");
    uniforms_.lightDir = glGetUniformLocation(shaderProgram_, "uLightDir");
    uniforms_.ambientColor = glGetUniformLocation(shaderProgram_, "uAmbientColor");
    uniforms_.diffuseColor = glGetUniformLocation(shaderProgram_, "uDiffuseColor");
    uniforms_.useUniformColor = glGetUniformLocation(shaderProgram_, "uUseUniformColor");
    uniforms_.uniformColor = glGetUniformLocation(shaderProgram_, "uUniformColor");
    uniforms_.useVertexColor = glGetUniformLocation(shaderProgram_, "uUseVertexColor");

    // Solid shader uniforms
    solidUniforms_.viewProj = glGetUniformLocation(solidShaderProgram_, "uViewProj");
    solidUniforms_.model = glGetUniformLocation(solidShaderProgram_, "uModel");
    solidUniforms_.useUniformColor = glGetUniformLocation(solidShaderProgram_, "uUseUniformColor");
    solidUniforms_.uniformColor = glGetUniformLocation(solidShaderProgram_, "uUniformColor");
    solidUniforms_.useVertexColor = glGetUniformLocation(solidShaderProgram_, "uUseVertexColor");

    glGenVertexArrays(1, &tempVAO_);
    glGenBuffers(1, &tempVBO_);
    glGenBuffers(1, &tempEBO_);

    return true;
}

void GLRenderer::setViewport(int width, int height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
}

void GLRenderer::setCamera(const OrbitCamera& camera) {
    Matrix4d model = Matrix4d::Identity();
    viewProj_ = camera.projectionMatrix() * camera.viewMatrix() * model;
    invViewProj_ = viewProj_.inverse();
}

void GLRenderer::beginFrame() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f);
}

void GLRenderer::drawMesh(const Mesh& mesh) {
    if (mesh.empty()) return;
    glUseProgram(shaderProgram_);
    setUniformMat4(uniforms_.viewProj, viewProj_);
    Matrix4d identity = Matrix4d::Identity();
    setUniformMat4(uniforms_.model, identity);
    glUniform3f(uniforms_.lightDir, 0.0f, 1.0f, 0.0f);
    glUniform3f(uniforms_.ambientColor, 1.0f, 1.0f, 1.0f);
    glUniform3f(uniforms_.diffuseColor, 0.0f, 0.0f, 0.0f);
    glUniform1i(uniforms_.useUniformColor, 0);
    glUniform1i(uniforms_.useVertexColor, 1);
    uploadAndDraw(mesh, GL_TRIANGLES, shaderProgram_);
}

void GLRenderer::drawWireframe(const Mesh& mesh) {
    if (mesh.empty()) return;
    glUseProgram(solidShaderProgram_);
    setUniformMat4(solidUniforms_.viewProj, viewProj_);
    Matrix4d identity = Matrix4d::Identity();
    setUniformMat4(solidUniforms_.model, identity);
    glUniform1i(solidUniforms_.useUniformColor, 0);
    glUniform1i(solidUniforms_.useVertexColor, 1);
    uploadAndDraw(mesh, GL_LINES, solidShaderProgram_);
}

void GLRenderer::drawPoint(const Vector3d& position, float size, uint32_t color) {
    drawPointColored(position, size, color);
}

void GLRenderer::drawPointColored(const Vector3d& position, float size, uint32_t color) {
    Vertex v(position, Vector3d::UnitZ(), color);
    glUseProgram(solidShaderProgram_);
    setUniformMat4(solidUniforms_.viewProj, viewProj_);
    Matrix4d identity = Matrix4d::Identity();
    setUniformMat4(solidUniforms_.model, identity);
    glUniform1i(solidUniforms_.useUniformColor, 0);
    glUniform1i(solidUniforms_.useVertexColor, 1);

    glBindVertexArray(tempVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &v, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glPointSize(size);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}

void GLRenderer::drawPoints(const std::vector<Vector3d>& positions, float size, uint32_t color) {
    if (positions.empty()) return;

    std::vector<Vertex> vertices;
    vertices.reserve(positions.size());
    for (const auto& pos : positions) {
        vertices.push_back(Vertex(pos, Vector3d::UnitZ(), color));
    }

    glUseProgram(solidShaderProgram_);
    setUniformMat4(solidUniforms_.viewProj, viewProj_);
    Matrix4d identity = Matrix4d::Identity();
    setUniformMat4(solidUniforms_.model, identity);
    glUniform1i(solidUniforms_.useUniformColor, 0);
    glUniform1i(solidUniforms_.useVertexColor, 1);

    glBindVertexArray(tempVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glPointSize(size);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
}

void GLRenderer::drawLine(const Vector3d& from, const Vector3d& to, uint32_t color) {
    Vertex verts[2] = {
        Vertex(from, Vector3d::UnitZ(), color),
        Vertex(to, Vector3d::UnitZ(), color)
    };

    glUseProgram(solidShaderProgram_);
    setUniformMat4(solidUniforms_.viewProj, viewProj_);
    Matrix4d identity = Matrix4d::Identity();
    setUniformMat4(solidUniforms_.model, identity);
    glUniform1i(solidUniforms_.useUniformColor, 0);
    glUniform1i(solidUniforms_.useVertexColor, 1);

    glBindVertexArray(tempVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void GLRenderer::drawAxes(float length) {
    drawLine(Vector3d::Zero(), Vector3d(length, 0, 0), Color4f(1.0f, 0.2f, 0.2f, 1).toABGR());
    drawLine(Vector3d::Zero(), Vector3d(0, length, 0), Color4f(0.2f, 1.0f, 0.2f, 1).toABGR());
    drawLine(Vector3d::Zero(), Vector3d(0, 0, length), Color4f(0.2f, 0.2f, 1.0f, 1).toABGR());
}

void GLRenderer::drawGrid(float size, int divisions) {
    float step = size / divisions;
    uint32_t color = Color4f(0.7f, 0.7f, 0.75f, 0.8f).toABGR();
    uint32_t axisColor = Color4f(0.9f, 0.9f, 0.95f, 0.9f).toABGR();
    for (int i = -divisions; i <= divisions; ++i) {
        float p = i * step;
        uint32_t c = (i == 0) ? axisColor : color;
        drawLine(Vector3d(p, -size, 0), Vector3d(p, size, 0), c);
        drawLine(Vector3d(-size, p, 0), Vector3d(size, p, 0), c);
    }
}

Matrix4d GLRenderer::invViewProjMatrix() const {
    return invViewProj_;
}

GLuint GLRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint GLRenderer::linkProgram(GLuint vertShader, GLuint fragShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void GLRenderer::uploadAndDraw(const Mesh& mesh, GLenum drawMode, GLuint) {
    glBindVertexArray(tempVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO_);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_DYNAMIC_DRAW);

    if (!mesh.indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempEBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_DYNAMIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    if (!mesh.indices.empty()) {
        glDrawElements(drawMode, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(drawMode, 0, static_cast<GLsizei>(mesh.vertices.size()));
    }
    glBindVertexArray(0);
}

} // namespace cagd

