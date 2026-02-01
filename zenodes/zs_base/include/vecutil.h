#pragma once

#include <vec.h>
#include <zvec.h>

inline zeno::vec2f toVec2f(const zeno::Vec2f& rhs) { return zeno::vec2f(rhs.x, rhs.y); }
inline zeno::Vec2f toAbiVec2f(const zeno::vec2f& rhs) { return zeno::Vec2f{ rhs[0], rhs[1] }; }

inline zeno::vec2i toVec2i(const zeno::Vec2i& rhs) { return zeno::vec2i(rhs.x, rhs.y); }
inline zeno::Vec2i toAbiVec2i(const zeno::vec2i& rhs) { return zeno::Vec2i{ rhs[0], rhs[1] }; }

inline zeno::vec3f toVec3f(const zeno::Vec3f& rhs) { return zeno::vec3f(rhs.x, rhs.y, rhs.z); }
inline zeno::Vec3f toAbiVec3f(const zeno::vec3f& rhs) { return zeno::Vec3f{ rhs[0], rhs[1], rhs[2] }; }

inline zeno::vec3i toVec3i(const zeno::Vec3i& rhs) { return zeno::vec3i(rhs.x, rhs.y, rhs.z); }
inline zeno::Vec3i toAbiVec3i(const zeno::vec3i& rhs) { return zeno::Vec3i{ rhs[0], rhs[1], rhs[2] }; }

inline zeno::vec4f toVec4f(const zeno::Vec4f& rhs) { return zeno::vec4f(rhs.x, rhs.y, rhs.z, rhs.w); }
inline zeno::Vec4f toAbiVec4f(const zeno::vec4f& rhs) { return zeno::Vec4f{ rhs[0], rhs[1], rhs[2], rhs[3] }; }

inline zeno::vec4i toVec4i(const zeno::Vec4i& rhs) { return zeno::vec4i(rhs.x, rhs.y, rhs.z, rhs.w); }
inline zeno::Vec4i toAbiVec4i(const zeno::vec4i& rhs) { return zeno::Vec4i{ rhs[0], rhs[1], rhs[2], rhs[3] }; }