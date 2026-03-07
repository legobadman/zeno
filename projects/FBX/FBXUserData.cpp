#include "FBXUserData.h"
#include <cstring>
#include <algorithm>
#include <set>

namespace zeno {

static bool key_in_any(const FBXUserData* self, const char* key) {
    std::string k(key);
    return self->m_strings.count(k) || self->m_ints.count(k) || self->m_floats.count(k)
        || self->m_bools.count(k) || self->m_vec2f.count(k) || self->m_vec2i.count(k)
        || self->m_vec3f.count(k) || self->m_vec3i.count(k) || self->m_vec4f.count(k)
        || self->m_vec4i.count(k) || self->m_float_arr.count(k);
}

static void collect_keys(const FBXUserData* self, std::set<std::string>& out) {
    for (const auto& p : self->m_strings) out.insert(p.first);
    for (const auto& p : self->m_ints) out.insert(p.first);
    for (const auto& p : self->m_floats) out.insert(p.first);
    for (const auto& p : self->m_bools) out.insert(p.first);
    for (const auto& p : self->m_vec2f) out.insert(p.first);
    for (const auto& p : self->m_vec2i) out.insert(p.first);
    for (const auto& p : self->m_vec3f) out.insert(p.first);
    for (const auto& p : self->m_vec3i) out.insert(p.first);
    for (const auto& p : self->m_vec4f) out.insert(p.first);
    for (const auto& p : self->m_vec4i) out.insert(p.first);
    for (const auto& p : self->m_float_arr) out.insert(p.first);
}

IUserData2* FBXUserData::clone() {
    return new FBXUserData(*this);
}

void FBXUserData::copy(IUserData2* pUserData) {
    auto* other = static_cast<FBXUserData*>(pUserData);
    if (other) {
        m_strings = other->m_strings;
        m_ints = other->m_ints;
        m_floats = other->m_floats;
        m_bools = other->m_bools;
        m_vec2f = other->m_vec2f;
        m_vec2i = other->m_vec2i;
        m_vec3f = other->m_vec3f;
        m_vec3i = other->m_vec3i;
        m_vec4f = other->m_vec4f;
        m_vec4i = other->m_vec4i;
        m_float_arr = other->m_float_arr;
    }
}

bool FBXUserData::has(const char* key) {
    return key_in_any(this, key);
}

size_t FBXUserData::size() const {
    std::set<std::string> keys;
    collect_keys(this, keys);
    return keys.size();
}

size_t FBXUserData::get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const {
    std::string k(key);
    std::string s;
    auto it = m_strings.find(k);
    if (it != m_strings.end())
        s = it->second;
    else
        s = defl ? defl : "";
    if (!ret_buf || cap == 0) return s.size();
    size_t n = std::min(s.size(), cap - 1);
    std::memcpy(ret_buf, s.data(), n);
    ret_buf[n] = '\0';
    return s.size();
}

void FBXUserData::set_string(const char* key, const char* sval) {
    m_strings[key] = sval ? sval : "";
}

bool FBXUserData::has_string(const char* key) const {
    return m_strings.count(key) != 0;
}

int FBXUserData::get_int(const char* key, int defl) const {
    auto it = m_ints.find(key);
    if (it != m_ints.end()) return it->second;
    auto itf = m_floats.find(key);
    if (itf != m_floats.end()) return static_cast<int>(itf->second);
    return defl;
}

void FBXUserData::set_int(const char* key, int iVal) {
    m_ints[key] = iVal;
}

bool FBXUserData::has_int(const char* key) const {
    return m_ints.count(key) != 0 || m_floats.count(key) != 0;
}

float FBXUserData::get_float(const char* key, float defl) const {
    auto it = m_floats.find(key);
    if (it != m_floats.end()) return it->second;
    auto iti = m_ints.find(key);
    if (iti != m_ints.end()) return static_cast<float>(iti->second);
    return defl;
}

void FBXUserData::set_float(const char* key, float fVal) {
    m_floats[key] = fVal;
}

bool FBXUserData::has_float(const char* key) const {
    return m_floats.count(key) != 0 || m_ints.count(key) != 0;
}

bool FBXUserData::get_bool(const char* key, bool defl) const {
    auto it = m_bools.find(key);
    if (it != m_bools.end()) return it->second;
    return defl;
}

void FBXUserData::set_bool(const char* key, bool val) {
    m_bools[key] = val;
}

bool FBXUserData::has_bool(const char* key) const {
    return m_bools.count(key) != 0;
}

Vec2f FBXUserData::get_vec2f(const char* key, Vec2f defl) const {
    auto it = m_vec2f.find(key);
    if (it != m_vec2f.end()) return it->second;
    return defl;
}

Vec2i FBXUserData::get_vec2i(const char* key) const {
    auto it = m_vec2i.find(key);
    if (it != m_vec2i.end()) return it->second;
    return Vec2i(0, 0);
}

void FBXUserData::set_vec2f(const char* key, const Vec2f& vec) {
    m_vec2f[key] = vec;
}

void FBXUserData::set_vec2i(const char* key, const Vec2i& vec) {
    m_vec2i[key] = vec;
}

bool FBXUserData::has_vec2f(const char* key) const {
    return m_vec2f.count(key) != 0;
}

bool FBXUserData::has_vec2i(const char* key) const {
    return m_vec2i.count(key) != 0;
}

Vec3f FBXUserData::get_vec3f(const char* key, Vec3f defl) const {
    auto it = m_vec3f.find(key);
    if (it != m_vec3f.end()) return it->second;
    return defl;
}

Vec3i FBXUserData::get_vec3i(const char* key) const {
    auto it = m_vec3i.find(key);
    if (it != m_vec3i.end()) return it->second;
    return Vec3i(0, 0, 0);
}

void FBXUserData::set_vec3f(const char* key, const Vec3f& vec) {
    m_vec3f[key] = vec;
}

void FBXUserData::set_vec3i(const char* key, const Vec3i& vec) {
    m_vec3i[key] = vec;
}

bool FBXUserData::has_vec3f(const char* key) const {
    return m_vec3f.count(key) != 0;
}

bool FBXUserData::has_vec3i(const char* key) const {
    return m_vec3i.count(key) != 0;
}

Vec4f FBXUserData::get_vec4f(const char* key) const {
    auto it = m_vec4f.find(key);
    if (it != m_vec4f.end()) return it->second;
    return Vec4f{0.f, 0.f, 0.f, 0.f};
}

Vec4i FBXUserData::get_vec4i(const char* key) const {
    auto it = m_vec4i.find(key);
    if (it != m_vec4i.end()) return it->second;
    return Vec4i{0, 0, 0, 0};
}

void FBXUserData::set_vec4f(const char* key, const Vec4f& vec) {
    m_vec4f[key] = vec;
}

void FBXUserData::set_vec4i(const char* key, const Vec4i& vec) {
    m_vec4i[key] = vec;
}

bool FBXUserData::has_vec4f(const char* key) const {
    return m_vec4f.count(key) != 0;
}

bool FBXUserData::has_vec4i(const char* key) const {
    return m_vec4i.count(key) != 0;
}

size_t FBXUserData::get_float_arr(const char* key, float* buf, size_t cap) const {
    auto it = m_float_arr.find(key);
    if (it == m_float_arr.end() || !buf) return 0;
    const auto& arr = it->second;
    size_t n = std::min(arr.size(), cap);
    std::memcpy(buf, arr.data(), n * sizeof(float));
    return n;
}

void FBXUserData::del(const char* key) {
    std::string k(key);
    m_strings.erase(k);
    m_ints.erase(k);
    m_floats.erase(k);
    m_bools.erase(k);
    m_vec2f.erase(k);
    m_vec2i.erase(k);
    m_vec3f.erase(k);
    m_vec3i.erase(k);
    m_vec4f.erase(k);
    m_vec4i.erase(k);
    m_float_arr.erase(k);
}

} // namespace zeno
