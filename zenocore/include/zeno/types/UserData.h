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

struct UserData : IUserData2 {
    std::map<std::string, zany> m_data;

    UserData() {}

    UserData(const UserData& rhs) {
        for (auto& [key, dat] : rhs.m_data) {
            m_data.insert(std::make_pair(key, dat->clone()));
        }
    }

    UserData& operator=(const UserData& rhs) = delete;

    IUserData2* clone() override {
        return new UserData(*this);
    }

    bool has(const char* key) override {
        std::string skey(key);
        return m_data.find(skey) != m_data.end();
    }

    size_t get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const override;

    void set_string(const char* key, const char* sval) override {
        std::string skey(key);
        set2(skey, zsString2Std(sval));
    }

    bool has_string(const char* key) const override {
        std::string skey(key);
        return has<std::string>(skey);
    }

    int get_int(const char* key, int defl = 0) const override {
        std::string skey(key);
        return get2<int>(skey, defl);
    }

    void set_int(const char* key, int iVal) override {
        std::string skey(key);
        set2(skey, iVal);
    }

    bool has_int(const char* key) const override {
        std::string skey(key);
        return has<int>(skey);
    }

    float get_float(const char* key, float defl = 0.f) const override {
        std::string skey(key);
        return get2<float>(skey, defl);
    }

    void set_float(const char* key, float fVal) override {
        std::string skey(key);
        set2(skey, fVal);
    }

    bool has_float(const char* key) const override {
        std::string skey(key);
        return has<float>(skey);
    }

    bool get_bool(const char* key, bool defl = false) const override {
        std::string skey(key);
        return get2<bool>(skey, defl);
    }

    void set_bool(const char* key, bool val = false) override {
        std::string skey(key);
        set2(skey, val);
    }

    bool has_bool(const char* key) const override {
        std::string skey(key);
        return has<bool>(skey);
    }

    Vec2f get_vec2f(const char* key, Vec2f defl) const override {
        std::string skey(key);
        return toAbiVec2f(get2<vec2f>(skey, toVec2f(defl)));
    }

    Vec2i get_vec2i(const char* key) const override {
        std::string skey(key);
        return toAbiVec2i(get2<vec2i>(skey));
    }

    void set_vec2f(const char* key, const Vec2f& vec) override {
        std::string skey(key);
        set2(skey, toVec2f(vec));
    }

    bool has_vec2f(const char* key) const override {
        std::string skey(key);
        return has<vec2f>(skey);
    }

    void set_vec2i(const char* key, const Vec2i& vec) override {
        std::string skey(key);
        set2(skey, toVec2i(vec));
    }

    bool has_vec2i(const char* key) const override {
        std::string skey(key);
        return has<vec2i>(skey);
    }

    Vec3f get_vec3f(const char* key, Vec3f defl) const override {
        std::string skey(key);
        return toAbiVec3f(get2<vec3f>(skey, toVec3f(defl)));
    }

    bool has_vec3f(const char* key) const override {
        std::string skey(key);
        return has<vec3f>(skey);
    }

    Vec3i get_vec3i(const char* key) const override {
        std::string skey(key);
        return toAbiVec3i(get2<vec3i>(skey));
    }

    bool has_vec3i(const char* key) const override {
        std::string skey(key);
        return has<vec3i>(skey);
    }

    void set_vec3f(const char* key, const Vec3f& vec) override {
        std::string skey(key);
        set2(skey, toVec3f(vec));
    }

    void set_vec3i(const char* key, const Vec3i& vec) override {
        std::string skey(key);
        set2(skey, toVec3i(vec));
    }

    bool has_vec4f(const char* key) const override {
        std::string skey(key);
        return has<vec4f>(skey);
    }

    Vec4f get_vec4f(const char* key) const override {
        std::string skey(key);
        return toAbiVec4f(get2<vec4f>(skey));
    }

    bool has_vec4i(const char* key) const override {
        std::string skey(key);
        return has<vec4i>(skey);
    }

    Vec4i get_vec4i(const char* key) const override {
        std::string skey(key);
        return toAbiVec4i(get2<vec4f>(skey));
    }

    void set_vec4f(const char* key, const Vec4f& vec) override {
        std::string skey(key);
        set2(skey, toVec4f(vec));
    }

    void set_vec4i(const char* key, const Vec4i& vec) override {
        std::string skey(key);
        set2(skey, toVec4i(vec));
    }

    //ZsVector<String> keys() const override {
    //    ZsVector<String> vec;
    //    for (const auto& [key, value] : m_data) {
    //        vec.push_back(stdString2zs(key));
    //    }
    //    return vec;
    //}

    //zany get(const char* key) const override {
    //    return get(zsString2Std(key), nullptr);
    //}

    //void set(const char* key, zany&& val) override {
    //    set(zsString2Std(key), std::move(val));
    //}

    void del(const char* key) override {
        m_data.erase(std::string(key));
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
        return objectIsLiterial<T>(it->second.get());
    }

    zany get(std::string const &name) const {
        return safe_at(m_data, name, "user data")->clone();
    }

    template <class T>
    bool isa(std::string const &name) const {
        return !!dynamic_cast<T *>(get(name).get());
    }

    zany get(std::string const &name, IObject* defl) const {
        return has(name) ? get(name)->clone() : defl->clone();
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
        const auto& tempobj = get(name);
        return objectToLiterial<T>(tempobj);
    }

    template <class T>
    T get2(std::string const &name, T defl) const {
        return has(name) ? getLiterial<T>(name) : defl;
    }

    void set(std::string const &name, zany&& value) {
        m_data[name] = value->clone();
    }

    void merge(const UserData& name) {
        for (const auto& pair : name.m_data) {
            m_data.insert(std::make_pair(pair.first, pair.second->clone()));
        }
    }

    template <class T>
    [[deprecated("use set2(name, value)")]]
    void setLiterial(std::string const &name, T &&value) {
        return set2(name, std::forward<T>(value));
    }

    template <class T>
    void set2(std::string const &name, T &&value) {
        m_data[name] = objectFromLiterial(std::forward<T>(value))->clone();
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
