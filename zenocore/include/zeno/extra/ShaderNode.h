#pragma once

#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/TextureObject.h>
#include <vector>
#include <string>
#include <map>

namespace zeno {

struct EmissionPass;

struct ShaderNode : INode {
    ZENO_API virtual void apply() override;
    ZENO_API virtual int determineType(EmissionPass *em) = 0;
    ZENO_API virtual void emitCode(EmissionPass *em) = 0;
    ZENO_API virtual std::shared_ptr<ShaderNode> clone() const = 0;

    ZENO_API ShaderNode();
};

template <class Derived>
struct ShaderNodeClone : ShaderNode {
    virtual std::shared_ptr<ShaderNode> clone() const override {
        //TODO: clone INode.
        return nullptr;
        //return std::make_shared<Derived>(static_cast<Derived const &>(*this));
    }
};

struct EmissionPass {
    enum Backend {
        GLSL = 0,
        HLSL,  /* DID U KNOW THAT MICROSOFT BUYS GITHUB */
    };

    Backend backend = GLSL;

    struct ConstInfo {
        int type;
        NumericValue value;
    };

    struct VarInfo {
        int type;
        ShaderNode *node;
    };

    struct CommonFunc {
        int rettype{};
        std::string name;
        std::vector<int> argTypes;
        std::string code;
    };

    std::map<ParamPath, int> constmap;
    std::vector<ConstInfo> constants;
    std::map<ShaderNode *, int> varmap;  /* varmap[node] = 40, then the variable of node is "tmp40" */
    std::vector<VarInfo> variables;  /* variables[40].type = 3, then the variable type will be "vec3 tmp40;" */
    std::vector<std::string> lines;  /* contains a list of operations, e.g. {"tmp40 = tmp41 + 1;", "tmp42 = tmp40 * 2;"} */
    std::vector<CommonFunc> commons;  /* definition of common functions, including custom functions and pre-defined functions */
    std::string commonCode;           /* other common codes written directly in GLSL, e.g. "void myutilfunc() {...}" */
    std::string extensionsCode;       /* OpenGL extensions, e.g. "#extension GL_EXT_gpu_shader4 : enable" */
    std::vector<std::shared_ptr<Texture2DObject>> tex2Ds;

    ZENO_API std::string typeNameOf(int type) const;
    ZENO_API std::string funcName(std::string const &fun) const;

    ZENO_API std::string finalizeCode(std::vector<std::pair<int, std::string>> const &keys,
                                      std::vector<ShaderData> const &vals);
    ZENO_API std::string finalizeCode();

    ZENO_API std::string addCommonFunc(CommonFunc func);
    ZENO_API std::string getCommonCode() const;

    ZENO_API void duplicateIfHlsl(int type, std::string &expr) const;
    ZENO_API static void translateToHlsl(std::string &code);
    ZENO_API void translateCommonCode();

    ZENO_API std::string determineExpr(const ShaderData& data) const;
    ZENO_API std::string determineExpr(const ShaderData& data, ShaderNode *node) const;
    ZENO_API std::string collectDefs() const;
    ZENO_API std::string collectCode() const;

    ZENO_API int currentType(ShaderNode *node) const;
    ZENO_API int determineType(const ShaderData& data);
    ZENO_API void emitCode(std::string const &line);
};

}
