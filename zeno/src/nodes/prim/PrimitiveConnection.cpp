#include "zeno/types/StringObject.h"
#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace zeno {


struct PrimitiveSimplePoints : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t points_count = prim->size();
    prim->points.resize(points_count);
    for (int i = 0; i < points_count; i++) {
      prim->points[i] = i;
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveSimplePoints, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
        {gParamType_Primitive, "prim"},
    },
    {
    },
    {
        "primitive",
    }
});


struct PrimitiveSimpleLines : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t lines_count = prim->size() / 2;
    prim->lines.resize(lines_count);
    for (int i = 0; i < lines_count; i++) {
      prim->lines[i] = zeno::vec2i(2 * i, 2 * i + 1);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveSimpleLines, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {},
    {"primitive"}
});


struct PrimitiveFarSimpleLines : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t lines_count = prim->size() / 2;
    prim->lines.resize(lines_count);
    for (int i = 0; i < lines_count; i++) {
      prim->lines[i] = zeno::vec2i(i, i + lines_count);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveFarSimpleLines, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {},
    {
    "primitive",
    }
});

struct PrimitiveNearSimpleLines : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t lines_count = prim->size() ? prim->size() - 1 : 0;
    prim->lines.resize(lines_count);
    for (int i = 0; i < lines_count; i++) {
      prim->lines[i] = zeno::vec2i(i, i + 1);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveNearSimpleLines,{
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {},
    {
        "primitive",
    }
});


struct PrimitiveSimpleTris : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t tris_count = prim->size() / 3;
    prim->tris.resize(tris_count);
    for (int i = 0; i < tris_count; i++) {
      prim->tris[i] = zeno::vec3i(3 * i, 3 * i + 1, 3 * i + 2);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveSimpleTris, {
    {
{gParamType_Primitive, "prim"},
},
    {
{gParamType_Primitive, "prim"},
},
    {
    },
    {
        "primitive",
    }
});


struct PrimitiveSimpleQuads : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    size_t quads_count = prim->size() / 4;
    prim->quads.resize(quads_count);
    for (int i = 0; i < quads_count; i++) {
      prim->quads[i] = zeno::vec4i(4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveSimpleQuads, {
    {
{gParamType_Primitive, "prim"},
},
    {
{gParamType_Primitive, "prim"},
},
    {},
    {
    "primitive",
    }
});


struct PrimitiveClearConnect : zeno::INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto type = ZImpl(get_input<StringObject>("type"))->value;
    
    if(type=="points" || type=="all")
      prim->points.clear();
    if(type=="edges" || type=="all")
      prim->lines.clear();
    if(type=="faces" || type=="tris" || type=="all"){
      prim->tris.clear();
    }
    if(type=="faces" || type=="quads" || type=="all"){
      prim->quads.clear();
    }
    if(type=="faces" || type=="polys" || type=="all"){
      prim->polys.clear();
      prim->loops.clear();
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveClearConnect, {
    {
        {gParamType_Primitive, "prim"},
        {"enum edges faces tris quads polys points all","type", "all"}
    },
    {
{gParamType_Primitive, "prim"},
},
    {},
    {
        "primitive",
    }
});

struct PrimitiveLineSimpleLink : zeno::INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<zeno::PrimitiveObject>("prim"));

        prim->lines.clear();
        intptr_t n = prim->verts.size();
        prim->lines.reserve(n);
        for (intptr_t i = 1; i < n; i++) {
            prim->lines.emplace_back(i - 1, i);
        }
        prim->lines.update();
        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimitiveLineSimpleLink, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});


struct PrimitiveSplitEdges : zeno::INode { // TODO: use PrimSplitFaces to replace this node
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));

    prim->foreach_attr([&] (auto &, auto &arr) {
        auto oldarr = arr;
        arr.resize(prim->tris.size() * 3);
        for (size_t i = 0; i < prim->tris.size(); i++) {
            auto ind = prim->tris[i];
            arr[i * 3 + 0] = oldarr[ind[0]];
            arr[i * 3 + 1] = oldarr[ind[1]];
            arr[i * 3 + 2] = oldarr[ind[2]];
        }
    });
    prim->resize(prim->tris.size() * 3);

    for (size_t i = 0; i < prim->tris.size(); i++) {
        prim->tris[i] = zeno::vec3i(i * 3 + 0, i * 3 + 1, i * 3 + 2);
    }

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveSplitEdges, {
    {{gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly}},
    {{gParamType_Primitive, "prim"}},
    {},
    {"deprecated"},
});


struct PrimitiveFaceToEdges : zeno::INode {
  std::pair<int, int> sorted(int x, int y) {
      return x < y ? std::make_pair(x, y) : std::make_pair(y, x);
  }

  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    std::set<std::pair<int, int>> lines;

    for (int i = 0; i < prim->tris.size(); i++) {
        auto uvw = prim->tris[i];
        int u = uvw[0], v = uvw[1], w = uvw[2];
        lines.insert(sorted(u, v));
        lines.insert(sorted(v, w));
        lines.insert(sorted(u, w));
    }
    for (auto [u, v]: lines) {
        prim->lines.emplace_back(u, v);
    }

    if (ZImpl(get_param<bool>("clearFaces"))) {
        prim->tris.clear();
    }
    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveFaceToEdges, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {
        {gParamType_Bool, "clearFaces", "1"},
    },
    {
    "deprecated",
    }
});


struct PrimitiveFlipPoly : zeno::INode {
    virtual void apply() override {
        auto surfIn = ZImpl(get_input<zeno::PrimitiveObject>("prim"));
        for(size_t i = 0;i < surfIn->tris.size();++i){
            auto& tri = surfIn->tris[i];
            size_t tri_idx_tmp = tri[1];
            tri[1] = tri[2];
            tri[2] = tri_idx_tmp;
        }

        ZImpl(set_output("primOut",surfIn));
    }
};

ZENDEFNODE(PrimitiveFlipPoly,{
    {{gParamType_Primitive, "prim"}},
    {{gParamType_Primitive, "primOut"}},
    {},
    {"deprecated"},
});


}
