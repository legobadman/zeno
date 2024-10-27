#include "zfxutil.h"
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
                    throw UnimplError("Unsupport type for converting to AttrVar");
                }
            }, zfxvec[0]);
        }
    }
}