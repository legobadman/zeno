#include <zeno/types/AttributeData.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;
    using namespace zeno::types;

    class AttributeImpl {
    public:
        void set(Any elemVal) {
            m_data = elemVal;
        }

        void set(size_t index, Any elemVal) {
            if (!m_data.has_value() || !elemVal.has_value()) {
                throw makeError<UnimplError>("empty value when set attribute data");
            }
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of array");
            }

            const auto& valType = m_data.type();
            const auto& elemType = elemVal.type().get_decayed_hash();
            if (valType == type_info<int>()) {
                int val = any_cast<int>(m_data);
                if (elemType == gParamType_Int ||
                    elemType == gParamType_Float ||
                    elemType == gParamType_Bool) {
                    val = any_cast<int>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<std::string>()) {
                std::string val = any_cast<std::string>(m_data);
                if (elemType == gParamType_String) {
                    val = any_cast<std::string>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<float>()) {
                float val = any_cast<float>(m_data);
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    val = any_cast<float>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<bool>()) {
                bool val = any_cast<bool>(m_data);
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    val = any_cast<float>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<std::vector<vec3f>>()) {
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec3f(fVal, fVal, fVal);
                }
                else if (elemType == gParamType_Vec3f) {
                    val[index] = any_cast<zeno::vec3f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec2f>>()) {
                std::vector<vec2f>& val = any_cast<std::vector<vec2f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec2f(fVal, fVal);
                }
                else if (elemType == gParamType_Vec2f) {
                    val[index] = any_cast<zeno::vec2f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec4f>>()) {
                std::vector<vec4f>& val = any_cast<std::vector<vec4f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec4f(fVal, fVal, fVal, fVal);
                }
                else if (elemType == gParamType_Vec4f) {
                    val[index] = any_cast<zeno::vec4f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec3i>>()) {
                std::vector<vec3i>& val = any_cast<std::vector<vec3i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec3i(iVal, iVal, iVal);
                }
                else if (elemType == gParamType_Vec3i) {
                    val[index] = any_cast<zeno::vec3i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec2i>>()) {
                std::vector<vec2i>& val = any_cast<std::vector<vec2i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec2i(iVal, iVal);
                }
                else if (elemType == gParamType_Vec2i) {
                    val[index] = any_cast<zeno::vec2i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec4i>>()) {
                std::vector<vec4i>& val = any_cast<std::vector<vec4i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec4i(iVal, iVal);
                }
                else if (elemType == gParamType_Vec4i) {
                    val[index] = any_cast<zeno::vec4i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<std::string>>()) {
                std::vector<std::string>& val = any_cast<std::vector<std::string>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_String) {
                    val[index] = any_cast<std::string>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<float>>()) {
                std::vector<float>& val = any_cast<std::vector<float>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float) {
                    val[index] = any_cast<float>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<int>>()) {
                std::vector<int>& val = any_cast<std::vector<int>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Int) {
                    val[index] = any_cast<int>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else {
                throw makeError<UnimplError>("error type when set attribute value");
            }
        }

        Any get(size_t index) const {
            if (!m_data.has_value()) {
                throw makeError<UnimplError>("empty value on attr");
            }

            const auto& valType = m_data.type();
            if (valType == type_info<int>()) {
                int val = any_cast<int>(m_data);
                return val;
            }
            else if (valType == type_info<std::string>()) {
                std::string val = any_cast<std::string>(m_data);
                return val;
            }
            else if (valType == type_info<float>()) {
                float val = any_cast<float>(m_data);
                return val;
            }
            else if (valType == type_info<bool>()) {
                bool val = any_cast<bool>(m_data);
                return val;
            }
            else if (valType == type_info<std::vector<vec3f>>()) {
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<vec2f>>()) {
                std::vector<vec2f>& val = any_cast<std::vector<vec2f>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<vec4f>>()) {
                std::vector<vec4f>& val = any_cast<std::vector<vec4f>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<vec3i>>()) {
                std::vector<vec3i>& val = any_cast<std::vector<vec3i>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<vec2i>>()) {
                std::vector<vec2i>& val = any_cast<std::vector<vec2i>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<vec4i>>()) {
                std::vector<vec4i>& val = any_cast<std::vector<vec4i>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<std::string>>()) {
                std::vector<std::string>& val = any_cast<std::vector<std::string>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<float>>()) {
                std::vector<float>& val = any_cast<std::vector<float>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else if (valType == type_info<std::vector<int>>()) {
                std::vector<int>& val = any_cast<std::vector<int>&>(m_data);
                assert(m_size == val.size());
                return val[index];
            }
            else {
                throw makeError<UnimplError>("unknown type on attr");
            }
        }

        Any get() const {
            return m_data;
        }

        void insert(size_t index, Any elemVal) {
            if (!m_data.has_value() || !elemVal.has_value()) {
                throw makeError<UnimplError>("empty value when set attribute data");
            }
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of array");
            }

            const auto& valType = m_data.type();
            const auto& elemType = elemVal.type().get_decayed_hash();
            if (valType == type_info<int>()) {
                int val = any_cast<int>(m_data);
                if (elemType == gParamType_Int ||
                    elemType == gParamType_Float ||
                    elemType == gParamType_Bool) {
                    int newVal = any_cast<int>(elemVal);
                    if (newVal != val) {
                        //值不一致，需要改用vector储存了
                        m_data = std::vector<int>(m_size, newVal);
                    }
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<std::string>()) {
                std::string val = any_cast<std::string>(m_data);
                if (elemType == gParamType_String) {
                    std::string newVal = any_cast<std::string>(elemVal);
                    if (newVal != val) {
                        m_data = std::vector<std::string>(m_size, newVal);
                    }
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<float>()) {
                float val = any_cast<float>(m_data);
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float newVal = any_cast<float>(elemVal);
                    if (newVal != val) {
                        m_data = std::vector<float>(m_size, newVal);
                    }
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<bool>()) {
                bool val = any_cast<bool>(m_data);
                if (elemType == gParamType_Bool) {
                    bool newVal = any_cast<bool>(elemVal);
                    if (newVal != val) {
                        m_data = std::vector<int>(m_size, newVal);
                    }
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
                m_data = val;
            }
            else if (valType == type_info<std::vector<vec3f>>()) {
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec3f(fVal, fVal, fVal);
                }
                else if (elemType == gParamType_Vec3f) {
                    val[index] = any_cast<zeno::vec3f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec2f>>()) {
                std::vector<vec2f>& val = any_cast<std::vector<vec2f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec2f(fVal, fVal);
                }
                else if (elemType == gParamType_Vec2f) {
                    val[index] = any_cast<zeno::vec2f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec4f>>()) {
                std::vector<vec4f>& val = any_cast<std::vector<vec4f>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    float fVal = any_cast<float>(elemVal);
                    val[index] = zeno::vec4f(fVal, fVal, fVal, fVal);
                }
                else if (elemType == gParamType_Vec4f) {
                    val[index] = any_cast<zeno::vec4f>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec3i>>()) {
                std::vector<vec3i>& val = any_cast<std::vector<vec3i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec3i(iVal, iVal, iVal);
                }
                else if (elemType == gParamType_Vec3i) {
                    val[index] = any_cast<zeno::vec3i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec2i>>()) {
                std::vector<vec2i>& val = any_cast<std::vector<vec2i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec2i(iVal, iVal);
                }
                else if (elemType == gParamType_Vec2i) {
                    val[index] = any_cast<zeno::vec2i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<vec4i>>()) {
                std::vector<vec4i>& val = any_cast<std::vector<vec4i>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float || elemType == gParamType_Int) {
                    int iVal = any_cast<int>(elemVal);
                    val[index] = zeno::vec4i(iVal, iVal);
                }
                else if (elemType == gParamType_Vec4i) {
                    val[index] = any_cast<zeno::vec4i>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<std::string>>()) {
                std::vector<std::string>& val = any_cast<std::vector<std::string>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_String) {
                    val[index] = any_cast<std::string>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<float>>()) {
                std::vector<float>& val = any_cast<std::vector<float>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Float) {
                    val[index] = any_cast<float>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else if (valType == type_info<std::vector<int>>()) {
                std::vector<int>& val = any_cast<std::vector<int>&>(m_data);
                assert(m_size == val.size());
                if (elemType == gParamType_Int) {
                    val[index] = any_cast<int>(elemVal);
                }
                else {
                    throw makeError<UnimplError>("error type when set attribute value");
                }
            }
            else {
                throw makeError<UnimplError>("error type when set attribute value");
            }
        }

        void remove(size_t index) {
            if (index < 0 || index >= m_size) {
                throw makeError<UnimplError>("index exceed the range of attribute data");
            }
            const auto& valType = m_data.type();
            if (valType == type_info<int>()) {
            }
            else if (valType == type_info<std::string>()) {
            }
            else if (valType == type_info<float>()) {
            }
            else if (valType == type_info<bool>()) {
            }
            else if (valType == type_info<std::vector<vec3f>>()) {
                std::vector<vec3f>& val = any_cast<std::vector<vec3f>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<vec2f>>()) {
                std::vector<vec2f>& val = any_cast<std::vector<vec2f>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<vec4f>>()) {
                std::vector<vec4f>& val = any_cast<std::vector<vec4f>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<vec3i>>()) {
                std::vector<vec3i>& val = any_cast<std::vector<vec3i>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<vec2i>>()) {
                std::vector<vec2i>& val = any_cast<std::vector<vec2i>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<vec4i>>()) {
                std::vector<vec4i>& val = any_cast<std::vector<vec4i>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<std::string>>()) {
                std::vector<std::string>& val = any_cast<std::vector<std::string>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<float>>()) {
                std::vector<float>& val = any_cast<std::vector<float>&>(m_data);
                val.erase(val.begin() + index);
            }
            else if (valType == type_info<std::vector<int>>()) {
                std::vector<int>& val = any_cast<std::vector<int>&>(m_data);
                val.erase(val.begin() + index);
            }
            else {
                throw makeError<UnimplError>("unknown type on attr");
            }
        }

        size_t m_size;
        //暂时不储存核心类型，不过有可能出现类型错误赋值的情况
        Any m_data;
    };

    AttributeData::AttributeData(Any value, size_t size) : m_pImpl(nullptr) {
        m_pImpl = new AttributeImpl;
        m_pImpl->m_data = value;
        m_pImpl->m_size = size;
    }

    AttributeData::AttributeData(const AttributeData& rhs) : m_pImpl(nullptr) {
        if (!m_pImpl)
            m_pImpl = new AttributeImpl;
        m_pImpl->m_data = rhs.m_pImpl->m_data;
        m_pImpl->m_size = rhs.m_pImpl->m_size;
    }

    AttributeData::~AttributeData() {
        delete m_pImpl;
    }

    Any& AttributeData::value() const {
        return m_pImpl->m_data;
    }

    Any AttributeData::get(size_t index) const {
        return m_pImpl->get(index);
    }

    Any AttributeData::get() const {
        return m_pImpl->m_data;
    }

    void AttributeData::set(size_t index, Any val) {
        return m_pImpl->set(index, val);
    }

    void AttributeData::set(const Any& val) {
        return m_pImpl->set(val);
    }

    void AttributeData::insert(size_t index, Any val) {
        return m_pImpl->insert(index, val);
    }

    void AttributeData::remove(size_t index) {
        return m_pImpl->remove(index);
    }

    size_t AttributeData::size() const {
        return m_pImpl->m_size;
    }
}