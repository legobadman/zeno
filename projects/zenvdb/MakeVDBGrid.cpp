#include <zeno/VDBGrid.h>
#include <zeno/zeno.h>
#include <zeno/StringObject.h>
#include <zeno/NumericObject.h>
#include <zeno/ZenoInc.h>

namespace zeno {

struct MakeVDBGrid : zeno::INode {
  virtual void apply() override {
    //auto dx = m_pAdapter->get_param_float("dx");
    float dx=0.08f;
    if(has_input("Dx"))
    {
        dx = get_input2_float("Dx");
    }
    auto type = zsString2Std(m_pAdapter->get_param_string("type"));
    auto structure = zsString2Std(m_pAdapter->get_param_string("structure"));
    auto name = zsString2Std(m_pAdapter->get_param_string("name"));
    std::shared_ptr<VDBGrid> data;
    if (type == "float") {
      auto tmp = !has_input("background") ? std::make_shared<VDBFloatGrid>()
          : std::make_shared<VDBFloatGrid>(openvdb::FloatGrid::create(
                  get_input2_float("background")));
      auto transform = openvdb::math::Transform::createLinearTransform(dx);
      if(structure== "vertex")
        transform->postTranslate(openvdb::Vec3d{ -0.5,-0.5,-0.5 }*double(dx));
      tmp->m_grid->setTransform(transform);
      tmp->m_grid->setName(name);
      data = std::move(tmp);
    } else if (type == "float3") {
      auto tmp = !has_input("background") ? std::make_shared<VDBFloat3Grid>()
          : std::make_shared<VDBFloat3Grid>(openvdb::Vec3fGrid::create(
                  zeno::vec_to_other<openvdb::Vec3f>(toVec3f(get_input2_vec3f("background")))));
      tmp->m_grid->setTransform(openvdb::math::Transform::createLinearTransform(dx));
      tmp->m_grid->setName(name);
      if (structure == "Staggered") {
        tmp->m_grid->setGridClass(openvdb::GridClass::GRID_STAGGERED);
      }
      data = std::move(tmp);
    } else if (type == "int") {
      auto tmp = std::make_shared<VDBIntGrid>();
      tmp->m_grid->setTransform(openvdb::math::Transform::createLinearTransform(dx));
      tmp->m_grid->setName(name);
      data = std::move(tmp);
    } else if (type == "int3") {
      auto tmp = std::make_shared<VDBInt3Grid>();
      tmp->m_grid->setTransform(openvdb::math::Transform::createLinearTransform(dx));
      tmp->m_grid->setName(name);
      data = std::move(tmp);
    } else if (type == "points") {
      auto tmp = std::make_shared<VDBPointsGrid>();
      tmp->m_grid->setTransform(openvdb::math::Transform::createLinearTransform(dx));
      tmp->m_grid->setName(name);
      data = std::move(tmp);
    } else {
      printf("%s\n", type.c_str());
      assert(0 && "bad VDBGrid type");
    }
    set_output("data", data);
  }
};

static int defMakeVDBGrid = zeno::defNodeClass<MakeVDBGrid>(
    "MakeVDBGrid", {/* inputs: */ {{gParamType_Float,"Dx","0.08"},{gParamType_Float,"background","0"}}, /* outputs: */
                    {
                        {gParamType_VDBGrid, "data"},
                    },
                    /* params: */
                    {
                        //{gParamType_Float, "dx", "0.08"},
                        {"enum float float3 int int3 points", "type", "float"},
                        {"enum vertex Centered Staggered", "structure", "Centered"},
                        {gParamType_String, "name", ""},
                    },
                    /* category: */
                    {
                        "openvdb",
                    }});

struct SetVDBGridName : zeno::INode {
    virtual void apply() override {
        auto grid = safe_dynamic_cast<VDBGrid>(get_input("grid"));
        auto name = zsString2Std(m_pAdapter->get_param_string("name"));
        grid->setName(name);
        set_output("grid", std::move(grid));
    }
};

static int defSetVDBGridName = zeno::defNodeClass<SetVDBGridName>("SetVDBGridName", {/* inputs: */ {
                                                                                         {"VDBGrid","grid"},
                                                                                     }, /* outputs: */
                                                                                     {
                                                                                         {"VDBGrid","grid"},
                                                                                     },
                                                                                     /* params: */
                                                                                     {
                                                                                         //{gParamType_Float, "dx", "0.08"},
                                                                                         {gParamType_String, "name", "density"},
                                                                                     },
                                                                                     /* category: */
                                                                                     {
                                                                                         "openvdb",
                                                                                     }});

struct SetVDBGridClass : zeno::INode {
    virtual void apply() override {
        auto grid = safe_dynamic_cast<VDBGrid>(get_input("grid"));
        auto VDBGridClass = zsString2Std(get_input2_string("VDBGridClass"));

        grid->setGridClass(VDBGridClass);
        set_output("grid", std::move(grid));
    }
};

ZENDEFNODE(SetVDBGridClass,
           {/* inputs: */ {{"VDBGrid","grid"}, {"enum UNKNOWN LEVEL_SET FOG_VOLUME STAGGERED", "VDBGridClass", "LEVEL_SET"}},
            /* outputs: */
            {
                {"VDBGrid","grid"},
            },
            /* params: */
            {},
            /* category: */
            {
                "openvdb",
            }});
}
