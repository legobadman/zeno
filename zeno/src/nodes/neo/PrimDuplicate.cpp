#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/wangsrng.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/utils/orthonormal.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/task_group.h>
#include <zeno/utils/overloaded.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/log.h>
#include <cstring>
#include <cstdlib>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {

ZENO_API std::shared_ptr<PrimitiveObject> primDuplicate(PrimitiveObject *parsPrim, PrimitiveObject *meshPrim, std::string dirAttr, std::string tanAttr, std::string radAttr, std::string onbType, float radius, bool copyParsAttr, bool copyMeshAttr) {
    auto prim = std::make_shared<PrimitiveObject>();
    auto hasDirAttr = boolean_variant(!dirAttr.empty());
    auto indOnbType = array_index({"XYZ", "YXZ", "YZX", "ZYX", "ZXY", "XZY"}, onbType);
    auto hasOnbType = boolean_variant(indOnbType != 0);
    auto hasRadAttr = boolean_variant(!radAttr.empty());
    auto hasRadius = boolean_variant(radius != 1);

    immediate_task_group tg;

    prim->verts.resize(parsPrim->verts.size() * meshPrim->verts.size());
    prim->points.resize(parsPrim->verts.size() * meshPrim->points.size());
    prim->lines.resize(parsPrim->verts.size() * meshPrim->lines.size());
    prim->tris.resize(parsPrim->verts.size() * meshPrim->tris.size());
    prim->quads.resize(parsPrim->verts.size() * meshPrim->quads.size());
    prim->loops.resize(parsPrim->verts.size() * meshPrim->loops.size());
    prim->polys.resize(parsPrim->verts.size() * meshPrim->polys.size());

    std::visit([&] (auto hasDirAttr, auto hasRadius, auto hasRadAttr, auto hasOnbType) {
        auto func = [&] (auto const &accRad) {
            auto func = [&] (auto const &accDir, auto hasTanAttr, auto const &accTan) {
                tg.add([&] {
                    parallel_for((size_t)0, parsPrim->verts.size(), [&] (size_t i) {
                        auto basePos = parsPrim->verts[i];
                        for (size_t j = 0; j < meshPrim->verts.size(); j++) {
                            auto pos = meshPrim->verts[j];
                            if constexpr (hasRadAttr.value) {
                                pos *= accRad[i];
                            }
                            if constexpr (hasRadius.value) {
                                pos *= radius;
                            }
                            if constexpr (hasOnbType.value) {
                                const std::array<std::size_t, 6> a0{0, 1, 1, 2, 2, 0};
                                const std::array<std::size_t, 6> a1{1, 0, 2, 1, 0, 2};
                                const std::array<std::size_t, 6> a2{2, 2, 0, 0, 1, 1};
                                auto i0 = a0[indOnbType];
                                auto i1 = a1[indOnbType];
                                auto i2 = a2[indOnbType];
                                pos = {pos[i0], pos[i1], pos[i2]};
                            }
                            if constexpr (hasDirAttr.value) {
                                auto t0 = accDir[i];
                                t0 = normalizeSafe(t0);
                                vec3f t1, t2;
                                if constexpr (hasTanAttr.value) {
                                    t1 = accTan[i];
                                    t1 = normalizeSafe(t1);
                                    t2 = cross(t0, t1);
                                    t2 = normalizeSafe(t2);
                                } else {
                                    pixarONB(t0, t1, t2);
                                }
                                pos = pos[2] * t0 + pos[1] * t1 + pos[0] * t2;
                            }
                            prim->verts[i * meshPrim->verts.size() + j] = basePos + pos;
                        }
                    });
                });
            };
            if constexpr (hasDirAttr.value) {
                auto const &accDir = parsPrim->attr<zeno::vec3f>(dirAttr);
                if (!tanAttr.empty())
                    func(accDir, std::true_type{}, parsPrim->attr<zeno::vec3f>(tanAttr));
                else
                    func(accDir, std::false_type{}, std::array<int, 0>{});
            } else {
                func(std::array<int, 0>{}, std::false_type{}, std::array<int, 0>{});
            }
        };
        if constexpr (hasRadAttr)
            parsPrim->verts.attr_visit(radAttr, func);
        else
            func(std::array<int, 0>{});
    }, hasDirAttr, hasRadius, hasRadAttr, hasOnbType);

    auto copyattr = [&] (auto &primAttrs, auto &meshAttrs, auto &parsAttrs) {
        if (copyMeshAttr) {
            meshAttrs.template foreach_attr<AttrAcceptAll>([&] (auto const &key, auto const &arrMesh) {
                using T = std::decay_t<decltype(arrMesh[0])>;
                primAttrs.template add_attr<T>(key);
                tg.add([&] {
                    auto &arrOut = primAttrs.template attr<T>(key);
                    parallel_for((size_t)0, parsAttrs.size(), [&] (size_t i) {
                        for (size_t j = 0; j < meshAttrs.size(); j++) {
                            arrOut[i * meshAttrs.size() + j] = arrMesh[j];
                        }
                    });
                });
            });
        }
        if (copyParsAttr) {
            parsAttrs.template foreach_attr<AttrAcceptAll>([&] (auto const &key, auto const &arrPars) {
                if (meshAttrs.has_attr(key)) return;
                using T = std::decay_t<decltype(arrPars[0])>;
                primAttrs.template add_attr<T>(key);
                tg.add([&] {
                    auto &arrOut = primAttrs.template attr<T>(key);
                    parallel_for((size_t)0, arrPars.size(), [&] (size_t i) {
                        auto value = arrPars[i];
                        for (size_t j = 0; j < meshAttrs.size(); j++) {
                            arrOut[i * meshAttrs.size() + j] = value;
                        }
                    });
                });
            });
        }
    };
    copyattr(prim->verts, meshPrim->verts, parsPrim->verts);
    auto advanceinds = [&] (auto &primAttrs, auto &meshAttrs, auto &parsAttrs, size_t parsVertsSize, size_t meshVertsSize) {
        copyattr(primAttrs, meshAttrs, parsAttrs);
        tg.add([&] {
            parallel_for((size_t)0, parsVertsSize, [&] (size_t i) {
                overloaded fixpairadd{
                    [] (auto &x, size_t y) {
                        x += y;
                    },
                    [] (std::pair<int, int> &x, size_t y) {
                        x.first += y;
                        x.second += y;
                    },
                };
                for (size_t j = 0; j < meshAttrs.size(); j++) {
                    auto index = meshAttrs[j];
                    fixpairadd(index, i * meshVertsSize);
                    primAttrs[i * meshAttrs.size() + j] = index;
                }
            });
        });
    };
    AttrVector<vec3f> dummyVec;
    advanceinds(prim->points, meshPrim->points, parsPrim->verts, parsPrim->verts.size(), meshPrim->verts.size());
    advanceinds(prim->lines, meshPrim->lines, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
    advanceinds(prim->tris, meshPrim->tris, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
    advanceinds(prim->quads, meshPrim->quads, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
    advanceinds(prim->polys, meshPrim->polys, dummyVec, parsPrim->verts.size(), meshPrim->loops.size());
    advanceinds(prim->loops, meshPrim->loops, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
    tg.add([&] {
        prim->uvs = meshPrim->uvs;
    });

    tg.run();

    return prim;
}

namespace {

struct PrimDuplicate : INode {
    virtual void apply() override {
        auto parsPrim = ZImpl(get_input<PrimitiveObject>("parsPrim"));
        auto meshPrim = ZImpl(get_input<PrimitiveObject>("meshPrim"));
        auto tanAttr = ZImpl(get_input2<std::string>("tanAttr"));
        auto dirAttr = ZImpl(get_input2<std::string>("dirAttr"));
        auto radAttr = ZImpl(get_input2<std::string>("radAttr"));
        auto onbType = ZImpl(get_input2<std::string>("onbType"));
        auto radius = ZImpl(get_input2<float>("radius"));
        auto copyParsAttr = ZImpl(get_input2<bool>("copyParsAttr"));
        auto copyMeshAttr = ZImpl(get_input2<bool>("copyMeshAttr"));
        auto prim = primDuplicate(parsPrim.get(), meshPrim.get(),
                                  dirAttr, tanAttr, radAttr, onbType,
                                  radius, copyParsAttr, copyMeshAttr);
        ZImpl(set_output("prim", prim));
    }
};

ZENDEFNODE(PrimDuplicate, {
    {
    {gParamType_Primitive, "parsPrim", "", zeno::Socket_ReadOnly},
    {gParamType_Primitive, "meshPrim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "dirAttr", ""},
    {gParamType_String, "tanAttr", ""},
    {gParamType_String, "radAttr", ""},
    {"enum XYZ YXZ YZX ZYX ZXY XZY", "onbType", "XYZ"},
    {gParamType_Float, "radius", "1"},
    {gParamType_Bool, "copyParsAttr", "1"},
    {gParamType_Bool, "copyMeshAttr", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimDuplicateConnLines : INode {
    virtual void apply() override {
        auto profPrim = ZImpl(get_input<PrimitiveObject>("parsPrim"));
        auto prim = ZImpl(get_input<PrimitiveObject>("meshPrim"));
        auto tanAttr = ZImpl(get_input2<std::string>("tanAttr"));
        auto dirAttr = ZImpl(get_input2<std::string>("dirAttr"));
        auto radAttr = ZImpl(get_input2<std::string>("radAttr"));
        auto onbType = ZImpl(get_input2<std::string>("onbType"));
        auto radius = ZImpl(get_input2<float>("radius"));
        auto copyParsAttr = ZImpl(get_input2<bool>("copyParsAttr"));
        auto copyMeshAttr = ZImpl(get_input2<bool>("copyMeshAttr"));
        auto outprim = primDuplicate(profPrim.get(), prim.get(),
                                  dirAttr, tanAttr, radAttr, onbType,
                                  radius, copyParsAttr, copyMeshAttr);
        outprim->lines.clear();
        outprim->quads.reserve(prim->lines.size() * profPrim->lines.size());
        for (size_t i = 0; i < prim->lines.size(); i++) {
            for (size_t j = 0; j < profPrim->lines.size(); j++) {
                auto [k1, k2] = profPrim->lines[j];
                auto a = prim->lines[i] + k1 * prim->verts.size();
                auto b = prim->lines[i] + k2 * prim->verts.size();
                outprim->quads.emplace_back(a[0], a[1], b[1], b[0]);
            }
        }
        outprim->quads.update();
        ZImpl(set_output("prim", std::move(outprim)));
    }
};

ZENDEFNODE(PrimDuplicateConnLines, {
    {
    {gParamType_Primitive, "parsPrim", "", zeno::Socket_ReadOnly},
    {gParamType_Primitive, "meshPrim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "dirAttr", ""},
    {gParamType_String, "tanAttr", ""},
    {gParamType_String, "radAttr", ""},
    {"enum XYZ YXZ YZX ZYX ZXY XZY", "onbType", "XYZ"},
    {gParamType_Float, "radius", "1"},
    {gParamType_Bool, "copyParsAttr", "1"},
    {gParamType_Bool, "copyMeshAttr", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

}
}
