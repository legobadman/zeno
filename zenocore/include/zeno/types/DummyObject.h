#pragma once

#include <iobject2.h>
#include <zeno/types/UserData.h>

namespace zeno {

struct DummyObject : IObject2 {
    /* nothing, just a empty object for fake static view object stubs */
public: //IObject2
    IObject2* clone() const override {
        return new DummyObject(*this);
    }
    ZObjectType type() const override {
        return ZObj_Dummy;
    }
    size_t key(char* buf, size_t buf_size) const override
    {
        const char* s = m_key.c_str();
        size_t len = m_key.size();   // ²»º¬ '\0'
        if (buf && buf_size > 0) {
            size_t copy = (len < buf_size - 1) ? len : (buf_size - 1);
            memcpy(buf, s, copy);
            buf[copy] = '\0';
        }
        return len;
    }
    void update_key(const char* key) override {
        m_key = key;
    }
    size_t serialize_json(char* buf, size_t buf_size) const override {
        return 0;
    }
    IUserData2* userData() override {
        return &m_userDat;
    }
    void Delete() override {
        delete this;
    }
private:
    std::string m_key;
    UserData m_userDat;
};

}
