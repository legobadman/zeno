#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <iobject2.h>

namespace zeno::zs_imgcv {

struct ImageObject;

namespace detail {

// IUserData2 implementation exposing image metadata (w, h, channels, isImage) and pixel data (pixels).
// Allows main program to use ImageObject via userData() without depending on ImageObject type.
struct ImageUserData : IUserData2 {
    ImageObject* m_owner = nullptr;
    explicit ImageUserData(ImageObject* owner);
    ImageUserData(const ImageUserData& other);

    IUserData2* clone() override;
    void copy(IUserData2* pUserData) override;
    bool has(const char* key) override;
    std::size_t size() const override;
    std::size_t get_string(const char* key, const char* defl, char* ret_buf, std::size_t cap) const override;
    void set_string(const char* key, const char* sval) override;
    bool has_string(const char* key) const override;
    int get_int(const char* key, int defl = 0) const override;
    void set_int(const char* key, int iVal) override;
    bool has_int(const char* key) const override;
    float get_float(const char* key, float defl = 0.f) const override;
    void set_float(const char* key, float fVal) override;
    bool has_float(const char* key) const override;
    Vec2f get_vec2f(const char* key, Vec2f defl = Vec2f()) const override;
    void set_vec2f(const char* key, const Vec2f& vec) override;
    bool has_vec2f(const char* key) const override;
    Vec2i get_vec2i(const char* key) const override;
    void set_vec2i(const char* key, const Vec2i& vec) override;
    bool has_vec2i(const char* key) const override;
    Vec3f get_vec3f(const char* key, Vec3f defl = Vec3f()) const override;
    void set_vec3f(const char* key, const Vec3f& vec) override;
    bool has_vec3f(const char* key) const override;
    Vec3i get_vec3i(const char* key) const override;
    void set_vec3i(const char* key, const Vec3i& vec) override;
    bool has_vec3i(const char* key) const override;
    Vec4f get_vec4f(const char* key) const override;
    void set_vec4f(const char* key, const Vec4f& vec) override;
    bool has_vec4f(const char* key) const override;
    Vec4i get_vec4i(const char* key) const override;
    void set_vec4i(const char* key, const Vec4i& vec) override;
    bool has_vec4i(const char* key) const override;
    bool get_bool(const char* key, bool defl = false) const override;
    void set_bool(const char* key, bool val = false) override;
    bool has_bool(const char* key) const override;
    std::size_t get_float_arr(const char* key, float* buf, std::size_t cap) const override;
    void del(const char* key) override;
};

// Local copy of 64-bit FNV-1a used by zeno::reflect::hash_64_typename.
constexpr std::uint64_t hash_64_typename(std::string_view str) noexcept {
    std::uint64_t hash = 0xcbf29ce484222325ULL;
    for (const unsigned char c : str) {
        hash ^= c;
        hash *= 0x100000001b3ULL;
    }
    return hash;
}
} // namespace detail

// Module-private image object type code.
// Outside zs_imgcv it is just an opaque unique number.
inline constexpr std::uint64_t gParamType_ImageObject =
    detail::hash_64_typename("shared_ptr<zeno::zs_imgcv::ImageObject>");

struct ImageObject final : IObject2 {
    ImageObject();
    ImageObject(int width, int height, int channels, float init_value = 0.0f);
    ImageObject(const ImageObject& other);

    // Basic CRUD-style image operations.
    bool create(int width, int height, int channels, float init_value = 0.0f);
    void clear();
    void fill(float value);

    [[nodiscard]] bool empty() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int channels() const;
    [[nodiscard]] std::size_t pixel_count() const;

    [[nodiscard]] bool valid_pixel(int x, int y) const;
    [[nodiscard]] bool valid_channel(int c) const;

    bool get_pixel(int x, int y, std::vector<float>& out) const;
    bool set_pixel(int x, int y, const std::vector<float>& in);
    bool get_channel(int x, int y, int c, float& out) const;
    bool set_channel(int x, int y, int c, float value);
    bool erase_pixel(int x, int y);

    const std::vector<float>& raw() const;
    std::vector<float>& raw();

public: // IObject2
    IObject2* clone() const override;
    std::size_t key(char* buf, std::size_t buf_size) const override;
    void update_key(const char* key) override;
    std::size_t serialize_json(char* buf, std::size_t buf_size) const override;
    IUserData2* userData() override;
    void Delete() override;
    ZObjectType type() const override;

private:
    std::size_t pixel_offset(int x, int y) const;

private:
    detail::ImageUserData m_userData;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    std::vector<float> m_pixels;
    std::string m_key;
};

} // namespace zeno::zs_imgcv
