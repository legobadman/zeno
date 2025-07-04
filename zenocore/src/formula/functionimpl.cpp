#include "functionimpl.h"
#include <zeno/extra/GlobalState.h>
#include <zeno/utils/Error.h>
#include <zeno/core/Graph.h>
#include <zeno/utils/log.h>
#include <zeno/utils/helper.h>
#include <regex>
#include <variant>
#include <functional>
#include <zeno/utils/format.h>
#include <numeric>
#include <zeno/geo/geometryutil.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/vectorutil.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/core/data.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/utils/parallel_reduce.h>
#include <zeno/extra/CalcContext.h>
#include <zeno/geo/kdsearch.h>
#include <random>
#include "../utils/zfxutil.h"

#define REF_DEPEND_APPLY

namespace zeno
{
    namespace zfx
    {
        template <class T>
        static T get_zfxvar(zfxvariant value) {
            return std::visit([](auto const& val) -> T {
                using V = std::decay_t<decltype(val)>;
                if constexpr (!std::is_constructible_v<T, V>) {
                    if constexpr (std::is_same_v<T, glm::vec3> && (std::is_same_v<V, zfxfloatarr> || std::is_same_v<V, zfxintarr>)) {
                        return glm::vec3(val[0], val[1], val[2]);
                    } else if constexpr (std::is_same_v<T, zeno::vec3f> && (std::is_same_v<V, zfxfloatarr> || std::is_same_v<V, zfxintarr>)) {
                        return zeno::vec3f(val[0], val[1], val[2]);
                    } else if constexpr (std::is_same_v<T, zeno::vec3i> && (std::is_same_v<V, zfxfloatarr> || std::is_same_v<V, zfxintarr>)) {
                        return zeno::vec3i(val[0], val[1], val[2]);
                    } 
                    throw makeError<TypeError>(typeid(T), typeid(V), "get<zfxvariant>");
                }
                else {
                    return T(val);
                }
                }, value);
        }

        template <class T>
        static T prim_reduce(std::vector<T> const& temp, std::string type)
        {
            if (type == std::string("avg")) {
                T total = zeno::parallel_reduce_array<T>(temp.size(), T(0), [&](size_t i) -> T { return temp[i]; },
                    [&](T i, T j) -> T { return i + j; });
                return total / (T)(temp.size());
            }
            if (type == std::string("max")) {
                T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&](size_t i) -> T { return temp[i]; },
                    [&](T i, T j) -> T { return zeno::max(i, j); });
                return total;
            }
            if (type == std::string("min")) {
                T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&](size_t i) -> T { return temp[i]; },
                    [&](T i, T j) -> T { return zeno::min(i, j); });
                return total;
            }
            if (type == std::string("absmax")) {
                T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&](size_t i) -> T { return zeno::abs(temp[i]); },
                    [&](T i, T j) -> T { return zeno::max(i, j); });
                return total;
            }
            return T(0);
        }

        template <class T>
        void removeElemsByIndice(std::vector<T>& vec, std::set<int> sorted_indice) {
            if (sorted_indice.empty())
                return;

            for (auto iter = sorted_indice.rbegin(); iter != sorted_indice.rend(); iter++) {
                int rmIdx = *iter;
                vec.erase(vec.begin() + rmIdx);
            }
        }

        static std::string format_variable_size(const char* fmt, std::vector<zfxvariant> args) {
            return std::accumulate(
                std::begin(args),
                std::end(args),
                std::string{ fmt },
                [](std::string toFmt, zfxvariant arg) {
                    return std::visit([toFmt](auto&& val)->std::string {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return format(toFmt, val);
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            return format(toFmt, val);
                        }
                        else if constexpr (std::is_same_v<T, std::string>) {
                            return format(toFmt, val);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec2>) {
                            return format(toFmt, "{" + std::to_string(val.x) + "," + std::to_string(val.y) + "}");
                        }
                        else if constexpr (std::is_same_v<T, glm::vec3>) {
                            return format(toFmt, "{" + std::to_string(val.x) + "," + std::to_string(val.y )+ "," + std::to_string(val.z) + "}");
                        }
                        else if constexpr (std::is_same_v<T, glm::vec4>) {
                            return format(toFmt, "{" + std::to_string(val.x) + "," + std::to_string(val.y) + "," + std::to_string(val.z) + "," + std::to_string(val.w) + "}");
                        }
                        else {
                            throw makeError<UnimplError>("error type on format string");
                        }
                        }, arg);
                }
            );
        }

        static AttrVar zfxVarToAttrVar(const ZfxVariable& var) {
            int N = var.value.size();
            if (N == 1) {
                return std::visit([&](auto&& arg)->AttrVar {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (
                        std::is_same_v<T, int> ||
                        std::is_same_v<T, float> ||
                        std::is_same_v<T, std::string> ||
                        std::is_same_v<T, glm::vec2> ||
                        std::is_same_v<T, glm::vec3> ||
                        std::is_same_v<T, glm::vec4>) {
                        return arg;
                    }
                    else if constexpr (
                        std::is_same_v<T, zfxintarr> ||
                        std::is_same_v<T, zfxfloatarr> ||
                        std::is_same_v<T, zfxstringarr>) {
                        throw makeError<UnimplError>("no support array as the attribute type.");
                    }
                    else {
                        throw makeError<UnimplError>("no supported attribute type.");
                    }
                }, var.value[0]);
            }
            else {
                return std::visit([&](auto&& arg)->AttrVar {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (
                        std::is_same_v<T, int> ||
                        std::is_same_v<T, float> ||
                        std::is_same_v<T, std::string> ||
                        std::is_same_v<T, glm::vec2> ||
                        std::is_same_v<T, glm::vec3> ||
                        std::is_same_v<T, glm::vec4>) {
                        std::vector<T> vec(N);
                        for (int i = 0; i < N; i++) {
                            vec[i] = std::get<T>(var.value[i]);
                        }
                        return vec;
                    }
                    else if constexpr (
                        std::is_same_v<T, zfxintarr> ||
                        std::is_same_v<T, zfxfloatarr> ||
                        std::is_same_v<T, zfxstringarr>) {
                        throw makeError<UnimplError>("no support array as the attribute type.");
                    }
                    else {
                        throw makeError<UnimplError>("no supported attribute type.");
                    }
                }, var.value[0]);
            }
        }

        static std::vector<zfxvariant> getAttrs(GeometryObject* spGeo, GeoAttrGroup grp, std::string name) {
            if (name == "P") name = "pos";
            if (name == "N") name = "nrm";
            
            GeoAttrType type = spGeo->get_attr_type(grp, name);
            std::vector<zfxvariant> zfxvariantVector;
            if (type == ATTR_INT) {
                std::vector<int> intVector = spGeo->get_attrs<int>(grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = intVector[i];
                }
            }
            else if (type == ATTR_FLOAT) {
                std::vector<float> intVector = spGeo->get_attrs<float>(grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = intVector[i];
                }
            }
            else if (type == ATTR_STRING) {
                std::vector<std::string> intVector = spGeo->get_attrs<std::string>(grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = intVector[i];
                }
            }
            else if (type == ATTR_VEC2) {
                std::vector<zeno::vec2f> intVector = spGeo->get_attrs < zeno::vec2f > (grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = glm::vec2(intVector[i][0], intVector[i][1]); ;
                }
            }
            else if (type == ATTR_VEC3) {
                std::vector<zeno::vec3f> intVector = spGeo->get_attrs < zeno::vec3f >(grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = glm::vec3(intVector[i][0], intVector[i][1], intVector[i][2]); ;
                }
            }
            else if (type == ATTR_VEC4) {
                std::vector<zeno::vec4f> intVector = spGeo->get_attrs < zeno::vec4f >(grp, name);
                zfxvariantVector.resize(intVector.size());
                for (size_t i = 0; i < intVector.size(); ++i) {
                    zfxvariantVector[i] = glm::vec4(intVector[i][0], intVector[i][1], intVector[i][2], intVector[i][3]); ;
                }
            }
            else if (type == ATTR_TYPE_UNKNOWN) {
                throw makeError<UnimplError>("unknown attr on geom");
            }
            return zfxvariantVector;
        }

        static ZfxVariable getParamValueFromRef(const std::string& ref, ZfxContext* pContext) {
            ParamPrimitive paramData;
            std::vector<std::string> items;

            if (zeno::starts_with(ref, "../")) {
                //直接取上层子图节点的参数
                std::string parampath = ref.substr(3);
                items = split_str(parampath, '.');
                std::string paramname = items[0];
                NodeImpl* parSbnNode = pContext->spNode->getGraph()->getParentSubnetNode();
                if (!parSbnNode) {
                    throw makeError<UnimplError>("cannot locate parent subnetnode, when refering params");
                }
                bool bExisted = false;
                paramData = parSbnNode->get_input_prim_param(paramname, &bExisted);
                if (!bExisted) {
                    throw makeError<UnimplError>("there is no param `" + paramname + "` in subnetnode");
                }
            }
            else {
                std::string parampath, _;
                auto spNode = zfx::getNodeAndParamFromRefString(ref, pContext, _, parampath);
                items = split_str(parampath, '.');
                std::string paramname = items[0];

                bool bExist = false;
                paramData = spNode->get_input_prim_param(paramname, &bExist);
                if (!bExist) {
                    //试一下拿prim_output，有些情况，比如获取SubInput的port，是需要拿Output的
                    paramData = spNode->get_output_prim_param(paramname, &bExist);
                    if (!bExist) {
                        throw makeError<UnimplError>("the refer param doesn't exist, may be deleted before");
                    }
                }
            }

            //直接拿引用源的计算结果，所以本节点在执行前，引用源必须先执行（比如引用源的数值也是依赖另一个节点），参考代码INode::requireInput
            //，所以要在preApply的基础上作提前依赖计算

            //如果取的是“静态值”，则不要求引用源必须执行，此时要拿的数据应该是defl.

            //以上方案取决于产品设计。

            //NOTE in 2025/3/25: 应该拿计算结果而不是默认值，并且支持跨图层获取

            float res = 0.f;
            if (items.size() == 1) {
#ifdef REF_DEPEND_APPLY
                if (!paramData.result.has_value()) {
                    throw makeError<UnimplError>("there is no result on refer source, should calc the source first.");
                }

                size_t primtype = paramData.result.type().hash_code();
                if (primtype == zeno::types::gParamType_Int) {
                    res = zeno::reflect::any_cast<int>(paramData.result);
                }
                else if (primtype == zeno::types::gParamType_Bool) {
                    bool bret = zeno::reflect::any_cast<bool>(paramData.result);
                    ZfxVariable varres;
                    varres.value.push_back(bret ? 1 : 0);
                    return varres;
                }
                else if (primtype == zeno::types::gParamType_Float) {
                    res = zeno::reflect::any_cast<float>(paramData.result);
                }
                else if (primtype == zeno::types::gParamType_String) {
                    auto str = zeno::reflect::any_cast<std::string>(paramData.result);
                    ZfxVariable varres;
                    varres.value.push_back(str);
                    return varres;
                }
                else if (primtype == zeno::types::gParamType_Vec2f) {
                    throw makeError<UnimplError>();
                }
                else if (primtype == zeno::types::gParamType_Vec2i) {
                    throw makeError<UnimplError>();
                }
                else if (primtype == zeno::types::gParamType_Vec3f) {
                    vec3f vec = zeno::reflect::any_cast<vec3f>(paramData.result);
                    ZfxVariable varres;
                    varres.value.push_back(glm::vec3(vec[0], vec[1], vec[2]));
                    return varres;
                }
                else if (primtype == zeno::types::gParamType_Vec3i) {
                    vec3i vec = zeno::reflect::any_cast<vec3i>(paramData.result);
                    ZfxVariable varres;
                    varres.value.push_back(glm::vec3(vec[0], vec[1], vec[2]));
                    return varres;
                }
                else if (primtype == zeno::types::gParamType_Vec4f) {
                    throw makeError<UnimplError>();
                }
                else if (primtype == zeno::types::gParamType_Vec4i) {
                    throw makeError<UnimplError>();
                }
                else {
                    throw makeError<UnimplError>();
                }
#else
                auto refVal = paramData.defl;
                if (!refVal.has_value()) {
                    throw makeError<UnimplError>("there is no result on refer source, should calc the source first.");
                }

                size_t primtype = refVal.type().hash_code();
                if (primtype == zeno::types::gParamType_Int) {
                    res = zeno::reflect::any_cast<int>(refVal);
                }
                else if (primtype == zeno::types::gParamType_Float) {
                    res = zeno::reflect::any_cast<float>(refVal);
                }
                else if (primtype == zeno::types::gParamType_PrimVariant) {
                    auto refvar = zeno::reflect::any_cast<PrimVar>(refVal);
                    res = std::visit([&](auto& arg)->float {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {
                            return (float)arg;
                        }
                        else if constexpr (std::is_same_v<T, std::string>) {
                            if (arg.find("ref(") != std::string::npos) {
                                throw makeError<UnimplError>("don't support recursive ref in this version");
                            }
                            else {
                                throw makeError<UnimplError>("don't support string right now.");
                            }
                        }
                        else {
                            throw makeError<UnimplError>("error param type from primvar");
                        }
                    }, refvar);
                }
                else {
                    throw makeError<UnimplError>();
                }
#endif
            }
            else if (items.size() == 2 &&
                (paramData.type == zeno::types::gParamType_Vec2f || paramData.type == zeno::types::gParamType_Vec2i ||
                    paramData.type == zeno::types::gParamType_Vec3f || paramData.type == zeno::types::gParamType_Vec3i ||
                    paramData.type == zeno::types::gParamType_Vec4f || paramData.type == zeno::types::gParamType_Vec4i))
            {
                if (items[1].size() != 1)
                    throw makeError<UnimplError>();

                int idx = -1;
                switch (items[1][0])
                {
                case 'x': idx = 0; break;
                case 'y': idx = 1; break;
                case 'z': idx = 2; break;
                case 'w': idx = 3; break;
                default:
                    throw makeError<UnimplError>();
                }
                if (paramData.type == zeno::types::gParamType_Vec2f || paramData.type == zeno::types::gParamType_Vec2i) {
                    if (idx < 2) {
                        res = paramData.type == zeno::types::gParamType_Vec2f ? any_cast<vec2f>(paramData.result)[idx] :
                            any_cast<vec2i>(paramData.result)[idx];
                    }
                    else {
                        throw makeError<UnimplError>();
                    }
                }
                if (paramData.type == zeno::types::gParamType_Vec3f || paramData.type == zeno::types::gParamType_Vec3i) {
                    if (idx < 3) {
                        res = paramData.type == zeno::types::gParamType_Vec3f ? any_cast<vec3f>(paramData.result)[idx] :
                            any_cast<vec3i>(paramData.result)[idx];
                    }
                    else {
                        throw makeError<UnimplError>();
                    }
                }
                if (paramData.type == zeno::types::gParamType_Vec4f || paramData.type == zeno::types::gParamType_Vec4i) {
                    if (idx < 4) {
                        res = paramData.type == zeno::types::gParamType_Vec4f ? any_cast<vec4f>(paramData.result)[idx] :
                            any_cast<vec4i>(paramData.result)[idx];
                    }
                    else {
                        throw makeError<UnimplError>();
                    }
                }
            }
            else {
                throw makeError<UnimplError>();
            }

            ZfxVariable varres;
            varres.value.push_back(res);
            return varres;
        };

        static std::shared_ptr<IObject> getObjFromRef(const std::string& ref, ZfxContext* pContext) {
            std::string parampath, _;
            auto spNode = zfx::getNodeAndParamFromRefString(ref, pContext, _, parampath);
            auto items = split_str(parampath, '.');
            std::string paramname = items[0];
            return spNode->get_output_obj(paramname);
        }

        static ZfxVariable callRef(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("only support non-attr value when using ref");

            //TODO: vec type.
            //TODO: resolve with zeno::reflect::any
            const std::string ref = get_zfxvar<std::string>(args[0].value[0]);
            return getParamValueFromRef(ref, pContext);
        }

        static ZfxVariable parameter(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1) {
                throw makeError<UnimplError>("error number of args on param(...)");
            }
            if (pContext->param_constrain.constrain_param.empty()) {
                throw makeError<UnimplError>("only support indexing param for param constrain");
            }
            const std::string& param = get_zfxvar<std::string>(args[0].value[0]);
            auto pnode = pContext->spNode;
            ZfxLValue lval;
            bool bExist = false;
            lval.var = pnode->get_input_obj_param(param, &bExist);
            if (bExist) {
                return lval;
            }
            else {
                lval.var = pnode->get_input_prim_param(param, &bExist);
                if (bExist)
                    return lval;
            }
            throw makeError<UnimplError>("the param does not exist when calling param(...)");
        }

        static ZfxVariable log(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.empty()) {
                throw makeError<UnimplError>("empty args on log");
            }
            const auto& formatStr = args[0];
            assert(formatStr.value.size() == 1);
            std::string formatString = get_zfxvar<std::string>(formatStr.value[0]);

            std::vector<ZfxVariable> _args = args;
            _args.erase(_args.begin());

            //有可能是： log("format", 2, @P.x, b);  //这里的_args的元素，可能是一个或多个。
            int maxSize = 1;
            for (auto& arg : _args) {
                maxSize = max(maxSize, arg.value.size());
            }

            //逐个调用输出
            if (maxSize > 1) {
                //属性或相关变量的调用
                for (int i = 0; i < maxSize; i++) {
                    if (!filter[i]) continue;
                    std::vector<zfxvariant> formatargs;
                    for (int j = 0; j < _args.size(); j++) {
                        auto& arg = _args[j];
                        assert(!arg.value.empty());
                        if (arg.value.size() < i) {
                            formatargs.push_back(arg.value[0]);
                        }
                        else {
                            formatargs.push_back(arg.value[i]);
                        }
                    }
                    std::string ret = format_variable_size(formatString.c_str(), formatargs);
                    zeno::log_only_print(ret);
                }
            }
            else {
                std::vector<zfxvariant> __args;
                for (auto __arg : _args) {
                    if (__arg.value.empty()) {
                        continue;
                    }
                    __args.push_back(__arg.value[0]);
                }
                std::string ret = format_variable_size(formatString.c_str(), __args);
                zeno::log_only_print(ret);
            }

            return ZfxVariable();
        }

        static ZfxVariable vec3(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 3)
                throw makeError<UnimplError>("the number of elements isn't 3");
            const ZfxVariable& xvar = args[0], & yvar = args[1], & zvar = args[2];
            int nx = xvar.value.size(), ny = yvar.value.size(), nz = zvar.value.size();
            int N = std::max(nx, std::max(ny, nz));
            ZfxVariable res;
            if (N == 1) {
                float x = get_zfxvar<float>(xvar.value[0]);
                float y = get_zfxvar<float>(yvar.value[0]);
                float z = get_zfxvar<float>(zvar.value[0]);
                res.value.push_back(glm::vec3(x, y, z));
            }
            else {
                for (int i = 0; i < N; i++) {
                    int ni = std::min(i, nx);
                    int nj = std::min(i, ny);
                    int nk = std::min(i, nz);
                    float x = get_zfxvar<float>(xvar.value[ni]);
                    float y = get_zfxvar<float>(yvar.value[nj]);
                    float z = get_zfxvar<float>(zvar.value[nk]);
                    res.value.push_back(glm::vec3(x, y, z));
                }
            }
            return res;
        }

        static ZfxVariable sin(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            int N = arg.value.size();
            ZfxVariable res;
            res.value.resize(N);
            assert(N >= 1);
            if (N > 1) {
                for (int i = 0; i < arg.value.size(); i++)
                {
                    if (!filter[i]) continue;
                    float val = get_zfxvar<float>(arg.value[i]);
                    res.value[i] = std::sin(val);
                }
            }
            else {
                float val = get_zfxvar<float>(arg.value[0]);
                res.value[0] = std::sin(val);
            }
            return res;
        }

        static ZfxVariable cos(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            ZfxVariable res;
            res.value.resize(arg.value.size());
            for (int i = 0; i < arg.value.size(); i++)
            {
                if (!filter[i]) continue;
                float val = get_zfxvar<float>(arg.value[i]);
                res.value[i] = std::cos(val);
            }
            return res;
        }

        static ZfxVariable sinh(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            ZfxVariable res;
            res.value.resize(arg.value.size());
            for (int i = 0; i < arg.value.size(); i++)
            {
                if (!filter[i]) continue;
                float val = get_zfxvar<float>(arg.value[i]);
                res.value[i] = std::sinh(val);
            }
            return res;
        }

        static ZfxVariable cosh(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            ZfxVariable res;
            res.value.resize(arg.value.size());
            for (int i = 0; i < arg.value.size(); i++)
            {
                if (!filter[i]) continue;
                float val = get_zfxvar<float>(arg.value[i]);
                res.value[i] = std::cosh(val);
            }
            return res;
        }

        static ZfxVariable fit01(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 3) throw makeError<UnimplError>();

            //只考虑单值的情况: fit01(.3,5,20)=9.5
            float value = get_zfxvar<float>(args[0].value[0]);
            float omin = 0;
            float omax = 1;
            float nmin = get_zfxvar<float>(args[1].value[0]);
            float nmax = get_zfxvar<float>(args[2].value[0]);

            if (nmin == nmax) {
                throw makeError<UnimplError>("the omin == omax or nmin == nmax");
            }
            if (value < omin || value >omax) {
                throw makeError<UnimplError>("the value is not between 0 and 1, when calling `fit01`");
            }
            float mp_value = ((value - omin) / (omax - omin)) * (nmax - nmin) + nmin;
            ZfxVariable ret;
            ret.value.push_back(mp_value);
            return ret;
        }

        static ZfxVariable fit(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 5) throw makeError<UnimplError>();

            //只考虑单值的情况: fit(.3, 0, 1, 10, 20) == 13
            float value = get_zfxvar<float>(args[0].value[0]);
            float omin = get_zfxvar<float>(args[1].value[0]);
            float omax = get_zfxvar<float>(args[2].value[0]);
            float nmin = get_zfxvar<float>(args[3].value[0]);
            float nmax = get_zfxvar<float>(args[4].value[0]);

            if (omin == omax || nmin == nmax) {
                throw makeError<UnimplError>("the omin == omax or nmin == nmax");
            }
            if (value < omin || value >omax) {
                throw makeError<UnimplError>("the value is not between omin and omax, when calling `fit`");
            }
            float mp_value = ((value - omin) / (omax - omin)) * (nmax - nmin) + nmin;
            ZfxVariable ret;
            ret.value.push_back(mp_value);
            return ret;
        }

        static ZfxVariable rand(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() > 1) throw makeError<UnimplError>();
            int N = 1;
            ZfxVariable res;
            if (args.size() == 1) {
                N = args[0].value.size();
                res.value.resize(N);
                for (int i = 0; i < N; i++) {
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            //srand(arg);
                            std::mt19937 rng(arg);
                            std::uniform_real_distribution<float> dist(0.0, 1.0);
                            res.value[i] = dist(rng);// std::rand();
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            //srand((int)arg);
                            std::mt19937 rng(arg);
                            std::uniform_real_distribution<float> dist(0.0, 1.0);
                            res.value[i] = dist(rng);// std::rand();
                        }
                    }, args[0].value[i]);
                }
            }
            else {
                std::mt19937 rng(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0, 1.0);
                res.value.push_back(dist(rng));
            }
            return res;
        }

        static ZfxVariable pow(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            const auto& idx = args[1];
            if (idx.value.size() != 1) {
                throw makeError<UnimplError>();
            }
            ZfxVariable res;
            res.value.resize(arg.value.size());
            for (int i = 0; i < arg.value.size(); i++)
            {
                if (!filter[i]) continue;
                float val = get_zfxvar<float>(arg.value[i]);
                res.value[i] = std::pow(val, get_zfxvar<float>(idx.value[0]));
            }
            return res;
        }

        static ZfxVariable add_point(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() == 1) {
                const auto& arg = args[0];
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                    //暂时只考虑一个点
                    const zfxvariant& varpos = arg.value[0];
                    int ptnum = -1;
                    std::visit([&](auto&& val) {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_same_v<T, glm::vec3> || std::is_same_v<T, zeno::vec3f> || std::is_same_v < T, zeno::vec3i > ) {
                            ptnum = spGeo->m_impl->add_point(zeno::vec3f(val[0], val[1], val[2]));
                        } else if constexpr (std::is_same_v<T, zfxintarr> || std::is_same_v<T, zfxfloatarr>) {
                            if (val.size() == 3) {
                                ptnum = spGeo->m_impl->add_point(zeno::vec3f(val[0], val[1], val[2]));
                            } else {
                                throw makeError<UnimplError>("the number of arguments of add_point is not matched.");
                            }
                        } else {
                            throw makeError<UnimplError>("the type of arguments of add_point is not matched.");
                        }
                        }, varpos);
                    ZfxVariable res;
                    res.value.push_back(ptnum);
                    return res;
                }
                else {
                    throw makeError<UnimplError>();
                }
            }
            else if (args.empty()) {
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                    //暂时只考虑一个点
                    int ptnum = spGeo->m_impl->add_point(zeno::vec3f(0, 0, 0));
                    ZfxVariable res;
                    res.value.push_back(ptnum);
                    return res;
                }
                else {
                    throw makeError<UnimplError>();
                }
            }
            else {
                throw makeError<UnimplError>();
            }
        }

        static ZfxVariable add_vertex(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2) {
                throw makeError<UnimplError>();
            }
            if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                int faceid = get_zfxvar<int>(args[0].value[0]);
                int pointid = get_zfxvar<int>(args[1].value[0]);
                int vertid = spGeo->m_impl->add_vertex(faceid, pointid);
                ZfxVariable res;
                res.value.push_back(vertid);
                return res;
            }
            else {
                throw makeError<UnimplError>();
            }
        }

        static ZfxVariable remove_vertex(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of remove_vertex is not matched.");

            const auto& arg = args[0];
            int N = arg.value.size();
            if (N == 0)
                return ZfxVariable(false);

            const auto& arg2 = args[1];
            N = arg2.value.size();
            if (N == 0)
                return ZfxVariable(false);

            bool bSucceed = false;

            if (N < filter.size()) {
                assert(N == 1);
                int faceid = get_zfxvar<int>(arg.value[0]);
                int vertid = get_zfxvar<int>(arg2.value[0]);

                /* 删除pointnum的点，如果成功，就返回原来下一个点的pointnum(应该就是pointnum)，失败就返回-1 */
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                    bSucceed = spGeo->m_impl->remove_vertex(faceid, vertid);
                }

                if (bSucceed) {
                } else {
                    throw makeError<UnimplError>("error on removeVertex");
                }
#if 0
                if (bSucceed) {
                    //要调整filter，移除掉第currrem位置的元素
                    filter.erase(filter.begin() + currrem);
                    //所有储存在m_globalAttrCached里的属性都移除第currrem号元素，如果有ptnum，也要调整

                    //移除点以后要调整已有的属性值
                    for (auto& [name, attrVar] : *pContext->zfxVariableTbl) {
                        auto& attrvalues = attrVar.value;
                        if (name == "@ptnum") {
                            assert(currrem < attrvalues.size());
                            for (int i = currrem + 1; i < attrvalues.size(); i++)
                                attrvalues[i] = i - 1;
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                        else {
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                    }
                }
                else {
                    throw makeError<UnimplError>("error on removePoint");
                }
#endif
            }
            else {
                //移除多个的情况
                //TODO:
            }
            return ZfxVariable(bSucceed);
        }

        static bool _remove_points(std::deque<int> remPoints, ZfxElemFilter& filter, ZfxContext* pContext) {
            bool bSucceed = false;
            while (!remPoints.empty())
            {
                int currrem = remPoints.front();
                remPoints.pop_front();
                bSucceed = false;
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                    bSucceed = spGeo->m_impl->remove_point(currrem);
                }
                if (bSucceed) {
                    //要调整filter，移除掉第currrem位置的元素
                    filter.erase(filter.begin() + currrem);
                    //所有储存在m_globalAttrCached里的属性都移除第currrem号元素，如果有ptnum，也要调整
                    //afterRemovePoint(currrem);
                    for (auto& [name, attrVar] : *pContext->zfxVariableTbl) {
                        auto& attrvalues = attrVar.value;
                        if (name == "@ptnum") {
                            assert(currrem < attrvalues.size());
                            for (int i = currrem + 1; i < attrvalues.size(); i++)
                                attrvalues[i] = i - 1;
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                        else {
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                    }

                    //最后将当前所有剩下的删除点的序号再自减
                    for (auto iter = remPoints.begin(); iter != remPoints.end(); iter++) {
                        *iter -= 1;
                    }
                }
                else {
                    throw makeError<UnimplError>("error on removePoint");
                }
            }
            return bSucceed;
        }

        static ZfxVariable remove_point(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of remove_point is not matched.");

            ZfxVariable arg = args[0];
            int N = arg.value.size();
            if (N == 0) return ZfxVariable(false);
            bool bSucceed = false;

            if (N == 1) {
                //可能是数组
                if (std::holds_alternative<zfxintarr>(arg.value[0])) {
                    const auto& arr = std::get<zfxintarr>(arg.value[0]);
                    std::deque<int> remPoints;
                    for (int pt : arr) {
                        remPoints.push_back(pt);
                    }
                    bSucceed = _remove_points(remPoints, filter, pContext);
                    return ZfxVariable(bSucceed);
                }
            }

            if (N < filter.size()) {
                assert(N == 1);
                int currrem = get_zfxvar<int>(arg.value[0]);

                /* 删除pointnum的点，如果成功，就返回原来下一个点的pointnum(应该就是pointnum)，失败就返回-1 */
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject)) {
                    bSucceed = spGeo->m_impl->remove_point(currrem);
                }
                if (bSucceed) {
                    //要调整filter，移除掉第currrem位置的元素
                    filter.erase(filter.begin() + currrem);
                    //所有储存在m_globalAttrCached里的属性都移除第currrem号元素，如果有ptnum，也要调整

                    //移除点以后要调整已有的属性值
                    for (auto& [name, attrVar] : *pContext->zfxVariableTbl) {
                        auto& attrvalues = attrVar.value;
                        if (name == "@ptnum") {
                            assert(currrem < attrvalues.size());
                            for (int i = currrem + 1; i < attrvalues.size(); i++)
                                attrvalues[i] = i - 1;
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                        else {
                            attrvalues.erase(attrvalues.begin() + currrem);
                        }
                    }
                }
                else {
                    throw makeError<UnimplError>("error on removePoint");
                }
            }
            else {
                std::deque<int> remPoints;
                assert(N == filter.size());
                for (int i = 0; i < N; i++) {
                    if (!filter[i]) continue;
                    int pointnum = get_zfxvar<int>(arg.value[i]);
                    remPoints.push_back(pointnum);
                }
                bSucceed = _remove_points(remPoints, filter, pContext);
            }
            return ZfxVariable(bSucceed);
        }

        static ZfxVariable add_face(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of add_face is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::vector<int> points = get_zfxvar<std::vector<int>>(args[0].value[0]);
            int ret = spGeo->m_impl->add_face(points);
            return ret;
        }

        static ZfxVariable remove_face(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() > 2 || args.empty())
                throw makeError<UnimplError>("the number of arguments of remove_face is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            const auto& arg = args[0];
            bool bIncludePoints = true;
            if (args.size() == 2) {
                bIncludePoints = get_zfxvar<bool>(args[1].value[0]);
            }

            int N = arg.value.size();
            if (N == 0) return ZfxVariable(false);
            bool bSucceed = false;

            std::set<int> remfaces;
            if (N < filter.size()) {
                assert(N == 1);
                std::visit([&remfaces](const auto& val) {//N为1的时候也可能是一个std::vector<int>，需判断
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, int>) {
                        int currrem = get_zfxvar<int>(val);
                        remfaces.insert(currrem);
                    }
                    else if constexpr (std::is_same_v<T, float>) {
                        int currrem = static_cast<int>(get_zfxvar<float>(val));
                        remfaces.insert(currrem);
                    }
                    else if constexpr (std::is_same_v<T, std::vector<int>>) {
                        for (const auto& i : val) {
                            remfaces.insert(i);
                        }
                    }
                }, arg.value[0]);
            }
            else {
                assert(N == filter.size());
                for (int i = 0; i < N; i++) {
                    if (!filter[i]) continue;
                    int pointnum = get_zfxvar<int>(arg.value[i]);
                    remfaces.insert(pointnum);
                }
            }

            bSucceed = spGeo->m_impl->remove_faces(remfaces, bIncludePoints);
            if (bSucceed) {
                //要调整filter，移除掉第currrem位置的元素
                removeElemsByIndice(filter, remfaces);
                //afterRemoveElements(remfaces);
                for (auto& [name, attrVar] : *pContext->zfxVariableTbl) {
                    auto& attrvalues = attrVar.value;
                    removeElemsByIndice(attrvalues, remfaces);
                }
            }
            else {
                throw makeError<UnimplError>("error on removeface");
            }
            ZfxVariable varRet(bSucceed);
            return varRet;
        }

        static ZfxVariable create_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 3)
                throw makeError<UnimplError>("the number of arguments of create_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);

            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;

            AttrVar defl = zfxVarToAttrVar(args[2]);
            int ret = spGeo->m_impl->create_attr(grp, name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable create_face_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of create_face_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->create_face_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable create_point_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of create_point_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->create_point_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable create_vertex_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of create_vertex_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->create_vertex_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable create_geometry_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of create_geometry_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->create_geometry_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable set_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 3)
                throw makeError<UnimplError>("the number of arguments of set_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);

            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;

            AttrVar defl = zfxVarToAttrVar(args[2]);
            int ret = spGeo->m_impl->set_attr(grp, name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable set_vertex_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of set_vertex_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->set_vertex_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable set_point_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of set_point_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->set_point_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable set_face_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of set_face_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->set_face_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable set_geometry_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of set_geometry_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            AttrVar defl = zfxVarToAttrVar(args[1]);
            int ret = spGeo->m_impl->set_geometry_attr(name, defl);
            ZfxVariable varRet(ret);
            return varRet;
        }

        static ZfxVariable has_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of has_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;

            bool hasattr = spGeo->m_impl->has_attr(grp, name);
            ZfxVariable ret(hasattr);
            return ret;
        }

        static ZfxVariable has_vertex_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of has_vertex_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            bool hasattr = spGeo->m_impl->has_vertex_attr(name);
            ZfxVariable ret(hasattr);
            return ret;
        }

        static ZfxVariable has_point_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of has_point_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            bool hasattr = spGeo->m_impl->has_point_attr(name);
            ZfxVariable ret(hasattr);
            return ret;
        }

        static ZfxVariable has_face_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of has_vertex_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            bool hasattr = spGeo->m_impl->has_face_attr(name);
            ZfxVariable ret(hasattr);
            return ret;
        }

        static ZfxVariable has_geometry_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of has_geometry_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            bool hasattr = spGeo->m_impl->has_geometry_attr(name);
            ZfxVariable ret(hasattr);
            return ret;
        }

        static ZfxVariable delete_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of delete_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;

            int ret = spGeo->m_impl->delete_attr(grp, name);
            return ret;
        }

        static ZfxVariable delete_vertex_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of delete_vertex_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            int ret = spGeo->m_impl->delete_vertex_attr(name);
            return ret;
        }

        static ZfxVariable delete_point_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of delete_point_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            int ret = spGeo->m_impl->delete_point_attr(name);
            return ret;
        }

        static ZfxVariable delete_face_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of delete_face_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            int ret = spGeo->m_impl->delete_face_attr(name);
            return ret;
        }

        static ZfxVariable delete_geometry_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of delete_geometry_attr is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            int ret = spGeo->m_impl->delete_geometry_attr(name);
            return ret;
        }

        static ZfxVariable npoints(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() > 1) {
                throw makeError<UnimplError>("the number of arguments of npoints is not matched.");
            } else if (args.size() == 1) {
                std::string ref = get_zfxvar<std::string>(args[0].value[0]);
                if (std::regex_search(ref, FunctionManager::refPattern)) {
                    std::shared_ptr<IObject> spObj = getObjFromRef(ref, pContext);
                    if (std::shared_ptr<GeometryObject_Adapter> spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(spObj)) {
                        return spGeo->m_impl->npoints();
                    } else {
                        throw makeError<UnimplError>("npoints function refers an empty output object.");
                    }
                } else {
                    throw makeError<UnimplError>("npoints can not resolve ref.");
                }
            } else {
                auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                int ret = spGeo->m_impl->npoints();
                return ret;
            }
        }

        static ZfxVariable nfaces(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 0)
                throw makeError<UnimplError>("the number of arguments of nfaces is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int ret = spGeo->m_impl->nfaces();
            return ret;
        }

        static ZfxVariable nvertices(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 0)
                throw makeError<UnimplError>("the number of arguments of nvertices is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int ret = spGeo->m_impl->nvertices();
            return ret;
        }

        /* 点相关 */
        static ZfxVariable point_faces(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of point_faces is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int pointid = get_zfxvar<int>(args[0].value[0]);
            std::vector<int> ret = spGeo->m_impl->point_faces(pointid);
            return ret;
        }

        static ZfxVariable point_vertex(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of point_vertex is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int pointid = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->point_vertex(pointid);
            return ret;
        }

        static ZfxVariable point_vertices(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of point_vertices is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int pointid = get_zfxvar<int>(args[0].value[0]);
            std::vector<int> ret = spGeo->m_impl->point_vertices(pointid);
            return ret;
        }

        /* 面相关 */
        static ZfxVariable face_point(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of face_point is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int faceid = get_zfxvar<int>(args[0].value[0]);
            int vertid = get_zfxvar<int>(args[1].value[0]);
            int ret = spGeo->m_impl->face_point(faceid, vertid);
            return ret;
        }

        static ZfxVariable face_points(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of face_points is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int N = std::min(args[0].value.size(), filter.size());
            ZfxVariable ret;
            ret.value.resize(N);
            for (int i = 0; i < N; i++) {
                int faceid = get_zfxvar<int>(args[0].value[i]);
                std::vector<int> pts = spGeo->m_impl->face_points(faceid);
                ret.value[i] = pts;
            }
            return ret;
        }

        static ZfxVariable face_vertex(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of face_vertex is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int faceid = get_zfxvar<int>(args[0].value[0]);
            int vertid = get_zfxvar<int>(args[1].value[0]);
            int ret = spGeo->m_impl->face_vertex(faceid, vertid);
            return ret;
        }

        static ZfxVariable face_vertex_count(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of face_vertex_count is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int faceid = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->face_vertex_count(faceid);
            return ret;
        }

        static ZfxVariable face_vertices(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of face_vertices is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int face_id = get_zfxvar<int>(args[0].value[0]);
            std::vector<int> ret = spGeo->m_impl->face_vertices(face_id);
            return ret;
        }

        /* Vertex相关 */
        static ZfxVariable vertex_index(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of vertex_index is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int faceid = get_zfxvar<int>(args[0].value[0]);
            int vertid = get_zfxvar<int>(args[1].value[0]);
            int ret = spGeo->m_impl->vertex_index(faceid, vertid);
            return ret;
        }

        static ZfxVariable vertex_next(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of vertex_next is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int linear_vertex_id = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->vertex_next(linear_vertex_id);
            return ret;
        }

        static ZfxVariable vertex_prev(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of vertex_prev is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int linear_vertex_id = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->vertex_prev(linear_vertex_id);
            return ret;
        }

        static ZfxVariable vertex_point(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of vertex_point is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int linear_vertex_id = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->vertex_point(linear_vertex_id);
            return ret;
        }

        static ZfxVariable vertex_face(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of vertex_face is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int linear_vertex_id = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->vertex_face(linear_vertex_id);
            return ret;
        }

        static ZfxVariable vertex_face_index(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of vertex_face_index is not matched.");

            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            int linear_vertex_id = get_zfxvar<int>(args[0].value[0]);
            int ret = spGeo->m_impl->vertex_face_index(linear_vertex_id);
            return ret;
        }

        static ZfxVariable get_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of get_attr is not matched.");

            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;
            
            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            assert(spGeo);
            ZfxVariable var;
            var.value = getAttrs(spGeo->m_impl, grp, name);
            return var;
        }

        static ZfxVariable get_vertex_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of get_vertex_attr is not matched.");

            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            assert(spGeo);
            ZfxVariable var;
            var.value = getAttrs(spGeo->m_impl, ATTR_VERTEX, name);
            return var;
        }

        static ZfxVariable get_point_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() > 2)
                throw makeError<UnimplError>("the number of arguments of get_point_attr is not matched.");

            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            assert(spGeo);

            ZfxVariable ret;
            std::vector<zfxvariant> attrs = getAttrs(spGeo->m_impl, ATTR_POINT, name);
            if (args.size() == 2) {
                //取索引
                //TODO: 调getElem，就不用整个attrs都拿出来，因为可能在循环下调用的
                int N = args[1].value.size();
                if (N > attrs.size())
                    throw makeError<UnimplError>("the attr size doesn't match");
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    int idx = get_zfxvar<int>(args[1].value[i]);
                    ret.value[i] = attrs[idx];
                }
            }
            else {
                ret.value = attrs;
            }
            return ret;
        }

        static ZfxVariable get_face_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of get_face_attr is not matched.");

            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            ZfxVariable var;
            std::vector<zfxvariant> attrs = getAttrs(spGeo->m_impl, ATTR_FACE, name);
            if (args.size() == 2) {
                //TODO:
                assert(false);
            }
            else {
                var.value = getAttrs(spGeo->m_impl, ATTR_FACE, name);
            }
            return var;
        }

        static ZfxVariable get_geometry_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("the number of arguments of get_geometry_attr is not matched.");

            std::string name = get_zfxvar<std::string>(args[0].value[0]);
            auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
            ZfxVariable var;
            var.value = getAttrs(spGeo->m_impl, ATTR_GEO, name);
            return var;
        }

        static ZfxVariable bbox(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2)
                throw makeError<UnimplError>("the number of arguments of get_geometry_attr is not matched.");

            std::string nodepath = get_zfxvar<std::string>(args[0].value[0]);
            std::string type = get_zfxvar<std::string>(args[1].value[0]);

            std::string parampath, _;
            auto spNode = zfx::getNodeAndParamFromRefString(nodepath, pContext, _, parampath);
            CalcContext ctx;
            spNode->doApply(&ctx);
            auto obj = getObjFromRef(nodepath, pContext);

            int res = 0;
            return res;
        }

        ZfxVariable callFunction(const std::string& funcname, const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext)
        {
            if (funcname == "ref") {
                return zeno::zfx::callRef(args, filter, pContext);
            }
            if (funcname == "param" || funcname == "parameter") {
                return parameter(args, filter, pContext);
            }
            if (funcname == "log") {
                return zeno::zfx::log(args, filter, pContext);
            }
            if (funcname == "vec3") {
                return zeno::zfx::vec3(args, filter, pContext);
            }
            if (funcname == "sin") {
                return zeno::zfx::sin(args, filter, pContext);
            }
            if (funcname == "cos") {
                return zeno::zfx::cos(args, filter, pContext);
            }
            if (funcname == "sinh") {
                return sinh(args, filter, pContext);
            }
            if (funcname == "cosh") {
                return cosh(args, filter, pContext);
            }
            if (funcname == "rand") {
                return rand(args, filter, pContext);
            }
            if (funcname == "pow") {
                return pow(args, filter, pContext);
            }
            if (funcname == "create_attr") {
                return create_attr(args, filter, pContext);
            }
            if (funcname == "create_face_attr") {
                return create_face_attr(args, filter, pContext);
            }
            if (funcname == "create_point_attr") {
                return create_point_attr(args, filter, pContext);
            }
            if (funcname == "create_vertex_attr") {
                return create_vertex_attr(args, filter, pContext);
            }
            if (funcname == "create_geometry_attr") {
                return create_geometry_attr(args, filter, pContext);
            }
            if (funcname == "set_attr") {
                return set_attr(args, filter, pContext);
            }
            if (funcname == "set_vertex_attr") {
                return set_vertex_attr(args, filter, pContext);
            }
            if (funcname == "set_point_attr") {
                return set_point_attr(args, filter, pContext);
            }
            if (funcname == "set_face_attr") {
                return set_face_attr(args, filter, pContext);
            }
            if (funcname == "set_geometry_attr") {
                return set_geometry_attr(args, filter, pContext);
            }
            if (funcname == "has_attr") {
                return has_attr(args, filter, pContext);
            }
            if (funcname == "has_vertex_attr") {
                return has_vertex_attr(args, filter, pContext);
            }
            if (funcname == "has_point_attr") {
                return has_point_attr(args, filter, pContext);
            }
            if (funcname == "has_face_attr") {
                return create_attr(args, filter, pContext);
            }
            if (funcname == "has_geometry_attr") {
                return has_geometry_attr(args, filter, pContext);
            }
            if (funcname == "delete_attr") {
                return delete_attr(args, filter, pContext);
            }
            if (funcname == "delete_vertex_attr") {
                return delete_vertex_attr(args, filter, pContext);
            }
            if (funcname == "delete_point_attr") {
                return delete_point_attr(args, filter, pContext);
            }
            if (funcname == "delete_face_attr") {
                return delete_face_attr(args, filter, pContext);
            }
            if (funcname == "delete_geometry_attr") {
                return delete_geometry_attr(args, filter, pContext);
            }
            if (funcname == "add_vertex") {
                return add_vertex(args, filter, pContext);
            }
            if (funcname == "add_point") {
                return add_point(args, filter, pContext);
            }
            if (funcname == "add_face") {
                return add_face(args, filter, pContext);
            }
            if (funcname == "remove_face") {
                return remove_face(args, filter, pContext);
            }
            if (funcname == "remove_point") {
                return remove_point(args, filter, pContext);
            }
            if (funcname == "remove_vertex") {
                return remove_vertex(args, filter, pContext);
            }
            if (funcname == "npoints") {
                return npoints(args, filter, pContext);
            }
            if (funcname == "nfaces") {
                return nfaces(args, filter, pContext);
            }
            if (funcname == "nvertices") {
                return nvertices(args, filter, pContext);
            }
            if (funcname == "point_faces") {
                return point_faces(args, filter, pContext);
            }
            if (funcname == "point_vertex") {
                return point_vertex(args, filter, pContext);
            }
            if (funcname == "point_vertices") {
                return point_vertices(args, filter, pContext);
            }
            if (funcname == "face_point") {
                return face_point(args, filter, pContext);
            }
            if (funcname == "face_points") {
                return face_points(args, filter, pContext);
            }
            if (funcname == "face_vertex") {
                return face_vertex(args, filter, pContext);
            }
            if (funcname == "face_vertex_count") {
                return face_vertex_count(args, filter, pContext);
            }
            if (funcname == "face_vertices") {
                return face_vertices(args, filter, pContext);
            }
            if (funcname == "vertex_index") {
                return vertex_index(args, filter, pContext);
            }
            if (funcname == "vertex_next") {
                return vertex_next(args, filter, pContext);
            }
            if (funcname == "vertex_prev") {
                return vertex_prev(args, filter, pContext);
            }
            if (funcname == "vertex_point") {
                return vertex_point(args, filter, pContext);
            }
            if (funcname == "vertex_face") {
                return vertex_face(args, filter, pContext);
            }
            if (funcname == "vertex_face_index") {
                return vertex_face_index(args, filter, pContext);
            }
            if (funcname == "get_attr") {
                return get_attr(args, filter, pContext);
            }            
            if (funcname == "get_vertex_attr") {
                return get_vertex_attr(args, filter, pContext);
            }            
            if (funcname == "get_point_attr") {
                return get_point_attr(args, filter, pContext);
            }
            if (funcname == "get_face_attr") {
                return get_face_attr(args, filter, pContext);
            }
            if (funcname == "get_geometry_attr") {
                return get_geometry_attr(args, filter, pContext);
            }
            if (funcname == "bbox") {
                return bbox(args, filter, pContext);
            }
            if (funcname == "rint" || funcname == "round") {
                ZfxVariable ret = args[0];
                for (int i = 0; i < args[0].value.size(); i++) {
                    float argval = get_zfxvar<float>(args[0].value[i]);
                    ret.value[i] = std::round(argval);
                }
                return ret;
            }
            if (funcname == "fit") {
                return fit(args, filter, pContext);
            }
            if (funcname == "fit01") {
                return fit01(args, filter, pContext);
            }
            if (funcname == "append") {
                const ZfxVariable& arr_var = args[0];
                const ZfxVariable& elem_var = args[1];
                const int N = arr_var.value.size();
                if (N != elem_var.value.size()) {
                    throw makeError<UnimplError>("the size of array and element doesn't match, when calling `append`");
                }

                ZfxVariable ret;    //引用机制实现有点麻烦，又得回填到局部变量表，目前先拷贝返回
                ret = arr_var;
                for (int i = 0; i < N; i++) {
                    std::visit([&](auto& _arg) {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, zfxintarr>) {
                            _arg.push_back(get_zfxvar<int>(elem_var.value[i]));
                        }
                        else if constexpr (std::is_same_v<T, zfxfloatarr>) {
                            _arg.push_back(get_zfxvar<float>(elem_var.value[i]));
                        }
                        else if constexpr (std::is_same_v<T, zfxstringarr>) {
                            _arg.push_back(get_zfxvar<std::string>(elem_var.value[i]));
                        }
                        else if constexpr (std::is_same_v<T, zfxvec2arr>) {
                            _arg.push_back(get_zfxvar<glm::vec2>(elem_var.value[i]));
                        }
                        else if constexpr (std::is_same_v<T, zfxvec3arr>) {
                            _arg.push_back(get_zfxvar<glm::vec3>(elem_var.value[i]));
                        }
                        else if constexpr (std::is_same_v<T, zfxvec4arr>) {
                            _arg.push_back(get_zfxvar<glm::vec4>(elem_var.value[i]));
                        }
                        else {
                            throw makeError<UnimplError>("only accept arr type on `append`");
                        }
                    }, ret.value[i]);
                }
                return ret;
            }
            if (funcname == "len") {
                //zfxintarr, zfxfloatarr, zfxstringarr,
                const ZfxVariable& var = args[0];
                const int N = var.value.size();
                ZfxVariable ret;
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    ret.value[i] = std::visit([&](auto&& _arg)->int {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, zfxintarr>) {
                            return _arg.size();
                        }
                        else if constexpr (std::is_same_v<T, zfxfloatarr>) {
                            return _arg.size();
                        }
                        else if constexpr (std::is_same_v<T, zfxstringarr>) {
                            return _arg.size();
                        }
                        else if constexpr (std::is_same_v<T, zfxvec2arr>) {
                            return _arg.size();
                        }
                        else if constexpr (std::is_same_v<T, zfxvec3arr>) {
                            return _arg.size();
                        }
                        else if constexpr (std::is_same_v<T, zfxvec4arr>) {
                            return _arg.size();
                        }
                        else {
                            throw makeError<UnimplError>("only accept arr type on `len`");
                        }
                        },var.value[i]);
                }
                return ret;
            }
            if (funcname == "get_bboxmin") {
                auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                assert(spGeo);
                std::pair<vec3f, vec3f> ret = geomBoundingBox(spGeo->m_impl);
                glm::vec3 bmin(ret.first[0], ret.first[1], ret.first[2]);
                return bmin;
            }
            if (funcname == "get_bboxmax") {
                auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                assert(spGeo);
                std::pair<vec3f, vec3f> ret = geomBoundingBox(spGeo->m_impl);
                glm::vec3 bmax(ret.second[0], ret.second[1], ret.second[2]);
                return bmax;
            }
            if (funcname == "pcopen") {
                std::string inputgeo = get_zfxvar<std::string>(args[0].value[0]);
                std::string attrname = get_zfxvar<std::string>(args[1].value[0]);
                const std::vector<zfxvariant>& testPts = args[2].value;
                float radius = get_zfxvar<float>(args[3].value[0]);
                int maxpoints = get_zfxvar<int>(args[4].value[0]);
                if (attrname != "P") {
                    throw makeError<UnimplError>("only support Pos as the base attribute of point clound");
                }

                std::vector<vec3f> points;
                if (inputgeo == "0") {
                    //this geometry.
                    auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                    assert(spGeo);
                    points = spGeo->m_impl->points_pos();
                    //test:
#if 0
                    std::vector<int> pcnumfind;
                    for (int i = 0; i < testPts.size(); i++) {
                        glm::vec3 _pt = get_zfxvar<glm::vec3>(testPts[i]);
                        zeno::vec3f pt(_pt[0], _pt[1], _pt[2]);
                        std::set<int> pts = kd.fix_radius_search(pt, radius);
                        pcnumfind.push_back(pts.size());
                    }
                    int j;
                    j = 0;
#endif
                }
                else {
                    //todo: other geometry
                }

                PointCloud pc;
                pc.radius = radius;
                pc.maxpoints = maxpoints;
                pc.pTree = std::make_shared<zeno::KdTree>(points, points.size());
                pc.testPoints.resize(testPts.size());
                for (int i = 0; i < pc.testPoints.size(); i++) {
                    glm::vec3 _pt = get_zfxvar<glm::vec3>(testPts[i]);
                    pc.testPoints[i] = zeno::vec3f(_pt[0], _pt[1], _pt[2]);
                }
                int handle = pContext->pchandles.size();
                pContext->pchandles.push_back(pc);
                return handle;
            }
            if (funcname == "pcnumfound") {
                int handle = get_zfxvar<int>(args[0].value[0]);
                if (handle < 0 || handle >= pContext->pchandles.size()) {
                    throw makeError<UnimplError>("invalid pchandle");
                }
                else {
                    PointCloud& pc = pContext->pchandles[handle];
                    std::vector<int> pcnumfind;
                    for (int i = 0; i < pc.testPoints.size(); i++) {
                        zeno::vec3f pt = pc.testPoints[i];
                        std::set<int> pts = pc.pTree->fix_radius_search(pt, pc.radius);
                        pcnumfind.push_back(pts.size());
                    }
                    ZfxVariable ret;
                    for (int i = 0; i < pcnumfind.size(); i++) {
                        ret.value.push_back(pcnumfind[i]);
                    }
                    return ret;
                }
            }
            if (funcname == "primreduce") {
                auto _attrName = get_zfxvar<std::string>(args[0].value[0]);
                zeno::String attrName = stdString2zs(_attrName);
                auto op = get_zfxvar<std::string>(args[1].value[0]);
                auto spGeo = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);
                zeno::GeoAttrType type = spGeo->get_attr_type(ATTR_POINT, attrName);
                zeno::NumericValue result;
                if (zeno::ATTR_FLOAT == type) {
                    std::vector<float> attrData = spGeo->get_float_attr(ATTR_POINT, attrName);
                    result = prim_reduce(attrData, op);
                }
                else if (zeno::ATTR_VEC3 == type) {
                    std::vector<zeno::vec3f> attrData = spGeo->get_vec3f_attr(ATTR_POINT, attrName);
                    result = prim_reduce(attrData, op);
                }
                else {
                    throw makeError<UnimplError>("attr type unknown when calling primreduce");
                }
                ZfxVariable ret;
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int>) {
                        ret.value.push_back(arg);
                    }
                    else if constexpr (std::is_same_v<T, float>) {
                        ret.value.push_back(arg);
                    }
                    else if constexpr (std::is_same_v<T, zeno::vec3f>) {
                        ret.value.push_back(glm::vec3(arg[0], arg[1], arg[2]));
                    }
                    else {
                        throw makeError<UnimplError>("unknown type");
                    }
                }, result);
                return ret;
            }
            if (funcname == "abs") {
                const ZfxVariable& var = args[0];
                const int N = var.value.size();
                ZfxVariable ret;
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    ret.value[i] = std::visit([&](auto&& _arg)->zfxvariant {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return std::abs(_arg);
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            return std::abs(_arg);
                        }
                        else {
                            throw makeError<UnimplError>("only accept int or float on `abs`");
                        }
                        }, var.value[i]);
                }
                return ret;
            }
            if (funcname == "min") {
                const ZfxVariable& var = args[0];
                float cmpval = get_zfxvar<float>(args[1].value[0]);
                //TODO: 目前只考虑一个数值的min
                const int N = var.value.size();
                ZfxVariable ret;
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    ret.value[i] = std::visit([&](auto&& _arg)->zfxvariant {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return zeno::min(cmpval, std::abs(_arg));
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            return zeno::min(cmpval, std::abs(_arg));
                        }
                        else {
                            throw makeError<UnimplError>("only accept int or float on `abs`");
                        }
                        }, var.value[i]);
                }
                return ret;
            }
            if (funcname == "max") {
                const ZfxVariable& var = args[0];
                float cmpval = get_zfxvar<float>(args[1].value[0]);
                //TODO: 目前只考虑一个数值的min
                const int N = var.value.size();
                ZfxVariable ret;
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    ret.value[i] = std::visit([&](auto&& _arg)->zfxvariant {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return zeno::max(cmpval, std::abs(_arg));
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            return zeno::max(cmpval, std::abs(_arg));
                        }
                        else {
                            throw makeError<UnimplError>("only accept int or float on `abs`");
                        }
                        }, var.value[i]);
                }
                return ret;
            }
            if (funcname == "clamp") {
                const ZfxVariable& var = args[0];
                const ZfxVariable& arg_min = args[1], arg_max = args[2];
                float min = 0, max = 0;
                min = get_zfxvar<float>(arg_min.value[0]);
                max = get_zfxvar<float>(arg_max.value[0]);

                const int N = var.value.size();
                ZfxVariable ret;
                ret.value.resize(N);
                for (int i = 0; i < N; i++) {
                    ret.value[i] = std::visit([&](auto&& _arg)->float {
                        using T = std::decay_t<decltype(_arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            return std::min(std::max((float)_arg, min), max);
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            return std::min(std::max(_arg, min), max);
                        }
                        else {
                            throw makeError<UnimplError>("only accept int or float on `clamp`");
                        }
                        }, var.value[i]);
                }
                return ret;
            }
            if (funcname == "setud") {
                zeno::String key = zeno::stdString2zs(get_zfxvar<std::string>(args[0].value[0]));
                auto ud = pContext->spObject->userData();
                const zfxvariant& val = args[1].value[0];

                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int>) {
                        ud->set_int(key, arg);
                    }
                    else if constexpr (std::is_same_v<T, float>) {
                        ud->set_float(key, arg);
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        ud->set_string(key, stdString2zs(arg));
                    }
                    else if constexpr (std::is_same_v<T, glm::vec2>) {
                        ud->set_vec2f(key, Vec2f(arg[0], arg[1]));
                    }
                    else if constexpr (std::is_same_v<T, glm::vec3>) {
                        ud->set_vec3f(key, Vec3f(arg[0], arg[1], arg[2]));
                    }
                    else if constexpr (std::is_same_v<T, glm::vec4>) {
                        //ud->set_vec4f(key, Vec4f(arg[0], arg[1], arg[2], arg[3]));
                    }
                    else {
                        throw makeError<UnimplError>("no supported value to set userdata.");
                    }
                    }, val);
                ZfxVariable ret;
                return ret;
            }
            if (funcname == "getud") {
                const std::string& nodename = get_zfxvar<std::string>(args[0].value[0]);
                const std::string& objparam = get_zfxvar<std::string>(args[1].value[0]);
                const std::string& key = get_zfxvar<std::string>(args[2].value[0]);

                NodeImpl* pObjNode = pContext->spNode->getGraph()->getNode(nodename);
                if (!pObjNode) {
                    throw makeError<UnimplError>("unknown node `" + nodename + "`");
                }
                zany targetObj = pObjNode->get_output_obj(objparam);
                if (!targetObj)
                    throw makeError<UnimplError>("get nullptr obj from `" + objparam + "` when `getud` is called");

                auto ud = targetObj->userData();
                zeno::UserData* pUserData = static_cast<zeno::UserData*>(ud);

                ZfxVariable ret;
                zany spud = pUserData->m_data[key];
                if (auto strobj = std::dynamic_pointer_cast<zeno::StringObject>(spud)) {
                    ret.value.push_back(strobj->get());
                }
                else if (auto numobj = std::dynamic_pointer_cast<zeno::NumericObject>(spud)) {
                    NumericValue val = numobj->value;
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            ret.value.push_back(arg);
                        }
                        else if constexpr (std::is_same_v<T, float>) {
                            ret.value.push_back(arg);
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec2i>) {
                            ret.value.push_back(glm::vec2(arg[0], arg[1]));
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec2f>) {
                            ret.value.push_back(glm::vec2(arg[0], arg[1]));
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec3i>) {
                            ret.value.push_back(glm::vec3(arg[0], arg[1], arg[2]));
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec3f>) {
                            ret.value.push_back(glm::vec3(arg[0], arg[1], arg[2]));
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec4i>) {
                            ret.value.push_back(glm::vec4(arg[0], arg[1], arg[2], arg[3]));
                        }
                        else if constexpr (std::is_same_v<T, zeno::vec4f>) {
                            ret.value.push_back(glm::vec4(arg[0], arg[1], arg[2], arg[3]));
                        }
                        }, val);
                }
                else {
                    throw makeError<UnimplError>("unknown type from userdata");
                }
                return ret;
            }
            throw makeError<UnimplError>("unknown function call `" + funcname + "`");
        }
    }
}