#pragma once

#include <string>
#include <map>
#include <any>
#include <reflect/container/any>
#include <iobject2.h>
#include <zvec.h>

namespace zeno {

struct UserData : IUserData2 {
    std::map<std::string, zeno::reflect::Any> m_data;

    UserData() = default;
    UserData(const UserData& rhs);
    UserData& operator=(const UserData& rhs) = delete;
    IUserData2* clone() override;
    bool has(const char* key) override;
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
    Vec2f get_vec2f(const char* key, Vec2f defl) const override;
    Vec2i get_vec2i(const char* key) const override;
    void set_vec2f(const char* key, const Vec2f& vec) override;
    bool has_vec2f(const char* key) const override;
    void set_vec2i(const char* key, const Vec2i& vec) override;
    bool has_vec2i(const char* key) const override;
    Vec3f get_vec3f(const char* key, Vec3f defl) const override;
    bool has_vec3f(const char* key) const override;
    Vec3i get_vec3i(const char* key) const override;
    bool has_vec3i(const char* key) const override;
    void set_vec3f(const char* key, const Vec3f& vec) override;
    void set_vec3i(const char* key, const Vec3i& vec) override;
    bool has_vec4f(const char* key) const override;
    Vec4f get_vec4f(const char* key) const override;
    bool has_vec4i(const char* key) const override;
    Vec4i get_vec4i(const char* key) const override;
    void set_vec4f(const char* key, const Vec4f& vec) override;
    void set_vec4i(const char* key, const Vec4i& vec) override;
    void del(const char* key) override;
    size_t size() const override;
    bool has(std::string const& name) const;

    template <class T>
    bool has(std::string const &name) const {
        auto it = m_data.find(name);
        if (it == m_data.end()) {
            return false;
        }

        const auto& anyType = it->second.type();
        if (anyType == zeno::reflect::type_info<T>()) {
            return true;
        }
        else {
            return false;
        }
    }

    template <class T>
    T get2(std::string const &name) const {
        auto iter = m_data.find(name);
        if (iter == m_data.end()) {
            throw;
        }
        const zeno::reflect::Any& val = iter->second;
        return zeno::reflect::any_cast<T>(val);
    }

    template <class T>
    T get2(std::string const &name, T defl) const {
        return has(name) ? get2<T>(name) : defl;
    }

    template <class T>
    void set2(std::string const& name, T&& value) {
        m_data[name] = zeno::reflect::Any(value);
    }

    void set(std::string const& name, zeno::reflect::Any&& value);
    void merge(const UserData& rhs);
    auto begin() const {
        return m_data.begin();
    }

    auto end() const {
        return m_data.end();
    }

    auto begin() {
        return m_data.begin();
    }

    auto end() {
        return m_data.end();
    }

    size_t erase(const std::string& key) {
        return m_data.erase(key);
    }
};

}
