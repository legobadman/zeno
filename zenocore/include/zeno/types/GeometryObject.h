#ifndef __GEOMETRY_OBJECT_H__
#define __GEOMETRY_OBJECT_H__

#include <vector>
#include <string>
#include <zeno/types/AttributeVector.h>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/utils/api.h>


namespace zeno
{
    using namespace zeno::reflect;

    struct PrimitiveObject;

    class GeometryTopology;
    class AttributeVector;

    using ATTR_VEC_PTR = std::shared_ptr<AttributeVector>;

    class GeometryObject : public IObjectClone<GeometryObject> {
    public:
        ZENO_API GeometryObject();
        ZENO_API GeometryObject(bool bTriangle, int nPoints, int nFaces);
        ZENO_API GeometryObject(const GeometryObject& rhs);
        ZENO_API GeometryObject(PrimitiveObject* prim);
        ZENO_API std::shared_ptr<PrimitiveObject> toPrimitive();
        ZENO_API ~GeometryObject();

        ZENO_API std::vector<vec3f> points_pos();
        ZENO_API std::vector<vec3i> tri_indice() const;
        ZENO_API std::vector<int> edge_list() const;
        ZENO_API bool is_base_triangle() const;
        ZENO_API bool is_Line() const;
        ZENO_API int get_group_count(GeoAttrGroup grp) const;
        ZENO_API GeoAttrType get_attr_type(GeoAttrGroup grp, std::string const& name);
        ZENO_API std::vector<std::string> get_attr_names(GeoAttrGroup grp);
        ZENO_API void initpoint(size_t point_id);
        ZENO_API void initLineNextPoint(size_t point_id);   //对象是line时，init点的下一个点
        ZENO_API void geomTriangulate(zeno::TriangulateInfo& info);

        ZENO_API void setLineNextPt(int currPt, int nextPt);  //对象是line时，修改当前点的下一个点为nextPt
        ZENO_API int getLineNextPt(int currPt); //对象是line时，获取当前pt的下一个点的编号

        //standard API

        //创建属性
        ZENO_API int create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& defl);
        ZENO_API int create_face_attr(std::string const& attr_name, const AttrVar& defl);
        CALLBACK_REGIST(create_face_attr, void, std::string)
        ZENO_API int create_point_attr(std::string const& attr_name, const AttrVar& defl);
        CALLBACK_REGIST(create_point_attr, void, std::string)
        ZENO_API int create_vertex_attr(std::string const& attr_name, const AttrVar& defl);
        CALLBACK_REGIST(create_vertex_attr, void, std::string)
        ZENO_API int create_geometry_attr(std::string const& attr_name, const AttrVar& defl);

        //设置属性
        ZENO_API int set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val);
        ZENO_API int set_vertex_attr(std::string const& attr_name, const AttrVar& defl);
        ZENO_API int set_point_attr(std::string const& attr_name, const AttrVar& defl);
        ZENO_API int set_face_attr(std::string const& attr_name, const AttrVar& defl);
        ZENO_API int set_geometry_attr(std::string const& attr_name, const AttrVar& defl);
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

        /* 检查属性是否存在 */
        ZENO_API bool has_attr(GeoAttrGroup grp, std::string const& name);
        ZENO_API bool has_vertex_attr(std::string const& name) const;
        ZENO_API bool has_point_attr(std::string const& name) const;
        ZENO_API bool has_face_attr(std::string const& name) const;
        ZENO_API bool has_geometry_attr(std::string const& name) const;

        //删除属性
        ZENO_API int delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        ZENO_API int delete_vertex_attr(std::string const& attr_name);
        ZENO_API int delete_point_attr(std::string const& attr_name);
        ZENO_API int delete_face_attr(std::string const& attr_name);
        ZENO_API int delete_geometry_attr(std::string const& attr_name);
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

        /* 添加元素 */
        ZENO_API int add_vertex(int face_id, int point_id);
        CALLBACK_REGIST(add_vertex, void, int)
        ZENO_API int add_point(zeno::vec3f pos);
        CALLBACK_REGIST(add_point, void, int)
        ZENO_API int add_face(const std::vector<int>& points);
        CALLBACK_REGIST(add_face, void, int)

        /* 移除元素相关 */
        ZENO_API bool remove_faces(const std::set<int>& faces, bool includePoints);
        CALLBACK_REGIST(remove_face, void, int)
        ZENO_API bool remove_point(int ptnum);
        CALLBACK_REGIST(remove_point, void, int)
        ZENO_API bool remove_vertex(int face_id, int vert_id);
        CALLBACK_REGIST(remove_vertex, void, int)
        CALLBACK_REGIST(reset_faces, void)
        CALLBACK_REGIST(reset_vertices, void)

        /* 返回元素个数 */
        ZENO_API int npoints() const;
        ZENO_API int nfaces() const;
        ZENO_API int nvertices() const;
        ZENO_API int nvertices(int face_id) const;
        ZENO_API int nattributes(GeoAttrGroup grp) const;

        /* 点相关 */
        ZENO_API std::vector<int> point_faces(int point_id);
        ZENO_API int point_vertex(int point_id);
        ZENO_API std::vector<int> point_vertices(int point_id);

        /* 面相关 */
        ZENO_API int face_point(int face_id, int vert_id) const;
        ZENO_API std::vector<int> face_points(int face_id);
        ZENO_API int face_vertex(int face_id, int vert_id);
        ZENO_API int face_vertex_count(int face_id);
        ZENO_API std::vector<int> face_vertices(int face_id);

        /* Vertex相关 */
        ZENO_API int vertex_index(int face_id, int vertex_id);
        ZENO_API int vertex_next(int linear_vertex_id);
        ZENO_API int vertex_prev(int linear_vertex_id);
        ZENO_API int vertex_point(int linear_vertex_id);
        ZENO_API int vertex_face(int linear_vertex_id);
        ZENO_API int vertex_face_index(int linear_vertex_id);
        ZENO_API std::tuple<int, int, int> vertex_info(int linear_vertex_id);

    private:
        void initFromPrim(PrimitiveObject* prim);
        ZENO_API std::map<std::string, AttributeVector>& get_container(GeoAttrGroup grp);
        ZENO_API const std::map<std::string, AttributeVector>& get_const_container(GeoAttrGroup grp) const;
        ZENO_API size_t get_attr_size(GeoAttrGroup grp) const;
        void copyTopologyAccordtoUseCount();
        void removeAttribElem(AttributeVector& attrib_vec, int idx);

        std::shared_ptr<GeometryTopology> m_spTopology; //如果拓扑结构发生变化，就得写时复制了
        std::map<std::string, AttributeVector> m_point_attrs;
        std::map<std::string, AttributeVector> m_face_attrs;
        std::map<std::string, AttributeVector> m_geo_attrs;
        std::map<std::string, AttributeVector> m_vert_attrs;
    };

}

#endif