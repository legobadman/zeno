#include "ImageObject.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace zeno::zs_imgcv::detail {

ImageUserData::ImageUserData(ImageObject* owner) : m_owner(owner) {}
ImageUserData::ImageUserData(const ImageUserData&) : m_owner(nullptr) {}

IUserData2* ImageUserData::clone() { return new ImageUserData(m_owner); }
void ImageUserData::copy(IUserData2* pUserData) { (void)pUserData; }

bool ImageUserData::has(const char* key) {
    if (!m_owner) return false;
    const char* k = key;
    return (std::strcmp(k, "w") == 0 || std::strcmp(k, "h") == 0 ||
            std::strcmp(k, "channels") == 0 || std::strcmp(k, "isImage") == 0 ||
            std::strcmp(k, "pixels") == 0);
}

std::size_t ImageUserData::size() const { return m_owner ? 4u : 0u; }

std::size_t ImageUserData::get_string(const char* key, const char* defl, char* ret_buf, std::size_t cap) const {
    (void)key;
    const char* s = (defl != nullptr) ? defl : "";
    const std::size_t len = std::strlen(s);
    if (ret_buf != nullptr && cap > 0) {
        const std::size_t copy = (len < cap - 1) ? len : cap - 1;
        std::memcpy(ret_buf, s, copy);
        ret_buf[copy] = '\0';
        return copy;
    }
    return len;
}
void ImageUserData::set_string(const char* key, const char* sval) { (void)key; (void)sval; }
bool ImageUserData::has_string(const char* key) const { (void)key; return false; }
void ImageUserData::set_int(const char* key, int iVal) { (void)key; (void)iVal; }
void ImageUserData::set_float(const char* key, float fVal) { (void)key; (void)fVal; }
void ImageUserData::set_vec2f(const char* key, const zeno::Vec2f& vec) { (void)key; (void)vec; }
void ImageUserData::set_vec2i(const char* key, const zeno::Vec2i& vec) { (void)key; (void)vec; }
void ImageUserData::set_vec3f(const char* key, const zeno::Vec3f& vec) { (void)key; (void)vec; }
void ImageUserData::set_vec3i(const char* key, const zeno::Vec3i& vec) { (void)key; (void)vec; }
void ImageUserData::set_vec4f(const char* key, const zeno::Vec4f& vec) { (void)key; (void)vec; }
void ImageUserData::set_vec4i(const char* key, const zeno::Vec4i& vec) { (void)key; (void)vec; }
void ImageUserData::set_bool(const char* key, bool val) { (void)key; (void)val; }

zeno::Vec2f ImageUserData::get_vec2f(const char* key, zeno::Vec2f defl) const { (void)key; return defl; }
zeno::Vec2i ImageUserData::get_vec2i(const char* key) const { (void)key; return zeno::Vec2i(); }
zeno::Vec3f ImageUserData::get_vec3f(const char* key, zeno::Vec3f defl) const { (void)key; return defl; }
zeno::Vec3i ImageUserData::get_vec3i(const char* key) const { (void)key; return zeno::Vec3i(); }
zeno::Vec4f ImageUserData::get_vec4f(const char* key) const { (void)key; return zeno::Vec4f(); }
zeno::Vec4i ImageUserData::get_vec4i(const char* key) const { (void)key; return zeno::Vec4i(); }
bool ImageUserData::has_vec2f(const char* key) const { (void)key; return false; }
bool ImageUserData::has_vec2i(const char* key) const { (void)key; return false; }
bool ImageUserData::has_vec3f(const char* key) const { (void)key; return false; }
bool ImageUserData::has_vec3i(const char* key) const { (void)key; return false; }
bool ImageUserData::has_vec4f(const char* key) const { (void)key; return false; }
bool ImageUserData::has_vec4i(const char* key) const { (void)key; return false; }
float ImageUserData::get_float(const char* key, float defl) const { (void)key; return defl; }
bool ImageUserData::has_float(const char* key) const { (void)key; return false; }

int ImageUserData::get_int(const char* key, int defl) const {
    if (!m_owner) return defl;
    if (std::strcmp(key, "w") == 0) return m_owner->width();
    if (std::strcmp(key, "h") == 0) return m_owner->height();
    if (std::strcmp(key, "channels") == 0) return m_owner->channels();
    if (std::strcmp(key, "isImage") == 0) return 1;
    return defl;
}

bool ImageUserData::has_int(const char* key) const {
    if (!m_owner) return false;
    return (std::strcmp(key, "w") == 0 || std::strcmp(key, "h") == 0 ||
            std::strcmp(key, "channels") == 0 || std::strcmp(key, "isImage") == 0);
}

bool ImageUserData::get_bool(const char* key, bool defl) const {
    if (!m_owner) return defl;
    if (std::strcmp(key, "isImage") == 0) return true;
    return defl;
}

bool ImageUserData::has_bool(const char* key) const {
    return m_owner && std::strcmp(key, "isImage") == 0;
}

std::size_t ImageUserData::get_float_arr(const char* key, float* buf, std::size_t cap) const {
    if (!m_owner || std::strcmp(key, "pixels") != 0) return 0;
    const auto& raw = m_owner->raw();
    const std::size_t n = raw.size();
    const std::size_t copy = (n < cap) ? n : cap;
    if (buf != nullptr && copy > 0) {
        std::memcpy(buf, raw.data(), copy * sizeof(float));
    }
    return copy;
}
void ImageUserData::del(const char* key) { (void)key; }

} // namespace zeno::zs_imgcv::detail

namespace zeno::zs_imgcv {

ImageObject::ImageObject() : m_userData(this) {}

ImageObject::ImageObject(int width, int height, int channels, float init_value) : m_userData(this) {
    create(width, height, channels, init_value);
}

ImageObject::ImageObject(const ImageObject& other)
    : m_width(other.m_width), m_height(other.m_height), m_channels(other.m_channels),
      m_pixels(other.m_pixels), m_key(other.m_key), m_userData(this) {}

bool ImageObject::create(int width, int height, int channels, float init_value) {
    if (width <= 0 || height <= 0 || channels <= 0) {
        clear();
        return false;
    }
    m_width = width;
    m_height = height;
    m_channels = channels;
    m_pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) *
                        static_cast<std::size_t>(channels),
                    init_value);
    return true;
}

void ImageObject::clear() {
    m_width = 0;
    m_height = 0;
    m_channels = 0;
    m_pixels.clear();
}

void ImageObject::fill(float value) {
    std::fill(m_pixels.begin(), m_pixels.end(), value);
}

bool ImageObject::empty() const {
    return m_pixels.empty();
}

int ImageObject::width() const {
    return m_width;
}

int ImageObject::height() const {
    return m_height;
}

int ImageObject::channels() const {
    return m_channels;
}

std::size_t ImageObject::pixel_count() const {
    return static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
}

bool ImageObject::valid_pixel(int x, int y) const {
    return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

bool ImageObject::valid_channel(int c) const {
    return c >= 0 && c < m_channels;
}

bool ImageObject::get_pixel(int x, int y, std::vector<float>& out) const {
    if (!valid_pixel(x, y) || m_channels <= 0) {
        return false;
    }
    const std::size_t base = pixel_offset(x, y);
    out.assign(m_pixels.begin() + static_cast<std::ptrdiff_t>(base),
               m_pixels.begin() + static_cast<std::ptrdiff_t>(base + m_channels));
    return true;
}

bool ImageObject::set_pixel(int x, int y, const std::vector<float>& in) {
    if (!valid_pixel(x, y) || static_cast<int>(in.size()) != m_channels) {
        return false;
    }
    const std::size_t base = pixel_offset(x, y);
    std::copy(in.begin(), in.end(), m_pixels.begin() + static_cast<std::ptrdiff_t>(base));
    return true;
}

bool ImageObject::get_channel(int x, int y, int c, float& out) const {
    if (!valid_pixel(x, y) || !valid_channel(c)) {
        return false;
    }
    out = m_pixels[pixel_offset(x, y) + static_cast<std::size_t>(c)];
    return true;
}

bool ImageObject::set_channel(int x, int y, int c, float value) {
    if (!valid_pixel(x, y) || !valid_channel(c)) {
        return false;
    }
    m_pixels[pixel_offset(x, y) + static_cast<std::size_t>(c)] = value;
    return true;
}

bool ImageObject::erase_pixel(int x, int y) {
    if (!valid_pixel(x, y)) {
        return false;
    }
    const std::size_t base = pixel_offset(x, y);
    std::fill(m_pixels.begin() + static_cast<std::ptrdiff_t>(base),
              m_pixels.begin() + static_cast<std::ptrdiff_t>(base + m_channels), 0.0f);
    return true;
}

const std::vector<float>& ImageObject::raw() const {
    return m_pixels;
}

std::vector<float>& ImageObject::raw() {
    return m_pixels;
}

IObject2* ImageObject::clone() const {
    return new ImageObject(*this);
}

std::size_t ImageObject::key(char* buf, std::size_t buf_size) const {
    const char* s = m_key.c_str();
    const std::size_t len = m_key.size();
    if (buf != nullptr && buf_size > 0) {
        const std::size_t copy = (len < (buf_size - 1)) ? len : (buf_size - 1);
        std::memcpy(buf, s, copy);
        buf[copy] = '\0';
    }
    return len;
}

void ImageObject::update_key(const char* key) {
    m_key = (key == nullptr) ? "" : key;
}

std::size_t ImageObject::serialize_json(char* buf, std::size_t buf_size) const {
    const int required = std::snprintf(
        nullptr, 0, "{\"width\":%d,\"height\":%d,\"channels\":%d}", m_width, m_height, m_channels);
    if (required < 0) {
        return 0;
    }
    const std::size_t len = static_cast<std::size_t>(required);
    if (buf != nullptr && buf_size > 0) {
        std::snprintf(buf, buf_size, "{\"width\":%d,\"height\":%d,\"channels\":%d}", m_width, m_height,
                      m_channels);
    }
    return len;
}

IUserData2* ImageObject::userData() {
    return &m_userData;
}

void ImageObject::Delete() {
    delete this;
}

ZObjectType ImageObject::type() const {
    return ZObj_Image;
}

std::size_t ImageObject::pixel_offset(int x, int y) const {
    return (static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
            static_cast<std::size_t>(x)) *
           static_cast<std::size_t>(m_channels);
}

} // namespace zeno::zs_imgcv

