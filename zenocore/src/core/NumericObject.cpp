#include <zeno/types/NumericObject.h>
#include <zeno/utils/interfaceutil.h>

namespace zeno {

    NumericObject::NumericObject(NumericValue const& value) : value(value) {}

    void NumericObject::Delete() {
        IObjectClone<NumericObject>::Delete();
    }

    NumericObject::~NumericObject() {

    }

    std::string NumericObject::serialize_json() const {
        return std::visit([](auto const& val) -> std::string {
            using V = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<int, V>) {
                return std::to_string(val);
            }
            else if constexpr (std::is_same_v<float, V>) {
                return std::to_string(val);
            }
            else if constexpr (std::is_same_v<zeno::vec2i, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "]";
            }
            else if constexpr (std::is_same_v<zeno::vec2f, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "]";
            }
            else if constexpr (std::is_same_v<zeno::vec3i, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "," + std::to_string(val[2]) + "]";
            }
            else if constexpr (std::is_same_v<zeno::vec3f, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "," + std::to_string(val[2]) + "]";
            }
            else if constexpr (std::is_same_v<zeno::vec4i, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "," + std::to_string(val[2]) + "," + std::to_string(val[3]) + "]";
            }
            else if constexpr (std::is_same_v<zeno::vec4f, V>) {
                return "[" + std::to_string(val[0]) + "," + std::to_string(val[1]) + "," + std::to_string(val[2]) + "," + std::to_string(val[3]) + "]";
            }
            }, value);
    }

}