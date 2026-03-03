//
// ReadJsonFromString - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/types/JsonObject.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <tinygltf/json.hpp>
#include <cstring>

namespace zeno {

struct ReadJsonFromString : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        char buf[4096];
        nd->get_input2_string("content", buf, sizeof(buf));
        std::string content(buf);

        auto json = std::make_unique<JsonObject>();
        json->json = nlohmann::json::parse(content);
        nd->set_output_object("json", json.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(ReadJsonFromString, {
    {{gParamType_String, "content"}},
    {{gParamType_JsonObject, "json"}},
    {},
    {"json"},
});

} // namespace zeno
