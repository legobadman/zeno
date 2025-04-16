#include <zeno/types/DictObject.h>
#include <zeno/types/UserData.h>


namespace zeno
{
    DictObject::DictObject() {
    }

    DictObject::DictObject(const DictObject& dictObj) {
        m_key = dictObj.m_key;
        for (auto& [str, obj] : dictObj.lut) {
            auto cloneobj = obj->clone();
            cloneobj->update_key(obj->key());
            lut.insert({ str, cloneobj });
        }
        m_modify = dictObj.m_modify;
        m_new_added = dictObj.m_new_added;
        m_new_removed = dictObj.m_new_removed;
    }

    DictObject::~DictObject() {
        //由于IObject没有虚析构（保证abi兼容），所以一般这里不会走到这，除非是用户强行构造实例，这种情况还是要手动清理
        clear_children();
    }

    void DictObject::clear_children() {
        for (auto const& [key, pObj] : lut) {
            pObj->Delete();
        }
        lut.clear();
    }

    void DictObject::Delete() {
        //这里手动delete所有children
        clear_children();
        delete this;
    }


    ZENO_API zeno::SharedPtr<DictObject> create_DictObject() {
        return std::make_shared<DictObject>();
    }
}