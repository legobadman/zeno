#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/common.h>
#include <zeno/types/UserData.h>
#include <set>

namespace zeno {

struct ZENO_API ListObject : public IListObject
{
public:
    ListObject();
    ListObject(const ListObject& rhs);
    ~ListObject();

    //IObject2
    ZObjectType type() const override {
        return ZObj_List;
    }
    IObject2* clone() const override {
        return new ListObject(*this);
    }
    size_t key(char* buf, size_t buf_size) const override
    {
        const char* s = m_key.c_str();
        size_t len = m_key.size();   // 不含 '\0'
        if (buf && buf_size > 0) {
            size_t copy = (len < buf_size - 1) ? len : (buf_size - 1);
            memcpy(buf, s, copy);
            buf[copy] = '\0';
        }
        return len;
    }
    void update_key(const char* key) override;
    size_t serialize_json(char* buf, size_t buf_size) const override {
        return 0;
    }
    IUserData2* userData() override { return &m_userDat; }
    void Delete() override {
        delete this;
    }
    ListObject& operator=(const ListObject& rhs) = delete;

private:
    std::string m_key;
    UserData m_userDat;

public:
    size_t size() const override;
    void resize(size_t sz) override;
    void clear() override;
    IObject2* get(size_t index) const override;
    void push_back(IObject2* detached_obj) override;
    void set(size_t index, IObject2* detached_obj) override;

    size_t get_items(IObject2** buf, size_t cap) const override;
    size_t get_int_arr(int* buf, size_t cap) const override;
    size_t get_float_arr(float* buf, size_t cap) const override;
    size_t get_string_arr(char** buf, size_t cap) const override;

    std::vector<IObject2*> get();
    //void set(const zeno::Vector<zany2>& arr);
    void set_obj(size_t index, zany2&& obj);
    bool has_change_info() const;
    void clear_crud_info();
    bool empty() const;

    template <class T = IObject2>
    std::vector<T*> get() const {
        std::vector<T*> res;
        for (auto const& val : m_objects) {
            res.push_back(dynamic_cast<T*>(val.get()));
        }
        return res;
    }

    template <class T = IObject2>
    std::vector<T*> getRaw() const {
        std::vector<T*> res;
        for (auto const& val : m_objects) {
            res.push_back(safe_dynamic_cast<T>(val.get()));
        }
        return res;
    }

    void push_back2(zany2&& detached_obj);

    std::set<std::string> m_modify, m_new_added, m_new_removed; //一次计算中发生变化的元素记录。
    std::vector<zany2> m_objects;
    std::set<int> dirtyIndice;
};

ZENO_API std::unique_ptr<ListObject> create_ListObject();
ZENO_API std::unique_ptr<ListObject> create_ListObject(std::vector<zany2>&& arrin);

}
