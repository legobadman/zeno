#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/StringObject.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/fileio.h>
#include <zeno/geo/commonutil.h>
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
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto path = ZImpl(get_input<StringObject>("path"))->get();
        path = create_directories_when_write_file(path);

        if (get_input2_bool("polygonate")) {
            primPolygonate(prim.get());
        }
        std::ofstream fout(path);
        dump_obj(prim.get(), fout);
        set_output("prim", create_GeometryObject(prim.get()));
    }
};

ZENDEFNODE(WriteObjPrim,
        { /* inputs: */ {
        {gParamType_Geometry, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::WritePathEdit},
        }, /* outputs: */ {
        {gParamType_Geometry, "prim"},
        }, /* params: */ {
        {gParamType_Bool, "polygonate", "1"},
        }, /* category: */ {
        "primitive",
        }});

}
}
