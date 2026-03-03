//
// CameraNode - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/types/CameraObject.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <regex>

namespace zeno {

namespace {
static std::string get_input2_string_helper(INodeData* nd, const char* name) {
    char buf[512];
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}
} // namespace

struct CameraNode : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto camera = std::make_unique<CameraObject>();
        auto pos = nd->get_input2_vec3f("pos");
        auto up = nd->get_input2_vec3f("up");
        auto view = nd->get_input2_vec3f("view");
        camera->pos = {pos.x, pos.y, pos.z};
        camera->up = {up.x, up.y, up.z};
        camera->view = {view.x, view.y, view.z};
        camera->fov = nd->get_input2_float("fov");
        camera->aperture = nd->get_input2_float("aperture");
        camera->focalPlaneDistance = nd->get_input2_float("focalPlaneDistance");
        camera->userData()->set_float("frame", nd->get_input2_float("frame"));

        auto other_props = get_input2_string_helper(nd, "other");
        std::regex reg(",");
        std::sregex_token_iterator p(other_props.begin(), other_props.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<float> prop_vals;
        while (p != end) {
            prop_vals.push_back(std::stof(*p));
            p++;
        }
        if (prop_vals.size() == 6) {
            camera->pivot = {prop_vals[0], prop_vals[1], prop_vals[2]};
        }

        nd->set_output_object("camera", camera.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(CameraNode, {
    {
        {gParamType_Vec3f, "pos", "0,0,5"},
        {gParamType_Vec3f, "up", "0,1,0"},
        {gParamType_Vec3f, "view", "0,0,-1"},
        {gParamType_Float, "fov", "45"},
        {gParamType_Float, "aperture", "11"},
        {gParamType_Float, "focalPlaneDistance", "2.0"},
        {gParamType_String, "other", ""},
        {gParamType_Int, "frame", "0"},
    },
    {{gParamType_Camera, "camera"}},
    {},
    {"FBX"},
});

} // namespace zeno
