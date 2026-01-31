#pragma once

#include <zeno/types/GeometryObject.h>

namespace zeno {

    struct ObjectRecorder
    {
        std::set<GeometryObject*> m_geom_impls;
    };
}

