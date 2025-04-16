#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/StringObject.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/fileio.h>
#include <string_view>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace zeno {
namespace {

void dump_obj(PrimitiveObject *prim, std::ostream &fout) {
    fout << "# https://github.com/zenustech/zeno\n";
    fout << std::setprecision(8);
    for (auto const &[x, y, z]: prim->verts) {
        fout << "v " << x << ' ' << y << ' ' << z << '\n';
    }
    if (prim->loops.size() && prim->loops.has_attr("uvs")) {
        auto &loop_uvs = prim->loops.attr<int>("uvs");
        for (auto const &[x, y]: prim->uvs) {
            fout << "vt " << x << ' ' << y << '\n';
        }
        for (auto const &[base, len]: prim->polys) {
            fout << 'f';
            for (int j = base; j < base + len; j++) {
                auto l = prim->loops[j] + 1;
                auto lt = loop_uvs[j] + 1;
                fout << ' ' << l << '/' << lt;
            }
            fout << '\n';
        }
    } else {
        for (auto const &[base, len]: prim->polys) {
            fout << 'f';
            for (int j = base; j < base + len; j++) {
                auto l = prim->loops[j] + 1;
                fout << ' ' << l;
            }
            fout << '\n';
        }
    }
}

struct WriteObjPrim : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto path = ZImpl(get_input<StringObject>("path"))->get();
        path = create_directories_when_write_file(path);

        if (ZImpl(get_param<bool>("polygonate"))) {
            primPolygonate(prim.get());
        }
        std::ofstream fout(path);
        dump_obj(prim.get(), fout);
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(WriteObjPrim,
        { /* inputs: */ {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::WritePathEdit},
        }, /* outputs: */ {
        {gParamType_Primitive, "prim"},
        }, /* params: */ {
        {gParamType_Bool, "polygonate", "1"},
        }, /* category: */ {
        "primitive",
        }});

}
}
