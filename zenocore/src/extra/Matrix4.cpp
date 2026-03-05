//
// Matrix4 - construct a 4x4 matrix from four vec4 rows (INode2 ABI)
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/utils/vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <glm/glm.hpp>

namespace zeno {

struct Matrix4 : INode2 {
    DEF_OVERRIDE_FOR_INODE

    ZErrorCode apply(INodeData* nd) override {
        ZNodeParams* params = static_cast<ZNodeParams*>(nd);
        // Inputs are ABI Vec4f; convert to glm::vec4 rows.
        Vec4f r0_abi = nd->get_input2_vec4f("r0");
        Vec4f r1_abi = nd->get_input2_vec4f("r1");
        Vec4f r2_abi = nd->get_input2_vec4f("r2");
        Vec4f r3_abi = nd->get_input2_vec4f("r3");

        glm::vec4 r0(r0_abi.x, r0_abi.y, r0_abi.z, r0_abi.w);
        glm::vec4 r1(r1_abi.x, r1_abi.y, r1_abi.z, r1_abi.w);
        glm::vec4 r2(r2_abi.x, r2_abi.y, r2_abi.z, r2_abi.w);
        glm::vec4 r3(r3_abi.x, r3_abi.y, r3_abi.z, r3_abi.w);

        // glm::mat4 is column-major; construct by columns explicitly.
        glm::mat4 m;
        m[0] = glm::vec4(r0.x, r1.x, r2.x, r3.x);
        m[1] = glm::vec4(r0.y, r1.y, r2.y, r3.y);
        m[2] = glm::vec4(r0.z, r1.z, r2.z, r3.z);
        m[3] = glm::vec4(r0.w, r1.w, r2.w, r3.w);

        // Output as Matrix4 primitive.
        params->set_primitive_output("matrix", m);
        return ZErr_OK;
    }
};

ZENDEFNODE(Matrix4, {
    {
        {gParamType_Vec4f, "r0", "0,0,0,0"},
        {gParamType_Vec4f, "r1", "0,0,0,0"},
        {gParamType_Vec4f, "r2", "0,0,0,0"},
        {gParamType_Vec4f, "r3", "0,0,0,0"},
    },
    {
        ParamPrimitive("matrix", gParamType_Matrix4, glm::mat4(1.0f))
    },
    {},
    {"numeric"}
});

} // namespace zeno

