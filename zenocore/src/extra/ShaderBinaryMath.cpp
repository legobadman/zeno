//
// ShaderBinaryMath - INode2/ShaderNode implementation
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/utils/Exception.h>
#include <zeno/utils/format.h>

namespace zeno {

static const char binops[] = "add sub mul div mod pow atan2 min max dot cross distance safepower step";

struct ShaderBinaryMath : ShaderNodeClone<ShaderBinaryMath> {
    virtual int determineType(EmissionPass *em, ZNodeParams* params) override {
        auto op = params->get_input2_string("op");
        auto in1 = params->get_input_shader("in1");
        auto in2 = params->get_input_shader("in2");
        auto t1 = em->determineType(in1);
        auto t2 = em->determineType(in2);

        if (op == "dot") {
            if (t1 != t2)
                throw zeno::Exception("both-side of dot must have same dimension");
            else if (t1 == 1)
                throw zeno::Exception("dot only work for vectors");
            else
                return 1;

        } else if (op == "cross") {
            if (t1 != t2)
                throw zeno::Exception("both-side of cross must have same dimension");
            else if (t1 == 2)
                return 1;
            else if (t1 == 3)
                return 3;
            else
                throw zeno::Exception("dot only work for 2d and 3d vectors");

        } else if (op == "distance") {
            if (t1 != t2)
                throw zeno::Exception("both-side of distance must have same dimension");
            else if (t1 == 1)
                throw zeno::Exception("distance only work for vectors");
            else
                return t1;

        } else if (t1 == 1) {
            return t2;
        } else if (t2 == 1) {
            return t1;
        } else if (t1 == t2) {
            return t1;
        } else {
            throw zeno::Exception("vector dimension mismatch: " + std::to_string(t1) + " != " + std::to_string(t2));
        }
    }

    virtual void emitCode(EmissionPass *em, ZNodeParams* params) override {
        auto op = params->get_input2_string("op");
        auto in1 = em->determineExpr(params->get_input_shader("in1"));
        auto in2 = em->determineExpr(params->get_input_shader("in2"));

        if (op == "add") {
            return em->emitCode(in1 + " + " + in2);
        } else if (op == "sub") {
            return em->emitCode(in1 + " - " + in2);
        } else if (op == "mul") {
            return em->emitCode(in1 + " * " + in2);
        } else if (op == "div") {
            return em->emitCode(in1 + " / " + in2);
        } else {
            return em->emitCode(em->funcName(op) + "(" + in1 + ", " + in2 + ")");
        }
    }
};

ZENDEFNODE(ShaderBinaryMath, {
    {
        {gParamType_AnyNumeric, "in1", "0.0", zeno::Socket_Primitve, zeno::Lineedit},
        {gParamType_AnyNumeric, "in2", "0.0", zeno::Socket_Primitve, zeno::Lineedit},
        {(std::string)"enum " + binops, "op", "add"},
    },
    {
        {gParamType_Shader, "out"},
    },
    {},
    {"shader"},
});

} // namespace zeno
