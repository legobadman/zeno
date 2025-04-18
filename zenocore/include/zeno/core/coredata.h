#pragma once

#ifndef __CORE_DATA_H__
#define __CORE_DATA_H__

#include <zeno/container/zstring.h>
#include <zeno/container/zvector.h>
#include <zeno/container/zsharedptr.h>
#include <reflect/container/any>
#include <cassert>

namespace zeno
{
    struct Vec2i {
        int x, y;
    };

    struct Vec2f {
        float x, y;
    };

    struct Vec3i {
        int x, y, z;

        int& operator[](size_t index) {
            assert(index < 3 && index >= 0 && "Vec3i index out of range");
            switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }

        const int& operator[](size_t index) const {
            assert(index < 3 && index >= 0 && "Vec3i index out of range");
            switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }
    };

    struct Vec3f {
        float x, y, z;

        float& operator[](size_t index) {
            assert(index < 3 && index >= 0 && "Vec3i index out of range");
            switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }

        const float& operator[](size_t index) const {
            assert(index < 3 && index >= 0 && "Vec3i index out of range");
            switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            }
        }
    };

    struct Vec4i {
        int x, y, z, w;
    };

    struct Vec4f {
        float x, y, z, w;
    };

    struct PrimParamABI {

    };

}

#endif