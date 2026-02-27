#include "ImageObject.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace zeno::zs_imgcv {

ImageObject::ImageObject(int width, int height, int channels, float init_value) {
    create(width, height, channels, init_value);
}

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
    return nullptr;
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
