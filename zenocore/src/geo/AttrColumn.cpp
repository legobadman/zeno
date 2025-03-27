#include <zeno/types/AttrColumn.h>


namespace zeno {

    AttrColumn::AttrColumn(AttrVarVec value, size_t size) : m_pImpl(nullptr) {
        m_pImpl = new AttributeImpl;
        m_pImpl->m_data = value;
        m_pImpl->m_size = size;
    }

    AttrColumn::AttrColumn(const AttrColumn& rhs) : m_pImpl(nullptr) {
        if (!m_pImpl)
            m_pImpl = new AttributeImpl;
        m_pImpl->m_data = rhs.m_pImpl->m_data;
        m_pImpl->m_size = rhs.m_pImpl->m_size;
    }

    AttrColumn::~AttrColumn() {
        delete m_pImpl;
    }

    AttrVarVec& AttrColumn::value() const {
        return m_pImpl->m_data;
    }

    AttrVarVec AttrColumn::get() const {
        return m_pImpl->m_data;
    }

    AttrValue AttrColumn::front() const {
        return m_pImpl->front();
    }

    void AttrColumn::set(const AttrVarVec& val) {
        return m_pImpl->set(val);
    }

    void AttrColumn::remove(size_t index) {
        return m_pImpl->remove(index);
    }

    size_t AttrColumn::size() const {
        return m_pImpl->m_size;
    }
}