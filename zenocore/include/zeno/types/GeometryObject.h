#ifndef __GEOMETRY_OBJECT_H__
#define __GEOMETRY_OBJECT_H__

#include <vector>
#include <string>
#include <zeno/types/AttrVector.h>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/AttributeData.h>


//属性向量的每一个维度的分量是否单独储存。
#define ATTR_VEC_STORE_ISOLATE

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
        bool create_attr_by_zfx(GeoAttrGroup grp, const std::string& attr_name, const zfxvariant& defl);
        ZENO_API bool create_attr(GeoAttrGroup grp, const std::string& attr_name, const Any& defl);
        ZENO_API bool delete_attr(GeoAttrGroup grp, const std::string& attr_name);
        ZENO_API bool has_attr(GeoAttrGroup grp, std::string const& name);
        std::vector<zfxvariant> get_attr_byzfx(GeoAttrGroup grp, std::string const& name);
        void set_attr_byzfx(GeoAttrGroup grp, std::string const& name, const ZfxVariable& val, ZfxElemFilter& filter);
        ZENO_API void set_attr(GeoAttrGroup grp, std::string const& name, const Any& val);

        template <class T>
        void create_attr_value(GeoAttrGroup grp, const std::string& attr_name, T&& val) {
            size_t n = get_attr_size(grp);
#ifdef ATTR_VEC_STORE_ISOLATE
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                create_attr_impl(grp, attr_name + "[x]", std::vector<ElemType>(n, val[0]));
                create_attr_impl(grp, attr_name + "[y]", std::vector<ElemType>(n, val[1]));
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                create_attr_impl(grp, attr_name + "[x]", std::vector<float>(n, val[0]));
                create_attr_impl(grp, attr_name + "[y]", std::vector<float>(n, val[1]));
                create_attr_impl(grp, attr_name + "[z]", std::vector<float>(n, val[2]));
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                create_attr_impl(grp, attr_name + "[x]", std::vector<float>(n, val[0]));
                create_attr_impl(grp, attr_name + "[y]", std::vector<float>(n, val[1]));
                create_attr_impl(grp, attr_name + "[z]", std::vector<float>(n, val[2]));
                create_attr_impl(grp, attr_name + "[w]", std::vector<float>(n, val[3]));
            }
            else
#endif
            {
                create_attr_impl(grp, attr_name, std::vector<T>(n, val));
            }
        }

        template <class T>
        void create_attr_value_by_vec(GeoAttrGroup grp, const std::string& attr_name, const std::vector<T>& vec) {
            size_t n = get_attr_size(grp);
#ifdef ATTR_VEC_STORE_ISOLATE
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                std::vector<ElemType> xvec(n), yvec(n);
                for (int i = 0; i < n; i++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                }
                create_attr_impl(grp, attr_name + "[x]", xvec);
                create_attr_impl(grp, attr_name + "[y]", yvec);
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                std::vector<ElemType> xvec(n), yvec(n), zvec(n);
                for (int i = 0; i < n; i++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                    zvec[i] = vec[i][2];
                }
                create_attr_impl(grp, attr_name + "[x]", xvec);
                create_attr_impl(grp, attr_name + "[y]", yvec);
                create_attr_impl(grp, attr_name + "[z]", zvec);
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                std::vector<ElemType> xvec(n), yvec(n), zvec(n), wvec(n);
                for (int i = 0; i < n; i++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                    zvec[i] = vec[i][2];
                    wvec[i] = vec[i][3];
                }
                create_attr_impl(grp, attr_name + "[x]", xvec);
                create_attr_impl(grp, attr_name + "[y]", yvec);
                create_attr_impl(grp, attr_name + "[z]", zvec);
                create_attr_impl(grp, attr_name + "[w]", wvec);
            }
            else
#endif
            {
                create_attr_impl(grp, attr_name, vec);
            }
        }

        template <class T>
        std::vector<T> get_attr(GeoAttrGroup grp, const std::string& attr_name) {
            size_t n = get_attr_size(grp);
#ifdef ATTR_VEC_STORE_ISOLATE
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec2f>, float, int>::type;
                std::vector<T> vec;
                std::vector<ElemType>& xvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[x]"));
                std::vector<ElemType>& yvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[y]"));
                vec.resize(n);
                for (int i = 0; i < n; i++) {
                    vec[i] = T(xvals[i], yvals[i]);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec3f>, float, int>::type;
                std::vector<T> vec;
                std::vector<ElemType>& xvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[x]"));
                std::vector<ElemType>& yvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[y]"));
                std::vector<ElemType>& zvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[z]"));
                vec.resize(n);
                for (int i = 0; i < n; i++) {
                    vec[i] = T(xvals[i], yvals[i], zvals[i]);
                }
                return vec;
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                using ElemType = std::conditional<std::is_same_v<T, vec4f>, float, int>::type;
                std::vector<T> vec;
                std::vector<ElemType>& xvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[x]"));
                std::vector<ElemType>& yvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[y]"));
                std::vector<ElemType>& zvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[z]"));
                std::vector<ElemType>& wvals = any_cast<std::vector<ElemType>&>(get_attr_impl(grp, attr_name + "[w]"));
                vec.resize(n);
                for (int i = 0; i < n; i++) {
                    vec[i] = T(xvals[i], yvals[i], zvals[i], wvals[i]);
                }
                return vec;
            }
            else
#endif
            {
                return any_cast<std::vector<T>>(get_attr_impl(grp, attr_name));
            }
        }

        template <class T>
        std::vector<T>& get_attr_value(GeoAttrGroup grp, const std::string& attr_name) {
            Any& vals = get_attr_impl(grp, attr_name);
            return any_cast<std::vector<T>&>(vals);
        }

#ifdef ATTR_VEC_STORE_ISOLATE
        template <class T>
        std::vector<std::vector<T>*> get_vec_attr_value(GeoAttrGroup grp, const std::string& attr_name) {
            std::vector<std::vector<T>*> comps;
            const std::string& xattr = attr_name + "[x]";
            if (has_attr(grp, xattr)) {
                Any& xvals = get_attr_impl(grp, xattr);
                comps.push_back(any_cast<std::vector<T>>(&xvals));

                const std::string& yattr = attr_name + "[y]";
                if (has_attr(grp, yattr)) {
                    Any& yvals = get_attr_impl(grp, yattr);
                    comps.push_back(any_cast<std::vector<T>>(&yvals));

                    const std::string& zattr = attr_name + "[z]";
                    if (has_attr(grp, zattr)) {
                        Any& zvals = get_attr_impl(grp, zattr);
                        comps.push_back(any_cast<std::vector<T>>(&zvals));

                        const std::string& wattr = attr_name + "[w]";
                        if (has_attr(grp, wattr)) {
                            Any& wvals = get_attr_impl(grp, wattr);
                            comps.push_back(any_cast<std::vector<T>>(&wvals));
                        }
                    }
                    return comps;
                }
                else {
                    throw;
                }
            }
            else {
                throw;
            }
        }
#endif

        template <class T>
        void set_attr_value(GeoAttrGroup grp, const std::string& attr_name, T&& val) {
            size_t n = get_attr_size(grp);
#ifdef ATTR_VEC_STORE_ISOLATE
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                set_attr(grp, attr_name + "[x]", std::vector<float>(n, val[0]));
                set_attr(grp, attr_name + "[y]", std::vector<float>(n, val[1]));
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                set_attr(grp, attr_name + "[x]", std::vector<float>(n, val[0]));
                set_attr(grp, attr_name + "[y]", std::vector<float>(n, val[1]));
                set_attr(grp, attr_name + "[z]", std::vector<float>(n, val[2]));
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                set_attr(grp, attr_name + "[x]", std::vector<float>(n, val[0]));
                set_attr(grp, attr_name + "[y]", std::vector<float>(n, val[1]));
                set_attr(grp, attr_name + "[z]", std::vector<float>(n, val[2]));
                set_attr(grp, attr_name + "[w]", std::vector<float>(n, val[3]));
            }
            else
#endif
            {
                set_attr(grp, attr_name, std::vector<T>(n, val));
            }
        }

        template <class T>
        void set_attr_value(GeoAttrGroup grp, const std::string& attr_name, const std::vector<T>& val) {
            size_t n = get_attr_size(grp);
#ifdef ATTR_VEC_STORE_ISOLATE
            if constexpr (std::is_same_v<T, vec2f> || std::is_same_v<T, vec2i>) {
                std::vector<T> xvec(n, 0), yvec(n, 0);
                for (int i = 0; i < n; i++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                }
                set_attr(grp, attr_name + "[x]", xvec);
                set_attr(grp, attr_name + "[y]", yvec);
            }
            else if constexpr (std::is_same_v<T, vec3f> || std::is_same_v<T, vec3i>) {
                std::vector<T> xvec(n, 0), yvec(n, 0), zvec(n, 0);
                for (int i = 0; i < n; j++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                    zvec[i] = vec[i][2];
                }
                set_attr(grp, attr_name + "[x]", xvec);
                set_attr(grp, attr_name + "[y]", yvec);
                set_attr(grp, attr_name + "[z]", zvec);
            }
            else if constexpr (std::is_same_v<T, vec4f> || std::is_same_v<T, vec4i>) {
                std::vector<T> xvec(n, 0), yvec(n, 0), zvec(n, 0), wvec(n, 0);
                for (int i = 0; i < n; j++) {
                    xvec[i] = vec[i][0];
                    yvec[i] = vec[i][1];
                    zvec[i] = vec[i][2];
                    wvec[i] = vec[i][3];
                }
                set_attr(grp, attr_name + "[x]", xvec);
                set_attr(grp, attr_name + "[y]", yvec);
                set_attr(grp, attr_name + "[z]", zvec);
                set_attr(grp, attr_name + "[w]", wvec);
            }
            else
#endif
            {
                set_attr(grp, attr_name, vec);
            }
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
        ZENO_API std::map<std::string, ATTR_DATA_PTR>& get_container(GeoAttrGroup grp);
        ZENO_API size_t get_attr_size(GeoAttrGroup grp) const;
        ZENO_API void create_attr_impl(GeoAttrGroup grp, const std::string& attr_name, const Any& vecAny);
        ZENO_API Any& get_attr_impl(GeoAttrGroup grp, std::string const& name);

        std::shared_ptr<GeometryTopology> m_spTopology;
        std::map<std::string, ATTR_DATA_PTR> m_point_attrs;
        std::map<std::string, ATTR_DATA_PTR> m_face_attrs;
        std::map<std::string, ATTR_DATA_PTR> m_geo_attrs;
    };

}

#endif