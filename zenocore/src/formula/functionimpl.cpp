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
#include <zeno/utils/vectorutil.h>
#include <zeno/core/data.h>


namespace zeno
{
    namespace zfx
    {
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
                        else {
                            throw makeError<UnimplError>("error type on format string");
                        }
                        }, arg);
                }
            );
        }


        ZfxVariable callRef(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>("only support non-attr value when using ref");

            //TODO: vec type.
            //TODO: resolve with zeno::reflect::any
            std::string fullPath, graphAbsPath;
            const std::string ref = get_zfxvar<std::string>(args[0].value[0]);
            if (ref.empty()) {
                throw makeError<UnimplError>();
            }

            float res = 0.f;
            auto thisNode = pContext->spNode.lock();
            const std::string& thisnodePath = thisNode->get_path();
            graphAbsPath = thisnodePath.substr(0, thisnodePath.find_last_of('/'));

            if (ref.front() == '/') {
                fullPath = ref;
            }
            else {
                fullPath = graphAbsPath + "/" + ref;
            }

            int idx = fullPath.find_last_of('/');
            if (idx == std::string::npos) {
                throw makeError<UnimplError>();
            }

            const std::string& nodePath = fullPath.substr(idx + 1);

            idx = nodePath.find('.');
            if (idx == std::string::npos) {
                throw makeError<UnimplError>();
            }
            std::string nodename = nodePath.substr(0, idx);
            std::string parampath = nodePath.substr(idx + 1);

            std::string nodeAbsPath = graphAbsPath + '/' + nodename;
            std::shared_ptr<INode> spNode = zeno::getSession().mainGraph->getNodeByPath(nodeAbsPath);

            if (!spNode) {
                throw makeError<UnimplError>("the refer node doesn't exist, may be deleted before");
            }

            auto items = split_str(parampath, '.');
            std::string paramname = items[0];

            bool bExist = false;
            ParamPrimitive paramData = spNode->get_input_prim_param(paramname, &bExist);
            if (!bExist)
                throw makeError<UnimplError>("the refer param doesn't exist, may be deleted before");

            //直接拿引用源的计算结果，所以本节点在执行前，引用源必须先执行（比如引用源的数值也是依赖另一个节点），参考代码INode::requireInput
            //，所以要在preApply的基础上作提前依赖计算

            //如果取的是“静态值”，则不要求引用源必须执行，此时要拿的数据应该是defl.

            //以上方案取决于产品设计。

            if (items.size() == 1) {
                if (!paramData.result.has_value()) {
                    throw makeError<UnimplError>("there is no result on refer source, should calc the source first.");
                }

                size_t primtype = paramData.result.type().hash_code();
                if (primtype == zeno::types::gParamType_Int) {
                    res = zeno::reflect::any_cast<int>(paramData.result);
                }
                else if (primtype == zeno::types::gParamType_Float) {
                    res = zeno::reflect::any_cast<float>(paramData.result);
                }
                else {
                    throw makeError<UnimplError>();
                }
            }

            if (items.size() == 2 &&
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
        }

        ZfxVariable parameter(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1) {
                throw makeError<UnimplError>("error number of args on param(...)");
            }
            if (pContext->param_constrain.constrain_param.empty()) {
                throw makeError<UnimplError>("only support indexing param for param constrain");
            }
            const std::string& param = get_zfxvar<std::string>(args[0].value[0]);
            auto pnode = pContext->spNode.lock();
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

        ZfxVariable log(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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
                    __args.push_back(__arg.value[0]);
                }
                std::string ret = format_variable_size(formatString.c_str(), __args);
                zeno::log_only_print(ret);
            }

            return ZfxVariable();
        }

        ZfxVariable vec3(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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

        ZfxVariable sin(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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

        ZfxVariable cos(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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

        ZfxVariable sinh(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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

        ZfxVariable cosh(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
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

        ZfxVariable rand(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (!args.empty()) throw makeError<UnimplError>();
            ZfxVariable res;
            res.value.push_back(std::rand());
            return res;
        }

        ZfxVariable addpoint(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() == 1) {
                const auto& arg = args[0];
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject)) {
                    //暂时只考虑一个点
                    const zfxvariant& varpos = arg.value[0];
                    int ptnum = -1;
                    std::visit([&](auto&& val) {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_same_v<T, glm::vec3>) {
                            ptnum = spGeo->add_point(zeno::vec3f(val[0], val[1], val[2]));
                        }
                        else {
                            //TODO: error
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
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject)) {
                    //暂时只考虑一个点
                    int ptnum = spGeo->add_point(zeno::vec3f(0, 0, 0));
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

        ZfxVariable addvertex(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 2) {
                throw makeError<UnimplError>();
            }
            if (auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject)) {
                int faceid = get_zfxvar<int>(args[0].value[0]);
                int pointid = get_zfxvar<int>(args[1].value[0]);
                int vertid = spGeo->add_vertex(faceid, pointid);
                ZfxVariable res;
                res.value.push_back(vertid);
                return res;
            }
            else {
                throw makeError<UnimplError>();
            }
        }

        ZfxVariable removepoint(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 1)
                throw makeError<UnimplError>();
            const auto& arg = args[0];
            int N = arg.value.size();
            if (N == 0) return ZfxVariable();
            bool bSucceed = false;

            if (N < filter.size()) {
                assert(N == 1);
                int currrem = get_zfxvar<int>(arg.value[0]);

                /* 删除pointnum的点，如果成功，就返回原来下一个点的pointnum(应该就是pointnum)，失败就返回-1 */
                if (auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject)) {
                    spGeo->remove_point(currrem);
                    bSucceed = true;
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

                while (!remPoints.empty())
                {
                    int currrem = remPoints.front();
                    remPoints.pop_front();
                    bSucceed = false;
                    if (auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject)) {
                        spGeo->remove_point(currrem);
                        bSucceed = true;
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
            }
            return ZfxVariable();
        }

        ZfxVariable removeface(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() > 2 || args.empty())
                throw makeError<UnimplError>();

            auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject);
            const auto& arg = args[0];
            bool bIncludePoints = true;
            if (args.size() == 2) {
                bIncludePoints = get_zfxvar<bool>(args[1].value[0]);
            }

            int N = arg.value.size();
            if (N == 0) return ZfxVariable();
            bool bSucceed = false;

            std::set<int> remfaces;
            if (N < filter.size()) {
                assert(N == 1);
                int currrem = get_zfxvar<int>(arg.value[0]);
                remfaces.insert(currrem);
            }
            else {
                assert(N == filter.size());
                for (int i = 0; i < N; i++) {
                    if (!filter[i]) continue;
                    int pointnum = get_zfxvar<int>(arg.value[i]);
                    remfaces.insert(pointnum);
                }
            }

            bSucceed = spGeo->remove_faces(remfaces, bIncludePoints);
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
            return ZfxVariable();
        }

        ZfxVariable create_attr(const std::vector<ZfxVariable>& args, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (args.size() != 3)
                throw makeError<UnimplError>("the number of arguments of create_attr is not 3.");
            
            auto spGeo = std::dynamic_pointer_cast<GeometryObject>(pContext->spObject);

            std::string group = get_zfxvar<std::string>(args[0].value[0]);
            std::string name = get_zfxvar<std::string>(args[1].value[0]);

            GeoAttrGroup grp = ATTR_POINT;
            if (group == "vertex") grp = ATTR_VERTEX;
            else if (group == "point") grp = ATTR_POINT;
            else if (group == "face") grp = ATTR_FACE;
            else if (group == "geometry") grp = ATTR_GEO;

            AttrVar defl;

            spGeo->create_attr(grp, name, defl);

            return ZfxVariable();
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
            if (funcname == "addpoint") {
                return addpoint(args, filter, pContext);
            }
            if (funcname == "addvertex") {
                return addvertex(args, filter, pContext);
            }
            if (funcname == "removepoint") {
                return removepoint(args, filter, pContext);
            }
            if (funcname == "removeface") {
                return removeface(args, filter, pContext);
            }
            if (funcname == "create_attr") {
                return create_attr(args, filter, pContext);
            }
            if (funcname == "create_face_attr") {

            }
            if (funcname == "create_point_attr") {

            }
            if (funcname == "create_vertex_attr") {

            }
            if (funcname == "create_geometry_attr") {

            }
            if (funcname == "set_attr") {

            }
            if (funcname == "set_vertex_attr") {

            }
            if (funcname == "set_point_attr") {

            }
            if (funcname == "set_face_attr") {

            }
            if (funcname == "set_geometry_attr") {

            }
            if (funcname == "has_attr") {

            }
            if (funcname == "has_vertex_attr") {

            }
            if (funcname == "has_point_attr") {

            }
            if (funcname == "has_face_attr") {

            }
            if (funcname == "has_geometry_attr") {

            }
            if (funcname == "delete_attr") {

            }
            if (funcname == "delete_vertex_attr") {

            }
            if (funcname == "delete_point_attr") {

            }
            if (funcname == "delete_face_attr") {

            }
            if (funcname == "delete_geometry_attr") {

            }
            if (funcname == "add_vertex") {

            }
            if (funcname == "add_point") {

            }
            if (funcname == "add_face") {

            }
            if (funcname == "remove_face") {

            }
            if (funcname == "remove_point") {

            }
            if (funcname == "remove_vertex") {

            }
            if (funcname == "npoints") {

            }
            if (funcname == "nfaces") {

            }
            if (funcname == "nvertices") {

            }
            if (funcname == "point_faces") {

            }
            if (funcname == "point_vertex") {

            }
            if (funcname == "point_vertices") {

            }
            if (funcname == "face_point") {

            }
            if (funcname == "face_points") {

            }
            if (funcname == "face_vertex") {

            }
            if (funcname == "face_vertex_count") {

            }
            if (funcname == "face_vertices") {

            }
            if (funcname == "point_vertices") {

            }
            if (funcname == "vertex_index") {

            }
            if (funcname == "vertex_next") {

            }
            if (funcname == "vertex_prev") {

            }
            if (funcname == "vertex_point") {

            }
            if (funcname == "vertex_face") {

            }
            if (funcname == "vertex_face_index") {

            }

            throw makeError<UnimplError>("unknown function call");
        }
    }
}