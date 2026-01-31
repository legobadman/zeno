#pragma once

#include <cstddef>
#include <memory>
#include <iobject2.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>

namespace zeno {

struct MeshObject : IObject2 {
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  size_t size()
  {
    return vertices.size();
  }
  void translate(const glm::vec3 &p)
  {
    #pragma omp parallel for
    for(int i=0;i<vertices.size();i++)
    {
      vertices[i] += p;
    }
  }
  std::shared_ptr<MeshObject> Clone()
  {
    std::shared_ptr<MeshObject> omesh = std::make_shared<MeshObject>();
    omesh->vertices.resize(vertices.size());
    omesh->uvs.resize(uvs.size());
    omesh->normals.resize(normals.size());
    #pragma omp parallel for
    for(int i=0;i<vertices.size();i++)
    {
      omesh->vertices[i] = vertices[i];
      omesh->uvs[i] = uvs[i];
      omesh->normals[i] = normals[i];
    }
    return omesh;
  }

  IObject2* clone() const override {
      return new MeshObject(*this);
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


