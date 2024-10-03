#include <zeno/types/AttributeData.h>

namespace zeno {

    class AttributeImpl {
    public:
        int m_size;
        zeno::reflect::Any m_data;
    };

    AttributeData::AttributeData(zeno::reflect::Any value, int size) {
        m_pImpl = new AttributeImpl;
        m_pImpl->m_data = value;
        m_pImpl->m_size = size;
    }

    AttributeData::~AttributeData() {
        delete m_pImpl;
    }

    Any& AttributeData::value() const {
        return m_pImpl->m_data;
    }

    Any AttributeData::get(size_t index) const {
        return Any();
    }

    Any AttributeData::get() const {
        return m_pImpl->m_data;
    }

    void AttributeData::set(size_t index, Any val) {

    }

    void AttributeData::set(const Any& val) {

    }

    void AttributeData::insert(size_t index, Any val) {

    }

    void AttributeData::remove(size_t index) {

    }

    size_t AttributeData::size() const {
        return m_pImpl->m_size;
    }
}