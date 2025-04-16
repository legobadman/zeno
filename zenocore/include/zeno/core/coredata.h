#pragma once

#ifndef __CORE_DATA_H__
#define __CORE_DATA_H__

#include <zeno/container/zstring.h>
#include <zeno/container/zvector.h>
#include <zeno/container/zsharedptr.h>
#include <reflect/container/any>

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
    };

    struct Vec3f {
        float x, y, z;
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