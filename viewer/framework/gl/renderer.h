#pragma once

// Lightweight OpenGL 3D renderer for CAGD visualization
// Handles shader compilation, mesh upload, and rendering with basic Phong lighting

#include "cagd_types.h"
#include "viewer/framework/gl/camera.h"
#include <glad/glad.h>
#include <cstdint>

namespace cagd {

class GLRenderer {
public:
    GLRenderer();
    ~GLRenderer();

    // Initialize OpenGL state and compile shaders
    // Call after OpenGL context is created
    bool initialize();

    // Set viewport size
    void setViewport(int width, int height);

    // Set camera
    void setCamera(const OrbitCamera& camera);

    // Begin frame - clear and setup state
    void beginFrame();

    // Draw a filled mesh with lighting
    void drawMesh(const Mesh& mesh);

    // Draw a mesh as wireframe (GL_LINES)
    void drawWireframe(const Mesh& mesh);

    // Draw a single point
    void drawPoint(const Vector3d& position, float size, uint32_t color);

    // Draw an array of points
    void drawPoints(const std::vector<Vector3d>& positions, float size, uint32_t color);

    // Draw a point with custom color
    void drawPointColored(const Vector3d& position, float size, uint32_t color);

    // Draw a line between two points
    void drawLine(const Vector3d& from, const Vector3d& to, uint32_t color);

    // Draw coordinate axes at origin
    void drawAxes(float length = 2.0f);

    // Draw a ground grid
    void drawGrid(float size = 10.0f, int divisions = 10);

    // Get the inverse view-projection matrix (for unprojection/picking)
    Matrix4d invViewProjMatrix() const;

private:
    // Shader program handles
    GLuint shaderProgram_;
    GLuint solidShaderProgram_;

    // Uniform locations for main shader
    struct Uniforms {
        GLuint viewProj;
        GLuint model;
        GLuint lightDir;
        GLuint ambientColor;
        GLuint diffuseColor;
        GLuint useUniformColor;
        GLuint uniformColor;
        GLuint useVertexColor;
    };
    Uniforms uniforms_;

    // Uniform locations for solid (no lighting) shader
    struct SolidUniforms {
        GLuint viewProj;
        GLuint model;
        GLuint useUniformColor;
        GLuint uniformColor;
        GLuint useVertexColor;
    };
    SolidUniforms solidUniforms_;

    // Temporary VAO/VBO for dynamic drawing
    GLuint tempVAO_;
    GLuint tempVBO_;
    GLuint tempEBO_;

    // Cached matrices
    Matrix4d viewProj_;
    Matrix4d invViewProj_;

    // Viewport
    int viewportWidth_;
    int viewportHeight_;

    // Compile a shader from source
    GLuint compileShader(GLenum type, const char* source);

    // Link a shader program
    GLuint linkProgram(GLuint vertShader, GLuint fragShader);

    // Upload mesh data to GPU and draw
    void uploadAndDraw(const Mesh& mesh, GLenum drawMode, GLuint program);
};

} // namespace cagd
