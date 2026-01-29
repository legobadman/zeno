#ifndef __IOBJECT2_H__
#define __IOBJECT2_H__

#include <cstddef>
#include <zvec.h>

namespace zeno {

struct IUserData2 {
    virtual IUserData2* clone() = 0;
    //virtual ~IUserData() = default; //虚析构函数在vtbl中的位置随着编译器的不同而不同

    virtual bool has(const char* key) = 0;
    virtual size_t size() const = 0;

    virtual size_t get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const = 0;
    virtual void set_string(const char* key, const char* sval) = 0;
    virtual bool has_string(const char* key) const = 0;

    virtual int get_int(const char* key, int defl = 0) const = 0;
    virtual void set_int(const char* key, int iVal) = 0;
    virtual bool has_int(const char* key) const = 0;

    virtual float get_float(const char* key, float defl = 0.f) const = 0;
    virtual void set_float(const char* key, float fVal) = 0;
    virtual bool has_float(const char* key) const = 0;

    virtual Vec2f get_vec2f(const char* key, Vec2f defl = Vec2f()) const = 0;
    virtual void set_vec2f(const char* key, const Vec2f& vec) = 0;
    virtual bool has_vec2f(const char* key) const = 0;

    virtual Vec2i get_vec2i(const char* key) const = 0;
    virtual void set_vec2i(const char* key, const Vec2i& vec) = 0;
    virtual bool has_vec2i(const char* key) const = 0;

    virtual Vec3f get_vec3f(const char* key, Vec3f defl = Vec3f()) const = 0;
    virtual void set_vec3f(const char* key, const Vec3f& vec) = 0;
    virtual bool has_vec3f(const char* key) const = 0;

    virtual Vec3i get_vec3i(const char* key) const = 0;
    virtual void set_vec3i(const char* key, const Vec3i& vec) = 0;
    virtual bool has_vec3i(const char* key) const = 0;

    virtual Vec4f get_vec4f(const char* key) const = 0;
    virtual void set_vec4f(const char* key, const Vec4f& vec) = 0;
    virtual bool has_vec4f(const char* key) const = 0;

    virtual Vec4i get_vec4i(const char* key) const = 0;
    virtual void set_vec4i(const char* key, const Vec4i& vec) = 0;
    virtual bool has_vec4i(const char* key) const = 0;

    virtual bool get_bool(const char* key, bool defl = false) const = 0;
    virtual void set_bool(const char* key, bool val = false) = 0;
    virtual bool has_bool(const char* key) const = 0;

    virtual void del(const char* key) = 0;
};

struct IObject2 {
    virtual IObject2* clone() const = 0; //TODO：abi compatible for shared_ptr
    virtual size_t key(char* buf, size_t buf_size) const = 0;
    virtual void update_key(const char* key) = 0;
    virtual size_t serialize_json(char* buf, size_t buf_size) const = 0;
    virtual IUserData2* userData() = 0;
    virtual void Delete() = 0;  //TODO: for abi compatiblity when dtor cann't be mark virutal.
};

struct IGeometryObject : IObject2 {
    virtual size_t points_pos(Vec3f* buf, size_t buf_size) = 0;
    virtual GeomTopoType topo_type() const = 0;

    virtual int add_vertex(int face_id, int point_id) = 0;
    virtual int add_point(Vec3f pos) = 0;
    virtual int add_face(int* points, size_t size, bool bClose = true) = 0;

    virtual int create_attr(
        GeoAttrGroup grp,
        const char* attr_name,
        const ZAttrValue& val
        ) = 0;

    virtual int create_attr_by_vec3(
        GeoAttrGroup grp,
        const char* attr_name,
        const Vec3f* arr,
        size_t size
        ) = 0;

    virtual int create_attr_by_float(
        GeoAttrGroup grp,
        const char* attr_name,
        const float* arr,
        size_t size
        ) = 0;

    virtual int set_attr2(
        GeoAttrGroup grp,
        const char* attr_name,
        const ZAttrValue& defl
        ) = 0;

    virtual size_t get_float_attr(
        GeoAttrGroup grp,
        const char* attr_name,
        float* buf,
        size_t buf_size
        ) = 0;

    virtual size_t get_vec3f_attr(
        GeoAttrGroup grp,
        const char* attr_name,
        Vec3f* buf,
        size_t buf_size
        ) = 0;

    virtual bool has_attr(
        GeoAttrGroup grp,
        const char* name,
        GeoAttrType type = ATTR_TYPE_UNKNOWN
        ) = 0;

    virtual int delete_attr(
        GeoAttrGroup grp,
        const char* name
        ) = 0;

    virtual int npoints() const = 0;
    virtual int nfaces() const = 0;
    virtual int nvertices() const = 0;
    virtual int nvertices(int face_id) const = 0;
    virtual int nattributes(GeoAttrGroup grp) const = 0;

    virtual size_t point_faces(int point_id, int* faces, size_t cap) = 0;
    virtual int point_vertex(int point_id) = 0;
    virtual size_t point_vertices(int point_id, int* vertices, size_t cap) = 0;

    virtual int face_point(int face_id, int vert_id) const = 0;
    virtual size_t face_points(int face_id, int* points, size_t cap) = 0;
    virtual int face_vertex(int face_id, int vert_id) = 0;
    virtual int face_vertex_count(int face_id) = 0;
    virtual size_t face_vertices(int face_id, int* vertices, size_t cap) = 0;
    virtual Vec3f face_nrm(int face_id) = 0;

    /* Vertex相关 */
    virtual int vertex_index(int face_id, int vertex_id) = 0;
    virtual int vertex_next(int linear_vertex_id) = 0;
    virtual int vertex_prev(int linear_vertex_id) = 0;
    virtual int vertex_point(int linear_vertex_id) = 0;
    virtual int vertex_face(int linear_vertex_id) = 0;
    virtual int vertex_face_index(int linear_vertex_id) = 0;
};

struct IPrimitiveObject : IObject2 {
    //DEPRECATED!!!
};

struct IListObject : IObject2 {
    virtual size_t size() const = 0;
    virtual void resize(size_t sz) = 0;
    virtual void clear() = 0;
    virtual IObject2* get(size_t index) const = 0;
    virtual void push_back(IObject2* obj) = 0;
    virtual void set(size_t index, IObject2* obj) = 0;
    virtual bool empty() const = 0;

    virtual size_t get_items(IObject2** buf, size_t cap) const = 0;
    virtual size_t get_int_arr(int* buf, size_t cap) const = 0;
    virtual size_t get_float_arr(float* buf, size_t cap) const = 0;
    virtual size_t get_string_arr(char** buf, size_t cap) const = 0;
};

}
#endif