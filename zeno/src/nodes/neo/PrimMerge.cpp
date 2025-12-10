#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/geo/commonutil.h>
#include <zeno/para/parallel_for.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/log.h>
#include <zeno/zeno.h>
#include <unordered_set>

namespace zeno {

static void set_special_attr_remap(PrimitiveObject *p, std::string attr_name, std::unordered_map<std::string, int> &facesetNameMap) {
    UserData* pUserData = dynamic_cast<UserData*>(p->userData());
    int faceset_count = pUserData->get_int(stdString2zs(attr_name + "_count"), 0);
    {
        std::unordered_map<int, int> cur_faceset_index_map;
        for (auto i = 0; i < faceset_count; i++) {
            auto path = pUserData->get2<std::string>(format("{}_{}", attr_name, i));
            if (facesetNameMap.count(path) == 0) {
                int new_index = facesetNameMap.size();
                facesetNameMap[path] = new_index;
            }
            cur_faceset_index_map[i] = facesetNameMap[path];
        }

        for (int i = 0; i < p->tris.size(); i++) {
            p->tris.attr<int>(attr_name)[i] = cur_faceset_index_map[p->tris.attr<int>(attr_name)[i]];
        }
        for (int i = 0; i < p->quads.size(); i++) {
            p->quads.attr<int>(attr_name)[i] = cur_faceset_index_map[p->quads.attr<int>(attr_name)[i]];
        }
        for (int i = 0; i < p->polys.size(); i++) {
            p->polys.attr<int>(attr_name)[i] = cur_faceset_index_map[p->polys.attr<int>(attr_name)[i]];
        }
    }
}


namespace {

struct PrimMerge : INode {
    virtual void apply() override {
        auto lst = get_input_ListObject("listPrim");
        std::vector<zeno::PrimitiveObject*> primList;
        std::vector<std::unique_ptr<PrimitiveObject>> spPrims;
        for (const auto& obj : lst->m_impl->m_objects) {
            if (auto geom = dynamic_cast<zeno::GeometryObject_Adapter*>(obj.get())) {
                auto spPrim = geom->toPrimitiveObject();
                primList.push_back(spPrim.get());
                spPrims.push_back(std::move(spPrim));
            }
        }

        auto tagAttr = ZImpl(get_input<StringObject>("tagAttr"))->get();
        //initialize
        bool tag_on_vert = false;
        bool tag_on_face = false;
        auto tag_scope = ZImpl(get_input2<std::string>("tag_scope"));
        if (tag_scope == "vert") {
            tag_on_vert = true;
        }
        else if (tag_scope == "face") {
            tag_on_face = true;
        }
        else {
            tag_on_vert = true;
            tag_on_face = true;
        }

        auto outprim = primMergeWithFacesetMatid2(primList, tagAttr, tag_on_vert, tag_on_face);

        //auto outprim = std::make_unique<PrimitiveObject>(*primList[0]);
        set_output("prim", create_GeometryObject(outprim.get()));
    }
};

ZENDEFNODE(PrimMerge, {
  {
      {gParamType_List, "listPrim", "", zeno::Socket_ReadOnly},
      {gParamType_String, "tagAttr", ""},
      {"enum vert face vert_face", "tag_scope", "vert"},
  },
  {
      {gParamType_Geometry, "prim"},
  },
  {},
  {"primitive"},
});

} // namespace
} // namespace zeno

