#pragma once

#include <iobject2.h>
#include <zvec.h>
#include <string>
#include <map>
#include <vector>

namespace zeno {

// Simplified UserData for zs_fbx: implements IUserData2 without zeno::reflect::Any.
// Storage is per-type maps (no type erasure).
struct FBXUserData : IUserData2 {
    std::map<std::string, std::string> m_strings;
    std::map<std::string, int> m_ints;
    std::map<std::string, float> m_floats;
    std::map<std::string, bool> m_bools;
    std::map<std::string, Vec2f> m_vec2f;
    std::map<std::string, Vec2i> m_vec2i;
    std::map<std::string, Vec3f> m_vec3f;
    std::map<std::string, Vec3i> m_vec3i;
    std::map<std::string, Vec4f> m_vec4f;
    std::map<std::string, Vec4i> m_vec4i;
    std::map<std::string, std::vector<float>> m_float_arr;

    FBXUserData() = default;
    FBXUserData(const FBXUserData&) = default;
    FBXUserData& operator=(const FBXUserData&) = default;

    IUserData2* clone() override;
    void copy(IUserData2* pUserData) override;

    bool has(const char* key) override;
    size_t size() const override;

    size_t get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const override;
    void set_string(const char* key, const char* sval) override;
    bool has_string(const char* key) const override;

    int get_int(const char* key, int defl = 0) const override;
    void set_int(const char* key, int iVal) override;
    bool has_int(const char* key) const override;

    float get_float(const char* key, float defl = 0.f) const override;
    void set_float(const char* key, float fVal) override;
    bool has_float(const char* key) const override;

    bool get_bool(const char* key, bool defl = false) const override;
    void set_bool(const char* key, bool val = false) override;
    bool has_bool(const char* key) const override;

    Vec2f get_vec2f(const char* key, Vec2f defl = Vec2f()) const override;
    Vec2i get_vec2i(const char* key) const override;
    void set_vec2f(const char* key, const Vec2f& vec) override;
    void set_vec2i(const char* key, const Vec2i& vec) override;
    bool has_vec2f(const char* key) const override;
    bool has_vec2i(const char* key) const override;

    Vec3f get_vec3f(const char* key, Vec3f defl = Vec3f()) const override;
    Vec3i get_vec3i(const char* key) const override;
    void set_vec3f(const char* key, const Vec3f& vec) override;
    void set_vec3i(const char* key, const Vec3i& vec) override;
    bool has_vec3f(const char* key) const override;
    bool has_vec3i(const char* key) const override;

    Vec4f get_vec4f(const char* key) const override;
    Vec4i get_vec4i(const char* key) const override;
    void set_vec4f(const char* key, const Vec4f& vec) override;
    void set_vec4i(const char* key, const Vec4i& vec) override;
    bool has_vec4f(const char* key) const override;
    bool has_vec4i(const char* key) const override;

    size_t get_float_arr(const char* key, float* buf, size_t cap) const override;

    void del(const char* key) override;
};

} // namespace zeno
