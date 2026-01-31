#ifndef __GEOMETRY_OBJECT_H__
#define __GEOMETRY_OBJECT_H__

#include <vector>
#include <string>
#include <zeno/types/AttributeVector.h>
#include <zeno/core/common.h>
#include <iobject2.h>
#include <zeno/utils/api.h>
#include <zeno/types/AttrVector.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/UserData.h>

namespace zeno
{
    using namespace zeno::reflect;

    struct PrimitiveObject;

    class IGeomTopology;
    class AttributeVector;

    using ATTR_VEC_PTR = std::shared_ptr<AttributeVector>;

    class ZENO_API GeometryObject : public IGeometryObject
    {
    public:
        GeometryObject() = delete;
        GeometryObject(GeomTopoType type);
        GeometryObject(GeomTopoType type, bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false);
        GeometryObject(GeomTopoType type, bool bTriangle, int nPoints, const std::vector<std::vector<int>>& faces);
        GeometryObject(const GeometryObject& rhs);
        GeometryObject(PrimitiveObject* spPrim);
        std::unique_ptr<PrimitiveObject> toPrimitive() const;

        void inheritAttributes(
            GeometryObject* rhs,
            int vtx_offset,
            int pt_offset,
            std::set<std::string> pt_nocopy,
            int face_offset,
            std::set<std::string> face_nocopy);
        ~GeometryObject();

        std::unique_ptr<GeometryObject> toIndiceMeshesTopo() const;
        std::unique_ptr<GeometryObject> toHalfEdgeTopo() const;

        std::vector<vec3i> tri_indice() const;
        std::vector<int> edge_list() const;
        std::vector<std::vector<int>> face_indice() const;
        bool is_base_triangle() const;
        bool is_Line() const;
        int get_group_count(GeoAttrGroup grp) const;
        GeoAttrType get_attr_type(GeoAttrGroup grp, std::string const& name);
#ifdef TRACE_GEOM_ATTR_DATA
        std::string get_attr_data_id(GeoAttrGroup grp, std::string const& name, std::string channel = "");
#endif
        std::vector<std::string> get_attr_names(GeoAttrGroup grp);
        void geomTriangulate(zeno::TriangulateInfo& info);
        //standard API

        //CALLBACK_REGIST(create_face_attr, void, std::string)//暂时不需要
        //CALLBACK_REGIST(create_point_attr, void, std::string)
        //CALLBACK_REGIST(create_vertex_attr, void, std::string)
        //CALLBACK_REGIST(create_geometry_attr, void, std::string)

        void copy_attr(GeoAttrGroup grp, const std::string& src_attr, const std::string& dest_attr);
        void copy_attr_from(GeoAttrGroup grp, GeometryObject* pSrcObject, const std::string& src_attr, const std::string& dest_attr);

        //CALLBACK_REGIST(set_vertex_attr, void, std::string&, const AttrVar&)
        //CALLBACK_REGIST(set_point_attr, void, std::string&, const AttrVar&)
        //CALLBACK_REGIST(set_face_attr, void, std::string&, const AttrVar&)

        template<class T>
        void foreach_attr_update(GeoAttrGroup grp, const std::string& attr_name, char channel, std::function<T(int idx, T old_elem_value)>&& evalf) {
            std::map<std::string, AttributeVector>& container = get_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            iter->second.foreach_attr_update<T>(channel, std::move(evalf));
        }

        template<class T, char CHANNEL = 0>
        void set_elem(GeoAttrGroup grp, const std::string& attr_name, size_t idx, T val) {
            std::map<std::string, AttributeVector>& container = get_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            iter->second.set_elem<T, CHANNEL>(idx, val);
        }

        //CALLBACK_REGIST(delete_vertex_attr, void, std::string)
        //CALLBACK_REGIST(delete_point_attr, void, std::string)
        //CALLBACK_REGIST(delete_face_attr, void, std::string)

        //获取属性，CHANNEL如果为'x','y','z','w'中的一个，就是获取相应通道的值，此时T应为float
        template<class T, char CHANNEL = 0>
        std::vector<T> get_attrs(GeoAttrGroup grp, const std::string& attr_name) const {
            const std::map<std::string, AttributeVector>& container = get_const_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            return iter->second.get_attrs<T, CHANNEL>();
        }

        AttrValue get_attr_elem(GeoAttrGroup grp, const std::string& attr_name, size_t idx);
        void set_attr_elem(GeoAttrGroup grp, const std::string& attr_name, size_t idx, AttrValue val);

        //获取属性对应某一个索引下的值
        template<class T>
        T get_elem(GeoAttrGroup grp, std::string const& attr_name, char channel, size_t idx) const {
            const std::map<std::string, AttributeVector>& container = get_const_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            return iter->second.get_elem<T>(channel, idx);
        }

        /*
            用指定的 point 列表创建 prim。
            type = poly 创建闭合多边形。
            type = polyline 创建开放多边形，就是折线。
         */
        //TODO: createPrim(type, point[])
        int isLineFace(int faceid);

    public:
        GeomTopoType topo_type() const override;

        std::vector<vec3f> points_pos() const;
        size_t points_pos(Vec3f* buf, size_t buf_size) override;

        /* 添加元素 */
        int add_vertex(int face_id, int point_id) override;
        CALLBACK_REGIST(add_vertex, void, int)

        int add_point(Vec3f pos) override;
        int add_point(zeno::vec3f pos);
        CALLBACK_REGIST(add_point, void, int)

        int add_face(int* points, size_t size, bool bClose = true) override;
        int add_face(const std::vector<int>& points, bool bClose = true);
        CALLBACK_REGIST(add_face, void, int)

        void set_face(int idx, const std::vector<int>& points, bool bClose = true);

        //创建属性
        int create_attr(
            GeoAttrGroup grp,
            const char* attr_name,
            const ZAttrValue& val
        ) override;

        int create_attr_by_vec3(
            GeoAttrGroup grp,
            const char* attr_name,
            const Vec3f* arr,
            size_t size
        ) override;

        int create_attr_by_float(
            GeoAttrGroup grp,
            const char* attr_name,
            const float* arr,
            size_t size
        ) override;

        int create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& defl);
        int create_face_attr(std::string const& attr_name, const AttrVar& defl);
        int create_point_attr(std::string const& attr_name, const AttrVar& defl);
        int create_vertex_attr(std::string const& attr_name, const AttrVar& defl);
        int create_geometry_attr(std::string const& attr_name, const AttrVar& defl);

        //设置属性
        int set_attr2(
            GeoAttrGroup grp,
            const char* attr_name,
            const ZAttrValue& defl
        ) override;
        int set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val);
        int set_vertex_attr(std::string const& attr_name, const AttrVar& defl);
        int set_point_attr(std::string const& attr_name, const AttrVar& defl);
        int set_face_attr(std::string const& attr_name, const AttrVar& defl);
        int set_geometry_attr(std::string const& attr_name, const AttrVar& defl);

        size_t get_float_attr(
            GeoAttrGroup grp,
            const char* attr_name,
            float* buf,
            size_t buf_size
        ) override;

        std::vector<float> GeometryObject::get_float_attr_vec3(GeoAttrGroup grp, const std::string& attr_name);

        size_t get_vec3f_attr(
            GeoAttrGroup grp,
            const char* attr_name,
            Vec3f* buf,
            size_t buf_size
        ) override;

        /* 检查属性是否存在 */
        bool has_attr(
            GeoAttrGroup grp,
            const char* name,
            GeoAttrType type = ATTR_TYPE_UNKNOWN
        ) override;
        bool has_attr(GeoAttrGroup grp, std::string const& name, GeoAttrType type = ATTR_TYPE_UNKNOWN);
        bool has_vertex_attr(std::string const& name) const;
        bool has_point_attr(std::string const& name) const;
        bool has_face_attr(std::string const& name) const;
        bool has_geometry_attr(std::string const& name) const;
        std::vector<std::string> attributes(GeoAttrGroup grp);

        /* 移除元素相关 */
        int delete_attr(
            GeoAttrGroup grp,
            const char* name
        ) override;
        int delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        int delete_vertex_attr(std::string const& attr_name);
        int delete_point_attr(std::string const& attr_name);
        int delete_face_attr(std::string const& attr_name);
        int delete_geometry_attr(std::string const& attr_name);
        bool remove_faces(const std::set<int>& faces, bool includePoints);
        CALLBACK_REGIST(remove_face, void, int)
        bool remove_point(int ptnum);
        CALLBACK_REGIST(remove_point, void, int)
        bool remove_vertex(int face_id, int vert_id);
        CALLBACK_REGIST(remove_vertex, void, int)
        CALLBACK_REGIST(reset_faces, void)
        CALLBACK_REGIST(reset_vertices, void)


        /* 返回元素个数 */
        int npoints() const override;
        int nfaces() const override;
        int nvertices() const override;
        int nvertices(int face_id) const override;
        int nattributes(GeoAttrGroup grp) const override;

        /* 点相关 */
        size_t point_faces(int point_id, int* faces, size_t cap) override;
        int point_vertex(int point_id) override;
        size_t point_vertices(int point_id, int* vertices, size_t cap) override;
        std::vector<int> point_faces(int point_id);
        std::vector<int> point_vertices(int point_id);

        /* 面相关 */
        int face_point(int face_id, int vert_id) const override;
        size_t face_points(int face_id, int* points, size_t cap) override;
        int face_vertex(int face_id, int vert_id) override;
        int face_vertex_count(int face_id) override;
        size_t face_vertices(int face_id, int* vertices, size_t cap) override;
        Vec3f face_nrm(int face_id) override;
        std::vector<int> face_points(int face_id);
        std::vector<int> face_vertices(int face_id);
        zeno::vec3f face_normal(int face_id);

        /* Vertex相关 */
        int vertex_index(int face_id, int vertex_id) override;
        int vertex_next(int linear_vertex_id) override;
        int vertex_prev(int linear_vertex_id) override;
        int vertex_point(int linear_vertex_id) override;
        int vertex_face(int linear_vertex_id) override;
        int vertex_face_index(int linear_vertex_id) override;
        std::tuple<int, int, int> vertex_info(int linear_vertex_id);

    public: //IObject2
        IObject2* clone() const override {
            return new GeometryObject(*this);
        }
        ZObjectType type() const override {
            return ZObj_Geometry;
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

    private:
        void _temp_code_regist();
        void _temp_code_unregist();

        void initFromPrim(PrimitiveObject* prim);
        std::map<std::string, AttributeVector>& get_container(GeoAttrGroup grp);
        const std::map<std::string, AttributeVector>& get_const_container(GeoAttrGroup grp) const;
        size_t get_attr_size(GeoAttrGroup grp) const;
        void copyTopologyAccordtoUseCount();
        void removeAttribElem(AttributeVector& attrib_vec, int idx);
        void create_attr_from_AttrVector(GeoAttrGroup grp, const std::string& attr_name, const AttrVectorVariant& vec);

        std::shared_ptr<IGeomTopology> m_spTopology; //如果拓扑结构发生变化，就得写时复制了

        std::map<std::string, AttributeVector> m_point_attrs;
        std::map<std::string, AttributeVector> m_face_attrs;
        std::map<std::string, AttributeVector> m_geo_attrs;
        std::map<std::string, AttributeVector> m_vert_attrs;

        AttrVector<vec2f> m_uvs;  //不清楚uvs是否挂在点线面，所以先放在这里中转

        const GeomTopoType m_type;
    };

    ZENO_API std::unique_ptr<GeometryObject> create_GeometryObject(GeomTopoType type);
    ZENO_API std::unique_ptr<GeometryObject> create_GeometryObject(GeomTopoType type, bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false);
    ZENO_API std::unique_ptr<GeometryObject> create_GeometryObject(
        GeomTopoType type,
        bool bTriangle,
        const std::vector<zeno::vec3f>& points,
        const std::vector<std::vector<int>>& faces);
    ZENO_API std::unique_ptr<GeometryObject> create_GeometryObject(PrimitiveObject* prim);
}

#endif