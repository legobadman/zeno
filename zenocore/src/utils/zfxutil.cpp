#include "zfxutil.h"
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <zeno/utils/string.h>
#include <zeno/types/IGeometryObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno
{
    using namespace zeno::reflect;

    namespace zfx
    {
        AttrVar zfxvarToAttrvar(const zfxvariant& var) {
            return std::visit([&](auto&& arg)->AttrVar {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, float>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    return zeno::vec2f(arg[0], arg[1]);
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    return zeno::vec3f(arg[0], arg[1], arg[2]);
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    return zeno::vec3f(arg[0], arg[1], arg[2], arg[3]);
                }
                else if constexpr (std::is_same_v<T, zfxintarr>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, zfxfloatarr>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, zfxstringarr>) {
                    return arg;
                }
                else {
                    throw makeError<UnimplError>("unsupport type to arg value");
                }
            }, var);
        }

        zeno::reflect::Any zfxvarToAny(const zfxvariant& var)
        {
            return std::visit([&](auto&& arg)->zeno::reflect::Any {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, float>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    return zeno::vec2f(arg[0], arg[1]);
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    return zeno::vec3f(arg[0], arg[1], arg[2]);
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    return zeno::vec3f(arg[0], arg[1], arg[2], arg[3]);
                }
                else if constexpr (std::is_same_v<T, zfxintarr>) {
                    throw makeError<UnimplError>("unsupport type to arg value");
                }
                else if constexpr (std::is_same_v<T, zfxfloatarr>) {
                    throw makeError<UnimplError>("unsupport type to arg value");
                }
                else if constexpr (std::is_same_v<T, zfxstringarr>) {
                    throw makeError<UnimplError>("unsupport type to arg value");
                }
                else {
                    throw makeError<UnimplError>("unsupport type to arg value");
                }
            }, var);
        }

        std::vector<zfxvariant> attrvarVecToZfxVec(AttrVarVec anyval, int size) {
            std::vector<zfxvariant> ret;
            return ret;
        }

        std::vector<zfxvariant> extractAttrValue(Any anyval, int size)
        {
            std::vector<zfxvariant> res;

            if (!anyval.has_value()) {
                throw makeError<UnimplError>("empty value on attr");
            }

            const auto& valType = anyval.type();
            if (valType == type_info<int>()) {
                int val = any_cast<int>(anyval);
                res = std::vector<zfxvariant>(size, val);
            }
            else if (valType == type_info<std::string>()) {
                std::string val = any_cast<std::string>(anyval);
                res = std::vector<zfxvariant>(size, val);
            }
            else if (valType == type_info<float>()) {
                float val = any_cast<float>(anyval);
                res = std::vector<zfxvariant>(size, val);
            }
            else if (valType == type_info<bool>()) {
                bool val = any_cast<bool>(anyval);
                res = std::vector<zfxvariant>(size, val);
            }
            else if (valType == type_info<std::vector<vec3f>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<vec2f>>()) {
                res.resize(size);
                std::vector<vec2f>& val = any_cast<std::vector<vec2f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec2(item[0], item[1]);
                }
            }
            else if (valType == type_info<std::vector<vec4f>>()) {
                res.resize(size);
                std::vector<vec4f>& val = any_cast<std::vector<vec4f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec4(item[0], item[1], item[2], item[3]);
                }
            }
            else if (valType == type_info<std::vector<vec3i>>()) {
                res.resize(size);
                std::vector<vec3i>& val = any_cast<std::vector<vec3i>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<vec2i>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<vec4i>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<std::string>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<float>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else if (valType == type_info<std::vector<int>>()) {
                res.resize(size);
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(anyval);
                assert(size == val.size());
                for (int i = 0; i < size; i++) {
                    auto& item = val[i];
                    res[i] = glm::vec3(item[0], item[1], item[2]);
                }
            }
            else {
                throw makeError<UnimplError>("unknown type on attr");
            }
            return res;
        }

        AttrVar getInitValueFromVariant(const std::vector<zfxvariant>& zfxvec)
        {
            assert(!zfxvec.empty());
            return std::visit([&](auto&& val)->AttrVar {
                using E = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<E, int>) {
                    return 0;
                }
                else if constexpr (std::is_same_v<E, float>) {
                    return 0.0f;
                }
                else if constexpr (std::is_same_v<E, std::string>) {
                    return "";
                }
                else if constexpr (std::is_same_v<E, glm::vec3>) {
                    return glm::vec3(0, 0, 0);
                }
                else if constexpr (std::is_same_v<E, glm::vec2>) {
                    return glm::vec2(0, 0);
                }
                else if constexpr (std::is_same_v<E, glm::vec4>) {
                    return glm::vec4(0, 0, 0, 0);
                }
                else {
                    throw makeError<UnimplError>("Unsupport type for converting to AttrVar");
                }
            }, zfxvec[0]);
        }

        AttrVar convertToAttrVar(const std::vector<zfxvariant>& zfxvec)
        {
            AttrVar wtf;
            assert(!zfxvec.empty());
            return std::visit([&](auto&& val)->AttrVar {
                using E = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<E, int>) {
                    if (zfxvec.size() == 1) {
                        return val;
                    }
                    else {
                        std::vector<int> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            vec[i] = std::get<int>(zfxvec[i]);
                        }
                        return vec;
                    }
                }
                else if constexpr (std::is_same_v<E, float>) {
                    if (zfxvec.size() == 1) {
                        return val;
                    }
                    else {
                        std::vector<float> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            vec[i] = std::get<float>(zfxvec[i]);
                        }
                        return vec;
                    }
                }
                else if constexpr (std::is_same_v<E, std::string>) {
                    if (zfxvec.size() == 1) {
                        return val;
                    }
                    else {
                        std::vector<std::string> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            vec[i] = std::get<std::string>(zfxvec[i]);
                        }
                        return vec;
                    }
                }
                else if constexpr (std::is_same_v<E, glm::vec3>) {
                    if (zfxvec.size() == 1) {
                        return zeno::vec3f(val[0], val[1], val[2]);
                    }
                    else {
                        std::vector<zeno::vec3f> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            glm::vec3 v = std::get<glm::vec3>(zfxvec[i]);
                            vec[i] = zeno::vec3f(v[0], v[1], v[2]);
                        }
                        return vec;
                    }
                }
                else if constexpr (std::is_same_v<E, glm::vec2>) {
                    if (zfxvec.size() == 1) {
                        return zeno::vec2f(val[0], val[1]);
                    }
                    else {
                        std::vector<zeno::vec2f> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            glm::vec2 v = std::get<glm::vec2>(zfxvec[i]);
                            vec[i] = zeno::vec2f(v[0], v[1]);
                        }
                        return vec;
                    }
                }
                else if constexpr (std::is_same_v<E, glm::vec4>) {
                    if (zfxvec.size() == 1) {
                        return zeno::vec4f(val[0], val[1], val[2], val[3]);
                    }
                    else {
                        std::vector<zeno::vec4f> vec(zfxvec.size());
                        for (int i = 0; i < zfxvec.size(); i++) {
                            glm::vec4 v = std::get<glm::vec4>(zfxvec[i]);
                            vec[i] = zeno::vec4f(v[0], v[1], v[2]);
                        }
                        return vec;
                    }
                }
                else {
                    throw makeError<UnimplError>("Unsupport type for converting to AttrVar");
                }
            }, zfxvec[0]);
        }

        zeno::ZfxVariable getAttrValue(const std::string& attrname, ZfxContext* pContext, char channel) {
            std::string attr_name = attrname;
            if (attrname[0] == '@')
                attr_name = attrname.substr(1);

            auto spGeom = std::dynamic_pointer_cast<zeno::GeometryObject_Adapter>(pContext->spObject);

            //观察是否内置属性，目前内置属性统统在外部处理，不耦合到GeometryObject的api上
            if (attr_name == "ptnum") {
                int N = spGeom->npoints();
                ZfxVariable res;
                res.value.resize(N);
                for (int i = 0; i < N; i++) {
                    res.value[i] = i;
                }
                res.bAttr = true;
                return res;
            }
            else if (attr_name == "facenum") {
                int N = spGeom->nfaces();
                ZfxVariable res;
                res.value.resize(N);
                for (int i = 0; i < N; i++) {
                    res.value[i] = i;
                }
                res.bAttr = true;
                return res;
            }
            else if (attr_name == "vtxnum") {
                int N = spGeom->nvertices();
                ZfxVariable res;
                res.value.resize(N);
                for (int i = 0; i < N; i++) {
                    res.value[i] = i;
                }
                res.bAttr = true;
                return res;
            }
            else if (attr_name == "pscale") {

            }
            else if (attr_name == "Frame") {

            }
            else if (attr_name == "P") {
                attr_name = "pos";
            }

            GeoAttrType type = spGeom->m_impl->get_attr_type(pContext->runover, attr_name);
            if (type == ATTR_TYPE_UNKNOWN) {//attrname可能是其他类型,尝试设置为其他类型
                if (spGeom->m_impl->has_point_attr(attr_name)) {
                    pContext->runover = ATTR_POINT;
                    type = spGeom->m_impl->get_attr_type(ATTR_POINT, attr_name);
                }
                else if (spGeom->m_impl->has_face_attr(attr_name)) {
                    pContext->runover = ATTR_FACE;
                    type = spGeom->m_impl->get_attr_type(ATTR_FACE, attr_name);
                }
                else if (spGeom->m_impl->has_vertex_attr(attr_name)) {
                    pContext->runover = ATTR_VERTEX;
                    type = spGeom->m_impl->get_attr_type(ATTR_VERTEX, attr_name);
                }
                else if (spGeom->m_impl->has_geometry_attr(attr_name)) {
                    pContext->runover = ATTR_GEO;
                    type = spGeom->m_impl->get_attr_type(ATTR_GEO, attr_name);
                }
            }

            switch (type) {
            case ATTR_INT: {
                std::vector<int> vec(spGeom->m_impl->get_attrs<int>(pContext->runover, attr_name));
                ZfxVariable res;
                res.value.resize(vec.size());
                res.bAttr = true;
                for (int i = 0; i < vec.size(); i++) {
                    res.value[i] = vec[i];
                }
                return res;
            }
            case ATTR_FLOAT: {
                std::vector<float> vec(spGeom->m_impl->get_attrs<float>(pContext->runover, attr_name));
                ZfxVariable res;
                res.value.resize(vec.size());
                res.bAttr = true;
                for (int i = 0; i < vec.size(); i++) {
                    res.value[i] = vec[i];
                }
                return res;
            }
            case ATTR_STRING: {
                std::vector<std::string> vec(spGeom->m_impl->get_attrs<std::string>(pContext->runover, attr_name));
                ZfxVariable res;
                res.value.resize(vec.size());
                res.bAttr = true;
                for (int i = 0; i < vec.size(); i++) {
                    res.value[i] = vec[i];
                }
                return res;
            }
            case ATTR_VEC2: {
                std::vector<glm::vec2> vec(spGeom->m_impl->get_attrs<glm::vec2>(pContext->runover, attr_name));
                ZfxVariable res;
                res.value.resize(vec.size());
                res.bAttr = true;
                for (int i = 0; i < vec.size(); i++) {
                    res.value[i] = vec[i];
                }
                return res;
            }
            case ATTR_VEC3: {
                if (channel == 'x') {
                    std::vector<float> vec(spGeom->m_impl->get_attrs<float, 'x'>(pContext->runover, attr_name));
                    ZfxVariable res;
                    res.value.resize(vec.size());
                    res.bAttr = true;
                    for (int i = 0; i < vec.size(); i++) {
                        res.value[i] = vec[i];
                    }
                    return res;
                }
                else if (channel == 'y') {
                    std::vector<float> vec(spGeom->m_impl->get_attrs<float, 'y'>(pContext->runover, attr_name));
                    ZfxVariable res;
                    res.value.resize(vec.size());
                    res.bAttr = true;
                    for (int i = 0; i < vec.size(); i++) {
                        res.value[i] = vec[i];
                    }
                    return res;
                }
                else if (channel == 'z') {
                    std::vector<float> vec(spGeom->m_impl->get_attrs<float, 'z'>(pContext->runover, attr_name));
                    ZfxVariable res;
                    res.value.resize(vec.size());
                    res.bAttr = true;
                    for (int i = 0; i < vec.size(); i++) {
                        res.value[i] = vec[i];
                    }
                    return res;
                }
                else {
                    assert(channel == 0);
                    std::vector<glm::vec3> vec(spGeom->m_impl->get_attrs<glm::vec3>(pContext->runover, attr_name));
                    ZfxVariable res;
                    res.value.resize(vec.size());
                    res.bAttr = true;
                    for (int i = 0; i < vec.size(); i++) {
                        res.value[i] = vec[i];
                    }
                    return res;
                }
            }
            case ATTR_VEC4: {
                std::vector<glm::vec4> vec(spGeom->m_impl->get_attrs<glm::vec4>(pContext->runover, attr_name));
                ZfxVariable res;
                res.value.resize(vec.size());
                res.bAttr = true;
                for (int i = 0; i < vec.size(); i++) {
                    res.value[i] = vec[i];
                }
                return res;
            }
            default:
                throw makeError<UnimplError>("unknown attr type");
            }
        }


        template <class T>
        static T get_zfxvar(zfxvariant value) {
            return std::visit([](auto const& val) -> T {
                using V = std::decay_t<decltype(val)>;
                if constexpr (!std::is_constructible_v<T, V>) {
                    throw makeError<TypeError>(typeid(T), typeid(V), "get<zfxvariant>");
                }
                else {
                    return T(val);
                }
            }, value);
        }

        template<class ElemType>
        static void set_attr_by_zfx(
                std::shared_ptr<GeometryObject_Adapter> spGeom,
                std::string attrname,
                std::string channel,
                const ZfxVariable& var,
                operatorVals opVal,
                const ZfxElemFilter& filter,
                ZfxContext* pContext)
        {
            GeoAttrGroup group = pContext->runover;
            int N = spGeom->m_impl->get_group_count(group);
            int nVariable = var.value.size();
            if (N != nVariable && nVariable != 1) {
                throw makeError<UnimplError>("size dismatch when assign value to attributes");
            }

            std::vector<ElemType> vec(N);
            for (int i = 0; i < N; i++) {
                vec[i] = get_zfxvar<ElemType>(var.value[std::min(i, nVariable - 1)]);
            }

            bool bAttrExist = spGeom->m_impl->has_attr(group, attrname);
            if (bAttrExist) {
                if constexpr (std::is_same_v<ElemType, std::string>) {
                    int N = spGeom->m_impl->get_group_count(pContext->runover), nVariable = var.value.size();
                    if (N != nVariable && nVariable != 1) {
                        throw makeError<UnimplError>("size dismatch when assign value to attributes");
                    }
                    std::vector<std::string> vec(N);
                    for (int i = 0; i < N; i++) {
                        vec[i] = get_zfxvar<std::string>(var.value[std::min(i, nVariable - 1)]);
                    }
                    spGeom->m_impl->foreach_attr_update<std::string>(pContext->runover, attrname, 0, [&](int idx, std::string old_val)->std::string {
                        return filter[idx] ? vec[idx] : old_val;
                        });
                }
                else {
                    char chn = 0;
                    if (!channel.empty()) {
                        chn = channel[0];
                    }
                    int chnidx = chn - 'x';
                    if (opVal == AddAssign) {
                        if (chn != 0) {
                            if constexpr (std::is_same_v<ElemType, glm::vec2> || std::is_same_v<ElemType, glm::vec3> || std::is_same_v<ElemType, glm::vec4>) {
                                spGeom->m_impl->foreach_attr_update<float>(group, attrname, chn, [&](int idx, float old_val)->float {
                                    //单值？
                                    return filter[idx] ? (old_val + vec[idx][chnidx]) : old_val;
                                    });
                            }
                        }
                        else {
                            spGeom->m_impl->foreach_attr_update<ElemType>(group, attrname, chn, [&](int idx, ElemType old_val)->ElemType {
                                //单值？
                                return filter[idx] ? (old_val + vec[idx]) : old_val;
                                });
                        }

                    }
                    else if (opVal == SubAssign) {
                        if (chn != 0) {
                            if constexpr (std::is_same_v<ElemType, glm::vec2> || std::is_same_v<ElemType, glm::vec3> || std::is_same_v<ElemType, glm::vec4>) {
                                spGeom->m_impl->foreach_attr_update<float>(group, attrname, chn, [&](int idx, float old_val)->float {
                                    return filter[idx] ? (old_val - vec[idx][chnidx]) : old_val;
                                    });
                            }
                        }
                        else {
                            spGeom->m_impl->foreach_attr_update<ElemType>(group, attrname, chn, [&](int idx, ElemType old_val)->ElemType {
                                return filter[idx] ? (old_val - vec[idx]) : old_val;
                                });
                        }
                    }
                    else if (opVal == MulAssign) {
                        if (chn != 0) {
                            if constexpr (std::is_same_v<ElemType, glm::vec2> || std::is_same_v<ElemType, glm::vec3> || std::is_same_v<ElemType, glm::vec4>) {
                                spGeom->m_impl->foreach_attr_update<float>(group, attrname, chn, [&](int idx, float old_val)->float {
                                    return filter[idx] ? (old_val * vec[idx][chnidx]) : old_val;
                                    });
                            }
                        }
                        else {
                            spGeom->m_impl->foreach_attr_update<ElemType>(group, attrname, chn, [&](int idx, ElemType old_val)->ElemType {
                                return filter[idx] ? (old_val * vec[idx]) : old_val;
                                });
                        }
                    }
                    else if (opVal == DivAssign) {
                        if (chn != 0) {
                            if constexpr (std::is_same_v<ElemType, glm::vec2> || std::is_same_v<ElemType, glm::vec3> || std::is_same_v<ElemType, glm::vec4>) {
                                spGeom->m_impl->foreach_attr_update<float>(group, attrname, chn, [&](int idx, float old_val)->float {
                                    return filter[idx] ? (old_val / vec[idx][chnidx]) : old_val;
                                    });
                            }
                        }
                        else {
                            spGeom->m_impl->foreach_attr_update<ElemType>(group, attrname, chn, [&](int idx, ElemType old_val)->ElemType {
                                return filter[idx] ? (old_val / vec[idx]) : old_val;
                                });
                        }
                    }
                    else {
                        if (chn != 0) {
                            if constexpr (std::is_same_v<ElemType, glm::vec2> || std::is_same_v<ElemType, glm::vec3> || std::is_same_v<ElemType, glm::vec4>) {
                                spGeom->m_impl->foreach_attr_update<float>(group, attrname, chn, [&](int idx, float old_val)->float {
                                    return filter[idx] ? vec[idx][chnidx] : old_val;
                                    });
                            }
                        }
                        else {
                            spGeom->m_impl->foreach_attr_update<ElemType>(group, attrname, chn, [&](int idx, ElemType old_val)->ElemType {
                                return filter[idx] ? vec[idx] : old_val;
                                });
                        }
                    }
                }
            }
            else {
                //创建反正都需要整个创建，暂时不用filter，（或者filter掉的部分元素为0)
                spGeom->m_impl->create_attr(group, attrname, vec);
            }
        }

        void setAttrValue(std::string attrname, const std::string& channel, const ZfxVariable& var, operatorVals opVal, ZfxElemFilter& filter, ZfxContext* pContext) {
            if (attrname[0] == '@')
                attrname = attrname.substr(1);

            if (attrname == "P") {
                attrname = "pos";
            }

            auto spGeom = std::dynamic_pointer_cast<GeometryObject_Adapter>(pContext->spObject);

            GeoAttrGroup group = pContext->runover;


            if (!channel.empty() && channel != "x" && channel != "y" && channel != "z" && channel != "w") {
                throw makeError<UnimplError>("Unknown channel name");
            }

            GeoAttrType type = spGeom->m_impl->get_attr_type(group, attrname);
            if (type == ATTR_TYPE_UNKNOWN) {//attrname可能是其他类型,尝试设置为其他类型
                if (spGeom->m_impl->has_point_attr(attrname)) {
                    type = spGeom->m_impl->get_attr_type(ATTR_POINT, attrname);
                }
                else if (spGeom->m_impl->has_face_attr(attrname)) {
                    type = spGeom->m_impl->get_attr_type(ATTR_FACE, attrname);
                }
                else if (spGeom->m_impl->has_vertex_attr(attrname)) {
                    type = spGeom->m_impl->get_attr_type(ATTR_VERTEX, attrname);
                }
                else if (spGeom->m_impl->has_geometry_attr(attrname)) {
                    type = spGeom->m_impl->get_attr_type(ATTR_GEO, attrname);
                }
            }
            switch (type) {
            case ATTR_INT: {
                set_attr_by_zfx<int>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            case ATTR_FLOAT: {
                set_attr_by_zfx<float>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            case ATTR_STRING: {
                set_attr_by_zfx<std::string>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            case ATTR_VEC2: {
                set_attr_by_zfx<glm::vec2>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            case ATTR_VEC3: {
                set_attr_by_zfx<glm::vec3>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            case ATTR_VEC4: {
                set_attr_by_zfx<glm::vec4>(spGeom, attrname, channel, var, opVal, filter, pContext);
                break;
            }
            default: {
                throw makeError<UnimplError>("Unknown type of attribute when set value");
            }
            }
        }

        NodeImpl* getNodeAndParamFromRefString(
            const std::string& ref, 
            ZfxContext* pContext,
            std::string& paramName,
            std::string& paramPath)
        {
            if (ref.empty()) {
                zeno::log_warn("ref empty");
            }
            std::string fullPath, graphAbsPath;
            auto thisNode = pContext->spNode;
            const std::string& thisnodePath = thisNode->get_path();
            graphAbsPath = thisnodePath.substr(0, thisnodePath.find_last_of('/'));

            if (ref.front() == '/') {
                fullPath = ref;
            }
            else {
                fullPath = graphAbsPath + "/" + ref;
            }

            size_t idx = fullPath.find_last_of('/');
            if (idx == std::string::npos) {
                zeno::log_warn("unresolve node");
            }

            const std::string& graph_correct_path = fullPath.substr(0, idx);
            //nodePrmPath可能是如下情况：
            //1. 节点名称.参数：  Cube1.Size.y  Tube1.Height
            //2. 参数名称.[通道]:  height  Size.y
            const std::string& nodePrmPath = fullPath.substr(idx + 1);

            idx = nodePrmPath.find('.');
            if (idx == std::string::npos) {
                zeno::log_warn("no param name when resolve ref path");
            }

            NodeImpl* pNodeImpl;

            {
                //尝试按照 节点名称.参数的方式进行解析
                std::string nodename = nodePrmPath.substr(0, idx);
                paramPath = nodePrmPath.substr(idx + 1);
                std::string nodeAbsPath = graph_correct_path + '/' + nodename;
                pNodeImpl = zeno::getSession().mainGraph->getNodeByPath(nodeAbsPath);
                if (!pNodeImpl) {
                    //找不到节点，说明可能是当前节点
                    pNodeImpl = pContext->spNode;
                    paramPath = nodePrmPath;    //这个路径可能也是一个不合法的路径
                }
            }

            auto paramPathItems = split_str(paramPath, '.');
            paramName = paramPathItems.empty() ? paramPath : paramPathItems[0];
            return pNodeImpl;
        }
    }
}