#include <zeno/zeno.h>
#include <zeno/NumericObject.h>
#include <vector>
#include <zeno/VDBGrid.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/points/PointAdvect.h>
#include <openvdb/tools/Morphology.h>
#include <openvdb/tools/MeshToVolume.h>
#include <zeno/types/ParticlesObject.h>
#include <zeno/PrimitiveObject.h>
#include <openvdb/openvdb.h>
#include <openvdb/points/PointConversion.h>
#include <zeno/ZenoInc.h>

namespace zeno {
static openvdb::points::PointDataGrid::Ptr particleArrayToGrid
    ( std::vector<openvdb::Vec3f> const &positions
    , std::vector<openvdb::Vec3f> const &velocitys
    , float dx
    ) {
    // The VDB Point-Partioner is used when bucketing points and requires a
    // specific interface. For convenience, we use the PointAttributeVector
    // wrapper around an stl vector wrapper here, however it is also possible to
    // write one for a custom data structure in order to match the interface
    // required.
    openvdb::points::PointAttributeVector<openvdb::Vec3f> positionsWrapper(positions);
    // This method computes a voxel-size to match the number of
    // points / voxel requested. Although it won't be exact, it typically offers
    // a good balance of memory against performance.
    int pointsPerVoxel = 8;
    float voxelSize = dx;
    // Print the voxel-size to cout
    // Create a transform using this voxel-size.
    openvdb::math::Transform::Ptr transform
        = openvdb::math::Transform::createLinearTransform(voxelSize);


    openvdb::tools::PointIndexGrid::Ptr pointIndexGrid =
        openvdb::tools::createPointIndexGrid<openvdb::tools::PointIndexGrid>(
            positionsWrapper, *transform);
    // Create a PointDataGrid containing these four points and using the
    // transform given. This function has two template parameters, (1) the codec
    // to use for storing the position, (2) the grid we want to create
    // (ie a PointDataGrid).
    // We use no compression here for the positions.
    using PositionCodec = openvdb::points::FixedPointCodec</*one byte*/false>;
	//using PositionCodec = openvdb::points::TruncateCodec;
	//using PositionCodec = openvdb::points::NullCodec;
	using position_attribute = openvdb::points::TypedAttributeArray<openvdb::Vec3f, PositionCodec>;
	using VelocityCodec = openvdb::points::TruncateCodec;
	//using VelocityCodec = openvdb::points::NullCodec;
	using velocity_attribute = openvdb::points::TypedAttributeArray<openvdb::Vec3f, VelocityCodec>;
    auto pnamepair = position_attribute::attributeType();
    auto vnamepair = velocity_attribute::attributeType();

    openvdb::points::PointDataGrid::Ptr grid =
        openvdb::points::createPointDataGrid<PositionCodec,
            openvdb::points::PointDataGrid>(*pointIndexGrid, positionsWrapper, *transform);


    openvdb::points::appendAttribute(grid->tree(), "v", vnamepair);

    openvdb::points::PointAttributeVector<openvdb::Vec3f> velocityWrapper(velocitys);

    openvdb::points::populateAttribute<openvdb::points::PointDataTree,
        openvdb::tools::PointIndexTree, openvdb::points::PointAttributeVector<openvdb::Vec3f>>(
            grid->tree(), pointIndexGrid->tree(), "v", velocityWrapper);
    
    grid->setName("Points");
    return grid;
}

struct PrimToVDBPointDataGrid : zeno::INode {
  virtual void apply() override {
    //auto dx = get_param_float("dx");
    //if(has_input("Dx"))
    //{
      //dx = get_input2_float("Dx");
    //}
    //auto dx = get_input2_float("Dx");
    auto dx = get_input2_float("Dx");
    auto prims = safe_dynamic_cast<PrimitiveObject>(get_input("ParticleGeo"));
    //auto particles = std::make_unique<ParticlesObject>();
    //particles->pos.resize(prims->attr<zeno::vec3f>("pos").size());
    //particles->vel.resize(prims->attr<zeno::vec3f>("pos").size());
    std::vector<openvdb::Vec3f> positions(prims->size());
    std::vector<openvdb::Vec3f> velocitys(prims->size());
    #pragma omp parallel for
    for(int i=0;i<prims->attr<zeno::vec3f>("pos").size();i++)
    {
        positions[i] = zeno::vec_to_other<openvdb::Vec3f>(prims->attr<zeno::vec3f>("pos")[i]);
        if(prims->has_attr("vel"))
            velocitys[i] = zeno::vec_to_other<openvdb::Vec3f>(prims->attr<zeno::vec3f>("vel")[i]);
        else
            velocitys[i] = {0,0,0};
    }
    
    if(has_input("vdbPoints"))
    {
        auto input = get_input("vdbPoints"); 
        auto data = safe_dynamic_cast<VDBPointsGrid>(input);
        dx = data->m_grid->transformPtr()->voxelSize()[0];
        data->m_grid = particleArrayToGrid(positions, velocitys, dx);
        set_output("Particles", std::move(input));
    }
    else{
        auto data = std::make_shared<VDBPointsGrid>();
        data->m_grid = particleArrayToGrid(positions, velocitys, dx);
        set_output("Particles", data);
    }
  }
};

static int defPrimToVDBPointDataGrid = zeno::defNodeClass<PrimToVDBPointDataGrid>("PrimToVDBPointDataGrid",
    { /* inputs: */ {
        {gParamType_Primitive, "ParticleGeo", "", zeno::Socket_ReadOnly},
        {gParamType_Float,"Dx"}, {gParamType_VDBGrid,"vdbPoints"},
    }, /* outputs: */ {
        {gParamType_VDBGrid,"Particles"},
    }, /* params: */ {
    //{gParamType_Float, "dx", "0.08"},
    }, /* category: */ {
        "openvdb",
    }});

struct SetVDBPointDataGrid : zeno::INode {
  virtual void apply() override {
    auto dx = get_param_float("dx");
    if(has_input("Dx"))
    {
      dx = get_input2_float("Dx");
    }
    auto particles = safe_dynamic_cast<ParticlesObject>(get_input("ParticleGeo"));
    std::vector<openvdb::Vec3f> positions(particles->size());
    std::vector<openvdb::Vec3f> velocitys(particles->size());
    for (auto i = 0; i < particles->size(); ++i){
      for (int d = 0; d < 3; ++d) {
            positions[i][d] = particles->pos[i][d];
            velocitys[i][d] = particles->vel[i][d];
          
          }
    }
    auto data = std::make_shared<VDBPointsGrid>();
    data->m_grid = particleArrayToGrid(positions, velocitys, dx);
    set_output("Particles", data);
  }
};

static int defSetVDBPointDataGrid = zeno::defNodeClass<SetVDBPointDataGrid>("SetVDBPointDataGrid",
    { /* inputs: */ {
        {gParamType_Particles, "ParticleGeo", "", zeno::Socket_ReadOnly},
        {gParamType_Float,"Dx"},
    }, /* outputs: */ {
        {gParamType_VDBGrid,"Particles"},
    }, /* params: */ {
    {gParamType_Float, "dx", "0.08"},
    }, /* category: */ {
        "deprecated",
    }});


}

