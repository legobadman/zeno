#pragma once

#include <zeno/core/Descriptor.h>
#include <zeno/core/data.h>
#include <zeno/core/Assets.h>
#include <zeno/core/ObjectManager.h>
#include <set>

namespace zeno {

class CalcManager {
public:
    CalcManager();
    ZENO_API void run();
    ZENO_API void mark_frame_change_dirty();

private:

};

}