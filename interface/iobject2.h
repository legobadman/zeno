#ifndef __IOBJECT2_H__
#define __IOBJECT2_H

#include <zvec.h>

namespace zeno {

struct IUserData2 {
    virtual IUserData2* clone() = 0;
    //virtual ~IUserData() = default;

    virtual bool has(const char* key) = 0;
    virtual size_t size() const = 0;

    virtual size_t get_string(const char* key, const char* defl = 0, char* ret_buf, size_t cap) const = 0;
    virtual void set_string(const char* key, const char* sval) = 0;
    virtual bool has_string(const char* key) const = 0;

    virtual int get_int(const char* key, int defl = 0) const = 0;
    virtual void set_int(const char* key, int iVal) = 0;
    virtual bool has_int(const char* key) const = 0;

    virtual float get_float(const char* key, float defl = 0.f) const = 0;
    virtual void set_float(const char* key, float fVal) = 0;
    virtual bool has_float(const char* key) const = 0;

    virtual Vec2f get_vec2f(const char* key, Vec2f defl = Vec2f()) const = 0;
    virtual void set_vec2f(const char* key, const Vec2f& vec) = 0;
    virtual bool has_vec2f(const char* key) const = 0;

    virtual Vec2i get_vec2i(const char* key) const = 0;
    virtual void set_vec2i(const char* key, const Vec2i& vec) = 0;
    virtual bool has_vec2i(const char* key) const = 0;

    virtual Vec3f get_vec3f(const char* key, Vec3f defl = Vec3f()) const = 0;
    virtual void set_vec3f(const char* key, const Vec3f& vec) = 0;
    virtual bool has_vec3f(const char* key) const = 0;

    virtual Vec3i get_vec3i(const char* key) const = 0;
    virtual void set_vec3i(const char* key, const Vec3i& vec) = 0;
    virtual bool has_vec3i(const char* key) const = 0;

    virtual Vec4f get_vec4f(const char* key) const = 0;
    virtual void set_vec4f(const char* key, const Vec4f& vec) = 0;
    virtual bool has_vec4f(const char* key) const = 0;

    virtual Vec4i get_vec4i(const char* key) const = 0;
    virtual void set_vec4i(const char* key, const Vec4i& vec) = 0;
    virtual bool has_vec4i(const char* key) const = 0;

    virtual bool get_bool(const char* key, bool defl = false) const = 0;
    virtual void set_bool(const char* key, bool val = false) = 0;
    virtual bool has_bool(const char* key) const = 0;

    virtual void del(const char* key) = 0;
};

struct IObject2 {
    virtual IObject2* clone() const = 0; //TODO£ºabi compatible for shared_ptr
    virtual size_t key(char* buf, size_t buf_size) const = 0;
    virtual void update_key(const char* key) = 0;
    virtual size_t serialize_json(char* buf, size_t buf_size) const = 0;
    virtual IUserData2* userData() = 0;
    virtual void Delete() = 0;  //TODO: for abi compatiblity when dtor cann't be mark virutal.
};

}
#endif