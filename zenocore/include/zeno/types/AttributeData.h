#pragma once

#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/core/data.h>


namespace zeno {

    using namespace zeno::reflect;

    class AttributeImpl;

    class AttributeData {
    public:
        AttributeData() = delete;
        AttributeData(const AttributeData& rhs) = delete;
        AttributeData(Any value, int size);
        ~AttributeData();
        Any& value() const;
        Any get(size_t index) const;
        Any get() const;
        void set(size_t index, Any val);
        void set(const Any& val);
        void insert(size_t index, Any val);
        void remove(size_t index);
        size_t size() const;

    private:
        AttributeImpl* m_pImpl;
    };

}