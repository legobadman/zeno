#include "zfxutil.h"
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno
{
    using namespace zeno::reflect;

    namespace zfx
    {
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
    }
}