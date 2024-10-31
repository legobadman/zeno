#include <zenovis/Camera.h>
#include <zenovis/Scene.h>
#include <zenovis/bate/IGraphic.h>
#include <zenovis/bate/GraphicHandlerUtils.h>
#include <zenovis/ShaderManager.h>
#include <zenovis/opengl/shader.h>
#include <zenovis/opengl/scope.h>
#include <glm/gtx/transform.hpp>
#include <zeno/types/GeometryObject.h>
#include <GL/freeglut.h>


namespace zenovis {
namespace {

using opengl::Buffer;
using opengl::Program;
using zeno::vec3f;

static const char* vert_code = R"(
    #version 120

    uniform mat4 mVP;
    uniform mat4 mInvVP;
    uniform mat4 mView;
    uniform mat4 mProj;
    uniform mat4 mInvView;
    uniform mat4 mInvProj;

    uniform mat4 mScale;

    attribute vec3 vPosition;

    varying vec3 position;

    void main() {
        position = vPosition;
        gl_Position = mVP * vec4(position, 1.0);
    }
)";

static const char* frag_code = R"(
    #version 120

    uniform mat4 mVP;
    uniform mat4 mInvVP;
    uniform mat4 mView;
    uniform mat4 mProj;
    uniform mat4 mInvView;
    uniform mat4 mInvProj;

    varying vec3 position;

    void main() {
        gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }
)";

struct PointIndicator final : IGraphicDraw {

    using CHAR_VBO_DATA = std::vector<GLfloat>;

    Scene* scene;
    std::vector<zeno::vec3f> m_pos;
    Program* lines_prog;
    std::unique_ptr<Buffer> vbo;
    std::vector<CHAR_VBO_DATA> m_data;

    explicit PointIndicator(zeno::GeometryObject* pObject, Scene* scene_) : scene(scene_) {
        m_pos = pObject->get_attrs<zeno::vec3f>(zeno::ATTR_POINT, "pos");
        vbo = std::make_unique<Buffer>(GL_ARRAY_BUFFER);
        init_all_points();
    }

    void init_all_points() {
        size_t n = m_pos.size();
        for (int iPoint = 0; iPoint < n; iPoint++) {
            //if (iPoint > 0) {
            //    continue;
            //}
            zeno::vec3f basepos = m_pos[iPoint];
            //float x = basepos[0], y = basepos[1], z = basepos[2];
            std::string drawNum = std::to_string(iPoint);
            GLfloat arr[1024];
            int size = 0;
            int arr_split[64];
            int split_size = 0;
            glutGetStrokeString(GLUT_STROKE_MONO_ROMAN, drawNum.c_str(), arr, &size, arr_split, &split_size);

            //找出整个字符串数字顶点的包围盒
            GLfloat xmin = 100000, ymin = 100000, xmax = -10000, ymax = -10000;
            for (int i = 0; i < size; i++) {
                if (i % 2 == 0) {
                    xmin = std::min(arr[i], xmin);
                    xmax = std::max(arr[i], xmax);
                }
                else {
                    ymin = std::min(arr[i], ymin);
                    ymax = std::max(arr[i], ymax);
                }
            }
            GLfloat width = xmax - xmin, height = ymax - ymin;
            for (int i = 0; i < size; i++) {
                if (i % 2 == 0) {
                    GLfloat xp = arr[i];
                    xp -= xmin;
                    xp /= width;
                    xp *= 0.03;
                    arr[i] = xp;
                }
                else {
                    GLfloat yp = arr[i];
                    yp -= ymin;
                    yp /= height;
                    yp *= 0.03;
                    arr[i] = yp;
                }
            }

            for (int j = 0; j < split_size; j++) {
                int i_start = (j == 0) ? 0 : arr_split[j - 1];
                int i_end = arr_split[j];
                int mem_size = i_end - i_start;
                assert(mem_size % 2 == 0);
                int nVertexs = mem_size / 2;
                CHAR_VBO_DATA mem;
                mem.reserve(nVertexs * 3);
                for (int k = 0; k < nVertexs; k++) {
                    GLfloat xp = basepos[0] + arr[i_start + k * 2];
                    GLfloat yp = basepos[1] + arr[i_start + k * 2 + 1];
                    GLfloat zp = basepos[2];
                    mem.push_back(xp);
                    mem.push_back(yp);
                    mem.push_back(zp);
                }
                m_data.push_back(mem);
            }
        }
    }

    void init_all_points2() {
        size_t n = m_pos.size();
        for (int i = 0; i < n; i++) {
            if (i == 0)
            {
                zeno::vec3f basepos = m_pos[i];
                float x = basepos[0], y = basepos[1], z = basepos[2];
                GLfloat arr[1024];
                int size = 0;
                GLfloat _basepos[3] = { x,y,z };
                std::string drawNum = std::to_string(i);
                drawNum = "012345678";
                int arr_split[64];
                int split_size = 0;
                glutGetStrokeString2(GLUT_STROKE_MONO_ROMAN, _basepos, drawNum.c_str(), arr, &size, arr_split, &split_size);

                for (int j = 0; j < split_size; j++) {
                    int i_start = (j == 0) ? 0 : arr_split[j - 1];
                    int i_end = arr_split[j];
                    int mem_size = i_end - i_start;
                    CHAR_VBO_DATA mem(mem_size);
                    for (int k = i_start; k < i_end; k++) {
                        mem[k - i_start] = arr[k];
                    }
                    m_data.push_back(mem);
                }
            }
        }
    }

    void draw() override {
        lines_prog = scene->shaderMan->compile_program(vert_code, frag_code);
        lines_prog->use();
        scene->camera->set_program_uniforms(lines_prog);

        for (const auto& vbo_data : m_data) {
            vbo->bind();
            vbo->bind_data(vbo_data.data(), vbo_data.size() * sizeof(GLfloat));
            vbo->attribute(0, sizeof(float) * 0, sizeof(GLfloat) * 3, GL_FLOAT, 3);
            CHECK_GL(glDrawArrays(GL_LINE_STRIP, 0, vbo_data.size() / 3));
            vbo->unbind();
        }

        size_t n = m_pos.size();
        for (int i = 0; i < n; i++) {
            zeno::vec3f basepos = m_pos[i];
            float x = basepos[0], y = basepos[1], z = basepos[2];

            if (0)
            {
                //尝试引入font
                //SFG_StrokeFont* font;
                GLfloat arr[1024];
                int size = 0;
                GLfloat basepos[3] = { x,y,z };
                std::string drawNum = std::to_string(i);
                drawNum = "4";
                int arr_split[64];
                int split_size = 0;
                glutGetStrokeString2(GLUT_STROKE_MONO_ROMAN, basepos, drawNum.c_str(), arr, &size, arr_split, &split_size);

                for (int j = 0; j < split_size; j++) {
                    int i_start = (j == 0) ? 0 : arr_split[j - 1];
                    int i_end = arr_split[j];
                    int mem_size = i_end - i_start;
                    std::vector<GLfloat> mem(mem_size);
                    for (int k = i_start; k < i_end; k++) {
                        mem[k - i_start] = arr[k];
                    }
                    vbo->bind();
                    vbo->bind_data(mem.data(), mem.size() * sizeof(GLfloat));
                    vbo->attribute(0, sizeof(float) * 0, sizeof(GLfloat) * 3, GL_FLOAT, 3);
                    CHECK_GL(glDrawArrays(GL_LINE_STRIP, 0, mem.size() / 3));
                    vbo->unbind();
                }
            }

            if (0)
            {
                //先画一个cube来看看
                std::vector<zeno::vec3f> mem;
                mem.push_back(basepos);
                mem.push_back(zeno::vec3f(x, y - 0.05, z));
                mem.push_back(zeno::vec3f(x, y - 0.05, z - 0.05));
                mem.push_back(zeno::vec3f(x, y, z - 0.05));

                vbo->bind();
                vbo->bind_data(mem.data(), mem.size() * sizeof(mem[0]));
                vbo->attribute(0, sizeof(float) * 0, sizeof(float) * 3, GL_FLOAT, 3);
                CHECK_GL(glDrawArrays(GL_LINE_STRIP, 0, 4));
                vbo->disable_attribute(0);
                vbo->unbind();
            }
        }
    }
};

}

std::shared_ptr<IGraphicDraw> makePointIndicators(Scene* scene, zeno::GeometryObject* pObject) {
    return std::make_shared<PointIndicator>(pObject, scene);
}

}