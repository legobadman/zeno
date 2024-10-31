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

using CHAR_VBO_DATA = std::vector<glm::vec3>;

struct CHAR_VBO_INFO {
    glm::vec3 pos;
    CHAR_VBO_DATA m_data;
};

struct PointIndicator final : IGraphicDraw {
    Scene* scene;
    std::vector<zeno::vec3f> m_pos;
    Program* lines_prog;
    std::unique_ptr<Buffer> vbo;
    std::vector<CHAR_VBO_INFO> m_data;

    explicit PointIndicator(zeno::GeometryObject* pObject, Scene* scene_) : scene(scene_) {
        m_pos = pObject->get_attrs<zeno::vec3f>(zeno::ATTR_POINT, "pos");
        vbo = std::make_unique<Buffer>(GL_ARRAY_BUFFER);
        init_all_points(pObject);
    }

    void init_all_points(zeno::GeometryObject* pObject) {
        size_t n = m_pos.size();
        for (int iPoint = 0; iPoint < n; iPoint++) {
            //if (iPoint != 12) {
            //    continue;
            //}
            zeno::vec3f basepos = m_pos[iPoint];
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
                    xp /= (width/2);
                    xp *= 0.01;
                    arr[i] = xp;
                }
                else {
                    GLfloat yp = arr[i];
                    yp -= ymin;
                    yp /= (height*1.5);
                    yp *= 0.01;
                    arr[i] = yp;
                }
            }

            for (int j = 0; j < split_size; j++) {
                int i_start = (j == 0) ? 0 : arr_split[j - 1];
                int i_end = arr_split[j];
                int mem_size = i_end - i_start;
                assert(mem_size % 2 == 0);
                int nVertexs = mem_size / 2;
                CHAR_VBO_DATA mem(nVertexs);
                for (int k = 0; k < nVertexs; k++) {
                    GLfloat xp = arr[i_start + k * 2];
                    GLfloat yp = arr[i_start + k * 2 + 1];
                    GLfloat zp = 0;
                    //先放置在原点，待会再实施旋转和平移
                    mem[k] = glm::vec3(xp, yp, zp);
                }
                CHAR_VBO_INFO info;
                info.m_data = mem;
                info.pos = glm::vec3(basepos[0], basepos[1], basepos[2]);
                m_data.emplace_back(info);
            }
        }
    }

    void draw() override {
        lines_prog = scene->shaderMan->compile_program(vert_code, frag_code);
        lines_prog->use();
        scene->camera->set_program_uniforms(lines_prog);

        glm::vec3 lodfront = scene->camera->get_lodfront();
        glm::vec3 lodup = scene->camera->get_lodup();
        glm::vec3 cam_pos = scene->camera->getPos();
        glm::vec3 pivot = scene->camera->getPivot();

        //zeno::log_info("\n\n\n\n\n");
        //zeno::log_info("lodfront: x={}, y={}, z={}", lodfront[0], lodfront[1], lodfront[2]);
        //zeno::log_info("lodup: x={}, y={}, z={}", lodup[0], lodup[1], lodup[2]);
        //zeno::log_info("pos: x={}, y={}, z={}", cam_pos[0], cam_pos[1], cam_pos[2]);
        //zeno::log_info("pivot: x={}, y={}, z={}", pivot[0], pivot[1], pivot[2]);
        //zeno::log_info("\n\n\n\n\n");

        glm::vec3 uz = glm::normalize(cam_pos);
        glm::vec3 uy = glm::normalize(lodup);
        glm::vec3 ux = glm::normalize(glm::cross(uy, uz));
        glm::mat3 rotateM(ux, uy, uz);

        for (CHAR_VBO_INFO& vbo_data : m_data) {
            int n = vbo_data.m_data.size();
            CHAR_VBO_DATA new_vbodata(n);
            for (int i = 0; i < n; i++) {
                auto& pos = vbo_data.m_data[i];
                glm::vec3 newpos = rotateM * pos + vbo_data.pos;
                newpos += (cam_pos - newpos) * 0.01f;
                new_vbodata[i] = newpos;
            }
            vbo->bind();
            vbo->bind_data(new_vbodata.data(), new_vbodata.size() * sizeof(new_vbodata[0]));
            vbo->attribute(0, 0, sizeof(GLfloat) * 3, GL_FLOAT, 3);
            CHECK_GL(glDrawArrays(GL_LINE_STRIP, 0, new_vbodata.size()));
            vbo->unbind();
        }
    }
};

}

std::shared_ptr<IGraphicDraw> makePointIndicators(Scene* scene, zeno::GeometryObject* pObject) {
    return std::make_shared<PointIndicator>(pObject, scene);
}

}