#pragma once

#include <utility>
#include <type_traits>
#include <functional>
#include <vector>
#include <memory>
#include <vec.h>
#include <zvec.h>

namespace zeno {

    template <class Func = std::function<void()>>
    class scope_exit {
        static_assert(std::is_same_v<std::decay_t<Func>, Func>);

        Func func;
        bool enabled{ false };

    public:
        scope_exit() = default;

        scope_exit(Func&& func) noexcept : func(std::move(func)), enabled(true) {
        }

        bool has_value() const noexcept {
            return enabled;
        }

        void release() noexcept {
            enabled = false;
        }

        void reset() {
            if (enabled) {
                func();
                enabled = false;
            }
        }

        ~scope_exit() noexcept {
            if (enabled)
                func();
        }

        scope_exit(scope_exit const&) = delete;
        scope_exit& operator=(scope_exit const&) = delete;

        scope_exit(scope_exit&& that) noexcept : func(std::move(that.func)), enabled(that.enabled) {
            that.enabled = false;
        }

        scope_exit& operator=(scope_exit&& that) noexcept {
            if (this != &that) {
                enabled = that.enabled;
                that.enabled = false;
                func = std::move(that.func);
            }
            return *this;
        }
    };

    std::unique_ptr<Vec3f[]> convert_points_to_abi(
        const std::vector<vec3f>& points,
        size_t& outSize
    );



}