#ifndef __GEOMETRY_OBJECT_H__
#define __GEOMETRY_OBJECT_H__

#include <vector>
#include <string>
#include <zeno/types/AttrVector.h>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/AttributeData.h>


namespace zeno
{
    struct PrimitiveObject;

    class GeometryTopology;

    using ATTR_DATA_PTR = std::shared_ptr<AttributeData>;

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
        ZENO_API std::shared_ptr<PrimitiveObject> toPrimitive() const;
        ZENO_API ~GeometryObject();

        ZENO_API int get_point_count() const;
        ZENO_API int get_face_count() const;
        ZENO_API std::vector<vec3f>& points_pos() const;
        ZENO_API std::vector<vec3i> tri_indice() const;

        //attr:
        bool create_attr_by_zfx(GeoAttrGroup grp, const std::string& attr_name, const zfxvariant& defl);
        ZENO_API bool create_attr(GeoAttrGroup grp, const std::string& attr_name, const Any& defl);
        ZENO_API bool delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        ZENO_API bool has_attr(GeoAttrGroup grp, std::string const& name);
        std::vector<zfxvariant> get_attr_byzfx(GeoAttrGroup grp, std::string const& name);
        ZENO_API Any& get_attr(GeoAttrGroup grp, std::string const& name);
        void set_attr_byzfx(GeoAttrGroup grp, std::string const& name, const ZfxVariable& val, ZfxElemFilter& filter);
        ZENO_API void set_attr(GeoAttrGroup grp, std::string const& name, const Any& val);

        //API:
        //给定 face_id 和 vert_id，返回顶点索引编号 point_idx。
        int facepoint(int face_id, int vert_id) const;

        //通过 face_id，获取此 face 所有 points 索引编号。
        zfxintarr facepoints(int face_id);

        //返回包含指定 point 的 prim 列表。
        zfxintarr pointfaces(int point_id);
        zfxintarr pointvertex(int point_id);

        void initpoint(size_t point_id);
        int addpoint(zfxvariant pos = zfxfloatarr({ 0,0,0 }));
        void addface(const std::vector<size_t>& points);
        int addvertex(size_t face_id, size_t point_id);

        bool remove_point(int ptnum);
        bool remove_faces(const std::set<int>& faces, bool includePoints);

        int npoints() const;
        int nfaces() const;
        int nvertices() const;
        int nvertices(int face_id) const;

        //vertex先不考虑
    private:
        void initFromPrim(PrimitiveObject* prim);
        std::map<std::string, ATTR_DATA_PTR>& get_container(GeoAttrGroup grp);
        int get_attr_size(GeoAttrGroup grp);

        std::shared_ptr<GeometryTopology> m_spTopology;
        std::map<std::string, ATTR_DATA_PTR> m_point_attrs;
        std::map<std::string, ATTR_DATA_PTR> m_face_attrs;
        std::map<std::string, ATTR_DATA_PTR> m_geo_attrs;
    };

}

#endif