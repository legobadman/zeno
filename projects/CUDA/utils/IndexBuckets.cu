#include "Structures.hpp"
#include "Utils.hpp"
#include "zensim/cuda/execution/ExecutionPolicy.cuh"
#include "zensim/omp/execution/ExecutionPolicy.hpp"
#include "zensim/types/Property.h"
#include "zensim/zpc_tpls/fmt/color.h"
#include "zensim/zpc_tpls/fmt/format.h"
#include <zeno/types/NumericObject.h>
#include <zeno/types/IGeometryObject.h>

namespace zeno {

struct MakeZSBuckets : zeno::INode {
    void apply() override {
        float radius = get_input2_float("radius");
        float radiusMin = has_input("radiusMin") ? get_input2_float("radiusMin") : 0.f;
        auto zspars = safe_uniqueptr_cast<ZenoParticles>(clone_input("ZSParticles"));
        auto &pars = zspars->getParticles();

        auto out = std::make_unique<ZenoIndexBuckets>();
        auto &ibs = out->get();

        using namespace zs;
        auto cudaPol = cuda_exec();
        spatial_hashing(cudaPol, pars, radius + radius, ibs);

        fmt::print("done building index buckets with {} entries, {} buckets\n", ibs.numEntries(), ibs.numBuckets());

        set_output("ZSIndexBuckets", std::move(out));
    }
};

ZENDEFNODE(MakeZSBuckets, {
    {
        {gParamType_Particles, "ZSParticles"},
        {gParamType_Float, "radius"},
        {gParamType_Float, "radiusMin"}
    },
    {{gParamType_IndexBuckets, "ZSIndexBuckets"}},
    {},
    {"MPM"}
});

struct MakeZSLBvh : zeno::INode {
    template <typename TileVecT>
    void buildBvh(zs::CudaExecutionPolicy &pol, TileVecT &verts, typename TileVecT::value_type radius,
                  ZenoLinearBvh::lbvh_t &bvh) {
        using namespace zs;
        using bv_t = typename ZenoLinearBvh::lbvh_t::Box;
        constexpr auto space = execspace_e::cuda;
        Vector<bv_t> bvs{verts.get_allocator(), verts.size()};
        pol(range(verts.size()),
            [verts = proxy<space>({}, verts), bvs = proxy<space>(bvs), radius] ZS_LAMBDA(int i) mutable {
                auto x = verts.template pack<3>("x", i);
                bv_t bv{x - radius, x + radius};
                bvs[i] = bv;
            });
        bvh.build(pol, bvs);
    }
    void apply() override {
        auto pars = safe_uniqueptr_cast<ZenoParticles>(clone_input("ZSParticles"));
        float radius = get_input2_float("radius");

        auto out = std::make_unique<ZenoLinearBvh>();
        auto &bvh = out->get();

        using namespace zs;
        auto cudaPol = cuda_exec();
        if (pars->hasImage(ZenoParticles::s_particleTag))
            buildBvh(cudaPol, pars->getParticles<true>(), radius, bvh);
        else
            buildBvh(cudaPol, pars->getParticles(), radius, bvh);
        out->thickness = radius;

        set_output("ZSLBvh", std::move(out));
    }
};

ZENDEFNODE(MakeZSLBvh, {
    {
        {gParamType_Particles, "ZSParticles"},
        {gParamType_Float, "radius"}
    },
    {{gParamType_IObject, "ZSLBvh"}},
    {},
    {"MPM"}
});

} // namespace zeno