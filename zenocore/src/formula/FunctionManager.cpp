#include <zeno/core/FunctionManager.h>
#include <zeno/core/Session.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/utils/Error.h>
#include <zeno/core/Graph.h>
#include <zeno/utils/log.h>
#include <zeno/utils/helper.h>
#include <variant>
#include <functional>
#include <zeno/utils/format.h>
#include <numeric>
#include <zeno/geo/geometryutil.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/vectorutil.h>
#include <reflect/type.hpp>
#include "../utils/zfxutil.h"
#include "functionimpl.h"
#include "funcDesc.h"


using namespace zeno::types;
using namespace zeno::reflect;
using namespace zeno::zfx;

namespace zeno {

    const std::regex FunctionManager::refPattern(R"([\.]?(\/\s*[a-zA-Z0-9\.]+\s*)+)");
    const std::regex FunctionManager::refStrPattern(R"(.*"[\.]?(\/\s*[a-zA-Z0-9\.]+\s*)+".*)");

    FunctionManager::FunctionManager() {
    }

    std::vector<std::string> FunctionManager::getCandidates(const std::string& prefix, bool bFunc) const {
        std::vector<std::string> candidates;
        if (bFunc && prefix.empty())
            return candidates;

        if (bFunc) {
            for (auto& [k, v] : funcsDesc) {
                //TODO: optimize the search
                if (k.substr(0, prefix.size()) == prefix) {
                    candidates.push_back(k);
                }
            }
        }
        else {
            static std::vector<std::string> vars = { "F", "FPS", "T", "PI" };
            for (auto& var : vars) {
                if (var.substr(0, prefix.size()) == prefix) {
                    candidates.push_back(var);
                }
            }
        }
        return candidates;
    }

    std::string FunctionManager::getFuncTip(const std::string& funcName, bool& bExist) const {
        auto iter = funcsDesc.find(funcName);
        if (iter == funcsDesc.end()) {
            bExist = false;
            return "";
        }
        bExist = true;
        return iter->second.tip;
    }

    ZENO_API FUNC_INFO FunctionManager::getFuncInfo(const std::string& funcName) const {
        auto iter = funcsDesc.find(funcName);
        if (iter == funcsDesc.end()) {
            return FUNC_INFO();
        }
        return iter->second;
    }

    static int getElementCount(std::shared_ptr<IObject> spObject, GeoAttrGroup runover) {
        switch (runover)
        {
        case ATTR_POINT: {
            if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(spObject)) {
                return spGeo->npoints();
            }
            else if (auto spPrim = std::dynamic_pointer_cast<PrimitiveObject>(spObject)) {
                return spPrim->verts->size();
            }
            else {
                return 0;
            }
        }
        case ATTR_FACE:
            if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(spObject)) {
                return spGeo->nfaces();
            }
            else if (auto spPrim = std::dynamic_pointer_cast<PrimitiveObject>(spObject)) {
                if (spPrim->tris.size() > 0)
                    return spPrim->tris.size();
                else
                    return spPrim->polys.size();
            }
            else {
                return 0;
            }
        case ATTR_GEO: {
            //only one element
            return 1;
        }
        }
    }

    void FunctionManager::executeZfx(std::shared_ptr<ZfxASTNode> root, ZfxContext* pCtx) {
        //printSyntaxTree(root, pCtx->code);
        pCtx->zfxVariableTbl = &m_globalAttrCached;
        if (pCtx->spObject) {
            int nFilterSize = getElementCount(pCtx->spObject, pCtx->runover);
            ZfxElemFilter filter(nFilterSize, 1);
            scope_exit sp([&] {m_globalAttrCached.clear(); });
            execute(root, filter, pCtx);
        }
        else if (!pCtx->param_constrain.constrain_param.empty()) {
            ZfxElemFilter filter(1, 1);
            execute(root, filter, pCtx);
        }
        else {
            throw makeError<UnimplError>("no object or param constrain when executing zfx");
        }
    }

    static zfxvariant anyToZfxVariant(Any const& var) {
        if (!var.has_value())
            return zfxvariant();
        if (get_type<bool>() == var.type()) {
            int res = any_cast<bool>(var);
            return res;
        }
        else if (get_type<int>() == var.type()) {
            return any_cast<int>(var);
        }
        else if (get_type<float>() == var.type()) {
            return any_cast<float>(var);
        }
        else if (get_type<std::string>() == var.type()) {
            return zeno::any_cast_to_string(var);
        }
        else if (get_type<const char*>() == var.type()) {
            return zeno::any_cast_to_string(var);
        }
        else if (get_type<zeno::vec2i>() == var.type()) {
            auto vec = any_cast<zeno::vec2i>(var);
            return glm::vec2{ vec[0], vec[1] };
        }
        else if (get_type<zeno::vec3i>() == var.type()) {
            auto vec = any_cast<zeno::vec3i>(var);
            return glm::vec3{ vec[0], vec[1], vec[2] };
        }
        else if (get_type<zeno::vec4i>() == var.type()) {
            auto vec = any_cast<zeno::vec4i>(var);
            return glm::vec4{ vec[0], vec[1], vec[2], vec[3] };
        }
        else if (get_type<zeno::vec2f>() == var.type()) {
            auto vec = any_cast<zeno::vec2f>(var);
            return glm::vec2{ vec[0], vec[1] };
        }
        else if (get_type<zeno::vec3f>() == var.type()) {
            auto vec = any_cast<zeno::vec3f>(var);
            return glm::vec3{ vec[0], vec[1], vec[2] };
        }
        else if (get_type<zeno::vec4f>() == var.type()) {
            auto vec = any_cast<zeno::vec4f>(var);
            return glm::vec4{ vec[0], vec[1], vec[2], vec[3] };
        }
        else if (get_type<zeno::PrimVar>() == var.type()) {
            zeno::PrimVar primvar = any_cast<zeno::PrimVar>(var);
            return std::visit([](zeno::PrimVar&& pvar) -> zfxvariant {
                using T = std::decay_t<decltype(pvar)>;
                if constexpr (std::is_same_v<int, T>) {
                    return std::get<int>(pvar);
                }
                else if constexpr (std::is_same_v<float, T>) {
                    return std::get<float>(pvar);
                }
                else if constexpr (std::is_same_v<std::string, T>) {
                    return std::get<std::string>(pvar);
                }
                else {
                    return zfxvariant();
                }
                }, primvar);
        }
        else {
            assert(false);
            return zfxvariant();
        }
    }

    template <class T>
    static T get_zfxvar(zfxvariant value) {
        return std::visit([](auto const& val) -> T {
            using V = std::decay_t<decltype(val)>;
            if constexpr (!std::is_constructible_v<T, V>) {
                if constexpr (std::is_same_v<T, glm::vec3> && std::is_same_v<V, zfxfloatarr>) {
                    return glm::vec3(val[0], val[1], val[2]);
                }
                throw makeError<TypeError>(typeid(T), typeid(V), "get<zfxvariant>");
            }
            else {
                return T(val);
            }
        }, value);
    }

    template<typename Operator>
    ZfxVariable calc_exp(const ZfxVariable& lhs, const ZfxVariable& rhs, const ZfxElemFilter& filter, Operator method) {

        int N1 = lhs.value.size();
        int N2 = rhs.value.size();
        int minsize = min(N1, N2);
        int maxsize = max(N1, N2);
        if (N1 != N2) {
            if (minsize != 1)
                throw makeError<UnimplError>("size invalidation on calc_exp");
        }

        ZfxVariable res;
        res.value.resize(maxsize);

        for (int i = 0; i < maxsize; i++)
        {
            if (!filter[i])
                continue;

            const zfxvariant& _lhs = N1 <= i ? lhs.value[0] : lhs.value[i];
            const zfxvariant& _rhs = N2 <= i ? rhs.value[0] : rhs.value[i];

            res.value[i] = std::visit([method](auto&& lval, auto&& rval)->zfxvariant {
                using T = std::decay_t<decltype(lval)>;
                using E = std::decay_t<decltype(rval)>;
                using Op = std::decay_t<decltype(method)>;

                if constexpr (std::is_same_v<Op, std::modulus<>>) {
                    if constexpr (std::is_same_v<T, int> && std::is_same_v<E, int>) {
                        return method((int)lval, (int)rval);
                    }
                    else if constexpr (std::is_same_v<T, int> && std::is_same_v<E, float>) {
                        return method(lval, (int)rval);
                    }
                    else if constexpr (std::is_same_v<T, float> && std::is_same_v<E, int>) {
                        return method((int)lval, rval);
                    }
                    else if constexpr (std::is_same_v<T, float> && std::is_same_v<E, float>) {
                        return method((int)lval, (int)rval);
                    }
                    throw makeError<UnimplError>("");
                } else if constexpr (std::is_same_v<T, int> && std::is_same_v<E, int>) {
                    return method(lval, rval);
                } else if constexpr (std::is_same_v<T, int> && std::is_same_v<E, float>) {
                    return method(E(lval), rval);
                } else if constexpr (std::is_same_v<T, float> && std::is_same_v<E, int>) {
                    return method(lval, T(rval));
                } else if constexpr (std::is_same_v<T, float> && std::is_same_v<E, float>) {
                    return method(lval, rval);
                } else if constexpr (std::is_same_v<T, glm::vec2> && std::is_same_v<T, E> ||
                    std::is_same_v<T, glm::vec3> && std::is_same_v<T, E> ||
                    std::is_same_v<T, glm::vec4> && std::is_same_v<T, E>)
                {
                    if constexpr (std::is_same_v<Op, std::less_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::less<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_or<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_and<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else
                    {
                        return method(lval, rval);
                    }
                } else if constexpr(std::is_same_v<T, glm::vec2> && (std::is_same_v<int, E> || std::is_same_v<float, E>) ||
                    std::is_same_v<T, glm::vec3> && (std::is_same_v<int, E> || std::is_same_v<float, E>) ||
                    std::is_same_v<T, glm::vec4> && (std::is_same_v<int, E> || std::is_same_v<float, E>))
                {
                    if constexpr (std::is_same_v<Op, std::less_equal<>>) {
                        throw makeError<UnimplError>("");
                    } else if constexpr (std::is_same_v<Op, std::less<>>) {
                        throw makeError<UnimplError>("");
                    } else if constexpr (std::is_same_v<Op, std::greater<>>) {
                        throw makeError<UnimplError>("");
                    } else if constexpr (std::is_same_v<Op, std::greater_equal<>>) {
                        throw makeError<UnimplError>("");
                    } else if constexpr (std::is_same_v<Op, std::logical_or<>>) {
                        throw makeError<UnimplError>("");
                    } else if constexpr (std::is_same_v<Op, std::logical_and<>>) {
                        throw makeError<UnimplError>("");
                    } else {
                        return method(lval, T(rval));
                    }
                }
                else if constexpr ((std::is_same_v<int, T> || std::is_same_v<float, T>) && std::is_same_v<E, glm::vec2> ||
                    (std::is_same_v<int, T> || std::is_same_v<float, T>) && std::is_same_v<E, glm::vec3> ||
                    (std::is_same_v<int, T> || std::is_same_v<float, T>) && std::is_same_v<E, glm::vec4>) {\
                    if constexpr (std::is_same_v<Op, std::less_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::less<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_or<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_and<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else {
                        return method(E(lval), rval);
                    }
                } else if constexpr (std::is_same_v<T, glm::mat3> && std::is_same_v<T, E> ||
                    std::is_same_v<T, glm::mat4> && std::is_same_v<T, E> ||
                    std::is_same_v<T, glm::mat2> && std::is_same_v<T, E>) {

                    if constexpr (std::is_same_v<Op, std::less_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::less<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::greater_equal<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::multiplies<>>)
                    {
                        //glm的实现里，乘法是顺序相反的，比如A*B, 其实是我们理解的B * A.
                        return method(rval, lval);
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_or<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else if constexpr (std::is_same_v<Op, std::logical_and<>>) {
                        throw makeError<UnimplError>("");
                    }
                    else {
                        return method(lval, rval);
                    }
                }
                else if constexpr ((std::is_same_v<Op, std::equal_to<>> || std::is_same_v<Op, std::not_equal_to<>>) &&
                    std::is_same_v<T, std::string> && std::is_same_v<T, E>) {
                    return method(lval, rval);
                }
                else{
                    throw makeError<UnimplError>("");
                }
            }, _lhs, _rhs);
        }
        return res;
    }

    void FunctionManager::testExp() {

        //glm::vec3 v1(1, 2, 3);
        //glm::vec3 v2(1, 3, 1);

        //zfxvariant vec1 = v1;
        //zfxvariant vec2 = v2;
        //zfxvariant vec3 = calc_exp(v1, v2, std::divides());

        //glm::mat3 mat1 = glm::mat3({ {1., 0, 2.}, {2., 1., -1.}, {0, 1., 1.} });
        //glm::mat3 mat2 = glm::mat3({ {1., 0, 0}, {0, -1., 1.}, {0, 0, -1.} });
        //glm::mat3 mat3 = glm::mat3({ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} });
        //glm::mat3 mat4 = glm::mat3({ {1, 0, 0}, {0, -1, 1}, {0, 0, -1} });
        //glm::mat3 mm = mat3 * mat4;
        //mm = mat1 * mat2;

        //zfxvariant bval = calc_exp(mat1, mat2, std::equal_to());
        //zfxvariant mmm = calc_exp(mat1, mat2, std::multiplies());

        //glm::mat3 mm2 = glm::dot(mat1, mat2);
    }

    static void set_array_element(ZfxVariable& zfxarr, ZfxVariable idxarr, const ZfxVariable& zfxvalue) {
        for (int i = 0; i < zfxarr.value.size(); i++) {
            auto& arr = zfxarr.value[i];
            auto& val = zfxvalue.value.size() == 1 ? zfxvalue.value[0] : zfxvalue.value[i];
            int idx = idxarr.value.size() == 1 ? get_zfxvar<int>(idxarr.value[0]) : get_zfxvar<int>(idxarr.value[i]);

            std::visit([idx](auto& arr, auto& value) {
                using T = std::decay_t<decltype(arr)>;
                using V = std::decay_t<decltype(value)>;
                //mat取一层索引可能就是vec...
                if constexpr ((std::is_same_v<T, zfxintarr> || std::is_same_v<T, zfxfloatarr>) &&
                    std::is_arithmetic_v<V>) {
                    arr[idx] = value;
                }
                else if constexpr ((std::is_same_v<T, glm::vec2> ||
                    std::is_same_v<T, glm::vec3> ||
                    std::is_same_v<T, glm::vec4>) && std::is_same_v<V, float>) {
                    arr[idx] = value;
                }
            }, arr, val);
        }
    }

    static ZfxVariable get_array_element(const ZfxVariable& arr, const ZfxVariable& varidx) {
        int idx = 0;
        int nIdx = varidx.value.size();
        if (nIdx == 1) {
            idx = get_zfxvar<int>(varidx.value[0]);
        }
        else {
            assert(arr.value.size() == nIdx);
        }

        ZfxVariable res;
#if 0
        if (false && arr.bArray) {
            //形如 int[] vec3[] float[] 这种
            for (int i = 0; i < nIdx; i++)
            {
                int idx = get_zfxvar<int>(varidx.value[i]);
                res.value.push_back(arr.value[idx]);
            }
        }
        else
#endif
        {
            for (int i = 0; i < arr.value.size(); i++) {
                if (varidx.value.size() == 1)
                    idx = get_zfxvar<int>(varidx.value[0]);
                else
                    idx = get_zfxvar<int>(varidx.value[i]);

                res.value.push_back(std::visit([idx](auto&& arg) -> zfxvariant {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, zfxintarr> ||
                        std::is_same_v<T, zfxfloatarr> ||
                        std::is_same_v<T, zfxstringarr> ||
                        std::is_same_v<T, zfxvec2arr> ||
                        std::is_same_v<T, zfxvec3arr> ||
                        std::is_same_v<T, zfxvec4arr> ||
                        std::is_same_v<T, glm::vec2> ||
                        std::is_same_v<T, glm::vec3> ||
                        std::is_same_v<T, glm::vec4> ||
                        std::is_same_v<T, glm::mat2> ||
                        std::is_same_v<T, glm::mat3> ||
                        std::is_same_v<T, glm::mat4>
                        ) {
                        return (T(arg))[idx];
                    }
                    else {
                        throw makeError<UnimplError>("get elemvar from arr");
                    }
                }, arr.value[i]));
            }
        }
        return res;
    }

    static ZfxVariable get_element_by_name(const ZfxVariable& arr, const std::string& name) {
        int idx = -1;
        if (name == "x") idx = 0;
        else if (name == "y") idx = 1;
        else if (name == "z") idx = 2;
        else if (name == "w") idx = 3;
        else
        {
            throw makeError<UnimplError>("Indexing Exceed");
        }
        ZfxVariable varidx;
        varidx.value.push_back(idx);
        return get_array_element(arr, varidx);
    }

    static void set_element_by_name(ZfxVariable& arr, const std::string& name, const ZfxVariable& value) {
        int idx = -1;
        if (name == "x") idx = 0;
        else if (name == "y") idx = 1;
        else if (name == "z") idx = 2;
        else if (name == "w") idx = 3;
        else throw makeError<UnimplError>("index error.");
        ZfxVariable varidx;
        varidx.value.push_back(idx);
        set_array_element(arr, varidx, value);
    }

    static void selfIncOrDec(ZfxVariable& var, bool bInc) {
        for (auto& val : var.value) {
            std::visit([bInc](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                    bInc ? (T)arg++ : (T)arg--;
                }
                else {
                    throw makeError<UnimplError>("Type Error");
                }
            }, val);
        }
    }

    std::vector<ZfxVariable> FunctionManager::process_args(std::shared_ptr<ZfxASTNode> parent, ZfxElemFilter& filter, ZfxContext* pContext) {
        std::vector<ZfxVariable> args;
        for (auto pChild : parent->children) {
            ZfxVariable argval = execute(pChild, filter, pContext);
            args.push_back(argval);
        }
        return args;
    }

    void FunctionManager::pushStack() {
        m_stacks.push_back(ZfxStackEnv());
    }

    void FunctionManager::popStack() {
        m_stacks.pop_back();
    }

    ZfxVariable& FunctionManager::getVariableRef(const std::string& name, ZfxContext* pContext) {
        assert(!name.empty());
        if (name.at(0) != '@'){
            for (auto iter = m_stacks.rbegin(); iter != m_stacks.rend(); iter++) {
                auto& stackvars = iter->table;
                auto iter_ = stackvars.find(name);
                if (iter_ != stackvars.end()) {
                    return stackvars.at(name);
                }
            }
        }
        throw makeError<KeyError>(name, "variable `" + name + "` not founded");
    }

    bool FunctionManager::declareVariable(const std::string& name) {
        if (m_stacks.empty()) {
            return false;
        }
        auto iterCurrentStack = m_stacks.rbegin();
        auto& vars = iterCurrentStack->table;
        if (vars.find(name) != vars.end()) {
            return false;
        }
        ZfxVariable variable;
        vars.insert(std::make_pair(name, variable));
        return true;
    }

    bool FunctionManager::assignVariable(const std::string& name, ZfxVariable var, ZfxContext* pContext) {
        if (m_stacks.empty()) {
            return false;
        }
        ZfxVariable& self = getVariableRef(name, pContext);
        self = var;
        return true;
    }

    void FunctionManager::validateVar(operatorVals vartype, ZfxVariable& newvars) {
        for (auto& newvar : newvars.value) {
            switch (vartype)
            {
            case TYPE_INT: {
                if (std::holds_alternative<float>(newvar)) {
                    newvar = (int)std::get<float>(newvar);
                }
                else if (std::holds_alternative<int>(newvar)) {

                }
                else {
                    throw makeError<UnimplError>("type dismatch TYPE_INT");
                }
                break;
            }
            case TYPE_INT_ARR: {
                if (std::holds_alternative<zfxfloatarr>(newvar)) {
                    zfxintarr intarr;
                    for (auto&& val : std::get<zfxfloatarr>(newvar))
                        intarr.push_back(val);
                    newvar = intarr;
                }
                else if (!std::holds_alternative<zfxintarr>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_INT_ARR");
                }
                break;
            }
            case TYPE_FLOAT: {
                if (std::holds_alternative<float>(newvar)) {

                }
                else if (std::holds_alternative<int>(newvar)) {
                    newvar = (float)std::get<int>(newvar);
                }
                else {
                    throw makeError<UnimplError>("type dismatch TYPE_FLOAT");
                }
                break;
            }
            case TYPE_FLOAT_ARR: {
                if (std::holds_alternative<zfxintarr>(newvar)) {
                    zfxfloatarr floatarr;
                    for (auto&& val : std::get<zfxintarr>(newvar))
                        floatarr.push_back(val);
                    newvar = floatarr;
                }
                else if (!std::holds_alternative<zfxfloatarr>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_FLOAT_ARR");
                }
                break;
            }
            case TYPE_STRING: {
                if (!std::holds_alternative<std::string>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_STRING");
                }
                break;
            }
            case TYPE_STRING_ARR: {
                if (!std::holds_alternative<zfxstringarr>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_STRING_ARR");
                }
                break;
            }
            case TYPE_VECTOR2: {
                if (std::holds_alternative<zfxfloatarr>(newvar)) {
                    zfxfloatarr arr = std::get<zfxfloatarr>(newvar);
                    if (arr.size() != 2) {
                        throw makeError<UnimplError>("num of elements of arr dismatch");
                    }
                    glm::vec2 vec = { arr[0], arr[1] };
                    newvar = vec;
                }
                else if (std::holds_alternative<glm::vec2>(newvar)) {

                }
                else {
                    throw makeError<UnimplError>("type dismatch TYPE_VECTOR2");
                }
                break;
            }
            case TYPE_VECTOR3: {
                if (std::holds_alternative<zfxfloatarr>(newvar)) {
                    zfxfloatarr arr = std::get<zfxfloatarr>(newvar);
                    if (arr.size() != 3) {
                        throw makeError<UnimplError>("num of elements of arr dismatch");
                    }
                    glm::vec3 vec = { arr[0], arr[1], arr[2] };
                    newvar = vec;
                }
                else if (std::holds_alternative<glm::vec3>(newvar)) {

                }
                else {
                    throw makeError<UnimplError>("type dismatch TYPE_VECTOR3");
                }
                break;
            }
            case TYPE_VECTOR4: {
                if (std::holds_alternative<zfxfloatarr>(newvar)) {
                    zfxfloatarr arr = std::get<zfxfloatarr>(newvar);
                    if (arr.size() != 4) {
                        throw makeError<UnimplError>("num of elements of arr dismatch");
                    }
                    glm::vec4 vec = { arr[0], arr[1], arr[2], arr[3] };
                    newvar = vec;
                }
                else if (std::holds_alternative<glm::vec4>(newvar)) {

                }
                else {
                    throw makeError<UnimplError>("type dismatch TYPE_VECTOR4");
                }
                break;
            }
            case TYPE_MATRIX2: {
                if (!std::holds_alternative<glm::mat2>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_MATRIX2");
                }
                break;
            }
            case TYPE_MATRIX3: {
                if (!std::holds_alternative<glm::mat3>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_MATRIX3");
                }
                break;
            }
            case TYPE_MATRIX4: {
                if (!std::holds_alternative<glm::mat4>(newvar)) {
                    throw makeError<UnimplError>("type dismatch TYPE_MATRIX4");
                }
                break;
            }
            }
        }
    }

    ZfxVariable FunctionManager::parseArray(std::shared_ptr<ZfxASTNode> pNode, ZfxElemFilter& filter, ZfxContext* pContext) {
        //目前暂时只考虑 {1,2,3}  {{x1, y1, z1}, {x2,y2,z2}}这种，如果再算上属性值扩展，比较麻烦，而且少见。
        std::vector<ZfxVariable> args = process_args(pNode, filter, pContext);
        //由于不清楚类型，所以先根据值类型构造一个zfxvariant返回出去，再由外部类型定义/赋值来处理
        //由于zfxvariant并没有多维数组，因此遇到多维数组，直接构造矩阵（如果构造失败，那就throw）
        if (args.empty()) {
            //直接返回默认的
            return ZfxVariable();
        }

        zfxvariant current;
        operatorVals dataType = UNDEFINE_OP;
        for (int idx = 0; idx < args.size(); idx++) {
            auto& arg = args[idx];
            if (std::holds_alternative<int>(arg.value[0])) {
                if (dataType == UNDEFINE_OP) {
                    dataType = TYPE_FLOAT_ARR;
                    current = zfxfloatarr();
                }
                if (dataType == TYPE_FLOAT_ARR) {
                    auto& arr = std::get<zfxfloatarr>(current);
                    arr.push_back(std::get<int>(arg.value[0]));
                }
                else {
                    throw makeError<UnimplError>("data type inconsistent");
                }
            }
            else if (std::holds_alternative<float>(arg.value[0])) {
                if (dataType != UNDEFINE_OP && dataType != TYPE_FLOAT_ARR) {
                    throw makeError<UnimplError>("data type inconsistent");
                }
                if (dataType == UNDEFINE_OP) {
                    dataType = TYPE_FLOAT_ARR;
                    current = zfxfloatarr();
                }
                if (dataType == TYPE_FLOAT_ARR) {
                    auto& arr = std::get<zfxfloatarr>(current);
                    arr.push_back(std::get<float>(arg.value[0]));
                }
            }
            else if (std::holds_alternative<std::string>(arg.value[0])) {
                if (dataType != UNDEFINE_OP && dataType != TYPE_STRING_ARR) {
                    throw makeError<UnimplError>("data type inconsistent");
                }
                if (dataType == UNDEFINE_OP) {
                    dataType = TYPE_STRING_ARR;
                    current = zfxstringarr();
                }
                if (dataType == TYPE_STRING_ARR) {
                    auto& arr = std::get<zfxstringarr>(current);
                    arr.push_back(std::get<std::string>(arg.value[0]));
                }
            }
            //不考虑intarr，因为glm的vector/matrix都是储存float
            else if (std::holds_alternative<zfxfloatarr>(arg.value[0])) {
                if (dataType != UNDEFINE_OP && dataType != TYPE_MATRIX2 && dataType != TYPE_MATRIX3 && dataType != TYPE_MATRIX4) {
                    throw makeError<UnimplError>("data type inconsistent");
                }

                auto& arr = std::get<zfxfloatarr>(arg.value[0]);

                if (dataType == UNDEFINE_OP) {
                    if (arr.size() == 2) {
                        dataType = TYPE_MATRIX2;
                        current = glm::mat2();
                    }
                    else if (arr.size() == 3) {
                        dataType = TYPE_MATRIX3;
                        current = glm::mat3();
                    }
                    else if (arr.size() == 4) {
                        dataType = TYPE_MATRIX4;
                        current = glm::mat4();
                    }
                }

                //{{0, 1}, {2, 3}}
                if (dataType == TYPE_MATRIX2 && arr.size() == 2 && idx < 2) {
                    auto& mat = std::get<glm::mat2>(current);
                    mat[idx][0] = arr[0];
                    mat[idx][1] = arr[1];
                }
                //{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}
                else if (dataType == TYPE_MATRIX3 && arr.size() == 3 && idx < 3) {
                    auto& mat = std::get<glm::mat3>(current);
                    mat[idx][0] = arr[0];
                    mat[idx][1] = arr[1];
                    mat[idx][2] = arr[2];
                }
                //{{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}}
                else if (dataType == TYPE_MATRIX4 && arr.size() == 4 && idx < 4) {
                    auto& mat = std::get<glm::mat4>(current);
                    mat[idx][0] = arr[0];
                    mat[idx][1] = arr[1];
                    mat[idx][2] = arr[2];
                    mat[idx][3] = arr[3];
                }
                else {
                    throw makeError<UnimplError>("mat element dims inconsistent");
                }
            }
            else {
                throw makeError<UnimplError>("data type inconsistent");
            }
        }

        ZfxVariable res;
        res.bArray = true;
        res.value.push_back(current);
        return res;
    }

    bool FunctionManager::hasTrue(const ZfxVariable& cond, const ZfxElemFilter& filter, ZfxElemFilter& ifFilter, ZfxElemFilter& elseFilter) const {
        int N = cond.value.size();
        assert(N == filter.size() || N == 1);
        ifFilter = filter;
        elseFilter = filter;
        bool bret = false;
        for (int i = 0; i < cond.value.size(); i++) {
            if (filter[i]) {
                if (get_zfxvar<int>(cond.value[i]) ||
                    get_zfxvar<float>(cond.value[i]))
                {
                    bret = true;
                    elseFilter[i] = 0;
                }
                else
                {
                    ifFilter[i] = 0;
                }
            }
        }
        return bret;
    }

    static void commitToObject(ZfxContext* pContext, const ZfxVariable& zfxvar, const std::string& attr_name, ZfxElemFilter& filter) {
        assert(!attr_name.empty());
        GeoAttrGroup grp = pContext->runover;
        auto& zfxvec = zfxvar.value;
        if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
            AttrVar wtf = zeno::zfx::convertToAttrVar(zfxvec);
            std::string attrname;
            if (attr_name[0] == '@')
                attrname = attr_name.substr(1);
            if (!spGeo->m_impl->has_attr(grp, attrname)) {
                spGeo->m_impl->create_attr(grp, attrname, wtf);
            }
            else {
                spGeo->m_impl->set_attr(grp, attrname, wtf);
            }
        }
        else {
            throw makeError<UnimplError>("only support Geometry when setting attributes");
        }
    }

    void FunctionManager::commitToPrim(const std::string& attrname, const ZfxVariable& val, ZfxElemFilter& filter, ZfxContext* pContext) {
        if (pContext->runover == ATTR_POINT) {
            if (attrname == "@P") {
                commitToObject(pContext, val, "pos", filter);
            }
            else if (attrname == "@ptnum") {
                throw makeError<UnimplError>("");
            }
            else if (attrname == "@N") {
                commitToObject(pContext, val, "nrm", filter);
            }
            else if (attrname == "@Cd") {
            }
            else {
                commitToObject(pContext, val, attrname, filter);
            }
        }
        else if (pContext->runover == ATTR_FACE) {
            
        }
        else if (pContext->runover == ATTR_GEO) {
            
        }
        else {
            
        }
    }

    static ZfxVariable getAttrValue_impl(std::shared_ptr<IObject> spObject, const std::string& attr_name) {
        if (auto spPrim = std::dynamic_pointer_cast<PrimitiveObject>(spObject)) {
            if (attr_name == "pos")
            {
                const auto& P = spPrim->attr<vec3f>("pos");
                ZfxVariable res;
                res.bAttr = true;
                for (auto pos : P) {
                    res.value.push_back(glm::vec3(pos[0], pos[1], pos[2]));
                }
                return res;
            }
            if (attr_name == "ptnum")
            {
                int N = spPrim->verts->size();
                ZfxVariable res;
                res.value.resize(N);
                res.bAttr = true;
                for (int i = 0; i < N; i++)
                    res.value[i] = i;
                return res;
            }
            if (attr_name == "nrm")
            {
                if (spPrim->has_attr("nrm")) {
                    const auto& nrms = spPrim->attr<vec3f>("nrm");
                    ZfxVariable res;
                    res.bAttr = true;
                    for (auto nrm : nrms) {
                        res.value.push_back(glm::vec3(nrm[0], nrm[1], nrm[2]));
                    }
                    return res;
                }
                else {
                    throw makeError<UnimplError>("the prim has no attr about normal, you can check whether the option `hasNormal` is on");
                }
            }
        }
        else if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(spObject)) {
            if (attr_name == "pos")
            {
                const auto& P = spGeo->points_pos();
                ZfxVariable res;
                res.bAttr = true;
                for (auto pos : P) {
                    res.value.push_back(glm::vec3(pos[0], pos[1], pos[2]));
                }
                return res;
            }
            if (attr_name == "ptnum")
            {
                int N = spGeo->npoints();
                ZfxVariable res;
                res.value.resize(N);
                res.bAttr = true;
                for (int i = 0; i < N; i++)
                    res.value[i] = i;
                return res;
            }
            if (attr_name == "nrm")
            {
                //if (spGeo->has_attr(ATTR_POINT, "nrm")) {
                //    ZfxVariable res;
                //    res.bAttr = true;
                //    res.value = spGeo->get_attr_byzfx(ATTR_POINT, "nrm");
                //    return res;
                //}
                //else {
                //    throw makeError<UnimplError>("the prim has no attr about normal, you can check whether the option `hasNormal` is on");
                //}
            }
        }
        else {
            return ZfxVariable();
        }
    }

    void FunctionManager::setAttrValue(const std::string& attrname, const std::string& channel, const ZfxVariable& var, operatorVals opVal, ZfxElemFilter& filter, ZfxContext* pContext) {
        zeno::zfx::setAttrValue(attrname, channel, var, opVal, filter, pContext);
    }

    ZfxVariable FunctionManager::getAttrValue(const std::string& attrname, ZfxContext* pContext, char channel) {
        return zeno::zfx::getAttrValue(attrname, pContext, channel);
    }

    ZfxVariable FunctionManager::trunkVariable(ZfxVariable origin, const ZfxElemFilter& filter) {
        int ndim = origin.value.size();
        int nfilter = filter.size();
        if (nfilter == ndim)
            return origin;
        else if (nfilter < ndim) {
            //裁剪origin
            ZfxVariable truncate;
            truncate.value.resize(nfilter);
            std::copy(origin.value.begin(), origin.value.begin() + nfilter, truncate.value.begin());
            return truncate;
        }
        else {
            //扩大origin
            origin.value.resize(nfilter);
            return origin;
        }
    }

    ZfxVariable FunctionManager::execute(std::shared_ptr<ZfxASTNode> root, ZfxElemFilter& filter, ZfxContext* pContext) {
        if (!root) {
            throw makeError<UnimplError>("Indexing Error.");
        }
        switch (root->type)
        {
            case NUMBER:
            case STRING:
            case ATTR_VAR:
            case BOOLTYPE: {
                ZfxVariable var;
                var.value.push_back(root->value);
                return var;
            }
            case ZENVAR: {
                //这里指的是取zenvar的值用于上层的计算或者输出，赋值并不会走到这里
                const std::string& varname = get_zfxvar<std::string>(root->value);
#if 1
                if (varname == "pos") {
                    int j;
                    j = 0;
                }
#endif
                if (root->bAttr && root->opVal == COMPVISIT) {
                    if (root->children.size() != 1) {
                        throw makeError<UnimplError>("Indexing Error on NameVisit");
                    }
                    std::string component = get_zfxvar<std::string>(root->children[0]->value);
                    char channel = 0;
                    if (component == "x")
                        channel = 'x';
                    else if (component == "y")
                        channel = 'y';
                    else if (component == "z")
                        channel = 'z';
                    else if (component == "w")
                        channel = 'w';
                    ZfxVariable var = getAttrValue(varname, pContext, channel);
                    return trunkVariable(var, filter);
                }
                else if (root->opVal == Indexing) {

                }

                if (root->bAttr && root->opVal == UNDEFINE_OP) {
                    ZfxVariable var = getAttrValue(varname, pContext);
                    return trunkVariable(var, filter);
                }

                ZfxVariable& var = getVariableRef(varname, pContext);

                switch (root->opVal) {
                case Indexing: {
                    if (root->children.size() != 1) {
                        throw makeError<UnimplError>("Indexing Error.");
                    }
                    const ZfxVariable& idx = execute(root->children[0], filter, pContext);
                    ZfxVariable elemvar = get_array_element(var, idx);
                    return elemvar;
                }
                case BulitInVar: {
                    std::string attrname = get_zfxvar<std::string>(root->value);
                    if (attrname.size() < 2 || attrname[0] != '$') {
                        throw makeError<UnimplError>("build in var");
                    }
                    attrname = attrname.substr(1);
                    if (attrname == "F") {
                        //TODO
                    }
                    else if (attrname == "T") {
                        //TODO
                    }
                    else if (attrname == "FPS") {
                        //TODO
                    }
                }
                case AutoIncreaseFirst: {
                    selfIncOrDec(var, true);
                    if (root->bAttr)
                        var.bAttrUpdated = true;
                    return var;
                }
                case AutoDecreaseFirst: {
                    selfIncOrDec(var, false);
                    if (root->bAttr)
                        var.bAttrUpdated = true;
                    return var;
                }
                case AutoIncreaseLast:
                case AutoDecreaseLast:   //在外面再自增/减
                {
                    selfIncOrDec(var, AutoIncreaseLast == root->opVal);
                    if (root->bAttr)
                        var.bAttrUpdated = true;
                    return var;
                }
                default: {
                    return var;
                }
                }
            }
            case ASSIGNMENT: {
                //赋值
                if (root->children.size() != 2) {
                    throw makeError<UnimplError>("assign variable failed.");
                }

                std::shared_ptr<ZfxASTNode> valNode = root->children[1];
                ZfxVariable res = execute(valNode, filter, pContext);

                std::shared_ptr<ZfxASTNode> zenvarNode = root->children[0];
                if (zenvarNode->bAttr) {
                    //无须把值拎出来再计算，直接往属性数据内部设置
                    const std::string& attrname = get_zfxvar<std::string>(zenvarNode->value).substr(1);
                    //DEBUG:
#if 0
                    if (attrname == "type") {
                        int j;
                        j = 0;
                    }
#endif

                    std::string channel;
                    if (zenvarNode->opVal == COMPVISIT) {
                        assert(zenvarNode->children.size() == 1);
                        channel = get_zfxvar<std::string>(zenvarNode->children[0]->value);
                    }
                    else if (zenvarNode->opVal == Indexing) {
                        //todo
                        throw makeError<UnimplError>("Not support indexing for internal attributes");
                    }

                    AttrVar initValue = getInitValueFromVariant(res.value); //拿初值就行
                    auto spGeom = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                    if (pContext->runover == ATTR_POINT) {
                        if (!spGeom->m_impl->has_point_attr(attrname)) {
                            spGeom->m_impl->create_point_attr(attrname, initValue);
                        }
                    }
                    else if (pContext->runover == ATTR_VERTEX) {
                        if (!spGeom->m_impl->has_vertex_attr(attrname)) {
                            spGeom->m_impl->create_vertex_attr(attrname, initValue);
                        }
                    }
                    else if (pContext->runover == ATTR_FACE) {
                        if (!spGeom->m_impl->has_face_attr(attrname)) {
                            spGeom->m_impl->create_face_attr(attrname, initValue);
                        }
                    }
                    else if (pContext->runover == ATTR_GEO) {
                        if (!spGeom->m_impl->has_geometry_attr(attrname)) {
                            spGeom->m_impl->create_geometry_attr(attrname, initValue);
                        }
                    }
                    setAttrValue(attrname, channel, res, root->opVal, filter, pContext);
                    return ZfxVariable();
                }

                const std::string& targetvar = get_zfxvar<std::string>(zenvarNode->value);

                if (root->opVal == AddAssign) {
                    ZfxVariable varres = execute(zenvarNode, filter, pContext);
                    res = calc_exp(varres, res, filter, std::plus());
                }
                else if (root->opVal == MulAssign) {
                    ZfxVariable varres = execute(zenvarNode, filter, pContext);
                    res = calc_exp(varres, res, filter, std::multiplies());
                }
                else if (root->opVal == SubAssign) {
                    ZfxVariable varres = execute(zenvarNode, filter, pContext);
                    res = calc_exp(varres, res, filter, std::minus());
                }
                else if (root->opVal == DivAssign) {
                    ZfxVariable varres = execute(zenvarNode, filter, pContext);
                    res = calc_exp(varres, res, filter, std::divides());
                }

                {
                    //能赋值的变量只有：1.普通zfx定义的变量    2.参数约束的参数变量
                    std::string nodeparam = pContext->param_constrain.constrain_param;
                    if (!nodeparam.empty()) {
                        auto spNode = pContext->spNode;
                        bool bInputParam = pContext->param_constrain.bInput;
                        bool bVal = get_zfxvar<int>(res.value[0]);
                        if (targetvar == "visible") {
                            pContext->param_constrain.update_nodeparam_prop = spNode->update_param_visible(nodeparam, bVal, bInputParam);
                        }
                        else if (targetvar == "enabled") {
                            pContext->param_constrain.update_nodeparam_prop = spNode->update_param_enable(nodeparam, bVal, bInputParam);
                        }
                        return ZfxVariable();
                    }

                    //直接解析变量
                    ZfxVariable& var = getVariableRef(targetvar, pContext);

                    if (root->bAttr)
                        var.bAttrUpdated = true;

                    //先赋值结束后直接提交到prim上面（可否在结束的时候提交？好像也行，还能统一操作）
                    scope_exit sp([&]() {
                        if (zenvarNode->bAttr) {
                            commitToPrim(targetvar, var, filter, pContext);
                        }
                    });

                    switch (zenvarNode->opVal) {
                    case Indexing: {
                        if (zenvarNode->children.size() != 1) {
                            throw makeError<UnimplError>("Indexing Error.");
                        }
                        const ZfxVariable& idx = execute(zenvarNode->children[0], filter, pContext);
                        set_array_element(var, idx, res);
                        return ZfxVariable();  //无需返回什么具体的值
                    }
                    case COMPVISIT: {
                        if (zenvarNode->children.size() != 1) {
                            throw makeError<UnimplError>("Indexing Error on NameVisit");
                        }
                        std::string component = get_zfxvar<std::string>(zenvarNode->children[0]->value);
                        set_element_by_name(var, component, res);
                        return ZfxVariable();
                    }
                    case BulitInVar: {
                        //TODO: 什么情况下需要修改这种变量
                        //$F $T这些貌似不能通过脚本去改，houdini也是这样，不知道有没有例外
                        throw makeError<UnimplError>("Read-only variable cannot be modified.");
                    }
                    case AutoDecreaseFirst:
                    case AutoIncreaseFirst:
                    case AutoIncreaseLast:
                    case AutoDecreaseLast:
                    default: {
                        //先自增/减,再赋值，似乎没有意义，所以忽略
                        if (zenvarNode->bAttr) {
                            //属性的赋值不能修改其原来的维度。
                            //assert(res.value.size() <= var.value.size());
                            if (res.value.size() < var.value.size()) {
                                //如果右边的值的容器大小比当前赋值属性要小，很可能是单值，先只考虑这种情况。
                                assert(res.value.size() == 1);
                                std::fill(var.value.begin(), var.value.end(), res.value[0]);
                            }
                            else {
                                var = std::move(res);
                            }
                            return ZfxVariable();
                        }
                        else {
                            var = std::move(res);
                            return ZfxVariable();
                        }
                    }
                    }
                }
                break;
            }
            case DECLARE: {
                //变量定义
                int nChildren = root->children.size();
                if (nChildren != 2 && nChildren != 3) {
                    throw makeError<UnimplError>("args of DECLARE");
                }
                std::shared_ptr<ZfxASTNode> typeNode = root->children[0];
                std::shared_ptr<ZfxASTNode> nameNode = root->children[1];
                ZfxVariable newvar;
                bool bOnlyDeclare = nChildren == 2;
                operatorVals vartype = typeNode->opVal;
                if (bOnlyDeclare) {
                    switch (vartype)
                    {
                    case TYPE_INT: {
                        newvar.value.push_back(0);
                        break;
                    }
                    case TYPE_INT_ARR: {
                        newvar.value.push_back(zfxintarr());
                        break;
                    }
                    case TYPE_FLOAT: {
                        newvar.value.push_back(0.f);
                        break;
                    }
                    case TYPE_FLOAT_ARR: {
                        newvar.value.push_back(zfxfloatarr());
                        break;
                    }
                    case TYPE_STRING: {
                        newvar.value.push_back("");
                        break;
                    }
                    case TYPE_STRING_ARR: {
                        newvar.value.push_back(zfxstringarr());
                        break;
                    }
                    case TYPE_VECTOR2: {
                        newvar.value.push_back(glm::vec2());
                        break;
                    }
                    case TYPE_VECTOR2_ARR: {
                        newvar.value.push_back(zfxvec2arr());
                        break;
                    }
                    case TYPE_VECTOR3: {
                        newvar.value.push_back(glm::vec3());
                        break;
                    }
                    case TYPE_VECTOR3_ARR: {
                        newvar.value.push_back(zfxvec3arr());
                        break;
                    }
                    case TYPE_VECTOR4:  newvar.value.push_back(glm::vec4()); break;
                    case TYPE_VECTOR4_ARR: {
                        newvar.value.push_back(zfxvec4arr());
                        break;
                    }
                    case TYPE_MATRIX2:  newvar.value.push_back(glm::mat2()); break;
                    case TYPE_MATRIX3:  newvar.value.push_back(glm::mat3()); break;
                    case TYPE_MATRIX4:  newvar.value.push_back(glm::mat4()); break;
                    }
                }
                else {
                    std::shared_ptr<ZfxASTNode> valueNode = root->children[2];
                    newvar = execute(valueNode, filter, pContext);
                }

                std::string varname = get_zfxvar<std::string>(nameNode->value);
                //暂时不考虑自定义结构体
                bool bret = declareVariable(varname);
                if (!bret) {
                    throw makeError<UnimplError>("assign variable failed.");
                }

                validateVar(vartype, newvar);

                bret = assignVariable(varname, newvar, pContext);
                if (!bret) {
                    throw makeError<UnimplError>("assign variable failed.");
                }
                break;
            }
            case FUNC: {
                //函数
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                const std::string& funcname = get_zfxvar<std::string>(root->value);
                ZfxVariable result = eval(funcname, args, filter, pContext);
                return result;
            }
            case UNARY_EXP: {
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                if (args.size() != 1) {
                    throw makeError<UnimplError>("num args of unary op should be 1");
                }
                const ZfxVariable& arg = args[0];
                const int N = arg.value.size();
                ZfxVariable result;
                result.value.resize(N);
                switch (root->opVal) {
                case NOT: {
                    for (int i = 0; i < N; i++)
                    {
                        result.value[i] = std::visit([&](auto&& argval)->zfxvariant {
                            using T = std::decay_t<decltype(argval)>;
                            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                                return (int)(argval <= 0);
                            }
                            else if constexpr (std::is_same_v<T, std::string>) {
                                return !argval.empty();
                            }
                            else {
                                //TODO: vectype
                                throw makeError<UnimplError>("not support of nor operator for other types.");
                            }
                        }, arg.value[i]);
                    }
                    break;
                }
                default: {
                    throw makeError<UnimplError>("unknown unary op.");
                }
                }
                return result;
            }
            case FOUROPERATIONS: {
                //四则运算+ - * / %
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                if (args.size() != 2) {
                    throw makeError<UnimplError>("op args");
                }
                switch (root->opVal) {
                case PLUS:  return calc_exp(args[0], args[1], filter, std::plus());
                case MINUS: return calc_exp(args[0], args[1], filter, std::minus());
                case MUL:   return calc_exp(args[0], args[1], filter, std::multiplies());
                case DIV:   return calc_exp(args[0], args[1], filter, std::divides());
                case MOD:   return calc_exp(args[0], args[1], filter, std::modulus());
                case AND:   return calc_exp(args[0], args[1], filter, std::logical_and());
                case OR:    return calc_exp(args[0], args[1], filter, std::logical_or());
                default:
                    throw makeError<UnimplError>("op error");
                }
            }
            case ATTR_VISIT: {
                if (root->children.size() != 2) {
                    throw makeError<UnimplError>("op args at attr visit");
                }
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                std::string visit_attr = get_zfxvar<std::string>(args[1].value[0]);

                int nVarSize = args[0].value.size();

                ZfxVariable res = std::visit([&](auto&& arg) -> ZfxVariable {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, ZfxLValue>) {
                        return std::visit([&](auto&& nodeparam) -> zfxvariant {
                            using E = std::decay_t<decltype(nodeparam)>;
                            if constexpr (std::is_same_v<E, ParamPrimitive>) {
                                if (visit_attr == "value") {
                                    return anyToZfxVariant(nodeparam.defl);
                                }
                                else if (visit_attr == "connected") {
                                    return !nodeparam.links.empty();
                                }
                                else if (visit_attr == "x") {
                                    //TODO
                                }
                                else if (visit_attr == "y") {

                                }
                                else if (visit_attr == "z") {

                                }
                                else if (visit_attr == "w") {

                                }
                                else {
                                    //unknown attr
                                    throw makeError<UnimplError>("unknown attr when visit nodeparam");
                                }
                            }
                            else if constexpr (std::is_same_v<E, ParamObject>) {
                                if (visit_attr == "connected") {
                                    return !nodeparam.links.empty();
                                }
                                else {
                                    throw makeError<UnimplError>("unknown attr when visit nodeparam");
                                }
                            }
                        }, arg.var);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec2>) {
                        if (visit_attr == "x") {
                            return arg[0];
                        }
                        else if (visit_attr == "y") {
                            return arg[1];
                        }
                        else {
                            throw makeError<UnimplError>("unknown comp in vec2");
                        }
                    }
                    else if constexpr (std::is_same_v<T, glm::vec3>) {
                        if (visit_attr == "x") {
                            return arg[0];
                        }
                        else if (visit_attr == "y") {
                            if (nVarSize > 1) {
                                ZfxVariable ret;
                                ret.value.resize(nVarSize);
                                for (int i = 0; i < nVarSize; i++) {
                                    auto& v = get_zfxvar<glm::vec3>(args[0].value[i]);
                                    ret.value[i] = v[1];
                                }
                                return ret;
                            }
                            else {
                                return arg[1];
                            }
                        }
                        else if (visit_attr == "z") {
                            return arg[2];
                        }
                        else {
                            throw makeError<UnimplError>("unknown comp in vec3");
                        }
                    }
                    else if constexpr (std::is_same_v<T, glm::vec4>) {
                        if (visit_attr == "x") {
                            return arg[0];
                        }
                        else if (visit_attr == "y") {
                            return arg[1];
                        }
                        else if (visit_attr == "z") {
                            return arg[2];
                        }
                        else if (visit_attr == "w") {
                            return arg[3];
                        }
                        else {
                            throw makeError<UnimplError>("unknown comp in vec4");
                        }
                    }
                    else {
                        throw makeError<UnimplError>("only support visit attr for ZfxLvalue");
                    }
                }, args[0].value[0]);

                return res;
            }
            case COMPOP: {
                //操作符
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                if (args.size() != 2) {
                    throw makeError<UnimplError>("compare op args");
                }
                switch (root->opVal) {
                case Less:          return calc_exp(args[0], args[1], filter, std::less());
                case LessEqual:     return calc_exp(args[0], args[1], filter, std::less_equal());
                case Greater:       return calc_exp(args[0], args[1], filter, std::greater());
                case GreaterEqual:  return calc_exp(args[0], args[1], filter, std::greater_equal());
                case Equal:         return calc_exp(args[0], args[1], filter, std::equal_to());
                case NotEqual:      return calc_exp(args[0], args[1], filter, std::not_equal_to());
                default:
                    throw makeError<UnimplError>("compare op error");
                }
            }
            case ARRAY:{
                return parseArray(root, filter, pContext);
            }
            case PLACEHOLDER:{
                return ZfxVariable();
            }
            case CONDEXP: {
                //条件表达式
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                if (args.size() != 3) {
                    throw makeError<UnimplError>("cond exp args");
                }
                auto& pCond = args[0];

                ZfxElemFilter newFilter, elseFilter;
                if (hasTrue(pCond, filter, newFilter, elseFilter)) {
                    auto pCodesExp = root->children[1];
                    return execute(pCodesExp, newFilter, pContext);
                }
                else {
                    auto pCodesExp = root->children[2];
                    return execute(pCodesExp, newFilter, pContext);
                }
            }
            case IF:{
                if (root->children.size() != 2 && root->children.size() != 3) {
                    throw makeError<UnimplError>("if cond failed.");
                }
                auto pCondExp = root->children[0];
                //todo: self inc
                const ZfxVariable& cond = execute(pCondExp, filter, pContext);
                if (cond.value.size() == 1) {//不是向量的情况
                    ZfxElemFilter newFilter, elseFilter;
                    if (hasTrue(cond, filter, newFilter, elseFilter)) {
                        auto pCodesExp = root->children[1];
                        execute(pCodesExp, newFilter, pContext);
                    } else if (root->children.size() == 3) {
                        auto pelseExp = root->children[2];
                        execute(pelseExp, filter, pContext);
                    }
                } else {//向量的情况，每个分支都要执行
                    ZfxElemFilter ifFilter, elseFilter;
                    if (hasTrue(cond, filter, ifFilter, elseFilter)) {
                        auto pCodesExp = root->children[1];
                        execute(pCodesExp, ifFilter, pContext);
                    }
                    if (root->children.size() == 3) {
                        auto pelseExp = root->children[2];
                        execute(pelseExp, elseFilter, pContext);
                    }
                }

                break;
            }
            case FOR:{
                if (root->children.size() != 4) {
                    throw makeError<UnimplError>("for failed.");
                }
                auto forBegin = root->children[0];
                auto forCond = root->children[1];
                auto forStep = root->children[2];
                auto loopContent = root->children[3];

                //压栈
                pushStack();
                scope_exit sp([this]() {this->popStack(); });

                //查看begin语句里是否有定义语句，赋值语句
                switch (forBegin->type)
                {
                case DECLARE:
                {
                    execute(forBegin, filter, pContext);
                    break;
                }
                case ASSIGNMENT:
                {
                    execute(forBegin, filter, pContext);
                    break;
                }
                case PLACEHOLDER:
                default:
                    //填其他东西没有意义，甚至可能导致解析出错。
                    execute(forBegin, filter, pContext);
                    break;
                }

                ZfxVariable cond = execute(forCond, filter, pContext);
                ZfxElemFilter ifFilter, elseFilter;
                while (hasTrue(cond, filter, ifFilter, elseFilter)) {
                    //TODO: check the passed element and mark in the newFilter.
                    execute(loopContent, ifFilter, pContext);     //CodeBlock里面可能会多压栈一次，没关系，变量都是看得到的

                    if (pContext->jumpFlag == JUMP_BREAK)
                        break;
                    if (pContext->jumpFlag == JUMP_CONTINUE)
                        continue;
                    if (pContext->jumpFlag == JUMP_RETURN)
                        return ZfxVariable();

                    execute(forStep, ifFilter, pContext);
                    cond = execute(forCond, ifFilter, pContext);
                }
                break;
            }
            case FOREACH:{
                //对应文法：FOREACH LPAREN foreach-step COLON zenvar RPAREN code-block
                //foreach-step 可能有一个或两个子成员，并返回vec<ASTNode>
                int nChild = root->children.size();
                if (nChild == 3 || nChild == 4) {
                    //没有索引
                    auto idxNode = nChild == 3 ? nullptr : root->children[0];
                    auto varNode = nChild == 3 ? root->children[0] : root->children[1];
                    auto arrNode = nChild == 3 ? root->children[1] : root->children[2];
                    auto codeSeg = nChild == 3 ? root->children[2] : root->children[3];
                    if (!varNode || !arrNode || !codeSeg)
                        throw makeError<UnimplError>("elements on foreach error.");

                    const ZfxVariable& arr = execute(arrNode, filter, pContext);

                    //压栈
                    pushStack();
                    scope_exit sp([this]() {this->popStack(); });

                    //定义idxNode所指向的名称为值 i
                    //定义varNode所指向的名称为值 arrtest[i]
                    std::string idxName;
                    if (idxNode) {
                        idxName = get_zfxvar<std::string>(idxNode->value);
                        declareVariable(idxName);
                    }
                    const std::string& varName = get_zfxvar<std::string>(varNode->value);
                    declareVariable(varName);

                    for (auto eacharr : arr.value)
                    {
                        std::visit([&](auto&& val) {
                            using T = std::decay_t<decltype(val)>;
                            if constexpr (std::is_same_v<T, zfxintarr> ||
                                std::is_same_v<T, zfxfloatarr> ||
                                std::is_same_v<T, zfxstringarr>) {

                                for (int i = 0; i < val.size(); i++) {
                                    //修改变量和索引的值为i, arrtest[i];
                                    if (idxNode) {
                                        ZfxVariable zfxvar;
                                        zfxvar.value.push_back(i);
                                        assignVariable(idxName, zfxvar, pContext);
                                    }

                                    ZfxVariable zfxvar;
                                    zfxvar.value.push_back(val[i]);
                                    assignVariable(varName, zfxvar, pContext);

                                    //修改定义后，再次运行code
                                    execute(codeSeg, filter, pContext);

                                    //检查是否有跳转, continue在execute内部已经跳出了，这里不需要处理
                                    if (pContext->jumpFlag == JUMP_BREAK ||
                                        pContext->jumpFlag == JUMP_RETURN) {
                                        return;
                                    }
                                }
                            }
                            else if constexpr (std::is_same_v<T, glm::vec2> ||
                                std::is_same_v<T, glm::vec3> ||
                                std::is_same_v<T, glm::vec4>) {

                                for (int i = 0; i < val.length(); i++) {
                                    //修改变量和索引的值为i, arrtest[i];
                                    if (idxNode) {
                                        ZfxVariable zfxvar;
                                        zfxvar.value.push_back(i);
                                        assignVariable(idxName, zfxvar, pContext);
                                    }

                                    ZfxVariable zfxvar;
                                    zfxvar.value.push_back(val[i]);
                                    assignVariable(varName, zfxvar, pContext);

                                    //修改定义后，再次运行code
                                    execute(codeSeg, filter, pContext);

                                    //检查是否有跳转, continue在execute内部已经跳出了，这里不需要处理
                                    if (pContext->jumpFlag == JUMP_BREAK ||
                                        pContext->jumpFlag == JUMP_RETURN) {
                                        return;
                                    }
                                }
                            }
                            else {
                                throw makeError<UnimplError>("foreach error: no array type");
                            }
                        }, eacharr);
                    }
                    return ZfxVariable();
                }
                else {
                    throw makeError<UnimplError>("foreach error.");
                }
                break;
            }
            case WHILE:{
                if (root->children.size() != 2) {
                    throw makeError<UnimplError>("while failed.");
                }
   
                auto forCond = root->children[0];
                auto loopContent = root->children[1];

                //压栈
                pushStack();
                scope_exit sp([this]() {this->popStack(); });

                auto cond = execute(forCond, filter, pContext);
                ZfxElemFilter newFilter, elseFilter;
                while (hasTrue(cond, filter, newFilter, elseFilter)) {
                    execute(loopContent, newFilter, pContext);     //CodeBlock里面可能会多压栈一次，没关系，变量都是看得到的

                    if (pContext->jumpFlag == JUMP_BREAK)
                        break;
                    if (pContext->jumpFlag == JUMP_CONTINUE)
                        continue;
                    if (pContext->jumpFlag == JUMP_RETURN)
                        return ZfxVariable();

                    cond = execute(forCond, newFilter, pContext);
                }
                break;
            }
            case DOWHILE:{
                if (root->children.size() != 2) {
                    throw makeError<UnimplError>("while failed.");
                }

                auto forCond = root->children[1];
                auto loopContent = root->children[0];

                //??
                pushStack();
                scope_exit sp([this]() {this->popStack(); });
                ZfxVariable cond;

                ZfxElemFilter newFilter = filter;
                ZfxElemFilter newFilter2, elsefilter;

                do {
                    newFilter = newFilter2;
                    execute(loopContent, newFilter, pContext);     //CodeBlock里面可能会多压栈一次，没关系，变量都是看得到的

                    if (pContext->jumpFlag == JUMP_BREAK)
                        break;
                    if (pContext->jumpFlag == JUMP_CONTINUE)
                        continue;
                    if (pContext->jumpFlag == JUMP_RETURN)
                        return ZfxVariable();
                    cond = execute(forCond, newFilter, pContext);
                } while (hasTrue(cond, newFilter, newFilter2, elsefilter));

                break;
            }
            case CODEBLOCK:{
                pushStack();
                scope_exit sp([this]() {this->popStack(); });
                for (auto pSegment : root->children) {
                    execute(pSegment, filter, pContext);
                    if (pContext->jumpFlag == JUMP_BREAK ||
                        pContext->jumpFlag == JUMP_CONTINUE ||
                        pContext->jumpFlag == JUMP_RETURN) {
                        return ZfxVariable();
                    }
                }
                break;
            }
            case JUMP:{
                pContext->jumpFlag = root->opVal;
                break;
            }
            case VARIABLETYPE:{
                //变量类型，比如int vector3 float string等
            }
            default: {
                break;
            }
        }
        return ZfxVariable();
    }

    std::set<std::pair<std::string, std::string>>
        FunctionManager::getReferSources(std::shared_ptr<ZfxASTNode> root, ZfxContext* pContext)
    {
        if (!root) {
            return {};
        }

        std::set<std::pair<std::string, std::string>> paths;
        if (nodeType::FUNC != root->type)
        {
            for (auto _childNode : root->children)
            {
                std::set<std::pair<std::string, std::string>> _paths = getReferSources(_childNode, pContext);
                if (!_paths.empty()) {
                    paths.insert(_paths.begin(), _paths.end());
                }
            }
        }
        else
        {
            const std::string& funcname = std::get<std::string>(root->value);
            if (funcname == "ref") {
                if (root->children.size() != 1)
                    throw makeError<UnimplError>();
                const zeno::zfxvariant& res = calc(root->children[0], pContext);
                const std::string ref = std::holds_alternative<std::string>(res) ? std::get<std::string>(res) : "";
                //收集ref信息源，包括源节点和参数
                std::string paramname, _;
                auto spNode = zfx::getNodeAndParamFromRefString(ref, pContext, paramname, _);
                if (spNode)
                    paths.insert(std::make_pair(spNode->get_uuid_path(), paramname));
            }
            else {
                //函数参数也可能调用引用：
                for (auto paramNode : root->children)
                {
                    std::set<std::pair<std::string, std::string>> _paths;
                    if (paramNode->type ==  nodeType::STRING) {
                        const zeno::zfxvariant& res = calc(paramNode, pContext);
                        const std::string ref = std::holds_alternative<std::string>(res) ? std::get<std::string>(res) : "";
                        if (std::regex_search(ref, refPattern)) {
                            std::string paramname, _;
                            auto spNode = zfx::getNodeAndParamFromRefString(ref, pContext, paramname, _);
                            if (spNode)
                                paths.insert(std::make_pair(spNode->get_uuid_path(), paramname));
                        }
                    } else {
                        _paths = getReferSources(paramNode, pContext);
                    }
                    if (!_paths.empty()) {
                        paths.insert(_paths.begin(), _paths.end());
                    }
                }
            }
        }
        return paths;
    }

    zfxvariant FunctionManager::calc(std::shared_ptr<ZfxASTNode> root, ZfxContext* pContext) {
        if (root)
        {
            switch (root->type)
            {
            case nodeType::NUMBER:
            case nodeType::STRING: return root->value;
            case nodeType::ZENVAR:
            {
                const std::string& var = std::get<std::string>(root->value);
                if (var == "F") {
                    return (float)zeno::getSession().globalState->getFrameId();
                }
                else if (var == "FPS") {
                    //TODO
                    return zfxvariant();
                }
                else if (var == "T") {
                    //TODO
                    return zfxvariant();
                }
            }
            case nodeType::FOUROPERATIONS:
            {
                if (root->children.size() != 2)
                {
                    throw makeError<UnimplError>();
                }
                zfxvariant lhs = calc(root->children[0], pContext);
                zfxvariant rhs = calc(root->children[1], pContext);

                const std::string& var = std::get<std::string>(root->value);
                if (var == "+") {
                    //TODO: vector
                    return std::get<float>(lhs) + std::get<float>(rhs);
                }
                else if (var == "-") {
                    return std::get<float>(lhs) - std::get<float>(rhs);
                }
                else if (var == "*") {
                    return std::get<float>(lhs) * std::get<float>(rhs);
                }
                else if (var == "/") {
                    if (std::get<float>(rhs) == 0)
                        throw makeError<UnimplError>();
                    return std::get<float>(lhs) / std::get<float>(rhs);
                }
                else {
                    return zfxvariant();
                }
            }
            case nodeType::NEGATIVE:
            {
                if (root->children.size() != 1)
                {
                    throw makeError<UnimplError>("NEGATIVE number is missing");
                }
                zfxvariant val = calc(root->children[0], pContext);
                val = std::visit([&](auto& arg) -> zfxvariant {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                        return -1 * arg;
                    }
                    else if constexpr (std::is_same_v<T, glm::vec2>)
                    {
                        return glm::vec2{ -1 * arg[0], -1 * arg[1] };
                    }
                    else if constexpr (std::is_same_v<T, glm::vec3>)
                    {
                        return glm::vec3{ -1 * arg[0], -1 * arg[1], -1 * arg[2] };
                    }
                    else if constexpr (std::is_same_v<T, glm::vec4>)
                    {
                        return glm::vec4{ -1 * arg[0], -1 * arg[1], -1 * arg[2], -1 * arg[3] };
                    }
                    else {
                        throw makeError<UnimplError>("NEGATIVE number type is invalid");
                    }
                }, val);
                return val;
            }
            case nodeType::FUNC:
            {
                ZfxElemFilter filter;
                filter.push_back(1);
                std::vector<ZfxVariable> args = process_args(root, filter, pContext);
                const std::string& funcname = get_zfxvar<std::string>(root->value);
                ZfxVariable result = eval(funcname, args, filter, pContext);
                return result.value[0];
#if 0
                if (funcname == "ref") {
                    if (root->children.size() != 1) throw makeError<UnimplError>();
                    const std::string ref = std::get<std::string>(calc(root->children[0], pContext));
                    
                    std::vector<ZfxVariable> args;
                    args.push_back(ZfxVariable(ref));
                    ZfxVariable res = callFunction("ref", args, filter, pContext);
                    return res.value[0];
                }
                else {
                    //先简单匹配调用
                    if (funcname == "sin") {
                        if (root->children.size() != 1) throw makeError<UnimplError>();
                        float val = std::get<float>(calc(root->children[0], pContext));
                        return sin(val);
                    }
                    else if (funcname == "cos") {
                        if (root->children.size() != 1) throw makeError<UnimplError>();
                        float val = std::get<float>(calc(root->children[0], pContext));
                        return cos(val);
                    }
                    else if (funcname == "sinh") {
                        if (root->children.size() != 1) throw makeError<UnimplError>();
                        float val = std::get<float>(calc(root->children[0], pContext));
                        return sinh(val);
                    }
                    else if (funcname == "cosh") {
                        if (root->children.size() != 1) throw makeError<UnimplError>();
                        float val = std::get<float>(calc(root->children[0], pContext));
                        return cosh(val);
                    }
                    else if (funcname == "rand") {
                        if (!root->children.empty()) throw makeError<UnimplError>();
                        return rand();
                    }
                    else {
                        throw makeError<UnimplError>();
                    }
                }
#endif
            }
            }
        }
        return zfxvariant();
    }



    ZfxVariable FunctionManager::eval(const std::string& funcname, const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
        return callFunction(funcname, args, filter, pContext);
    }

}