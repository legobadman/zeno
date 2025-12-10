#include <zeno/zeno.h>
#include <zeno/types/NumericObject.h>
#include <iostream>
#include <cstdlib>
#include <fstream>

namespace zeno {
namespace {

struct PrintNumeric : zeno::INode {
    template <class S, class T>
    struct do_print {
        do_print(S& stream, T const &x) {
            stream << x;
        }
    };

    template <class S, size_t N, class T>
    struct do_print<S, zeno::vec<N, T>> {
        do_print(S& stream, zeno::vec<N, T> const &x) {
            stream << "(";
            for (int i = 0; i < N; i++) {
                stream << x[i];
                if (i != N - 1) {
                    stream << ", ";
                }
            }
            stream << ")";
        }
    };

    virtual void apply() override {
        auto num = ZImpl(get_param_result("value"));
        bool bSucceed = false;
        auto objvalue = AnyToNumeric(num, bSucceed);
        if (!bSucceed)
            throw makeError<UnimplError>("AnyToNumeric");
        auto hint = ZImpl(get_param<std::string>("hint"));
        auto path = ZImpl(get_param<std::string>("write path"));
        bool bEnable = get_input2_bool("Enable");
        if (bEnable) {
            if (path.empty()) {
                zeno::log_critical("{}: ", hint);
                std::clog << hint << ": ";
                std::visit([](auto const& val) {
                    using T = std::decay_t<decltype(val)>;
                    std::clog << (std::string)typeid(T).name() + " :";
                    do_print _(std::clog, val);
                    }, objvalue);
                std::clog << std::endl;
            }
            else {
                bool bAppend = get_input2_bool("write append");
                std::ofstream out(path, bAppend ? std::ios::app : std::ios::out);
                out << hint << ": ";
                std::visit([&](auto const& val) {
                    using T = std::decay_t<decltype(val)>;
                    out << (std::string)typeid(T).name() + " :";
                    do_print _(out, val);
                    }, objvalue);
                out << std::endl;
                out.close();
            }
        }
        ZImpl(set_primitive_output("value", num));
    }
};

ZENDEFNODE(PrintNumeric, {
    {
        {gParamType_AnyNumeric, "value", "0"},
        {gParamType_String, "write path", "", Socket_Primitve, zeno::WritePathEdit},
        {gParamType_Bool, "write append"},
        {gParamType_Bool, "Enable", "1"}
    },
    {{gParamType_AnyNumeric, "value"}},
    {{gParamType_String, "hint", "PrintNumeric"}},
    {"numeric"},
});


//struct ToVisualize_NumericObject : PrintNumeric {
    //virtual void apply() override {
        //inputs["hint:"] = std::make_unique<zeno::StringObject>("VIEW of NumericObject");
        //PrintNumeric::apply();
    //}
//};

//ZENO_DEFOVERLOADNODE(ToVisualize, _NumericObject, typeid(zeno::NumericObject).name())({
        //{"value"},
        //{},
        //{{gParamType_String, "path", ""}},
        //{"numeric"},
//});

}
}
