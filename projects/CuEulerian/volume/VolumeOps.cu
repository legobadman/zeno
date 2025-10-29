#if 0
#include "Structures.hpp"
#include "zensim/cuda/execution/ExecutionPolicy.cuh"
#include "zensim/geometry/SparseGrid.hpp"
#include "zensim/omp/execution/ExecutionPolicy.hpp"
#include "zensim/zpc_tpls/fmt/color.h"
#include "zensim/zpc_tpls/fmt/format.h"

#include <zeno/types/ListObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>

// #include <zeno/VDBGrid.h>

#include "../utils.cuh"
#include "zeno/utils/log.h"

namespace zeno {

struct ZSSParseGridDifference : INode {
  void apply() override {
    using namespace zs;
    auto grid = safe_uniqueptr_cast<ZenoSparseGrid>(clone_input("ZSGrid"));

    auto attrTag = zsString2Std(get_input2_string("attrName"));
    auto chnOffset = get_input2_int("channelOffset");

    auto outputAttrTag = zsString2Std(get_input2_string("outputAttrName"));
    if (outputAttrTag.empty())
      throw std::runtime_error(
          "[outputAttrName] should not be an empty string.");

    auto orientationStr = zsString2Std(get_input2_string("orientation"));
    int orientation =
        orientationStr == "ddx" ? 0 : (orientationStr == "ddy" ? 1 : 2);

    auto boundaryStr = zsString2Std(get_input2_string("boundary_type"));
    int boundaryType = boundaryStr == "neumann" ? 0 : /*dirichlet*/ 1;

    auto &spg = grid->spg;
    auto block_cnt = spg.numBlocks();
    auto pol = cuda_exec();
    constexpr auto space = execspace_e::cuda;

    spg.append_channels(pol, {{outputAttrTag, 1}});

    pol(Collapse{block_cnt, spg.block_size},
        [spgv = proxy<space>(spg),
         srcOffset = spg.getPropertyOffset(attrTag) + chnOffset, orientation,
         dstOffset = spg.getPropertyOffset(outputAttrTag), boundaryType,
         twodx = 2 * spg.voxelSize()[0]] __device__(int blockno,
                                                    int cellno) mutable {
          auto icoord = spgv.iCoord(blockno, cellno);
          auto val = spgv(srcOffset, blockno, cellno);
          auto iCoordA = icoord;
          iCoordA[orientation]++;
          auto iCoordB = icoord;
          iCoordB[orientation]--;

          auto getVal = [&](const auto &coord) -> zs::f32 {
            auto [bno, cno] = spgv.decomposeCoord(coord);
            if (bno == spgv.sentinel_v) {
              // boundary
              if (boundaryType == 0) // neumann
                return val;
              else
                return 0.f;
            } else {
              return spgv(srcOffset, bno, cno);
            }
          };
          auto tmp = (getVal(iCoordA) - getVal(iCoordB)) / twodx;
#if 0
          if (zs::abs(tmp) > 0.01) {
            printf("coord (%d, %d, %d) - (%d, %d, %d) diff: %f\n", iCoordA[0],
                   iCoordA[1], iCoordA[2], iCoordB[0], iCoordB[1], iCoordB[2],
                   (float)tmp);
          }
#endif
          spgv(dstOffset, blockno, cellno) = tmp;
        });

    set_output("ZSGrid", grid);
  }
};

ZENDEFNODE(ZSSParseGridDifference,
           {/* inputs: */
            {
                "ZSGrid",
                {gParamType_String, "attrName", "sdf"},
                {gParamType_Int, "channelOffset", "0"},
                {"enum ddx ddy ddz", "orientation", "ddx"},
                {gParamType_String, "outputAttrName", ""},
                {"enum neumann dirichlet", "boundary_type", "neumann"},
            },
            /* outputs: */
            {"ZSGrid"},
            /* params: */
            {},
            /* category: */
            {"Eulerian"}});

} // namespace zeno
#endif