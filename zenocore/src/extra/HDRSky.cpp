//
// HDRSky - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/fileio.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <filesystem>

namespace zeno {

struct HDRSky : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto prim = std::make_unique<PrimitiveObject>();
        prim->resize(1);  // zenocore create_GeometryObject requires non-empty verts
        std::string path = "";
        if (nd->has_input("path")) {
            char buf[4096];
            nd->get_input2_string("path", buf, sizeof(buf));
            path = buf;
            std::string native_path = std::filesystem::u8path(path).string();
            if (!path.empty() && !file_exists(native_path)) {
                nd->report_error("HDRSky file not exists");
                return ZErr_ParamError;
            }
        }
        auto* pUserData = dynamic_cast<UserData*>(prim->userData());
        if (pUserData) {
            pUserData->set2("isRealTimeObject", 1);
            pUserData->set2("HDRSky", std::move(path));
            pUserData->set2("evnTexRotation", nd->get_input2_float("rotation"));
            auto r3d = nd->get_input2_vec3f("rotation3d");
            pUserData->set2("evnTex3DRotation", zeno::vec3f(r3d.x, r3d.y, r3d.z));
            pUserData->set2("evnTexStrength", nd->get_input2_float("strength"));
            pUserData->set2("enable", nd->get_input2_bool("enable"));
        }
        auto geom = create_GeometryObject(prim.get());
        if (!geom) {
            nd->report_error("HDRSky: create_GeometryObject failed");
            return ZErr_ParamError;
        }
        prim.release();
        nd->set_output_object("HDRSky", geom.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(HDRSky, {
    {
        {gParamType_Bool, "enable", "1"},
        {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::ReadPathEdit},
        {gParamType_Float, "rotation", "0"},
        {gParamType_Vec3f, "rotation3d", "0,0,0"},
        {gParamType_Float, "strength", "1"},
    },
    {{gParamType_Geometry, "HDRSky"}},
    {},
    {"shader"},
});

} // namespace zeno
