#include "zeno/types/PrimitiveObject.h"
#include "zeno/types/StringObject.h"
#include <openvdb/Types.h>
#include <thread>
#include <map>
#include <zeno/NumericObject.h>
#include <zeno/PrimitiveObject.h>
#include <zeno/StringObject.h>
#include <zeno/VDBGrid.h>
#include <zeno/utils/log.h>
#include <zeno/utils/vec.h>
#include <zeno/zeno.h>
#include <zeno/ZenoInc.h>

namespace zeno {
struct ParticleAsVoxels : INode{
    virtual void apply() override{
        auto type = safe_dynamic_cast<VDBGrid>(get_input("vdbGrid"))->getType();
        
        if(type=="FloatGrid"){
            auto ingrid = safe_dynamic_cast<VDBFloatGrid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;
            auto inparticles = get_input_PrimitiveObject("particles");
            auto attrName = zsString2Std(get_input2_string("Attr"));

            inparticles->attr_visit(attrName, [&](auto &arr) {
            #pragma omp parallel for
                for (int i = 0; i < arr.size(); i++) {
                    if constexpr (is_decay_same_v<decltype(arr[i]), vec3f>) {
                    } else {
                        auto accessor = grid->getUnsafeAccessor();
                        openvdb::Vec3d p(inparticles->verts[i][0], inparticles->verts[i][1], inparticles->verts[i][2]);
                        openvdb::Coord coord(grid->worldToIndex(p).x(),grid->worldToIndex(p).y(),grid->worldToIndex(p).z());
                        accessor.setValue(coord, arr[i]);
                    }
                }
            });
            set_output("oGrid", std::move(ingrid));
        }
        if(type=="Vec3fGrid") {
            auto ingrid = safe_dynamic_cast<VDBFloat3Grid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;
            auto inparticles = get_input_PrimitiveObject("particles");
            auto attrName = zsString2Std(get_input2_string("Attr"));
            inparticles->attr_visit(attrName, [&](auto &arr) {
            #pragma omp parallel for
                for (int i = 0; i < arr.size(); i++) {
                    if constexpr (is_decay_same_v<decltype(arr[i]), vec3f>) {

                        auto accessor = grid->getUnsafeAccessor();
                        openvdb::Vec3d p(inparticles->verts[i][0], inparticles->verts[i][1], inparticles->verts[i][2]);
                        openvdb::Coord coord(grid->worldToIndex(p).x(),grid->worldToIndex(p).y(),grid->worldToIndex(p).z());
                        accessor.setValue(coord, openvdb::Vec3f(arr[i][0], arr[i][1], arr[i][2]));
                    } else {
                    }
                }
            });
            set_output("oGrid", std::move(ingrid));
        }
        
        
    }
};
ZENDEFNODE(ParticleAsVoxels, {
                            {{gParamType_VDBGrid,"vdbGrid"}, 
                             {gParamType_String, "Attr"},
                             {gParamType_Primitive, "particles"},
                            },
                            {{gParamType_VDBGrid,"oGrid"}},
                            
                            {
                             
                            },
                            {"visualize"},
                        });
struct VDBVoxelAsParticles : INode {
  virtual void apply() override {
    auto valToAttr = has_input("valToAttr") ? zsString2Std(get_input2_string("valToAttr")) : std::string();
    if (valToAttr.empty())
    {
        auto type = safe_dynamic_cast<VDBGrid>(get_input("vdbGrid"))->getType();
        if(type == "FloatGrid"){
            auto ingrid = safe_dynamic_cast<VDBFloatGrid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;

            auto hasInactive = get_param_bool("hasInactive");
            // tbb::concurrent_vector<vec3f> pos;
            // wxl
#if 1
            using MapT = std::map<std::thread::id, std::vector<vec3f>>;
            using IterT = typename MapT::iterator;
            std::map<std::thread::id, std::vector<vec3f>> poses;
            std::mutex mutex;
#endif

            //tbb::concurrent_vector<float> sdf;
            auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
                // wxl
#if 1
                IterT iter;
                {
                    std::lock_guard<std::mutex> lk(mutex);
                    bool tag;
                    std::tie(iter, tag) = poses.insert(std::make_pair(std::this_thread::get_id(), std::vector<vec3f>{}));
                }
                auto &pos_ = iter->second;
#endif
                for (auto iter = leaf.cbeginValueOn(); iter != leaf.cendValueOn(); ++iter) {
                    auto coord = iter.getCoord();
                    auto value = iter.getValue();
                    auto p = grid->transform().indexToWorld(coord.asVec3d());
                    // pos.emplace_back(p[0], p[1], p[2]);
                    // wxl
                    pos_.emplace_back(p[0], p[1], p[2]);
                    //sdf.emplace_back(value);
                }
                if (hasInactive) {
                    for (auto iter = leaf.cbeginValueOff(); iter != leaf.cendValueOff(); ++iter) {
                        auto coord = iter.getCoord();
                        auto value = iter.getValue();
                        auto p = grid->transform().indexToWorld(coord.asVec3d());
                        // pos.emplace_back(p[0], p[1], p[2]);
                        // wxl
                        pos_.emplace_back(p[0], p[1], p[2]);
                        //sdf.emplace_back(value);
                    }
                }
            };
            openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>> leafman(grid->tree());
            leafman.foreach(wrangler);

#if 1
            std::vector<vec3f> zspos;
            for (const auto &[_, pos] : poses) {
                zspos.insert(std::end(zspos), std::begin(pos), std::end(pos));
            }
            // printf("concurrent vec of size %d, zs pos size %d\n", pos.size(), zspos.size());
#endif

            auto prim = std::make_shared<zeno::PrimitiveObject>();
            prim->resize(zspos.size());
            auto &primPos = prim->add_attr<vec3f>("pos");
            // wxl
            primPos = zspos;
#if 0
            for (int i = 0; i < zspos.size(); i++) {
                primPos[i] = zspos[i];
            }
#endif
            set_output("primPars", std::move(prim));
        }
        else if(type == "Vec3fGrid")
        {
            auto ingrid = safe_dynamic_cast<VDBFloat3Grid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;

            auto hasInactive = get_param_bool("hasInactive");
            auto asStaggers = get_param_bool("asStaggers");
            // tbb::concurrent_vector<vec3f> pos;
            //tbb::concurrent_vector<float> sdf;
            // wxl
#if 1
            using MapT = std::map<std::thread::id, std::vector<vec3f>>;
            using IterT = typename MapT::iterator;
            std::map<std::thread::id, std::vector<vec3f>> poses;
            std::mutex mutex;
#endif
            auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
                // wxl
#if 1
                IterT iter;
                {
                    std::lock_guard<std::mutex> lk(mutex);
                    bool tag;
                    std::tie(iter, tag) = poses.insert(std::make_pair(std::this_thread::get_id(), std::vector<vec3f>{}));
                }
                auto &pos_ = iter->second;
#endif
                for (auto iter = leaf.cbeginValueOn(); iter != leaf.cendValueOn(); ++iter) {
                    auto coord = iter.getCoord();
                    auto value = iter.getValue();
                        if (!asStaggers) {
                            auto p = grid->transform().indexToWorld(coord.asVec3d());
                            pos_.emplace_back(p[0], p[1], p[2]);
                        } else {
                    auto p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0.5, 0, 0));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.emplace_back(p[0], p[1], p[2]);
                    p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0.5, 0));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.emplace_back(p[0], p[1], p[2]);
                    p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0, 0.5));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.emplace_back(p[0], p[1], p[2]);
                    //sdf.emplace_back(value);
                        }
                }
                if (hasInactive) {
                    for (auto iter = leaf.cbeginValueOff(); iter != leaf.cendValueOff(); ++iter) {
                        auto coord = iter.getCoord();
                        auto value = iter.getValue();
                        if (!asStaggers) {
                            auto p = grid->transform().indexToWorld(coord.asVec3d());
                            pos_.emplace_back(p[0], p[1], p[2]);
                        } else {
                            auto p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0.5, 0, 0));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.emplace_back(p[0], p[1], p[2]);
                            p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0.5, 0));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.emplace_back(p[0], p[1], p[2]);
                            p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0, 0.5));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.emplace_back(p[0], p[1], p[2]);
                            //sdf.emplace_back(value);
                        }
                    }
                }
            };
            openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>> leafman(grid->tree());
            leafman.foreach(wrangler);

#if 1
            std::vector<vec3f> zspos;
            for (const auto &[_, pos] : poses) {
                zspos.insert(std::end(zspos), std::begin(pos), std::end(pos));
            }
            // printf("concurrent vec of size %d, zs pos size %d\n", pos.size(), zspos.size());
#endif

            auto prim = std::make_shared<zeno::PrimitiveObject>();
            prim->resize(zspos.size());
            auto &primPos = prim->add_attr<vec3f>("pos");
            // wxl
            primPos = std::move(zspos);
#if 0
            for (int i = 0; i < pos.size(); i++) {
                primPos[i] = pos[i];
            }
#endif
            set_output("primPars", std::move(prim));
        }
    }
    else
    {
        auto type = safe_dynamic_cast<VDBGrid>(get_input("vdbGrid"))->getType();
        zeno::log_info("VDBVoxelAsParticles got vdbGrid type: {}", type);
        if(type == "FloatGrid"){
            auto ingrid = safe_dynamic_cast<VDBFloatGrid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;

            auto hasInactive = get_param_bool("hasInactive");
            // tbb::concurrent_vector<vec3f> pos;
            // wxl
#if 1
            using MapT = std::map<std::thread::id, std::vector<vec4f>>;
            using IterT = typename MapT::iterator;
            std::map<std::thread::id, std::vector<vec4f>> poses;
            std::mutex mutex;
#endif

            //tbb::concurrent_vector<float> sdf;
            auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
                // wxl
#if 1
                IterT iter;
                {
                    std::lock_guard<std::mutex> lk(mutex);
                    bool tag;
                    std::tie(iter, tag) = poses.insert(std::make_pair(std::this_thread::get_id(), std::vector<vec4f>{}));
                }
                auto &pos_ = iter->second;
#endif
                for (auto iter = leaf.cbeginValueOn(); iter != leaf.cendValueOn(); ++iter) {
                    auto coord = iter.getCoord();
                    auto value = iter.getValue();
                    auto p = grid->transform().indexToWorld(coord.asVec3d());
                    // pos.emplace_back(p[0], p[1], p[2]);
                    // wxl
                    pos_.emplace_back(p[0], p[1], p[2], value);
                    //sdf.emplace_back(value);
                }
                if (hasInactive) {
                    for (auto iter = leaf.cbeginValueOff(); iter != leaf.cendValueOff(); ++iter) {
                        auto coord = iter.getCoord();
                        auto value = iter.getValue();
                        auto p = grid->transform().indexToWorld(coord.asVec3d());
                        // pos.emplace_back(p[0], p[1], p[2]);
                        // wxl
                        pos_.emplace_back(p[0], p[1], p[2], value);
                        //sdf.emplace_back(value);
                    }
                }
            };
            openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>> leafman(grid->tree());
            leafman.foreach(wrangler);

#if 1
            std::vector<vec4f> zspos;
            for (const auto &[_, pos] : poses) {
                zspos.insert(std::end(zspos), std::begin(pos), std::end(pos));
            }
            // printf("concurrent vec of size %d, zs pos size %d\n", pos.size(), zspos.size());
#endif

            auto prim = std::make_shared<zeno::PrimitiveObject>();
            prim->resize(zspos.size());
            auto &primPos = prim->add_attr<vec3f>("pos");
            auto &primVal = prim->add_attr<float>(valToAttr);
            // wxl
            //primPos = zspos;
//#if 1
            for (int i = 0; i < zspos.size(); i++) {
                primPos[i] = {zspos[i][0], zspos[i][1], zspos[i][2]};
            }
            for (int i = 0; i < zspos.size(); i++) {
                primVal[i] = {zspos[i][3]};
            }
//#endif
            set_output("primPars", std::move(prim));
        }
        else if(type == "Vec3fGrid")
        {
            auto ingrid = safe_dynamic_cast<VDBFloat3Grid>(get_input("vdbGrid"));
            auto const &grid = ingrid->m_grid;

            auto hasInactive = get_param_bool("hasInactive");
            auto asStaggers = get_param_bool("asStaggers");
            // tbb::concurrent_vector<vec3f> pos;
            //tbb::concurrent_vector<float> sdf;
            // wxl
#if 1
            using MapT = std::map<std::thread::id, std::vector<vec<6, float>>>;
            using IterT = typename MapT::iterator;
            std::map<std::thread::id, std::vector<vec<6, float>>> poses;
            std::mutex mutex;
#endif
            auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
                // wxl
#if 1
                IterT iter;
                {
                    std::lock_guard<std::mutex> lk(mutex);
                    bool tag;
                    std::tie(iter, tag) = poses.insert(std::make_pair(std::this_thread::get_id(), std::vector<vec<6, float>>{}));
                }
                auto &pos_ = iter->second;
#endif
                for (auto iter = leaf.cbeginValueOn(); iter != leaf.cendValueOn(); ++iter) {
                    auto coord = iter.getCoord();
                    auto value = iter.getValue();
                        if (!asStaggers) {
                            auto p = grid->transform().indexToWorld(coord.asVec3d());
                            pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                        } else {
                    auto p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0.5, 0, 0));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                    p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0.5, 0));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                    p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0, 0.5));
                    // pos.emplace_back(p[0], p[1], p[2]);
                    pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                    //sdf.emplace_back(value);
                        }
                }
                if (hasInactive) {
                    for (auto iter = leaf.cbeginValueOff(); iter != leaf.cendValueOff(); ++iter) {
                        auto coord = iter.getCoord();
                        auto value = iter.getValue();
                        if (!asStaggers) {
                            auto p = grid->transform().indexToWorld(coord.asVec3d());
                            pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                        } else {
                            auto p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0.5, 0, 0));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                            p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0.5, 0));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                            p = grid->transform().indexToWorld(coord.asVec3d() - openvdb::Vec3d(0, 0, 0.5));
                            // pos.emplace_back(p[0], p[1], p[2]);
                            pos_.push_back({(float)p[0], (float)p[1], (float)p[2], value[0], value[1], value[2]});
                            //sdf.emplace_back(value);
                        }
                    }
                }
            };
            openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>> leafman(grid->tree());
            leafman.foreach(wrangler);

#if 1
            std::vector<vec<6, float>> zspos;
            for (const auto &[_, pos] : poses) {
                zspos.insert(std::end(zspos), std::begin(pos), std::end(pos));
            }
            // printf("concurrent vec of size %d, zs pos size %d\n", pos.size(), zspos.size());
#endif

            auto prim = std::make_shared<zeno::PrimitiveObject>();
            prim->resize(zspos.size());
            auto &primPos = prim->add_attr<vec3f>("pos");
            auto &primVal = prim->add_attr<vec3f>(valToAttr);
            // wxl
            //primPos = std::move(zspos);
//#if 0
            for (int i = 0; i < zspos.size(); i++) {
                primPos[i] = {zspos[i][0], zspos[i][1], zspos[i][2]};
            }
            for (int i = 0; i < zspos.size(); i++) {
                primVal[i] = {zspos[i][3], zspos[i][4], zspos[i][5]};
            }
//#endif
            set_output("primPars", std::move(prim));
        }
    }
  }
};

ZENDEFNODE(VDBVoxelAsParticles, {
                            {{gParamType_VDBGrid, "vdbGrid", "", zeno::Socket_ReadOnly},
                             {gParamType_String, "valToAttr", "sdf"}},
                            {{gParamType_Primitive, "primPars"}},
                            {
                             {gParamType_Bool, "hasInactive", "0"},
                             {gParamType_Bool, "asStaggers", "1"},
                            },
                            {"visualize"},
                        });


struct VDBLeafAsParticles : INode {
    template <typename VDBGridPtr>
    auto LeafAsParticle(VDBGridPtr ingrid) {
        auto const &grid = ingrid->m_grid;
        auto h = grid->voxelSize()[0];
        tbb::concurrent_vector<vec3f> pos;
        auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
            auto coord = leaf.origin();
            auto p = grid->transform().indexToWorld(coord + decltype(coord)(8/2));
            //auto bbox = leaf.getNodeBoundingBox();
            //auto p = grid->transform().indexToWorld(bbox.min() + bbox.max());
            pos.emplace_back(p[0]-0.5*h, p[1]-0.5*h, p[2]-0.5*h);
        };
        openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>> leafman(grid->tree());
        leafman.foreach(wrangler);

        auto prim = std::make_shared<zeno::PrimitiveObject>();
        prim->resize(pos.size());
        auto &primPos = prim->add_attr<vec3f>("pos");
        for (int i = 0; i < pos.size(); i++) {
            primPos[i] = pos[i];
        }

        return prim;
    }
    virtual void apply() override {
        auto ingrid = safe_dynamic_cast<VDBGrid>(get_input("vdbGrid"));
        auto vdbType = ingrid->getType();

        std::shared_ptr<zeno::PrimitiveObject> prim(nullptr);

        if (vdbType == "FloatGrid")
            prim = LeafAsParticle(std::dynamic_pointer_cast<VDBFloatGrid>(ingrid));
        else if (vdbType == "Int32Grid")
            prim = LeafAsParticle(std::dynamic_pointer_cast<VDBIntGrid>(ingrid));
        else if (vdbType == "Vec3fGrid")
            prim = LeafAsParticle(std::dynamic_pointer_cast<VDBFloat3Grid>(ingrid));
        else if (vdbType == "Vec3IGrid")
            prim = LeafAsParticle(std::dynamic_pointer_cast<VDBInt3Grid>(ingrid));
        else if (vdbType == "PointDataGrid")
            prim = LeafAsParticle(std::dynamic_pointer_cast<VDBPointsGrid>(ingrid));
        else 
            throw std::runtime_error("VDB type not found.");

        set_output("primPars", std::move(prim));
    }
};

ZENDEFNODE(VDBLeafAsParticles, {
                            {{gParamType_VDBGrid, "vdbGrid", "", zeno::Socket_ReadOnly}},
                            {{gParamType_Primitive, "primPars"}},
                            {},
                            {"visualize"},
                        });

}
