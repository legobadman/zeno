#include <zeno/types/UserData.h>
#include <zeno/utils/helper.h>

namespace zeno
{
    size_t UserData::get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const {
        std::string skey(key);
        std::string sval = get2<std::string>(skey, std::string(defl));
        return stdStr2charArr(sval, ret_buf, cap);
    }

    void UserData::copy(IUserData2* pUserData) {
        m_data = ((UserData*)pUserData)->m_data;
    }

    IUserData2* UserData::clone() {
        return new UserData(*this);
    }

    bool UserData::has(const char* key) {
        std::string skey(key);
        return m_data.find(skey) != m_data.end();
    }

    void UserData::set_string(const char* key, const char* sval) {
        std::string skey(key);
        set2(skey, std::string(sval));
    }

    bool UserData::has_string(const char* key) const {
        std::string skey(key);
        return has<std::string>(skey);
    }

    int UserData::get_int(const char* key, int defl) const {
        std::string skey(key);
        return get2<int>(skey, defl);
    }

    void UserData::set_int(const char* key, int iVal) {
        std::string skey(key);
        set2(skey, iVal);
    }

    bool UserData::has_int(const char* key) const {
        std::string skey(key);
        return has<int>(skey);
    }

    float UserData::get_float(const char* key, float defl) const {
        std::string skey(key);
        return get2<float>(skey, defl);
    }

    void UserData::set_float(const char* key, float fVal) {
        std::string skey(key);
        set2(skey, fVal);
    }

    bool UserData::has_float(const char* key) const {
        std::string skey(key);
        return has<float>(skey);
    }

    bool UserData::get_bool(const char* key, bool defl) const {
        std::string skey(key);
        return get2<bool>(skey, defl);
    }

    void UserData::set_bool(const char* key, bool val) {
        std::string skey(key);
        set2(skey, val);
    }

    bool UserData::has_bool(const char* key) const {
        std::string skey(key);
        return has<bool>(skey);
    }

    Vec2f UserData::get_vec2f(const char* key, Vec2f defl) const {
        std::string skey(key);
        return get2<Vec2f>(skey, defl);
    }

    Vec2i UserData::get_vec2i(const char* key) const {
        std::string skey(key);
        return get2<Vec2i>(skey);
    }

    void UserData::set_vec2f(const char* key, const Vec2f& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    bool UserData::has_vec2f(const char* key) const {
        std::string skey(key);
        return has<Vec2f>(skey);
    }

    void UserData::set_vec2i(const char* key, const Vec2i& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    bool UserData::has_vec2i(const char* key) const {
        std::string skey(key);
        return has<Vec2i>(skey);
    }

    Vec3f UserData::get_vec3f(const char* key, Vec3f defl) const {
        std::string skey(key);
        return get2<Vec3f>(skey, defl);
    }

    bool UserData::has_vec3f(const char* key) const {
        std::string skey(key);
        return has<Vec3f>(skey);
    }

    Vec3i UserData::get_vec3i(const char* key) const {
        std::string skey(key);
        return get2<Vec3i>(skey);
    }

    bool UserData::has_vec3i(const char* key) const {
        std::string skey(key);
        return has<Vec3i>(skey);
    }

    void UserData::set_vec3f(const char* key, const Vec3f& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    void UserData::set_vec3i(const char* key, const Vec3i& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    bool UserData::has_vec4f(const char* key) const {
        std::string skey(key);
        return has<Vec4f>(skey);
    }

    Vec4f UserData::get_vec4f(const char* key) const {
        std::string skey(key);
        return get2<Vec4f>(skey);
    }

    bool UserData::has_vec4i(const char* key) const {
        std::string skey(key);
        return has<Vec4i>(skey);
    }

    Vec4i UserData::get_vec4i(const char* key) const {
        std::string skey(key);
        return get2<Vec4i>(skey);
    }

    void UserData::set_vec4f(const char* key, const Vec4f& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    void UserData::set_vec4i(const char* key, const Vec4i& vec) {
        std::string skey(key);
        set2(skey, vec);
    }

    void UserData::del(const char* key) {
        m_data.erase(std::string(key));
    }

    size_t UserData::size() const {
        return m_data.size();
    }

    bool UserData::has(std::string const& name) const {
        return m_data.find(name) != m_data.end();
    }

    void UserData::set(std::string const& name, zeno::reflect::Any&& value) {
        m_data[name] = value;
    }

    void UserData::merge(const UserData& rhs) {
        m_data.insert(rhs.m_data.begin(), rhs.m_data.end());
    }
}