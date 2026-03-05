#pragma once

#include <cstddef>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <zvec.h>

namespace zeno {

// Local helpers --------------------------------------------------------------

// Trait: check if T is one of the types in a std::variant.
template <class T, class Variant>
struct variant_contains;

template <class T, class... Ts>
struct variant_contains<T, std::variant<Ts...>> {
    static constexpr bool value = (std::is_same_v<T, Ts> || ...);
};

// Dimension helper for Vec* types.
template <class T>
struct vec_dim {
    static constexpr std::size_t value = 1;
};

template <>
struct vec_dim<Vec2f> {
    static constexpr std::size_t value = 2;
};
template <>
struct vec_dim<Vec2i> {
    static constexpr std::size_t value = 2;
};
template <>
struct vec_dim<Vec3f> {
    static constexpr std::size_t value = 3;
};
template <>
struct vec_dim<Vec3i> {
    static constexpr std::size_t value = 3;
};
template <>
struct vec_dim<Vec4f> {
    static constexpr std::size_t value = 4;
};
template <>
struct vec_dim<Vec4i> {
    static constexpr std::size_t value = 4;
};

template <class T>
inline constexpr std::size_t vec_dim_v = vec_dim<T>::value;

// Attribute containers -------------------------------------------------------

using AttrAcceptAll = std::variant
    < Vec3f
    , float
    , Vec3i
    , int
    , Vec2f
    , Vec2i
    , Vec4f
    , Vec4i
    >;

struct AttrVectorIndex {
    std::size_t attrIndex = 0;
    std::size_t elementIndex = 0;
    std::size_t attrDim = 1;
};

using AttrVectorVariant = std::variant
        < std::vector<Vec3f>
        , std::vector<float>
        , std::vector<Vec3i>
        , std::vector<int>
        , std::vector<Vec2f>
        , std::vector<Vec2i>
        , std::vector<Vec4f>
        , std::vector<Vec4i>
        >;

// AttrVector = BaseVector + attrs
template <class ValT>
struct AttrVector {
    using value_type = ValT;
    using BaseVector = std::vector<ValT>;
    using size_type = typename BaseVector::size_type;
    using pointer = typename BaseVector::pointer;
    using reference = typename BaseVector::reference;
    using const_pointer = typename BaseVector::const_pointer;
    using const_reference = typename BaseVector::const_reference;
    using iterator = typename BaseVector::iterator;
    using const_iterator = typename BaseVector::const_iterator;

    inline static const std::string kpos = "pos";

    BaseVector values;
    std::map<std::string, AttrVectorVariant> attrs;

    AttrVector() = default;
    AttrVector(std::vector<ValT> const& values_) : values(values_) {}
    AttrVector(std::vector<ValT>&& values_) : values(std::move(values_)) {}
    explicit AttrVector(std::size_t size) : values(size) {}

    decltype(auto) begin() const { return values.begin(); }
    decltype(auto) end() const { return values.end(); }
    decltype(auto) data() const { return values.data(); }

    decltype(auto) begin() { return values.begin(); }
    decltype(auto) end() { return values.end(); }
    decltype(auto) data() { return values.data(); }

    decltype(auto) at(std::size_t idx) const { return values.at(idx); }
    decltype(auto) at(std::size_t idx) { return values.at(idx); }

    void push_back(ValT const& t) { values.push_back(t); }
    void push_back(ValT&& t) { values.push_back(std::move(t)); }

    void update() {
        for (auto& [key, val] : attrs) {
            (void)key;
            std::visit([&](auto& v) { v.resize(this->size()); }, val);
        }
    }

    decltype(auto) operator[](std::size_t idx) const { return values[idx]; }
    decltype(auto) operator[](std::size_t idx) { return values[idx]; }

    auto const* operator->() const { return &values; }
    auto* operator->() { return &values; }

    operator auto const&() const { return values; }
    operator auto&() { return values; }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void attr_visit(std::string const& name, F const& f) const {
        if (name == kpos) {
            f(values);
            return;
        }
        auto it = attrs.find(name);
        if (it == attrs.end()) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' not found");
        }
        std::visit(
            [&](auto& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                if constexpr (variant_contains<T, Accept>::value) {
                    f(arr);
                }
            },
            it->second);
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void attr_visit(std::string const& name, F const& f) {
        if constexpr (variant_contains<ValT, Accept>::value) {
            if (name == kpos) {
                f(values);
                return;
            }
        }
        auto it = attrs.find(name);
        if (it == attrs.end()) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' not found");
        }
        std::visit(
            [&](auto& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                if constexpr (variant_contains<T, Accept>::value) {
                    f(arr);
                }
            },
            it->second);
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void foreach_attr(F&& f) const {
        for (auto const& [key, arr] : attrs) {
            auto const& k = key;
            std::visit(
                [&](auto const& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, Accept>::value) {
                        f(k, a);
                    }
                },
                arr);
        }
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void foreach_attr(F&& f) {
        for (auto& [key, arr] : attrs) {
            auto const& k = key;
            std::visit(
                [&](auto& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, Accept>::value) {
                        f(k, a);
                    }
                },
                arr);
        }
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void forall_attr(F&& f) const {
        f(kpos, values);
        for (auto const& [key, arr] : attrs) {
            auto const& k = key;
            std::visit(
                [&](auto const& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, Accept>::value) {
                        f(k, a);
                    }
                },
                arr);
        }
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void forall_attr(F&& f) {
        f(kpos, values);
        for (auto& [key, arr] : attrs) {
            auto const& k = key;
            std::visit(
                [&](auto& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, Accept>::value) {
                        f(k, a);
                    }
                },
                arr);
        }
    }

    template <class T>
    std::size_t type_dim() const {
        return vec_dim_v<T>;
    }

    std::size_t total_dim() const {
        std::size_t dim = 0;
        // base
        dim += type_dim<value_type>();
        // attrs
        for (auto& [key, arr] : attrs) {
            (void)key;
            std::visit(
                [&](auto& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, AttrAcceptAll>::value) {
                        dim += type_dim<T>();
                    }
                },
                arr);
        }
        return dim;
    }

    AttrVectorIndex attr_index(std::size_t index) const {
        std::size_t attrIndex = 0;
        std::size_t elementIndex = 0;
        std::size_t attrDim = type_dim<value_type>();
        std::size_t dim = 0;
        // base
        dim += type_dim<value_type>();
        if (index < dim) {
            return {attrIndex, index % dim, attrDim};
        }
        attrIndex++;
        // attrs (std::map is sorted, so attrIndex++ is stable)
        for (auto& [key, arr] : attrs) {
            (void)key;
            std::visit(
                [&](auto& a) {
                    using T = std::decay_t<decltype(a[0])>;
                    if constexpr (variant_contains<T, AttrAcceptAll>::value) {
                        auto current_dim = type_dim<T>();
                        if (index < dim + current_dim) {
                            elementIndex = (index - dim) % current_dim;
                            attrDim = current_dim;
                        }
                        dim += current_dim;
                    }
                },
                arr);
            if (index < dim) {
                break;
            }
            attrIndex++;
        }
        return {attrIndex, elementIndex, attrDim};
    }

    template <class Accept = std::variant<Vec3f, float>>
    std::size_t num_attrs() const {
        if constexpr (std::is_same_v<Accept, AttrAcceptAll>) {
            return attrs.size();
        } else {
            std::size_t count = 0;
            foreach_attr<Accept>([&](auto const&, auto const&) { ++count; });
            return count;
        }
    }

    template <class Accept = std::variant<Vec3f, float>>
    auto attr_keys() const {
        std::vector<std::string> keys;
        foreach_attr<Accept>(
            [&](auto const& key, auto const&) {
                keys.push_back(key);
            });
        return keys;
    }

    template <class... Ts>
    decltype(auto) emplace_back(Ts&&... ts) {
        return values.emplace_back(std::forward<Ts>(ts)...);
    }

    template <class T>
    auto& add_attr(std::string const& name) {
        if (!attr_is<T>(name)) {
            attrs[name] = std::vector<T>(size());
        }
        return attr<T>(name);
    }

    template <class T>
    auto& add_attr(std::string const& name, T const& val) {
        if (!attr_is<T>(name)) {
            attrs[name] = std::vector<T>(size(), val);
        }
        return attr<T>(name);
    }

    template <class T>
    auto const& attr(std::string const& name) const {
        if (name == kpos) {
            if constexpr (!std::is_same_v<T, ValT>) {
                throw std::runtime_error("AttrVector: attribute 'pos' has incompatible value type");
            } else {
                return values;
            }
        }
        auto const& arr = attr(name);
        if (!std::holds_alternative<std::vector<T>>(arr)) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' type mismatch");
        }
        return std::get<std::vector<T>>(arr);
    }

    template <class T>
    auto& attr(std::string const& name) {
        if (name == kpos) {
            if constexpr (!std::is_same_v<T, ValT>) {
                throw std::runtime_error("AttrVector: attribute 'pos' has incompatible value type");
            } else {
                return values;
            }
        }
        auto& arr = attr(name);
        if (!std::holds_alternative<std::vector<T>>(arr)) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' type mismatch");
        }
        return std::get<std::vector<T>>(arr);
    }

    auto const& attr(std::string const& name) const {
        auto it = attrs.find(name);
        if (it == attrs.end()) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' not found");
        }
        return it->second;
    }

    auto& attr(std::string const& name) {
        auto it = attrs.find(name);
        if (it == attrs.end()) {
            throw std::runtime_error("AttrVector: attribute '" + name + "' not found");
        }
        return it->second;
    }

    bool has_attr(std::string const& name) const {
        if (name == kpos) return true;
        return attrs.find(name) != attrs.end();
    }

    void erase_attr(std::string const& name) {
        attrs.erase(name);
    }

    template <class T>
    bool attr_is(std::string const& name) const {
        if (name == kpos) return std::is_same_v<T, ValT>;
        auto it = attrs.find(name);
        return it != attrs.end() && std::holds_alternative<std::vector<T>>(it->second);
    }

    void clear_attrs() {
        attrs.clear();
    }

    std::size_t size() const { return values.size(); }

    void reserve(std::size_t sz) {
        values.reserve(sz);
        for (auto& [key, val] : attrs) {
            (void)key;
            std::visit([&](auto& v) { v.reserve(sz); }, val);
        }
    }

    void shrink_to_fit() {
        values.shrink_to_fit();
        for (auto& [key, val] : attrs) {
            (void)key;
            std::visit([&](auto& v) { v.shrink_to_fit(); }, val);
        }
    }

    void resize(std::size_t sz) {
        values.resize(sz);
        for (auto& [key, val] : attrs) {
            (void)key;
            std::visit([&](auto& v) { v.resize(sz); }, val);
        }
        shrink_to_fit();
    }

    void clear() {
        values.clear();
        for (auto& [key, val] : attrs) {
            (void)key;
            std::visit([&](auto& v) { v.clear(); }, val);
        }
    }

    void clear_with_attr() {
        values.clear();
        attrs.clear();
    }
};

// PrimitiveObject (local copy, no base class, no UserData/mtl/inst) ----------

// Minimal Vec3f helpers so we can reuse polygon decomposition utilities.
inline Vec3f operator+(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vec3f operator-(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline Vec3f operator*(const Vec3f& a, float s) {
    return Vec3f(a.x * s, a.y * s, a.z * s);
}
inline Vec3f operator*(float s, const Vec3f& a) {
    return a * s;
}

inline Vec3f cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

inline float dot(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float length(const Vec3f& v) {
    return std::sqrt(dot(v, v));
}

inline Vec3f normalize(const Vec3f& v) {
    const float len = length(v);
    if (len <= 0.0f) {
        return v;
    }
    const float inv = 1.0f / len;
    return Vec3f(v.x * inv, v.y * inv, v.z * inv);
}

inline Vec3f polyNormal(std::vector<Vec3f>& verts, std::vector<int>& poly) {
    Vec3f normal(0.f, 0.f, 0.f);
    if (poly.size() < 3) {
        return normal;
    }
    for (int i = 1; i < static_cast<int>(poly.size()) - 1; ++i) {
        normal = normal + cross(verts[poly[i + 1]] - verts[poly[0]],
                                verts[poly[i]] - verts[poly[0]]);
    }
    normal = normalize(normal) * -1.0f;
    return normal;
}

inline char isReflex(Vec3f a, Vec3f b, Vec3f c, Vec3f n) {
    return dot(cross(b - a, c - b), n) <= 0.0f ? 1 : 0;
}

inline void markReflex(std::vector<Vec3f>& verts,
                       std::vector<int>& poly,
                       std::vector<char>& reflexMark,
                       Vec3f n) {
    reflexMark.resize(poly.size());
    for (int i = 0; i < static_cast<int>(poly.size()); ++i) {
        int prev = i == 0 ? static_cast<int>(poly.size()) - 1 : i - 1;
        int next = i == static_cast<int>(poly.size()) - 1 ? 0 : i + 1;
        reflexMark[i] = isReflex(verts[poly[prev]], verts[poly[i]], verts[poly[next]], n);
    }
}

inline void takeOutTriangle(std::vector<Vec3f>& verts,
                            std::vector<int>& poly,
                            std::vector<char>& reflexMark,
                            std::vector<Vec3i>& triangles) {
    if (poly.size() == 3) {
        triangles.emplace_back(poly[0], poly[1], poly[2]);
        poly.clear();
        return;
    } else {
        char anyReflex = 0;
        for (auto r : reflexMark) {
            anyReflex = static_cast<char>(anyReflex | r);
        }

        int a = 0;
        int b = 1;
        int c = 2;
        while (length(cross(verts[poly[b]] - verts[poly[a]],
                            verts[poly[c]] - verts[poly[b]])) <= 1e-7f) {
            ++a;
            ++b;
            ++c;
        }

        if (anyReflex) {
            while (a < static_cast<int>(poly.size())) {
                b = (a + 1) % static_cast<int>(poly.size());
                if (reflexMark[a] && !reflexMark[b]) {
                    break;
                }
                ++a;
            }
        }
        c = (a + 2) % static_cast<int>(poly.size());

        triangles.emplace_back(poly[a], poly[b], poly[c]);

        poly.erase(poly.begin() + b);

        auto n = polyNormal(verts, poly);
        markReflex(verts, poly, reflexMark, n);
        takeOutTriangle(verts, poly, reflexMark, triangles);
    }
}

inline void polygonDecompose(std::vector<Vec3f>& verts,
                             std::vector<int>& poly,
                             std::vector<Vec3i>& triangles) {
    triangles.clear();
    if (poly.empty()) {
        return;
    }
    std::vector<char> reflexMark;
    auto n = polyNormal(verts, poly);
    markReflex(verts, poly, reflexMark, n);
    takeOutTriangle(verts, poly, reflexMark, triangles);
}

struct PrimitiveObject {
    AttrVector<Vec3f> verts;
    AttrVector<int>   points;
    AttrVector<Vec2i> lines;
    AttrVector<Vec3i> tris;
    AttrVector<Vec4i> quads;

    AttrVector<int>   loops;
    AttrVector<Vec2i> polys;
    AttrVector<Vec2i> edges;

    AttrVector<Vec2f> uvs;

    PrimitiveObject() = default;

    template <class Accept = std::variant<Vec3f, float>, class F>
    void foreach_attr(F&& f) {
        std::string pos_name = "pos";
        f(pos_name, verts.values);
        verts.template foreach_attr<Accept>(std::forward<F>(f));
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    void foreach_attr(F&& f) const {
        std::string const pos_name = "pos";
        f(pos_name, verts.values);
        verts.template foreach_attr<Accept>(std::forward<F>(f));
    }

    std::size_t num_attrs() const {
        return 1 + verts.num_attrs();
    }

    auto attr_keys() const {
        auto keys = verts.attr_keys();
        keys.insert(keys.begin(), "pos");
        return keys;
    }

    template <class T>
    auto& add_attr(std::string const& name) {
        if constexpr (std::is_same_v<T, Vec3f>) {
            if (name == "pos") return verts.values;
        } else {
            if (name == "pos") {
                throw std::runtime_error("PrimitiveObject: attribute 'pos' must be Vec3f");
            }
        }
        return verts.template add_attr<T>(name);
    }

    template <class T>
    auto& add_attr(std::string const& name, T const& value) {
        if constexpr (std::is_same_v<T, Vec3f>) {
            if (name == "pos") return verts.values;
        } else {
            if (name == "pos") {
                throw std::runtime_error("PrimitiveObject: attribute 'pos' must be Vec3f");
            }
        }
        if (verts.template attr_is<T>(name)) {
            return verts.template attr<T>(name);
        } else {
            auto& ret = verts.template add_attr<T>(name);
            ret.assign(size(), value);
            return ret;
        }
    }

    template <class T>
    auto const& attr(std::string const& name) const {
        if constexpr (std::is_same_v<T, Vec3f>) {
            if (name == "pos") return verts.values;
        } else {
            if (name == "pos") {
                throw std::runtime_error("PrimitiveObject: attribute 'pos' must be Vec3f");
            }
        }
        return verts.template attr<T>(name);
    }

    template <class T>
    auto& attr(std::string const& name) {
        if constexpr (std::is_same_v<T, Vec3f>) {
            if (name == "pos") return verts.values;
        } else {
            if (name == "pos") {
                throw std::runtime_error("PrimitiveObject: attribute 'pos' must be Vec3f");
            }
        }
        return verts.template attr<T>(name);
    }

    auto const& attr(std::string const& name) const {
        return verts.attr(name);
    }

    auto& attr(std::string const& name) {
        return verts.attr(name);
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    auto attr_visit(std::string const& name, F const& f) const {
        if (name == "pos") {
            return f(verts.values);
        } else {
            return verts.template attr_visit<Accept>(name, f);
        }
    }

    template <class Accept = std::variant<Vec3f, float>, class F>
    auto attr_visit(std::string const& name, F const& f) {
        if (name == "pos") {
            return f(verts.values);
        } else {
            return verts.template attr_visit<Accept>(name, f);
        }
    }

    bool has_attr(std::string const& name) const {
        if (name == "pos") return true;
        return verts.has_attr(name);
    }

    template <class T>
    bool attr_is(std::string const& name) const {
        if constexpr (std::is_same_v<T, Vec3f>) {
            if (name == "pos") return true;
        } else {
            if (name == "pos") return false;
        }
        return verts.template attr_is<T>(name);
    }

    std::size_t size() const {
        return verts.size();
    }

    void resize(std::size_t n) {
        verts.resize(n);
    }
};

} // namespace zeno::zs_fbx

