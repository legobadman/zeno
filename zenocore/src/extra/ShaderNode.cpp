#include <zeno/zeno.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/core/NodeImpl.h>
#include <sstream>
#include <cassert>

namespace zeno {

static std::string ftos(float x) {
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

ZENO_API ShaderNode::ShaderNode() = default;

ZENO_API void ShaderNode::apply() {
    ShaderData shader;
    shader.data = m_pAdapter->get_uuid_path();
    m_pAdapter->set_primitive_output("out", shader);
}

ZENO_API std::string EmissionPass::finalizeCode() {
    auto defs = collectDefs();
    for (auto const &var: variables) {
        var.node->emitCode(this);
    }
    translateCommonCode();
    auto code = collectCode();
    return defs + code;
}

ZENO_API int EmissionPass::determineType(const ShaderData& shader) {

    if (shader.data.index() == 0) {
        NumericValue numvalue = std::get<NumericValue>(shader.data);
        if (auto it = constmap.find(shader.curr_param); it != constmap.end())
            return constants.at(it->second).type;

        int type = std::visit([&](auto const& value) -> int {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<float, T>) {
                return 1;
            }
            else if constexpr (std::is_same_v<vec2f, T>) {
                return 2;
            }
            else if constexpr (std::is_same_v<vec3f, T>) {
                return 3;
            }
            else if constexpr (std::is_same_v<vec4f, T>) {
                return 4;
            }
            else {
                throw zeno::Exception("bad numeric object type: " + (std::string)typeid(T).name());
            }
        }, numvalue);

        constmap[shader.curr_param] = constants.size();
        constants.push_back(ConstInfo{ type, numvalue });
        return type;
    }
    else {
        ShaderNodePath path = std::get<ShaderNodePath>(shader.data);
        NodeImpl* pNode = zeno::getSession().getNodeByUuidPath(path);
        assert(pNode);
        ShaderNode* treenode = dynamic_cast<ShaderNode*>(pNode->coreNode());
        if (auto it = varmap.find(treenode); it != varmap.end())
            return variables.at(it->second).type;
        int type = treenode->determineType(this);
        varmap[treenode] = variables.size();
        variables.push_back(VarInfo{ type, treenode });
        return type;
    }
}

ZENO_API int EmissionPass::currentType(ShaderNode *node) const {
    return variables[varmap.at(node)].type;
}

ZENO_API std::string EmissionPass::determineExpr(const ShaderData& data, ShaderNode *node) const {
    auto type = currentType(node);
    auto expr = determineExpr(data);
    duplicateIfHlsl(type, expr);
    return typeNameOf(type) + "(" + expr + ")";
}

ZENO_API void EmissionPass::duplicateIfHlsl(int type, std::string &expr) const {
    if (backend == HLSL && type > 1) {
        /* WHY IS MICROSOFT SO STUPID? THIS IS A PROBLEM. */
        expr += " * float" + std::to_string(type) + "(1";
        auto tmp = ", 1";
        for (int i = 1; i < type; i++)
            expr += tmp;
        expr += ")";
    }
}

static const auto cihou = [] {
    std::map<std::string, std::string> cihou;
    cihou["mix"] = "lerp";
    cihou["inversesqrt"] = "rsqrt";
    cihou["mod"] = "fmod";
    cihou["vec2"] = "float2";
    cihou["vec3"] = "float3";
    cihou["vec4"] = "float4";
    return cihou;
}();

ZENO_API std::string EmissionPass::funcName(std::string const &fun) const {
    if (backend == HLSL) {
        if (auto it = cihou.find(fun); it != cihou.end())
            return it->second;
    }
    return fun;
}

ZENO_API std::string EmissionPass::addCommonFunc(EmissionPass::CommonFunc comm) {
    int idx = commons.size();
    if (comm.name.empty())
        comm.name = "fun" + std::to_string(idx);
    commons.push_back(std::move(comm));
    return commons.back().name;
}

ZENO_API std::string EmissionPass::getCommonCode() const {
    std::string ret = commonCode;
    for (int i = 0; i < commons.size(); i++) {
        ret += "\n" + typeNameOf(commons[i].rettype) + " " + commons[i].name + commons[i].code + "\n";
    }
    return ret;
}

ZENO_API std::string EmissionPass::typeNameOf(int type) const {
    if (type == 1) return "float";
    else return (backend == HLSL ? "float" : "vec") + std::to_string(type);
}

ZENO_API std::string EmissionPass::collectDefs() const {
    std::string res;
    int cnt = 0;
    for (auto const &var: constants) {
        auto expr = std::visit([&] (auto const &value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<float, T>) {
                return typeNameOf(1) + "(" + ftos(value) + ")";
            } else if constexpr (std::is_same_v<vec2f, T>) {
                return typeNameOf(2) + "(" + ftos(value[0]) + ", " + ftos(value[1]) + ")";
            } else if constexpr (std::is_same_v<vec3f, T>) {
                return typeNameOf(3) + "(" + ftos(value[0]) + ", " + ftos(value[1]) + ", "
                + ftos(value[2]) + ")";
            } else if constexpr (std::is_same_v<vec4f, T>) {
                return typeNameOf(4) + "(" + ftos(value[0]) + ", " + ftos(value[1]) + ", "
                + ftos(value[2]) + ", " + ftos(value[3]) + ")";
            } else {
                throw zeno::Exception("bad numeric object type: " + (std::string)typeid(T).name());
            }
        }, var.value);
        res += typeNameOf(var.type) + " constmp" + std::to_string(cnt) + " = " + expr + ";\n";
        cnt++;
    }
    cnt = 0;
    for (auto const &var: variables) {
        res += typeNameOf(var.type) + " tmp" + std::to_string(cnt) + ";\n";
        cnt++;
    }
    return res;
}

ZENO_API std::string EmissionPass::collectCode() const {
    std::string res;
    for (auto const &line: lines) {
        res += line + "\n";
    }
    return res;
}

ZENO_API std::string EmissionPass::determineExpr(const ShaderData& shader) const {
    return std::visit([&](auto&& value)->std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, NumericValue>) {
            return "constmp" + std::to_string(constmap.at(shader.curr_param));
        }
        else if constexpr (std::is_same_v<T, ShaderNodePath>) {
            NodeImpl* pNode = zeno::getSession().getNodeByUuidPath(value);
            ShaderNode* treenode = dynamic_cast<ShaderNode*>(pNode->coreNode());
            return "tmp" + std::to_string(varmap.at(treenode));
        }
        else {
            return {};
        }
    }, shader.data);
}

ZENO_API void EmissionPass::emitCode(std::string const &line) {
    int idx = lines.size();
    lines.push_back("tmp" + std::to_string(idx) + " = " + line + ";");
}

ZENO_API std::string EmissionPass::finalizeCode(std::vector<std::pair<int, std::string>> const &keys,
                                                std::vector<ShaderData> const &vals) {
    std::vector<int> vartypes;
    vartypes.reserve(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        const auto& shader_data = vals[i];
        int their_type = determineType(shader_data);
        int our_type = keys[i].first;
        if (their_type != our_type && their_type != 1)
            throw zeno::Exception("unexpected input for " + keys[i].second + " which requires "
                                  + typeNameOf(our_type) + " but got " + typeNameOf(their_type));
        vartypes.push_back(their_type);
    }
    auto code = finalizeCode();
    for (int i = 0; i < keys.size(); i++) {
        const auto& shader_data = vals[i];
        auto expr = determineExpr(shader_data);
        int our_type = keys[i].first;
        duplicateIfHlsl(our_type, expr);
        //printf("!!!!!!!!!!!!%d %s\n", our_type, expr.c_str());
        code += typeNameOf(our_type) + " " + keys[i].second + " = " + typeNameOf(our_type) + "(" + expr + ");\n";
    }
    return code;
}

ZENO_API void EmissionPass::translateToHlsl(std::string &code) {
    std::string ret;
    for (auto const &[key, val]: cihou) {
        size_t pos = 0, last_pos = 0;
        ret.clear();
        auto isident = [] (char c) -> bool {
            return ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
        };
        while ((pos = code.find(key, pos)) != std::string::npos) {
            if (pos != 0 && isident(code[pos - 1])) {
                pos += key.size();
                continue;
            }
            if (pos + key.size() != code.size() && isident(code[pos + key.size()])) {
                pos += key.size();
                continue;
            }
            //printf("%s %s %d %d\n", key.c_str(), code.substr(pos).c_str(), last_pos, pos);
            ret.append(code, last_pos, pos - last_pos);
            ret.append(val);
            pos += key.size();
            last_pos = pos;
        }
        if (!ret.empty() && last_pos != 0) {
            ret.append(code, last_pos, std::string::npos);
            code = std::move(ret);
        }
    }
}

ZENO_API void EmissionPass::translateCommonCode() {
    if (backend != HLSL) return;
    for (auto &comm: commons)
        translateToHlsl(comm.code);
    translateToHlsl(commonCode);
}

}
