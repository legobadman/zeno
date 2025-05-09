#if 0
#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/DictObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/core/Graph.h>
#include <zfx/zfx.h>
#include <zfx/x64.h>
#include <cassert>
#include "dbg_printf.h"

namespace zeno {
    std::string preApplyRefs(const std::string& code, Graph* pGraph);

namespace {

static zfx::Compiler compiler;
static zfx::x64::Assembler assembler;

struct Buffer {
    float *base = nullptr;
    size_t count = 0;
    size_t stride = 0;
};
template <typename T>
static void vectors_wrangle
    ( zfx::x64::Executable *exec
    , std::vector<Buffer> const &chs
    , T *maskarr
    ) {
    if (chs.size() == 0)
        return;
    size_t size = chs[0].count;
    for (int i = 1; i < chs.size(); i++) {
        size = std::min(chs[i].count, size);
    }

    #pragma omp parallel for
    for (int i = 0; i < size - exec->SimdWidth + 1; i += exec->SimdWidth) {
        auto ctx = exec->make_context();
        for (int j = 0; j < chs.size(); j++) {
            for (int k = 0; k < exec->SimdWidth; k++)
                ctx.channel(j)[k] = chs[j].base[chs[j].stride * (i + k)];
        }
        ctx.execute();
        for (int k = 0; k < exec->SimdWidth; k++) {
            for (int j = 0; j < chs.size(); j++) {
                if (maskarr[i + k] != 0)
                    chs[j].base[chs[j].stride * (i + k)] = ctx.channel(j)[k];
            }
        }
    }
    for (int i = size / exec->SimdWidth * exec->SimdWidth; i < size; i++) {
        auto ctx = exec->make_context();
        for (int j = 0; j < chs.size(); j++) {
            ctx.channel(j)[0] = chs[j].base[chs[j].stride * i];
        }
        ctx.execute();
        for (int j = 0; j < chs.size(); j++) {
            if (maskarr[i] != 0) {
                chs[j].base[chs[j].stride * i] = ctx.channel(j)[0];
            }
        }
    }
}

struct ParticlesMaskedWrangle : zeno::INode {
    virtual void apply() override {
        auto prim = get_input<zeno::PrimitiveObject>("prim");
        auto code = get_input<zeno::StringObject>("zfxCode")->get();

        // BEGIN张心欣快乐自动加@IND
        if (auto pos = code.find("@IND"); pos != code.npos && (code.size() <= pos + 4 || !(isalnum(code[pos + 4]) || strchr("_@$", code[pos + 4]))) && (pos == 0 || !(isalnum(code[pos - 1]) || strchr("_@$", code[pos - 1])))) {
            auto &indatt = prim->verts.add_attr<float>("IND");
            for (size_t i = 0; i < indatt.size(); i++) indatt[i] = float(i);
        }
        // END张心欣快乐自动加@IND

        zfx::Options opts(zfx::Options::for_x64);
        opts.detect_new_symbols = true;
        prim->foreach_attr([&] (auto const &key, auto const &attr) {
            int dim = ([] (auto const &v) {
                using T = std::decay_t<decltype(v[0])>;
                if constexpr (std::is_same_v<T, zeno::vec3f>) return 3;
                else if constexpr (std::is_same_v<T, float>) return 1;
                else return 0;
            })(attr);
            dbg_printf("define symbol: @%s dim %d\n", key.c_str(), dim);
            opts.define_symbol('@' + key, dim);
        });

        auto params = has_input("params") ?
            get_input<zeno::DictObject>("params") :
            std::make_shared<zeno::DictObject>();
        {
        // BEGIN心欣你也可以把这段代码加到其他wrangle节点去，这样这些wrangle也可以自动有$F$DT$T做参数
        auto const &gs = *this->getGlobalState();
        params->lut["PI"] = objectFromLiterial((float)(std::atan(1.f) * 4));
        params->lut["F"] = objectFromLiterial((float)gs.getFrameId());
        params->lut["DT"] = objectFromLiterial(gs.frame_time);
        params->lut["T"] = objectFromLiterial(gs.frame_time * gs.getFrameId() + gs.frame_time_elapsed);
        // END心欣你也可以把这段代码加到其他wrangle节点去，这样这些wrangle也可以自动有$F$DT$T做参数
        // BEGIN心欣你也可以把这段代码加到其他wrangle节点去，这样这些wrangle也可以自动引用portal做参数
        for (auto const &[key, ref]: getThisGraph()->portalIns) {
            if (auto i = code.find('$' + key); i != std::string::npos) {
                i = i + key.size() + 1;
                if (code.size() <= i || !std::isalnum(code[i])) {
                    if (params->lut.count(key)) continue;
                    dbg_printf("ref portal %s\n", key.c_str());
                    auto res = getThisGraph()->callTempNode("PortalOut",
                          {{"name:", objectFromLiterial(key)}}).at("port");
                    params->lut[key] = std::move(res);
                }
            }
        }
        // END心欣你也可以把这段代码加到其他wrangle节点去，这样这些wrangle也可以自动引用portal做参数
        // BEGIN伺候心欣伺候懒得extract出变量了
        std::vector<std::string> keys;
        for (auto const &[key, val]: params->lut) {
            keys.push_back(key);
        }
        for (auto const &key: keys) {
            if (!dynamic_cast<zeno::NumericObject*>(params->lut.at(key).get())) {
                dbg_printf("ignored non-numeric %s\n", key.c_str());
                params->lut.erase(key);
            }
        }
        // END伺候心欣伺候懒得extract出变量了
        }
        std::vector<float> parvals;
        std::vector<std::pair<std::string, int>> parnames;
        for (auto const &[key_, par]: params->getLiterial<zeno::NumericValue>()) {
            auto key = '$' + key_;
                auto dim = std::visit([&] (auto const &v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_convertible_v<T, zeno::vec3f>) {
                        parvals.push_back(v[0]);
                        parvals.push_back(v[1]);
                        parvals.push_back(v[2]);
                        parnames.emplace_back(key, 0);
                        parnames.emplace_back(key, 1);
                        parnames.emplace_back(key, 2);
                        return 3;
                    } else if constexpr (std::is_convertible_v<T, float>) {
                        parvals.push_back(v);
                        parnames.emplace_back(key, 0);
                        return 1;
                    } else if constexpr (std::is_convertible_v<T, vec2f>) {
                        parvals.push_back(v[0]);
                        parvals.push_back(v[1]);
                        parnames.emplace_back(key, 0);
                        parnames.emplace_back(key, 1);
                        return 2;
                    } else {
                        printf("invalid parameter type encountered: `%s`\n",
                                typeid(T).name());
                        return 0;
                    }
                }, par);
                dbg_printf("define param: %s dim %d\n", key.c_str(), dim);
                opts.define_param(key, dim);
            //auto par = zeno::safe_any_cast<zeno::NumericValue>(obj);
            
        }
        //if (1)
        //{
        //    // BEGIN 引用预解析：将其他节点参数引用到此处，可能涉及提前对该参数的计算
        //    // 方法是: 搜索code里所有ref(...)，然后对于每一个ref(...)，解析ref内部的引用，
        //    // 然后将计算结果替换对应ref(...)，相当于预处理操作。
        //    code = preApplyRefs(code, getThisGraph());
        //    // END 引用预解析
        //}

        auto prog = compiler.compile(code, opts);
        auto exec = assembler.assemble(prog->assembly);

        for (auto const &[name, dim]: prog->newsyms) {
            dbg_printf("auto-defined new attribute: %s with dim %d\n",
                    name.c_str(), dim);
            assert(name[0] == '@');
            auto key = name.substr(1);
            if (dim == 3) {
                prim->add_attr<zeno::vec3f>(key);
            } else if (dim == 1) {
                prim->add_attr<float>(key);
            } else {
                err_printf("ERROR: bad attribute dimension for primitive: %d\n",
                    dim);
            }
        }

        for (int i = 0; i < prog->params.size(); i++) {
            auto [name, dimid] = prog->params[i];
            dbg_printf("parameter %d: %s.%d\n", i, name.c_str(), dimid);
            assert(name[0] == '$');
            auto it = std::find(parnames.begin(),
                parnames.end(), std::pair{name, dimid});
            auto value = parvals.at(it - parnames.begin());
            dbg_printf("(valued %f)\n", value);
            exec->parameter(prog->param_id(name, dimid)) = value;
        }

        std::vector<Buffer> chs(prog->symbols.size());
        for (int i = 0; i < chs.size(); i++) {
            auto [name, dimid] = prog->symbols[i];
            dbg_printf("channel %d: %s.%d\n", i, name.c_str(), dimid);
            assert(name[0] == '@');
            Buffer iob;
            prim->attr_visit(name.substr(1),
            [&, dimid_ = dimid] (auto const &arr) {
                iob.base = (float *)arr.data() + dimid_;
                iob.count = arr.size();
                iob.stride = sizeof(arr[0]) / sizeof(float);
            });
            chs[i] = iob;
        }
        std::string maskAttr = get_input2<std::string>("maskAttr");
        if(prim->attr_is<float>(maskAttr)){
            auto &maskarr = prim->attr<float>(maskAttr);
            vectors_wrangle(exec, chs, maskarr.data());
        }
        else if(prim->attr_is<int>(maskAttr)){
            auto &maskarr = prim->attr<int>(maskAttr);
            vectors_wrangle(exec, chs, maskarr.data());
        }
        else{
            throw std::runtime_error("mask type not supported");
        }

        set_output("prim", std::move(prim));
    }
};

ZENDEFNODE(ParticlesMaskedWrangle, {
    {{gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
     {gParamType_String, "zfxCode", "", Socket_Primitve, CodeEditor},
     {gParamType_Dict, "params", "", zeno::Socket_ReadOnly},
     {gParamType_String, "maskAttr", "mask"}},
    {{gParamType_Primitive, "prim"}},
    {},
    {"zenofx"},
});

//struct PrimWrangle : ParticlesWrangle {
//};

//ZENDEFNODE(PrimWrangle, {
    //{{gParamType_Primitive, "prim"},
     //{gParamType_String, "zfxCode"}, {gParamType_Dict, "params"}},
    //{{gParamType_Primitive, "prim"}},
    //{},
    //{"zenofx"},
//});


}
}
#endif