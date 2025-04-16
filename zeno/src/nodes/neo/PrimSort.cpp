#include <numeric>
#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/para/parallel_for.h>
#include <stdexcept>
#include <zeno/para/parallel_sort.h>

namespace zeno {

struct PrimSort : INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto attr = ZImpl(get_input2<std::string>("Attribute"));
    auto mode = ZImpl(get_input2<std::string>("Vertex Sort"));
    auto reverse = ZImpl(get_input2<bool>("Reverse"));
    
    if (mode == "NoChange") {
        if (reverse) {
            prim->verts.forall_attr<AttrAcceptAll>([&] (auto &key, auto &arr) {
                std::reverse(arr.begin(), arr.end());
            });
            /*for (auto& tri : prim->tris) {
                std::swap(tri[0], tri[2]);
            }*/
        }
      ZImpl(set_output("prim", std::move(prim)));
    }
    else if (mode == "ByAttribute") {
        auto &tris = prim->tris.values;
        std::vector<size_t> indices(prim->verts.size());
        std::iota(indices.begin(), indices.end(), 0);
        
        if (prim->attr_is<float>(attr)){
            auto &tag = prim->verts.attr<float>(attr);
            zeno::parallel_stable_sort(indices.begin(), indices.end(), [&tag, reverse] (size_t a, size_t b) {// or std::stable_sort
                return reverse ? tag[a] > tag[b] : tag[a] < tag[b];
            });
        }
        else if (prim->attr_is<int>(attr)){
            auto &tag = prim->verts.attr<int>(attr);
            zeno::parallel_stable_sort(indices.begin(), indices.end(), [&tag, reverse] (size_t a, size_t b) {
                return reverse ? tag[a] > tag[b] : tag[a] < tag[b];
            });
        }
        else{
            throw std::runtime_error("Attribute type not supported");
        }

        prim->verts.forall_attr<AttrAcceptAll>([&] (auto &key, auto &arr) {
            auto oldarr = std::move(arr);
            arr.resize(oldarr.size());
            parallel_for(oldarr.size(), [&] (size_t i) {
                arr[i] = oldarr[indices[i]];
            });
        });

        // inverse mapping
        std::vector<size_t> reverse_indices(indices.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            reverse_indices[indices[i]] = i;
        }
        for (auto& tri : tris) {
            for (auto& idx : tri) {
                idx = reverse_indices[idx];
            }
        }
        ZImpl(set_output("prim", std::move(prim)));
    }
  }
};

ZENDEFNODE(PrimSort, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {"enum NoChange ByAttribute", "Vertex Sort", "NoChange"},//Add more methods
    {gParamType_String, "Attribute", "index"},
    {gParamType_Bool, "Reverse", "0"}
    //{gParamType_Int, "component", "0"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

}