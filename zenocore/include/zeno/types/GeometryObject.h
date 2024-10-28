#ifndef __GEOMETRY_OBJECT_H__
#define __GEOMETRY_OBJECT_H__

#include <vector>
#include <string>
#include <zeno/types/AttributeVector.h>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>


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

        ZENO_API int get_point_count() const;
        ZENO_API int get_face_count() const;
        ZENO_API std::vector<vec3f> points_pos();
        ZENO_API std::vector<vec3i> tri_indice() const;
        ZENO_API std::vector<int> edge_list() const;
        ZENO_API bool is_base_triangle() const;
        ZENO_API int get_group_count(GeoAttrGroup grp) const;

        //attr:
        ZENO_API bool create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& defl);
        ZENO_API bool delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        ZENO_API bool has_attr(GeoAttrGroup grp, std::string const& name);
        ZENO_API GeoAttrType get_attr_type(GeoAttrGroup grp, std::string const& name);
        ZENO_API void set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val);

        template<class T, char CHANNEL = 0>
        std::vector<T> get_attrs(GeoAttrGroup grp, const std::string& attr_name) const {
            const std::map<std::string, AttributeVector>& container = get_const_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            return iter->second.get_attrs<T, CHANNEL>();
        }

        template<class T>
        void foreach_attr_update(GeoAttrGroup grp, const std::string& attr_name, std::function<T(T old_elem_value)>&& evalf) {
            std::map<std::string, AttributeVector>& container = get_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            iter->second.foreach_attr_update<T>(std::move(evalf));
        }

        template<class T, char CHANNEL = 0>
        T get_elem(GeoAttrGroup grp, std::string const& attr_name, size_t idx) const {
            const std::map<std::string, AttributeVector>& container = get_const_container(grp);
            auto iter = container.find(attr_name);
            if (iter == container.end()) {
                throw makeError<KeyError>(attr_name, "not exist on point attr");
            }
            return iter->second.get_elem<T, CHANNEL>(idx);
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

        //API:
        //给定 face_id 和 vert_id，返回顶点索引编号 point_idx。
        ZENO_API int facepoint(int face_id, int vert_id) const;

        //通过 face_id，获取此 face 所有 points 索引编号。
        zfxintarr facepoints(int face_id);

        //返回包含指定 point 的 prim 列表。
        zfxintarr pointfaces(int point_id);
        zfxintarr pointvertex(int point_id);

        ZENO_API void initpoint(size_t point_id);
        ZENO_API int addpoint(zfxvariant pos = zfxfloatarr({ 0,0,0 }));
        ZENO_API void addface(const std::vector<size_t>& points);
        ZENO_API void setface(size_t face_id, const std::vector<size_t>& points);
        ZENO_API int addvertex(size_t face_id, size_t point_id);

        ZENO_API bool remove_point(int ptnum);
        ZENO_API bool remove_faces(const std::set<int>& faces, bool includePoints);

        ZENO_API int npoints() const;
        ZENO_API int nfaces() const;
        ZENO_API int nvertices() const;
        ZENO_API int nvertices(int face_id) const;

        ZENO_API void geomTriangulate(zeno::TriangulateInfo& info);

        //vertex先不考虑
    private:
        void initFromPrim(PrimitiveObject* prim);
        ZENO_API std::map<std::string, AttributeVector>& get_container(GeoAttrGroup grp);
        ZENO_API const std::map<std::string, AttributeVector>& get_const_container(GeoAttrGroup grp) const;
        ZENO_API size_t get_attr_size(GeoAttrGroup grp) const;

        std::shared_ptr<GeometryTopology> m_spTopology;
        std::map<std::string, AttributeVector> m_point_attrs;
        std::map<std::string, AttributeVector> m_face_attrs;
        std::map<std::string, AttributeVector> m_geo_attrs;
    };

}

#endif