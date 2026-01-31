#pragma once


#include <iobject2.h>
#include <glm/vec3.hpp>
#include <vector>

namespace zeno {

struct ParticlesObject : IObject2 {
  
  std::vector<glm::vec3> pos;
  std::vector<glm::vec3> vel;

  size_t size() const {
    return pos.size();
  }

  IObject2* clone() const override {
      return new ParticlesObject(*this);
  }
  ZObjectType type() const override {
      return ZObj_Geometry;
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
      //delete this;
  }
private:
    std::string m_key;
    UserData m_userDat;
};

}
