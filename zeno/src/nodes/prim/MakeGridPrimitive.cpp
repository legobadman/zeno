#include <cassert>
#include <cstdlib>
#include <cstring>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/vec.h>
#include <zeno/zeno.h>

namespace zeno {
struct MakePointPrimitive :INode{
    virtual void apply() override {
        auto p = ZImpl(get_input<NumericObject>("vec3"))->get<zeno::vec3f>();
        auto prim = std::make_unique<PrimitiveObject>();
        prim->resize(1);
        auto &pos = prim->add_attr<zeno::vec3f>("pos");
        pos[0] = p;
        ZImpl(set_output("prim", std::move(prim)));
    }
};
ZENDEFNODE(MakePointPrimitive,
    { /* inputs: */ {
        {gParamType_Vec3f, "vec3"},
    }, /* outputs: */ {
{gParamType_Primitive, "prim"},
}, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});

static size_t makepositive(int i) {
    if (i < 0) return 0;
    else return (size_t)i;
}

struct Make1DLinePrimitive : INode {
    virtual void apply() override {
        size_t nx = makepositive(ZImpl(get_input<NumericObject>("n"))->get<int>());
        nx = std::max(nx, (size_t)1);
        float dx = 1.f / std::max(nx - 1, (size_t)1);
        
        vec3f o = ZImpl(has_input("origin")) ?
            ZImpl(get_input<NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0);
        
    vec3f ax = vec3f(1,0,0);
    auto dir = ZImpl(get_param<std::string>("Direction"));
    if(dir == "Y")
    {
        ax = zeno::vec3f(0,ax[0],0);
    }
    if(dir == "Z")
    {
        ax = zeno::vec3f(0,0,ax[0]);
    }
    ax = ZImpl(has_input("direction")) ?
            ZImpl(get_input<NumericObject>("direction"))->get<zeno::vec3f>()
            : ax;
    if (ZImpl(has_input("scale"))) {
        auto scale = ZImpl(get_input<NumericObject>("scale"))->get<float>();
        ax *= scale;
    }
    if (ZImpl(get_param<bool>("isCentered")))
        o -= (ax) / 2;
    ax *= dx;

    auto prim = std::make_unique<PrimitiveObject>();
    prim->resize(nx);
    auto &pos = prim->add_attr<zeno::vec3f>("pos");
#pragma omp parallel for
    for (intptr_t x = 0; x < nx; x++) {
      vec3f p = o + x * ax;
      pos[x] = p;
    }
    if (ZImpl(get_param<bool>("hasLines"))) {
        prim->lines.resize((nx - 1));
#pragma omp parallel for
        for (intptr_t x = 0; x < nx-1; x++) {
          prim->lines[x][0] = x;
          prim->lines[x][1] = x + 1;
        }
    }
    prim->userData()->set_int("nx", (int)nx);//zhxx
    ZImpl(set_output("prim", std::move(prim)));
  }
};

ZENDEFNODE(Make1DLinePrimitive,
        { /* inputs: */ {
        {gParamType_Int, "n", "2"},
        {gParamType_Vec3f, "direction", "1,0,0"},
        {gParamType_Float, "scale", "1"},
        {gParamType_Vec3f, "origin", "0,0,0"},
        }, /* outputs: */ {
        {gParamType_Primitive, "prim"},
        }, /* params: */ {
        {"enum X Y Z", "Direction", "X"}, // zhxxhappy
        {gParamType_Bool, "isCentered", "0"},
        {gParamType_Bool, "hasLines", "1"},
        }, /* category: */ {
        "primitive",
        }});

struct Make2DGridPrimitive : INode {
    virtual void apply() override {
        size_t nx = ZImpl(get_input<NumericObject>("nx"))->get<int>();
        nx = std::max(nx, (size_t)1);
        size_t ny = ZImpl(has_input("ny")) ?
            makepositive(ZImpl(get_input<NumericObject>("ny"))->get<int>()) : 0;
        if (!ny) ny = nx;
        float dx = 1.f / std::max(nx - 1, (size_t)1);
        float dy = 1.f / std::max(ny - 1, (size_t)1);
        vec3f ax = ZImpl(has_input("sizeX")) ?
            ZImpl(get_input<NumericObject>("sizeX"))->get<zeno::vec3f>()
            : vec3f(1, 0, 0);
        vec3f ay = ZImpl(has_input("sizeY")) ?
            ZImpl(get_input<NumericObject>("sizeY"))->get<zeno::vec3f>()
            : vec3f(0, 1, 0);
        vec3f o = ZImpl(has_input("origin")) ?
            ZImpl(get_input<NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0);
        if (ZImpl(has_input("scale"))) {
            auto obj = ZImpl(get_input<NumericObject>("scale"));
            auto scale = obj->is<int>() ? obj->get<int>() : obj->get<float>();
            ax *= scale;
            ay *= scale;
        }
        auto dir = ZImpl(get_param<std::string>("Direction"));
        if(dir == "YZ")
        {
            ax = zeno::vec3f(0,ax[0],0);
            ay = zeno::vec3f(0, 0, ay[1]);
        }
        if(dir == "XZ")
        {
            ay = zeno::vec3f(0,0,ay[1]);
        }

        if (ZImpl(get_param<bool>("isCentered")))
            o -= (ax + ay) / 2;
        ax *= dx; ay *= dy;

        auto prim = std::make_unique<PrimitiveObject>();
        prim->resize(nx * ny);
        auto &pos = prim->add_attr<zeno::vec3f>("pos");

        auto layout = ZImpl(get_param<std::string>("Layout"));
        if (layout == "Column-major") {
#pragma omp parallel for
            for (intptr_t y = 0; y < ny; y++)
                for (intptr_t x = 0; x < nx; x++) {
                    intptr_t index = y * nx + x;
                    vec3f p = o + x * ax + y * ay;
                    size_t i = x + y * nx;
                    pos[i] = p;
                }
            if (ZImpl(get_param<bool>("hasUV"))) {
                auto &uv = prim->verts.add_attr<zeno::vec3f>("uv");
                for (intptr_t y = 0; y < ny; y++)
                    for (intptr_t x = 0; x < nx; x++) {
                        size_t i = x + y * nx;
                        uv[i] = {float(x) / float(nx - 1), float(y) / float(ny - 1), 0};
                    }
            }
            if (ZImpl(get_param<bool>("hasFaces"))) {
                prim->tris.resize((nx - 1) * (ny - 1) * 2);
#pragma omp parallel for
                for (intptr_t y = 0; y < ny - 1; y++)
                    for (intptr_t x = 0; x < nx - 1; x++) {
                        intptr_t index = y * (nx - 1) + x;
                        prim->tris[index * 2][2] = y * nx + x;
                        prim->tris[index * 2][1] = y * nx + x + 1;
                        prim->tris[index * 2][0] = (y + 1) * nx + x + 1;
                        prim->tris[index * 2 + 1][2] = (y + 1) * nx + x + 1;
                        prim->tris[index * 2 + 1][1] = (y + 1) * nx + x;
                        prim->tris[index * 2 + 1][0] = y * nx + x;
                    }
            }
        } else {
#pragma omp parallel for
            for (intptr_t x = 0; x < nx; x++)
                for (intptr_t y = 0; y < ny; y++) {
                    intptr_t index = x * ny + y;
                    vec3f p = o + x * ax + y * ay;
                    size_t i = x * ny + y;
                    pos[i] = p;
                }

            if (ZImpl(get_param<bool>("hasUV"))) {
                auto &uv = prim->verts.add_attr<zeno::vec3f>("uv");
                for (intptr_t x = 0; x < nx; x++)
                    for (intptr_t y = 0; y < ny; y++) {
                        size_t i = x * ny + y;
                        uv[i] = {float(x) / float(nx - 1), float(y) / float(ny - 1), 0};
                    }
            }

            if (ZImpl(get_param<bool>("hasFaces"))) {
                prim->tris.resize((nx - 1) * (ny - 1) * 2);
#pragma omp parallel for
                for (intptr_t x = 0; x < nx - 1; x++)
                    for (intptr_t y = 0; y < ny - 1; y++) {
                        intptr_t index = x * (ny - 1) + y;
                        prim->tris[index * 2][2] = x * ny + y;
                        prim->tris[index * 2][1] = x * ny + y + 1;
                        prim->tris[index * 2][0] = (x + 1) * ny + y + 1;
                        prim->tris[index * 2 + 1][2] = (x + 1) * ny + y + 1;
                        prim->tris[index * 2 + 1][1] = (x + 1) * ny + y;
                        prim->tris[index * 2 + 1][0] = x * ny + y;
                    }
            }
        }

    prim->userData()->set_int("nx", (int)nx);//zhxx
    prim->userData()->set_int("ny", (int)ny);//zhxx

    auto geom = create_GeometryObject(prim.get());
    ZImpl(set_output("prim", std::move(geom)));
  }
};

ZENDEFNODE(Make2DGridPrimitive,
        { /* inputs: */ {
        {gParamType_Int, "nx", "2"},
        {gParamType_Int, "ny", "0"},
        {gParamType_Vec3f, "sizeX", "1,0,0"},
        {gParamType_Vec3f, "sizeY", "0,1,0"},
        {gParamType_Float, "scale", "1"},
        {gParamType_Vec3f, "origin", "0,0,0"},
        }, /* outputs: */ {
        {gParamType_Geometry, "prim"},
        }, /* params: */ {
        {"enum XZ XY YZ", "Direction", "XZ"}, // zhxxhappy
        {"enum Column-major Row-major", "Layout", "Column-major"},
        {gParamType_Bool, "isCentered", "0"},
        {gParamType_Bool, "hasFaces", "1"},
        {gParamType_Bool, "hasUV", "0"},
        }, /* category: */ {
        "primitive",
        }});

struct Make3DGridPrimitive : INode {
    virtual void apply() override {
        size_t nx = ZImpl(get_input<NumericObject>("nx"))->get<int>();
        nx = std::max(nx, (size_t)1);
        size_t ny = ZImpl(has_input("ny")) ?
            makepositive(ZImpl(get_input<NumericObject>("ny"))->get<int>()) : 0;
        if (!ny) ny = nx;
        size_t nz = ZImpl(has_input("nz")) ?
            makepositive(ZImpl(get_input<NumericObject>("nz"))->get<int>()) : 0;
        if (!nz) nz = nx;
        float dx = 1.f / std::max(nx - 1, (size_t)1);
        float dy = 1.f / std::max(ny - 1, (size_t)1);
        float dz = 1.f / std::max(nz - 1, (size_t)1);
        vec3f ax = ZImpl(has_input("sizeX")) ?
            ZImpl(get_input<NumericObject>("sizeX"))->get<zeno::vec3f>()
            : vec3f(1, 0, 0);
        vec3f ay = ZImpl(has_input("sizeY")) ?
            ZImpl(get_input<NumericObject>("sizeY"))->get<zeno::vec3f>()
            : vec3f(0, 1, 0);
        vec3f az = ZImpl(has_input("sizeZ")) ?
            ZImpl(get_input<NumericObject>("sizeZ"))->get<zeno::vec3f>()
            : vec3f(0, 0, 1);
        vec3f o = ZImpl(has_input("origin")) ?
            ZImpl(get_input<NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0);
        if (ZImpl(has_input("scale"))) {
            auto scale = ZImpl(get_input<NumericObject>("scale"))->get<float>();
            ax *= scale;
            ay *= scale;
            az *= scale;
        }


    if (ZImpl(get_param<bool>("isCentered")))
      o -= (ax + ay + az) / 2;
    ax *= dx; ay *= dy; az *= dz;

    auto prim = std::make_unique<PrimitiveObject>();
    prim->resize(nx * ny * nz);
    auto &pos = prim->add_attr<zeno::vec3f>("pos");
    // for (size_t y = 0; y < ny; y++) {
    //     for (size_t x = 0; x < nx; x++) {
    /*
#pragma omp parallel for
    for (int index = 0; index < nx * ny * nz; index++) {
      int x = index % nx;
      int y = index / nx % ny;
      int z = index / nx / ny;*/
#pragma omp parallel for collapse(3)
        for (intptr_t z = 0; z < nz; z++) for (intptr_t y = 0; y < ny; y++) for (intptr_t x = 0; x < nx; x++) {
          intptr_t index = (z * (ny) + y) * (nx) + x;
      vec3f p = o + x * ax + y * ay + z * az;
      pos[index] = p;
    }
      // }
    ZImpl(set_output("prim", std::move(prim)));
  }
};

ZENDEFNODE(Make3DGridPrimitive,
        { /* inputs: */ {
        {gParamType_Int, "nx", "2"},
        {gParamType_Int, "ny", "0"},
        {gParamType_Int, "nz", "0"},
        {gParamType_Vec3f, "sizeX", "1,0,0"},
        {gParamType_Vec3f, "sizeY", "0,1,0"},
        {gParamType_Vec3f, "sizeZ", "0,0,1"},
        {gParamType_Float, "scale", "1"},
        {gParamType_Vec3f, "origin", "0,0,0"},
        }, /* outputs: */ {
{gParamType_Primitive, "prim"},
}, /* params: */ {
        {gParamType_Bool, "isCentered", "0"},
        }, /* category: */ {
        "primitive",
        }});


struct Make3DGridPointsInAABB : INode {//xubenhappy
    virtual void apply() override {
        size_t nx = makepositive(ZImpl(get_input<NumericObject>("nx"))->get<int>());
        nx = std::max(nx, (size_t)1);
        size_t ny = ZImpl(has_input("ny")) ?
            makepositive(ZImpl(get_input<NumericObject>("ny"))->get<int>()) : 0;
        if (!ny) ny = nx;
        size_t nz = ZImpl(has_input("nz")) ?
            makepositive(ZImpl(get_input<NumericObject>("nz"))->get<int>()) : 0;
        if (!nz) nz = nx;
        float dx = 1.f / std::max(nx - 1, (size_t)1);
        float dy = 1.f / std::max(ny - 1, (size_t)1);
        float dz = 1.f / std::max(nz - 1, (size_t)1);

        vec3f bmin = ZImpl(has_input("bmin")) ?
            ZImpl(get_input<NumericObject>("bmin"))->get<zeno::vec3f>() : vec3f(-1);
        vec3f bmax = ZImpl(has_input("bmax")) ?
            ZImpl(get_input<NumericObject>("bmax"))->get<zeno::vec3f>() : vec3f(1);
        auto delta = (bmax - bmin) * vec3f(dx, dy, dz);

        if (ZImpl(get_param<bool>("isStaggered"))) {
            nx--, ny--, nz--;
            bmin += 0.5f * delta;
        }

        auto prim = std::make_unique<PrimitiveObject>();
        prim->resize(nx * ny * nz);
        auto &pos = prim->add_attr<zeno::vec3f>("pos");
#pragma omp parallel for
        for (int index = 0; index < nx * ny * nz; index++) {
            int x = index % nx;
            int y = index / nx % ny;
            int z = index / nx / ny;
            vec3f p = bmin + vec3f(x, y, z) * delta;
            pos[index] = p;
        }
        ZImpl(set_output("prim", std::move(prim)));
  }
};

ZENDEFNODE(Make3DGridPointsInAABB,
        { /* inputs: */ {
        {gParamType_Int, "nx", "4"},
        {gParamType_Int, "ny", "0"},
        {gParamType_Int, "nz", "0"},
        {gParamType_Vec3f, "bmin", "-1,-1,-1"},
        {gParamType_Vec3f, "bmax", "1,1,1"},
        }, /* outputs: */ {
{gParamType_Primitive, "prim"},
}, /* params: */ {
        {gParamType_Bool, "isStaggered", "1"},
        }, /* category: */ {
        "primitive",
        }});


// TODO: deprecate this xuben-happy node
struct MakeCubePrimitive : INode {
    virtual void apply() override {
        float spacing = ZImpl(get_input<NumericObject>("spacing"))->get<float>();
        size_t nx = ZImpl(get_input<NumericObject>("nx"))->get<int>();
        nx = std::max(nx, (size_t)1);
        size_t ny = ZImpl(has_input("ny")) ?
            ZImpl(get_input<NumericObject>("ny"))->get<int>() : nx;
        size_t nz = ZImpl(has_input("nz")) ?
            ZImpl(get_input<NumericObject>("nz"))->get<int>() : nx;

        vec3f o = ZImpl(has_input("origin")) ?
            ZImpl(get_input<NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0);
    
    auto prim = std::make_unique<PrimitiveObject>();
    prim->resize(nx * ny * nz);
    auto &pos = prim->add_attr<zeno::vec3f>("pos");
#pragma omp parallel for
    // for (size_t y = 0; y < ny; y++) {
    //     for (size_t x = 0; x < nx; x++) {
    for (int index = 0; index < nx * ny * nz; index++) {
      int x = index % nx;
      int y = index / nx % ny;
      int z = index / nx / ny;
      vec3f p = o + vec3f(x * spacing, y * spacing, z * spacing);
      pos[index] = p;
      // }
    }
    ZImpl(set_output("prim", std::move(prim)));
  }
};

ZENDEFNODE(MakeCubePrimitive,
        { /* inputs: */
        {
            {gParamType_Float,"spacing"},
            {gParamType_Int,"nx"},
            {gParamType_Int,"ny"},
            {gParamType_Vec3f,"origin"},
        }, /* outputs: */ {
{gParamType_Primitive, "prim"},
}, /* params: */ {
        {},
        }, /* category: */ {
        "deprecated",
        }});

struct MakeBoxPrimitive : INode {
    virtual void apply() override {
        float size_x = ZImpl(get_input<NumericObject>("size_x"))->get<float>();
        float size_y = ZImpl(get_input<NumericObject>("size_y"))->get<float>();
        float size_z = ZImpl(get_input<NumericObject>("size_z"))->get<float>();
        vec3f o = ZImpl(get_input<NumericObject>("origin"))->get<zeno::vec3f>();
        auto prim = std::make_unique<PrimitiveObject>();
        prim->resize(8);
        auto& pos = prim->add_attr<zeno::vec3f>("pos");

        for (int index = 0; index < 8; index++) {
            int x = index / 2 / 2;
            int y = index / 2 % 2;
            int z = index % 2;
            vec3f p = o + vec3f(size_x * (x - 0.5), size_y * (y - 0.5), size_z * (z - 0.5));
            pos[index] = p;
        }

        if (ZImpl(get_param<bool>("use_quads"))) {
            std::vector<vec4i> cubeQuads{
            vec4i(0, 4, 5, 1),
            vec4i(4, 6, 7, 5),
            vec4i(2, 3, 7, 6),
            vec4i(0, 1, 3, 2),
            vec4i(1, 5, 7, 3),
            vec4i(0, 2, 6, 4), };
            prim->quads.values = cubeQuads;
            ZImpl(set_output("prim", std::move(prim)));
        }
        else {
            std::vector<vec3i> cubeTris{
            vec3i(4, 2, 0),
            vec3i(2, 7, 3),
            vec3i(6, 5, 7),
            vec3i(1, 7, 5),
            vec3i(0, 3, 1),
            vec3i(4, 1, 5),
            vec3i(4, 6, 2),
            vec3i(2, 6, 7),
            vec3i(6, 4, 5),
            vec3i(1, 3, 7),
            vec3i(0, 2, 3),
            vec3i(4, 0, 1) };
            prim->tris.values = cubeTris;
            ZImpl(set_output("prim", std::move(prim)));
        }
    }
};
ZENO_DEFNODE(MakeBoxPrimitive)(
    { /* inputs: */ {
        {gParamType_Float,"size_x","2.0"},
        {gParamType_Float,"size_y","2.0"},
        {gParamType_Float,"size_z","2.0"},
        {gParamType_Vec3f,"origin","0,0,0"},
    }, /* outputs: */ {
{gParamType_Primitive, "prim"},
}, /* params: */ {
        {gParamType_Bool,"use_quads","0"},
    }, /* category: */ {
        "primitive",
    } }
);

} // namespace zeno