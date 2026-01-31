#include <zeno/types/LightObject.h>
#include <zeno/utils/interfaceutil.h>


namespace zeno {
    LightData const& LightObject::get() const {
        return static_cast<LightData const&>(*this);
    }

    void LightObject::set(LightData const& lit) {
        static_cast<LightData&>(*this) = lit;
    }

    Vec3f LightObject::get_lightDir() const {
        return toAbiVec3f(lightDir);
    }

    float LightObject::get_intensity() const {
        return intensity;
    }

    Vec3f LightObject::get_shadowTint() const {
        return toAbiVec3f(shadowTint);
    }

    float LightObject::get_lightHight() const {
        return lightHight;
    }

    float LightObject::get_shadowSoftness() const {
        return shadowSoftness;
    }

    Vec3f LightObject::get_lightColor() const {
        return toAbiVec3f(lightColor);
    }

    float LightObject::get_lightScale() const {
        return lightScale;
    }

    bool LightObject::get_isEnabled() const {
        return isEnabled;
    }
}