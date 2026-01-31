#pragma once

#ifndef __CUDACC_RTC__ 

#include <iobject2.h>
#include <zeno/utils/vec.h>
#include <zenum.h>
#include <zeno/types/UserData.h>

#else

#ifndef vec3f
#define vec3f vec3
#endif

#endif

namespace zeno {

    enum struct LightType {
        Diffuse=0u, Direction=1u, IES=2u, Spot=3u, Projector=4u
    };

    enum struct LightShape {
        Plane=0u, Ellipse=1u, Sphere=2u, Point=3u, TriangleMesh=4u
    };

    enum LightConfigMask {
        LightConfigNull       = 0u,
        LightConfigVisible    = 1u,
        LightConfigDoubleside = 2u
    };

    struct DistantLightData {
        vec3f direction;
        float angle;
        vec3f color;
        float intensity;

        DistantLightData() = default;
    };

#ifndef __CUDACC_RTC__ 

struct LightData {
    //vec3f pos{1, 1, 0};
    vec3f lightDir{normalize(vec3f(1, 1, 0))};
    float intensity{10.0f};
    vec3f shadowTint{0.2f};
    float lightHight{1000.0f};
    float shadowSoftness{1.0f};
    vec3f lightColor{1.0f};
    float lightScale{1.0f};
    bool isEnabled{true};
};

struct ZENO_API LightObject : ILightObject, LightData {

public: //IObject2
    IObject2* clone() const override {
        return new LightObject(*this);
    }
    ZObjectType type() const override {
        return ZObj_Light;
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
    LightData const& get() const;
    void set(LightData const& lit);
    Vec3f get_lightDir() const override;
    float get_intensity() const override;
    Vec3f get_shadowTint() const override;
    float get_lightHight() const override;
    float get_shadowSoftness() const override;
    Vec3f get_lightColor() const override;
    float get_lightScale() const override;
    bool get_isEnabled() const override;
};

#endif
}
