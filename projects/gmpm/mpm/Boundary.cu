#include "../Structures.hpp"
#include "../Utils.hpp"

#include "zensim/cuda/execution/ExecutionPolicy.cuh"
#include "zensim/io/ParticleIO.hpp"
#include "zensim/math/matrix/QRSVD.hpp"
#include "zensim/omp/execution/ExecutionPolicy.hpp"
#include "zensim/simulation/Utils.hpp"
#include "zensim/tpls/fmt/color.h"
#include "zensim/tpls/fmt/format.h"
#include <zeno/types/ListObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>

namespace zeno {

struct ApplyGridBoundaryOnZSGrid : INode {
  void apply() override {
    fmt::print(fg(fmt::color::green),
               "begin executing ApplyGridBoundaryOnZSGrid\n");

    auto zsgrid = get_input<ZenoGrid>("ZSGrid");
    auto &grid = zsgrid->get();
    auto &partition = get_input<ZenoPartition>("ZSPartition")->get();
    auto bg = get_input<ZenoGrid>("BoundaryZSGrid");
    if (bg->transferScheme != "boundary")
      throw std::runtime_error("boundary grid is not of boundary type!");
    auto &boundaryGrid = get_input<ZenoGrid>("BoundaryZSGrid")->get();
    auto &boundaryPartition =
        get_input<ZenoPartition>("BoundaryZSPartition")->get();

    using namespace zs;

    auto cudaPol = cuda_exec().device(0);
    auto typeStr = get_param<std::string>("type");
    collider_e type =
        typeStr == "sticky"
            ? collider_e::Sticky
            : (typeStr == "slip" ? collider_e::Slip : collider_e::Separate);

    cudaPol(Collapse{boundaryPartition.size(), boundaryGrid.block_size},
            [grid = proxy<execspace_e::cuda>({}, grid),
             boundaryGrid = proxy<execspace_e::cuda>({}, boundaryGrid),
             table = proxy<execspace_e::cuda>(partition),
             boundaryTable = proxy<execspace_e::cuda>(boundaryPartition),
             type] __device__(int bi, int ci) mutable {
              using table_t = RM_CVREF_T(table);
              auto boundaryBlock = boundaryGrid.block(bi);
              if (boundaryBlock("m", ci) == 0.f)
                return;
              auto blockid = boundaryTable._activeKeys[bi];
              auto blockno = table.query(blockid);
              if (blockno == table_t::sentinel_v)
                return;

              auto block = grid.block(blockno);
              if (block("m", ci) == 0.f)
                return;
              if (type == collider_e::Sticky)
                block.set("v", ci, boundaryBlock.pack<3>("v", ci));
              else {
                auto v_object = boundaryBlock.pack<3>("v", ci);
                auto normal = boundaryBlock.pack<3>("nrm", ci);
                auto v = block.pack<3>("v", ci);
                v -= v_object;
                auto proj = normal.dot(v);
                if ((type == collider_e::Separate && proj < 0) ||
                    type == collider_e::Slip)
                  v -= proj * normal;
                v += v_object;
                block.set("v", ci, v);
              }
            });

    fmt::print(fg(fmt::color::cyan),
               "done executing ApplyGridBoundaryOnZSGrid\n");
    set_output("ZSGrid", zsgrid);
  }
};

ZENDEFNODE(ApplyGridBoundaryOnZSGrid,
           {
               {"ZSPartition", "ZSGrid", "BoundaryZSPartition",
                "BoundaryZSGrid"},
               {"ZSGrid"},
               {{"enum sticky slip separate", "type", "sticky"}},
               {"MPM"},
           });

struct ApplyBoundaryOnZSGrid : INode {
  template <typename LsView>
  constexpr void
  projectBoundary(zs::CudaExecutionPolicy &cudaPol, LsView lsv,
                  const ZenoBoundary &boundary,
                  const typename ZenoPartition::table_t &partition,
                  typename ZenoGrid::grid_t &grid,
                  std::string_view transferScheme) {
    using namespace zs;
    auto collider = boundary.getBoundary(lsv);
    if (transferScheme == "apic")
      cudaPol(Collapse{partition.size(), ZenoGrid::grid_t::block_space()},
              [grid = proxy<execspace_e::cuda>({}, grid),
               table = proxy<execspace_e::cuda>(partition),
               boundary = collider] __device__(int bi, int ci) mutable {
                auto block = grid.block(bi);
                auto mass = block("m", ci);
                if (mass != 0.f) {
                  auto vel = block.pack<3>("v", ci);
                  auto pos =
                      (table._activeKeys[bi] + grid.cellid_to_coord(ci)) *
                      grid.dx;
                  boundary.resolveCollision(pos, vel);
                  block.set("v", ci, vel);
                }
              });
    else if (transferScheme == "flip")
      cudaPol(Collapse{partition.size(), ZenoGrid::grid_t::block_space()},
              [grid = proxy<execspace_e::cuda>({}, grid),
               table = proxy<execspace_e::cuda>(partition),
               boundary = collider] __device__(int bi, int ci) mutable {
                auto block = grid.block(bi);
                auto mass = block("m", ci);
                if (mass != 0.f) {
                  auto vel = block.pack<3>("v", ci);
                  auto pos =
                      (table._activeKeys[bi] + grid.cellid_to_coord(ci)) *
                      grid.dx;
                  boundary.resolveCollision(pos, vel);
                  block.set("v", ci, vel);

                  auto vdiff = block.pack<3>("vdiff", ci);
                  boundary.resolveCollision(pos, vdiff);
                  block.set("vdiff", ci, vdiff);
                }
              });
  }
  void apply() override {
    fmt::print(fg(fmt::color::green),
               "begin executing ApplyBoundaryOnZSGrid\n");

    auto zsgrid = get_input<ZenoGrid>("ZSGrid");
    auto &grid = zsgrid->get();

    using namespace zs;

    auto cudaPol = cuda_exec().device(0);

    if (has_input<ZenoBoundary>("ZSBoundary")) {
      auto boundary = get_input<ZenoBoundary>("ZSBoundary");
      auto &partition = get_input<ZenoPartition>("ZSPartition")->get();

      using basic_ls_t = typename ZenoLevelSet::basic_ls_t;
      using const_sdf_vel_ls_t = typename ZenoLevelSet::const_sdf_vel_ls_t;
      using const_transition_ls_t =
          typename ZenoLevelSet::const_transition_ls_t;
      if (boundary->zsls)
        match([&](const auto &ls) {
          if constexpr (is_same_v<RM_CVREF_T(ls), basic_ls_t>) {
            match([&](const auto &lsPtr) {
              auto lsv = get_level_set_view<execspace_e::cuda>(lsPtr);
              projectBoundary(cudaPol, lsv, *boundary, partition, grid,
                              zsgrid->transferScheme);
            })(ls._ls);
          } else if constexpr (is_same_v<RM_CVREF_T(ls), const_sdf_vel_ls_t>) {
            match([&](auto lsv) {
              projectBoundary(cudaPol, SdfVelFieldView{lsv}, *boundary,
                              partition, grid, zsgrid->transferScheme);
            })(ls.template getView<execspace_e::cuda>());
          } else if constexpr (is_same_v<RM_CVREF_T(ls),
                                         const_transition_ls_t>) {
            match([&](auto fieldPair) {
              auto &fvSrc = std::get<0>(fieldPair);
              auto &fvDst = std::get<1>(fieldPair);
              projectBoundary(cudaPol,
                              TransitionLevelSetView{SdfVelFieldView{fvSrc},
                                                     SdfVelFieldView{fvDst},
                                                     ls._stepDt, ls._alpha},
                              *boundary, partition, grid,
                              zsgrid->transferScheme);
            })(ls.template getView<zs::execspace_e::cuda>());
          }
        })(boundary->zsls->getLevelSet());
    }

    fmt::print(fg(fmt::color::cyan), "done executing ApplyBoundaryOnZSGrid \n");
    set_output("ZSGrid", zsgrid);
  }
};

ZENDEFNODE(ApplyBoundaryOnZSGrid, {
                                      {"ZSPartition", "ZSGrid", "ZSBoundary"},
                                      {"ZSGrid"},
                                      {},
                                      {"MPM"},
                                  });

struct ApplyWindImpulseOnZSGrid : INode {
  template <typename VelSplsViewT>
  void computeWindImpulse(zs::CudaExecutionPolicy &cudaPol, float windDragCoeff,
                          float windDensity, VelSplsViewT velLs,
                          const typename ZenoParticles::particles_t &pars,
                          const typename ZenoParticles::particles_t &eles,
                          const typename ZenoPartition::table_t &partition,
                          typename ZenoGrid::grid_t &grid, float dt) {
    using namespace zs;
    cudaPol(
        range(eles.size()),
        [windDragCoeff, windDensity, velLs,
         pars = proxy<execspace_e::cuda>({}, pars), // for normal compute
         eles = proxy<execspace_e::cuda>({}, eles),
         table = proxy<execspace_e::cuda>(partition),
         grid = proxy<execspace_e::cuda>({}, grid),
         Dinv = 4.f / grid.dx / grid.dx, dt] __device__(size_t ei) mutable {
          using grid_t = RM_CVREF_T(grid);
          zs::vec<float, 3> n{};
          float area{};
          {
            auto p0 =
                pars.pack<3>("pos", reinterpret_bits<int>(eles("inds", 0, ei)));
            auto p1 =
                pars.pack<3>("pos", reinterpret_bits<int>(eles("inds", 1, ei)));
            auto p2 =
                pars.pack<3>("pos", reinterpret_bits<int>(eles("inds", 2, ei)));
            auto cp = (p1 - p0).cross(p2 - p0);
            area = cp.length();
            n = cp / area;
            area *= 0.5f;
          }
          auto pos = eles.pack<3>("pos", ei);
          auto windVel = velLs.getMaterialVelocity(pos);

          auto vel = eles.pack<3>("vel", ei);
          auto vrel = windVel - vel;
          float vnSignedLength = n.dot(vrel);
          auto vn = n * vnSignedLength;
          auto vt = vrel - vn; // tangent
          auto windForce = windDensity * area * zs::abs(vnSignedLength) * vn +
                           windDragCoeff * area * vt;
          auto fdt = windForce * dt;

          auto arena =
              make_local_arena<grid_e::collocated, kernel_e::quadratic>(grid.dx,
                                                                        pos);

          for (auto loc : arena.range()) {
            auto coord = arena.coord(loc);
            auto localIndex = coord & (grid_t::side_length - 1);
            auto blockno = table.query(coord - localIndex);
            if (blockno < 0)
              printf("THE HELL!");
            auto block = grid.block(blockno);
            auto W = arena.weight(loc);
            const auto cellid = grid_t::coord_to_cellid(localIndex);
            for (int d = 0; d != 3; ++d)
              atomic_add(exec_cuda, &block("v", d, cellid), W * fdt[d]);
          }
        });
  }
  void apply() override {
    fmt::print(fg(fmt::color::green),
               "begin executing ApplyWindImpulseOnZSGrid\n");

    using namespace zs;

    // this could possibly be the same staggered velocity field too
    auto parObjPtrs = RETRIEVE_OBJECT_PTRS(ZenoParticles, "ZSParticles");
    auto &partition = get_input<ZenoPartition>("ZSPartition")->get();
    auto zsgrid = get_input<ZenoGrid>("ZSGrid");
    auto &grid = zsgrid->get();

    auto velZsField = get_input<ZenoLevelSet>("ZSVelField");
    const auto &velField = velZsField->getBasicLevelSet()._ls;

    auto stepDt = get_input2<float>("dt");
    auto windDrag = get_input2<float>("windDrag");
    auto windDensity = get_input2<float>("windDensity");

    match([&](const auto &velLsPtr) {
      auto cudaPol = cuda_exec().device(0);
      for (auto &&parObjPtr : parObjPtrs) {
        auto &pars = parObjPtr->getParticles();
        if (parObjPtr->category == ZenoParticles::surface ||
            parObjPtr->category == ZenoParticles::tracker) {
          auto &eles = parObjPtr->getQuadraturePoints();
          computeWindImpulse(cudaPol, windDrag, windDensity,
                             get_level_set_view<execspace_e::cuda>(velLsPtr),
                             pars, eles, partition, grid, stepDt);
        }
      }
    })(velField);

    fmt::print(fg(fmt::color::cyan),
               "done executing ApplyWindImpulseOnZSGrid\n");
    set_output("ZSGrid", zsgrid);
  }
};

ZENDEFNODE(ApplyWindImpulseOnZSGrid, {
                                         {"ZSParticles",
                                          "ZSVelField",
                                          "ZSPartition",
                                          "ZSGrid",
                                          {"float", "windDrag", "0"},
                                          {"float", "windDensity", "1"},
                                          {"float", "dt", "0.1"}},
                                         {"ZSGrid"},
                                         {},
                                         {"MPM"},
                                     });

struct TransformZSLevelSet : INode {
  void apply() override {
    fmt::print(fg(fmt::color::green), "begin executing TransformZSLevelSet\n");
    auto zsls = get_input<ZenoLevelSet>("ZSLevelSet");
    auto &ls = zsls->getLevelSet();

    using namespace zs;
    using basic_ls_t = typename ZenoLevelSet::basic_ls_t;
    // translation
    if (has_input("translation")) {
      auto b = get_input<NumericObject>("translation")->get<vec3f>();
      match(
          [&b](basic_ls_t &basicLs) {
            match(
                [b](std::shared_ptr<typename basic_ls_t::clspls_t> lsPtr) {
                  lsPtr->translate(zs::vec<float, 3>{b[0], b[1], b[2]});
                },
                [b](std::shared_ptr<typename basic_ls_t::ccspls_t> lsPtr) {
                  lsPtr->translate(zs::vec<float, 3>{b[0], b[1], b[2]});
                },
                [b](std::shared_ptr<typename basic_ls_t::sgspls_t> lsPtr) {
                  lsPtr->translate(zs::vec<float, 3>{b[0], b[1], b[2]});
                },
                [](auto &lsPtr) {
                  auto msg = get_var_type_str(*lsPtr);
                  throw std::runtime_error(fmt::format(
                      "levelset of type [{}] cannot be transformed yet.", msg));
                })(basicLs._ls);
          },
          [](auto &ls) {
            auto msg = get_var_type_str(ls);
            throw std::runtime_error(
                fmt::format("levelset of special type [{}] are const-.", msg));
          })(ls);
    }

    // scale
    if (has_input("scaling")) {
      auto s = get_input<NumericObject>("scaling")->get<float>();
      match(
          [&s](basic_ls_t &basicLs) {
            match(
                [s](std::shared_ptr<typename basic_ls_t::clspls_t> lsPtr) {
                  lsPtr->scale(s);
                },
                [s](std::shared_ptr<typename basic_ls_t::ccspls_t> lsPtr) {
                  lsPtr->scale(s);
                },
                [s](std::shared_ptr<typename basic_ls_t::sgspls_t> lsPtr) {
                  lsPtr->scale(s);
                },
                [](auto &lsPtr) {
                  auto msg = get_var_type_str(*lsPtr);
                  throw std::runtime_error(fmt::format(
                      "levelset of type [{}] cannot be transformed yet.", msg));
                })(basicLs._ls);
          },
          [](auto &ls) {
            auto msg = get_var_type_str(ls);
            throw std::runtime_error(
                fmt::format("levelset of special type [{}] are const-.", msg));
          })(ls);
    }
    // rotation
    if (has_input("eulerXYZ")) {
      auto yprAngles = get_input<NumericObject>("eulerXYZ")->get<vec3f>();
      auto rot = zs::Rotation<float, 3>{yprAngles[0], yprAngles[1],
                                        yprAngles[2], zs::degree_c, zs::ypr_c};
      match(
          [&rot](basic_ls_t &basicLs) {
            match(
                [rot](std::shared_ptr<typename basic_ls_t::clspls_t> lsPtr) {
                  lsPtr->rotate(rot.transpose());
                },
                [rot](std::shared_ptr<typename basic_ls_t::ccspls_t> lsPtr) {
                  lsPtr->rotate(rot.transpose());
                },
                [rot](std::shared_ptr<typename basic_ls_t::sgspls_t> lsPtr) {
                  lsPtr->rotate(rot.transpose());
                },
                [](auto &lsPtr) {
                  auto msg = get_var_type_str(*lsPtr);
                  throw std::runtime_error(fmt::format(
                      "levelset of type [{}] cannot be transformed yet.", msg));
                })(basicLs._ls);
          },
          [](auto &ls) {
            auto msg = get_var_type_str(ls);
            throw std::runtime_error(
                fmt::format("levelset of special type [{}] are const-.", msg));
          })(ls);
    }

    fmt::print(fg(fmt::color::cyan), "done executing TransformZSLevelSet\n");
    set_output("ZSLevelSet", zsls);
  }
};
// refer to nodes/prim/TransformPrimitive.cpp
ZENDEFNODE(TransformZSLevelSet,
           {
               {"ZSLevelSet", "translation", "eulerXYZ", "scaling"},
               {"ZSLevelSet"},
               {},
               {"MPM"},
           });

} // namespace zeno