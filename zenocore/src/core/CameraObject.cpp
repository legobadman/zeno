#include <zeno/utils/interfaceutil.h>
#include <zeno/types/CameraObject.h>


namespace zeno {
    Vec3f CameraObject::get_pos() const { return toAbiVec3f(pos); }
    bool CameraObject::set_pos(const Vec3f& v) {
        pos = toVec3f(v);
        return true;
    }
    Vec3f CameraObject::get_up() const { return toAbiVec3f(up); }
    bool CameraObject::set_up(const Vec3f& v) {
        up = toVec3f(v);
        return true;
    }
    Vec3f CameraObject::get_view() const { return toAbiVec3f(view); }
    bool CameraObject::set_view(const Vec3f& v) {
        view = toVec3f(v);
        return true;
    }
    float CameraObject::get_fov() const { return fov; }
    bool CameraObject::set_fov(float v) {
        fov = v;
        return true;
    }
    float CameraObject::get_fnear() const { return fnear; }
    bool CameraObject::set_fnear(float v) {
        fnear = v;
        return true;
    }
    float CameraObject::get_ffar() const { return ffar; }
    bool CameraObject::set_ffar(float v) {
        ffar = v;
        return true;
    }
    float CameraObject::get_aperture() const { return aperture; }
    bool CameraObject::set_aperture(float v) {
        aperture = v;
        return true;
    }
    float CameraObject::get_focalPlaneDistance() const { return focalPlaneDistance; }

    bool CameraObject::set_focalPlaneDistance(float v) {
        focalPlaneDistance = v;
        return true;
    }

    void CameraObject::set(CameraData const& src) {
        static_cast<CameraData&>(*this) = src;
    }

    CameraData const& CameraObject::get() const {
        return static_cast<CameraData const&>(*this);
    }
}