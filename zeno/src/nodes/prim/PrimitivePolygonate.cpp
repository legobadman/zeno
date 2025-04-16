#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/PrimitiveUtils.h>
#include <zeno/utils/log.h>

namespace zeno {

ZENO_API void primPolygonate(PrimitiveObject *prim, bool with_uv) {
    prim->loops.reserve(prim->loops.size() + prim->tris.size() * 3 +
                        prim->quads.size() * 4 + prim->lines.size() * 2 +
                        prim->points.size());
    prim->polys.reserve(prim->polys.size() + prim->tris.size() +
                        prim->quads.size() + prim->lines.size() +
                        prim->points.size());

    int old_loop_base = prim->loops.size();
    int polynum = prim->polys.size();
    if (prim->tris.size()) {
        int base = prim->loops.size();
        for (int i = 0; i < prim->tris.size(); i++) {
            auto const &ind = prim->tris[i];
            prim->loops.push_back(ind[0]);
            prim->loops.push_back(ind[1]);
            prim->loops.push_back(ind[2]);
            prim->polys.push_back({base + i * 3, 3});
        }
        prim->polys.update();

        prim->tris.foreach_attr<AttrAcceptAll>([&](auto const &key, auto const &arr) {
            if (key == "uv0" || key == "uv1" || key == "uv2") {
                return;
            }
            using T = std::decay_t<decltype(arr[0])>;
            auto &newarr = prim->polys.add_attr<T>(key);
            for (auto i = 0; i < arr.size(); i++) {
                newarr[polynum + i] = arr[i];
            }
        });
    }

    polynum = prim->polys.size();
    if (prim->quads.size()) {
        int base = prim->loops.size();
        for (int i = 0; i < prim->quads.size(); i++) {
            auto const &ind = prim->quads[i];
            prim->loops.push_back(ind[0]);
            prim->loops.push_back(ind[1]);
            prim->loops.push_back(ind[2]);
            prim->loops.push_back(ind[3]);
            prim->polys.push_back({base + i * 4, 4});
        }
        prim->polys.update();

        prim->quads.foreach_attr<AttrAcceptAll>([&](auto const &key, auto const &arr) {
            if (key == "uv0" || key == "uv1" || key == "uv2" || key == "uv3") {
                return;
            }
            using T = std::decay_t<decltype(arr[0])>;
            auto &newarr = prim->polys.add_attr<T>(key);
            for (auto i = 0; i < arr.size(); i++) {
                newarr[polynum + i] = arr[i];
            }
        });
    }

    polynum = prim->polys.size();
    if (prim->lines.size()) {
        int base = prim->loops.size();
        for (int i = 0; i < prim->lines.size(); i++) {
            auto const &ind = prim->lines[i];
            prim->loops.push_back(ind[0]);
            prim->loops.push_back(ind[1]);
            prim->polys.push_back({base + i * 2, 2});
        }
        prim->polys.update();

        prim->lines.foreach_attr([&](auto const &key, auto const &arr) {
            using T = std::decay_t<decltype(arr[0])>;
            auto &newarr = prim->polys.add_attr<T>(key);
            for (auto i = 0; i < arr.size(); i++) {
                newarr[polynum + i] = arr[i];
            }
        });
    }

    polynum = prim->polys.size();
    if (prim->points.size()) {
        int base = prim->loops.size();
        for (int i = 0; i < prim->points.size(); i++) {
            auto ind = prim->points[i];
            prim->loops.push_back(ind);
            prim->polys.push_back({base + i, 1});
        }
        prim->polys.update();

        prim->points.foreach_attr([&](auto const &key, auto const &arr) {
            using T = std::decay_t<decltype(arr[0])>;
            auto &newarr = prim->polys.add_attr<T>(key);
            for (auto i = 0; i < arr.size(); i++) {
                newarr[polynum + i] = arr[i];
            }
        });
    }

    prim->loops.update();
    prim->polys.update();

    if (!(!prim->tris.has_attr("uv0") || !prim->tris.has_attr("uv1") ||
          !prim->tris.has_attr("uv2") || !with_uv)) {
        auto old_uvs_base = prim->uvs.size();
        prim->loops.add_attr<int>("uvs");
        auto &uv0 = prim->tris.attr<zeno::vec3f>("uv0");
        auto &uv1 = prim->tris.attr<zeno::vec3f>("uv1");
        auto &uv2 = prim->tris.attr<zeno::vec3f>("uv2");
        for (int i = 0; i < prim->tris.size(); i++) {
            prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 0] = old_uvs_base + i * 3 + 0;
            prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 1] = old_uvs_base + i * 3 + 1;
            prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 2] = old_uvs_base + i * 3 + 2;
            prim->uvs.emplace_back(uv0[i][0], uv0[i][1]);
            prim->uvs.emplace_back(uv1[i][0], uv1[i][1]);
            prim->uvs.emplace_back(uv2[i][0], uv2[i][1]);
        }
        // remove duplicate uv index
        {
            std::map<std::tuple<float, float>, int> mapping;
            auto &loopsuv = prim->loops.attr<int>("uvs");
            for (auto i = 0; i < prim->loops.size(); i++) {
                vec2f uv = prim->uvs[loopsuv[i]];
                if (mapping.count({uv[0], uv[1]}) == false) {
                    auto index = mapping.size();
                    mapping[{uv[0], uv[1]}] = index;
                }
                loopsuv[i] = mapping[{uv[0], uv[1]}];
            }
            prim->uvs.resize(mapping.size());
            for (auto const&[uv, index]: mapping) {
                prim->uvs[index] = {std::get<0>(uv), std::get<1>(uv)};
            }
        }
    }

    prim->tris.clear_with_attr();
    prim->quads.clear_with_attr();
    prim->lines.clear_with_attr();
    prim->points.clear_with_attr();
}

namespace {

struct PrimitivePolygonate : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        primPolygonate(prim.get(), ZImpl(get_param<bool>("with_uv")));
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimitivePolygonate,
        { /* inputs: */ {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        }, /* outputs: */ {
        {gParamType_Primitive, "prim"},
        }, /* params: */ {
        {gParamType_Bool, "with_uv", "1"},
        }, /* category: */ {
        "primitive",
        }});

}
}
