#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <iobject2.h>

namespace zeno::zs_imgcv {

namespace detail {
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
    ImageObject() = default;
    ImageObject(int width, int height, int channels, float init_value = 0.0f);

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
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    std::vector<float> m_pixels;
    std::string m_key;
};

} // namespace zeno::zs_imgcv
