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

    enum GeoAttrGroup {
        ATTR_GEO,
        ATTR_FACE,
        ATTR_POINT,
    };

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
        ZENO_API ATTR_VEC_PTR get_attr(GeoAttrGroup grp, const std::string& attr_name);

        //attr:
        bool create_attr_by_zfx(GeoAttrGroup grp, const std::string& attr_name, const zfxvariant& defl);
        ZENO_API bool create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& defl);
        ZENO_API bool delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        ZENO_API bool has_attr(GeoAttrGroup grp, std::string const& name);
        std::vector<zfxvariant> get_attr_byzfx(GeoAttrGroup grp, std::string const& name);
        void set_attr_byzfx(GeoAttrGroup grp, std::string const& name, const ZfxVariable& val, ZfxElemFilter& filter);
        ZENO_API void set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val);

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
        ZENO_API std::map<std::string, ATTR_VEC_PTR>& get_container(GeoAttrGroup grp);
        ZENO_API size_t get_attr_size(GeoAttrGroup grp) const;

        std::shared_ptr<GeometryTopology> m_spTopology;
        std::map<std::string, ATTR_VEC_PTR> m_point_attrs;
        std::map<std::string, ATTR_VEC_PTR> m_face_attrs;
        std::map<std::string, ATTR_VEC_PTR> m_geo_attrs;
    };

}

#endif