#include <zvec.h>
#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <map>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>


namespace zeno {

struct Bevel : INode2 {

    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.f; }
    void clearCalcResults() override {}

    ZErrorCode apply(zeno::INodeData* ptrNodeData) override {
        return ZErr_OK;
    }
};

// PolyBevel
// ------------------------------------------------------------
// 功能说明：
// 对输入多边形几何体的边/角进行倒角（bevel）处理，创建新的倒角面。
// 提供对倒角大小、形状、细分程度等控制。
// 此节点不依赖 Houdini 的 Group 系统，参数简化以适配 ZENO 实现。
// 本节点将对整个输入几何体进行统一倒角。
//
// 输入：
//   - Input：待倒角的几何体
//
// 参数：
//   Distance
//     倒角的大小（从原始边/角到倒角面的位置距离）。
//
//   Divisions
//     倒角面的细分行数（越大越圆滑）。
//
//   Shape
//     倒角形状类型：None、Solid、Crease、Chamfer、Round。
//     - None：只执行切断，不生成倒角面。
//     - Solid：标准倒角，沿相邻曲面形状。
//     - Crease：类似 Solid，但在角处生成几何细节。
//     - Chamfer：平面倒角。
//     - Round：圆滑倒角（需更多 Divisions）。
//
//   Convexity
//     倒角表面的凹/凸程度（1.0 为标准圆弧凸出，0 平直，-1 凹入）。
//
//   Profile Curve 控制选项：
//     用于控制倒角曲线的自定义轮廓（如 Ramp/Curve）
//     - Profile Type：None / Ramp / Curve 输入（默认 None）
//     - Profile Scale：轮廓曲线缩放
//     - Profile Reverse：翻转轮廓方向
//     - Profile Symmetrize：是否镜像对称
//
// 输出：
//   - Output：倒角后的新几何体
//
ZENDEFNODE_ABI(Bevel,
    Z_INPUTS(
        { "Input", _gParamType_Geometry },

        { "Distance", _gParamType_Float, ZFloat(0.1f) },

        { "Divisions", _gParamType_Int,
            ZInt(1),
            Slider,
            Z_ARRAY(1, 10, 1)
        },

        { "Shape", _gParamType_String,
            ZString("Solid"),
            Combobox,
            Z_STRING_ARRAY("Solid", "Chamfer", "Round", "Crease")
        },

        { "Convexity", _gParamType_Float,
            ZFloat(1.0f),
            Slider,
            Z_ARRAY(-1.0f, 1.0f, 0.1f)
        }
    ),

    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),

    "prim",
    "",
    "",
    ""
);

}