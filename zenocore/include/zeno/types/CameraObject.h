#pragma once

#include <iobject2.h>
#include <zeno/utils/vec.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <zeno/types/UserData.h>


namespace zeno {
static glm::quat from_theta_phi(float theta, float phi) {
    float cos_t = glm::cos(theta), sin_t = glm::sin(theta);
    float cos_p = glm::cos(phi), sin_p = glm::sin(phi);
    glm::vec3 front(cos_t * sin_p, sin_t, -cos_t * cos_p);
    glm::vec3 up(-sin_t * sin_p, cos_t, sin_t * cos_p);
    glm::vec3 right = glm::cross(front, up);
    glm::mat3 rotation;
    rotation[0] = right;
    rotation[1] = up;
    rotation[2] = -front;
    return glm::quat_cast(rotation);
}

struct CameraData {
    vec3f pos{0, 0, 1};
    vec3f up{0, 1, 0};
    vec3f view{0, 0, -1};
    float fov{45.f};
    float fnear{0.01f};
    float ffar{20000.f};
    //float dof{-1.f};
    float aperture{0.0f};
    float focalPlaneDistance{2.0f};

    std::optional<vec3f> pivot = std::nullopt;
};

struct ZENO_API CameraObject : ICameraObject, CameraData {

public: //IObject2
    IObject2* clone() const override {
        return new CameraObject(*this);
    }
    ZObjectType type() const override {
        return ZObj_Geometry;
    }
    size_t key(char* buf, size_t buf_size) const override
    {
        const char* s = m_key.c_str();
        size_t len = m_key.size();   // ²»º¬ '\0'
        if (buf && buf_size > 0) {
            size_t copy = (len < buf_size - 1) ? len : (buf_size - 1);
            memcpy(buf, s, copy);
            buf[copy] = '\0';
        }
        return len;
    }
    void update_key(const char* key) override {
        m_key = key;
    }
    size_t serialize_json(char* buf, size_t buf_size) const override {
        return 0;
    }
    IUserData2* userData() override {
        return &m_userDat;
    }
    void Delete() override {
        delete this;
    }
private:
    std::string m_key;
    UserData m_userDat;

public:
    Vec3f get_pos() const override;
    bool set_pos(const Vec3f& v) override;
    Vec3f get_up() const override;
    bool set_up(const Vec3f& v) override;
    Vec3f get_view() const override;
    bool set_view(const Vec3f& v) override;
    float get_fov() const override;
    bool set_fov(float v) override;
    float get_fnear() const override;
    bool set_fnear(float v) override;
    float get_ffar() const override;
    bool set_ffar(float v) override;
    float get_aperture() const override;
    bool set_aperture(float v) override;
    float get_focalPlaneDistance() const override;
    bool set_focalPlaneDistance(float v) override;
    void set(CameraData const& src);
    CameraData const& get() const;
};

}
