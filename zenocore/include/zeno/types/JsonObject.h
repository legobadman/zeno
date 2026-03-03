#pragma once

#include <zeno/core/typeinfo.h>
#include <iobject2.h>
#include <tinygltf/json.hpp>

namespace zeno {

struct JsonObject : IObject2 {
    nlohmann::json json;
    ZObjectType type() const override { return (ZObjectType)gParamType_JsonObject; }
    IObject2* clone() const override { auto* c = new JsonObject; c->json = json; return c; }
    size_t key(char* buf, size_t buf_size) const override { (void)buf; (void)buf_size; return 0; }
    void update_key(const char*) override {}
    size_t serialize_json(char* buf, size_t buf_size) const override { (void)buf; (void)buf_size; return 0; }
    IUserData2* userData() override { return nullptr; }
    void Delete() override { delete this; }
};

} // namespace zeno
