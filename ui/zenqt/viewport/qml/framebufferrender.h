#pragma once

#include <zenovis/Scene.h>
#include <zenovis/Camera.h>
#include <QObject>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>


static const char* vert_code = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoords;

    out vec2 TexCoords;

    void main()
    {
        TexCoords = aTexCoords;
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

static const char* frag_code = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoords;

    uniform sampler2D screenTexture;

    void main()
    {
        vec3 col = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(col, 1.0);
    }
)";

static float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};



struct FrameBufferRender {
    zenovis::Scene* scene;

    QScopedPointer<QOpenGLFramebufferObject> fbo;
    QScopedPointer<QOpenGLTexture> picking_texture;
    QScopedPointer<QOpenGLTexture> depth_texture;

    QScopedPointer<QOpenGLFramebufferObject> intermediate_fbo;
    QScopedPointer<QOpenGLTexture> screen_depth_tex;
    QScopedPointer<QOpenGLTexture> screen_tex;

    QScopedPointer<QOpenGLVertexArrayObject> quad_vao;
    QScopedPointer<QOpenGLBuffer> quad_vbo;
    QScopedPointer<QOpenGLShaderProgram> shader;

    int w = 0;
    int h = 0;
    int samples = 16;

    explicit FrameBufferRender(zenovis::Scene* s) : scene(s) {
        QOpenGLFunctions* functions = QOpenGLContext::currentContext()->functions();

        shader.reset(new QOpenGLShaderProgram);
        if (!shader->create())
            qFatal("Unable to create shader program");
        if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vert_code))
            qFatal("Vertex shader compilation failed");
        if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, frag_code))
            qFatal("Fragment shader compilation failed");
        if (!shader->link())
            qFatal("Shader program not linked");
        shader->bind();
        shader->setUniformValue("screenTexture", 0);

        quad_vbo.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
        quad_vao.reset(new QOpenGLVertexArrayObject);

        quad_vbo->bind();
        quad_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
        quad_vbo->allocate(quadVertices, sizeof(quadVertices));

        functions->glEnableVertexAttribArray(0);
        functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        functions->glEnableVertexAttribArray(1);
        functions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    ~FrameBufferRender() {

    }

    void generate_buffers() {
        QOpenGLFunctions* functions = QOpenGLContext::currentContext()->functions();

        // get viewport size
        w = scene->camera->m_nx;
        h = scene->camera->m_ny;

        fbo.reset(new QOpenGLFramebufferObject(w, h));

        // generate picking texture
        picking_texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2DMultisample));
        picking_texture->bind();
        picking_texture->setSamples(samples);

    }

};