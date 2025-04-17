#pragma once

#include <zeno/utils/interfaceutil.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/utils/memory.h>
#include <zeno/core/IObject.h>
#include <zeno/funcs/LiterialConverter.h>
#include <string>
#include <map>
#include <any>

namespace zeno {

struct UserData : IUserData {
    std::map<std::string, zany> m_data;

    bool has(const String& key) override {
        std::string skey(key.c_str());
        return m_data.find(skey) != m_data.end();
    }

    String get_string(const String& key, String defl = "") override {
        std::string skey(key.c_str());
        std::string sval = get2<std::string>(skey, zsString2Std(defl));
        return stdString2zs(sval);
    }

    void set_string(const String& key, const String& sval) override {
        std::string skey(key.c_str());
        set2(skey, zsString2Std(sval));
    }

    int get_int(const String& key, int defl = 0) override {
        std::string skey(key.c_str());
        return get2<int>(skey, defl);
    }

    void set_int(const String& key, int iVal) override {
        std::string skey(key.c_str());
        set2(skey, iVal);
    }

    float get_float(const String& key, float defl = 0.f) override {
        std::string skey(key.c_str());
        return get2<float>(skey, defl);
    }

    void set_float(const String& key, float fVal) override {
        std::string skey(key.c_str());
        set2(skey, fVal);
    }

    bool get_bool(const String& key, bool defl = false) override {
        std::string skey(key.c_str());
        return get2<bool>(skey, defl);
    }

    void set_bool(const String& key, bool val = false) override {
        std::string skey(key.c_str());
        set2(skey, val);
    }

    Vec2f get_vec2f(const String& key) override {
        std::string skey(key.c_str());
        return toAbiVec2f(get2<vec2f>(skey));
    }

    void set_vec2f(const String& key, const Vec2f& vec) override {
        std::string skey(key.c_str());
        set2(skey, toVec2f(vec));
    }

    Vec3f get_vec3f(const String& key) override {
        std::string skey(key.c_str());
        return toAbiVec3f(get2<vec3f>(skey));
    }

    void set_vec3f(const String& key, const Vec3f& vec) override {
        std::string skey(key.c_str());
        set2(skey, toVec3f(vec));
    }

    Vec4f get_vec4f(const String& key) override {
        std::string skey(key.c_str());
        return toAbiVec4f(get2<vec4f>(skey));
    }

    void set_vec4f(const String& key, const Vec4f& vec) override {
        std::string skey(key.c_str());
        set2(skey, toVec4f(vec));
    }

    void del(String const& name) override {
        m_data.erase(zsString2Std(name));
    }

    size_t size() const override {
        return m_data.size();
    }

    bool has(std::string const &name) const {
        return m_data.find(name) != m_data.end();
    }

    template <class T>
    bool has(std::string const &name) const {
        auto it = m_data.find(name);
        if (it == m_data.end()) {
            return false;
        }
        return objectIsLiterial<T>(it->second);
    }

    std::shared_ptr<IObject> const &get(std::string const &name) const {
        return safe_at(m_data, name, "user data");
    }

    template <class T>
    bool isa(std::string const &name) const {
        return !!dynamic_cast<T *>(get(name).get());
    }

    std::shared_ptr<IObject> get(std::string const &name, std::shared_ptr<IObject> defl) const {
        return has(name) ? get(name) : defl;
    }

    template <class T>
    std::shared_ptr<T> get(std::string const &name) const {
        return safe_dynamic_cast<T>(get(name));
    }

    template <class T>
    std::shared_ptr<T> get(std::string const &name, std::decay_t<std::shared_ptr<T>> defl) const {
        return has(name) ? get<T>(name) : defl;
    }

    template <class T>
    [[deprecated("use get2<T>(name)")]]
    T getLiterial(std::string const &name) const {
        return get2<T>(name);
    }

    template <class T>
    [[deprecated("use get2<T>(name, value)")]]
    T getLiterial(std::string const &name, T defl) const {
        return get2<T>(name, std::move(defl));
    }

    template <class T>
    T get2(std::string const &name) const {
        return objectToLiterial<T>(get(name));
    }

    template <class T>
    T get2(std::string const &name, T defl) const {
        return has(name) ? getLiterial<T>(name) : defl;
    }

    void set(std::string const &name, std::shared_ptr<IObject> value) {
        m_data[name] = std::move(value);
    }

    void merge(const UserData& name) {
        for (const auto& pair : name.m_data) {
            m_data.insert(pair);
        }
    }

    template <class T>
    [[deprecated("use set2(name, value)")]]
    void setLiterial(std::string const &name, T &&value) {
        return set2(name, std::forward<T>(value));
    }

    template <class T>
    void set2(std::string const &name, T &&value) {
        m_data[name] = objectFromLiterial(std::forward<T>(value));
    }

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
