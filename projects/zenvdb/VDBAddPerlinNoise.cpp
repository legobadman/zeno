#include <zeno/zeno.h>
#include <zeno/VDBGrid.h>
#include <zeno/types/NumericObject.h>
#include <openvdb/openvdb.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/points/PointAdvect.h>
#include <openvdb/tools/Interpolation.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/utils/perlin.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/log.h>
#include <zeno/utils/zeno_p.h>
#include <zeno/utils/interfaceutil.h>

namespace {
using namespace zeno;

template <class T>
struct fuck_openvdb_vec {
    using type = T;
};

template <>
struct fuck_openvdb_vec<openvdb::Vec3f> {
    using type = zeno::vec3f;
};

struct VDBPerlinNoise : INode {
  virtual void apply() override {
    auto inoutSDF = safe_uniqueptr_cast<VDBFloatGrid>(clone_input("inoutSDF"));
    auto scale = get_input2_float("scale");
    auto scale3d = toVec3f(get_input2_vec3f("scale3d"));
    auto detail = get_input2_float("detail");
    auto roughness = get_input2_float("roughness");
    auto disortion = get_input2_float("disortion");
    auto offset = toVec3f(get_input2_vec3f("offset"));
    auto average = get_input2_float("average");
    auto strength = get_input2_float("strength");

    auto grid = inoutSDF->m_grid;
    float dx = grid->voxelSize()[0];
    strength *= dx;
    scale3d *= scale * dx;

    auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
        for (auto iter = leaf.beginValueOn(); iter != leaf.endValueOn(); ++iter) {
            auto coord = iter.getCoord();
            using OutT = typename fuck_openvdb_vec<std::decay_t<
                typename std::decay_t<decltype(leaf)>::ValueType>>::type;
            OutT noise;
            {
                zeno::vec3f p(coord[0], coord[1], coord[2]);
                p = scale3d * (p - offset);
                OutT o;
                if constexpr (std::is_same_v<OutT, float>) {
                    o = PerlinNoise::perlin(p, roughness, detail);
                } else if constexpr (std::is_same_v<OutT, zeno::vec3f>) {
                    o = OutT(
                        PerlinNoise::perlin(zeno::vec3f(p[0], p[1], p[2]), roughness, detail),
                        PerlinNoise::perlin(zeno::vec3f(p[1], p[2], p[0]), roughness, detail),
                        PerlinNoise::perlin(zeno::vec3f(p[2], p[0], p[1]), roughness, detail));
                } else {
                    throw makeError<TypeError>(typeid(zeno::vec3f), typeid(OutT), "outType");
                }
                noise = average + o * strength;
            }
            iter.modifyValue([&] (auto &v) {
                v += noise;
            });
        }
    };
    auto velman = openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>>(grid->tree());
    velman.foreach(wrangler);

    set_output("inoutSDF", std::move(inoutSDF));
  }
};

ZENO_DEFNODE(VDBPerlinNoise)(
     { /* inputs: */ {
     {gParamType_VDBGrid,"inoutSDF", "", zeno::Socket_ReadOnly},
    {gParamType_Float, "scale", "5"},
    {gParamType_Vec3f, "scale3d", "1,1,1"},
    {gParamType_Float, "detail", "2"},
    {gParamType_Float, "roughness", "0.5"},
    {gParamType_Float, "disortion", "0"},
    {gParamType_Vec3f, "offset", "0,0,0"},
    {gParamType_Float, "average", "0"},
    {gParamType_Float, "strength", "1"},
     }, /* outputs: */ {
         {gParamType_VDBGrid,"inoutSDF"},
     }, /* params: */ {
     }, /* category: */ {
     "openvdb",
     }});

}
