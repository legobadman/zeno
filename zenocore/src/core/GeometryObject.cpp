#include <zeno/types/GeometryObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <assert.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/utils/vectorutil.h>
#include <zeno/utils/format.h>
#include "geotopology.h"
#include "../utils/zfxutil.h"
#include "zeno_types/reflect/reflection.generated.hpp"
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/utils/variantswitch.h>
#include <regex>


namespace zeno
{
    template <class T>
    static T get_zfxvar(zfxvariant value) {
        return std::visit([](auto const& val) -> T {
            using V = std::decay_t<decltype(val)>;
            if constexpr (!std::is_constructible_v<T, V>) {
                throw makeError<TypeError>(typeid(T), typeid(V), "get<zfxvariant>");
            }
            else {
                return T(val);
            }
            }, value);
    }

    ZENO_API GeometryObject::GeometryObject()
        : m_spTopology(std::make_shared<GeometryTopology>())
    {
    }

    ZENO_API GeometryObject::GeometryObject(bool bTriangle, int nPoints, int nFaces)
        : m_spTopology(std::make_shared<GeometryTopology>(bTriangle, nPoints, nFaces))
    {
    }

    ZENO_API GeometryObject::GeometryObject(const GeometryObject& rhs)
        : m_spTopology(rhs.m_spTopology)
    {
        m_point_attrs = rhs.m_point_attrs;
        m_face_attrs = rhs.m_face_attrs;
        m_geo_attrs = rhs.m_geo_attrs;
    }

    ZENO_API GeometryObject::~GeometryObject() {
    }

    ZENO_API GeometryObject::GeometryObject(PrimitiveObject* prim)
        : m_spTopology(std::make_shared<GeometryTopology>())
    {
        initFromPrim(prim);
    }

    ZENO_API std::shared_ptr<PrimitiveObject> GeometryObject::toPrimitive() {
        std::shared_ptr<PrimitiveObject> spPrim = std::make_shared<PrimitiveObject>();
        std::vector<vec3f>& vec_pos = points_pos();
        int nPoints = m_spTopology->npoints();
        assert(nPoints == vec_pos.size());
        spPrim->resize(nPoints);
        for (int i = 0; i < nPoints; i++) {
            spPrim->verts[i] = vec_pos[i];
        }

        for (auto& [name, sp_attr_data] : m_point_attrs) {
            if (name == "pos") {
                continue;
            }
            sp_attr_data.to_prim_attr(spPrim, true, false, name);
        }

        for (auto& [name, sp_attr_data] : m_face_attrs) {
            sp_attr_data.to_prim_attr(spPrim, false, m_spTopology->is_base_triangle(), name);
        }

        m_spTopology->toPrimitive(spPrim);
        spPrim->m_userData = m_userData;
        return spPrim;
    }

    void GeometryObject::initFromPrim(PrimitiveObject* prim) {
        create_attr(ATTR_POINT, "pos", prim->verts.values);
        m_spTopology->initFromPrim(prim);
    }

    std::vector<vec3f> GeometryObject::points_pos() {
        std::map<std::string, AttributeVector>& container = get_container(ATTR_POINT);
        auto iter = container.find("pos");
        if (iter == container.end()) {
            throw makeError<KeyError>("pos", "not exist on point attr");
        }
        return iter->second.get_attrs<vec3f>();
    }

    ZENO_API std::vector<vec3i> GeometryObject::tri_indice() const {
        return m_spTopology->tri_indice();
    }

    ZENO_API std::vector<int> GeometryObject::edge_list() const {
        return m_spTopology->edge_list();
    }

    ZENO_API bool GeometryObject::is_base_triangle() const {
        return m_spTopology->is_base_triangle();
    }

    ZENO_API bool GeometryObject::is_Line() const {
        return m_spTopology->is_line();
    }

    ZENO_API int GeometryObject::get_group_count(GeoAttrGroup grp) const {
        switch (grp) {
        case ATTR_POINT: return m_spTopology->npoints();
        case ATTR_FACE: return m_spTopology->nfaces();
        case ATTR_VERTEX: return m_spTopology->nvertices();
        case ATTR_GEO: return 1;
        default:
            return 0;
        }
    }

    ZENO_API void GeometryObject::geomTriangulate(zeno::TriangulateInfo& info) {
        if (is_base_triangle()) {
            //TODO
            return;
        }
        m_spTopology->geomTriangulate(info);
        //TODO: uv
    }

    ZENO_API void GeometryObject::setLineNextPt(int currPt, int nextPt) {
        m_spTopology->setLineNextPt(currPt, nextPt);
    }

    ZENO_API int GeometryObject::getLineNextPt(int currPt) {
        return m_spTopology->getLineNextPt(currPt);
    }

    ZENO_API bool GeometryObject::remove_point(int ptnum) {
        std::vector<vec3f>& vec_pos = points_pos();
        assert(m_spTopology->npoints() == vec_pos.size(), false);

        if (m_vert_attrs.size() > 0) {
            auto firstIter = m_vert_attrs.begin();
            assert(firstIter->second.size() == m_spTopology->nvertices(), false);
        }

        copyTopologyAccordtoUseCount();
        std::vector<int>& linearVertexIdx = m_spTopology->point_vertices(ptnum);
        std::vector<int>& facesIdx = m_spTopology->point_faces(ptnum);

        bool ret = m_spTopology->remove_point(ptnum);
        if (ret) {
            //属性也要同步删除,包括point和vertices
            for (auto& [name, attrib_vec] : m_point_attrs) {
                removeAttribElem(attrib_vec, ptnum);
            }

            for (auto& [name, attrib_vec] : m_vert_attrs) {
                for (int i = linearVertexIdx.size() - 1; i >= 0; i--) {
                    removeAttribElem(attrib_vec, linearVertexIdx[i]);
                }
            }

            for (auto& [name, attrib_vec] : m_face_attrs) {
                for (int i = facesIdx.size() - 1; i >= 0; i--) {
                    removeAttribElem(attrib_vec, facesIdx[i]);
                }
            }

            CALLBACK_NOTIFY(remove_point, ptnum)
            CALLBACK_NOTIFY(reset_faces)
            CALLBACK_NOTIFY(reset_vertices)
        }
        return ret;
    }

    ZENO_API bool GeometryObject::remove_vertex(int face_id, int vert_id) {
        copyTopologyAccordtoUseCount();

        size_t linear_vertex = m_spTopology->vertex_index(face_id, vert_id);
        int vertexCount = m_spTopology->face_vertex_count(face_id);

        bool ret = m_spTopology->remove_vertex(face_id, vert_id);
        if (ret) {
            for (auto& [name, attrib_vec] : m_vert_attrs) {
                removeAttribElem(attrib_vec, linear_vertex);
            }
            for (auto& [name, attrib_vec] : m_face_attrs) {
                removeAttribElem(attrib_vec, face_id);
            }
            if (m_spTopology->is_base_triangle() || vertexCount == 3) {//如果有3个顶点删除一个也要删除face
                CALLBACK_NOTIFY(reset_vertices)
                CALLBACK_NOTIFY(remove_face, face_id)
            } else {
                CALLBACK_NOTIFY(remove_vertex, linear_vertex)
            }
            return true;
        } else {
            return false;
        }
    }

    //给定 face_id 和 vert_id，返回顶点索引编号 point_idx。
    ZENO_API int GeometryObject::face_point(int face_id, int vert_id) const
    {
        return m_spTopology->face_point(face_id, vert_id);
    }

    //通过 face_id，获取此 face 所有 points 索引编号。
    ZENO_API std::vector<int> GeometryObject::face_points(int face_id)
    {
        return m_spTopology->face_points(face_id);
    }

    //通过 face_id和内部的vertex_id索引，返回其linearindex.
    ZENO_API int GeometryObject::face_vertex(int face_id, int vert_id)
    {
        return m_spTopology->face_vertex(face_id, vert_id);
    }

    ZENO_API int GeometryObject::face_vertex_count(int face_id)
    {
        return m_spTopology->face_vertex_count(face_id);
    }

    ZENO_API std::vector<int> GeometryObject::face_vertices(int face_id)
    {
        return m_spTopology->face_vertices(face_id);
    }

    //返回包含指定 point 的 face 列表。
    ZENO_API std::vector<int> GeometryObject::point_faces(int point_id)
    {
        return m_spTopology->point_faces(point_id);
    }

    /*
        Returns the linear vertex number of the first vertex to share this point.
        Returns -1 if no vertices share this point.
    */
    ZENO_API int GeometryObject::point_vertex(int point_id)
    {
        return m_spTopology->point_vertex(point_id);
    }

    /*
        An array of linear vertices that are wired to the given point. You should not rely on the numbers being in a particular order.

        If the given point contains no vertices, the array will be empty.
     */
    ZENO_API std::vector<int> GeometryObject::point_vertices(int point_id)
    {
        return m_spTopology->point_vertices(point_id);
    }

    ZENO_API size_t GeometryObject::get_attr_size(GeoAttrGroup grp) const {
        if (grp == ATTR_GEO) {
            return 1;
        }
        else if (grp == ATTR_POINT) {
            return m_spTopology->npoints();
        }
        else if (grp == ATTR_FACE) {
            return m_spTopology->nfaces();
        }
        else if (grp == ATTR_VERTEX) {
            return m_spTopology->nvertices();
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }

    void GeometryObject::copyTopologyAccordtoUseCount() {
        if (m_spTopology.use_count() > 1) {
            m_spTopology = std::make_shared<GeometryTopology>(*m_spTopology.get());
        }
    }

    void GeometryObject::removeAttribElem(AttributeVector& attrib_vec, int idx)
    {
        GeoAttrType attrType = attrib_vec.type();
        if (attrType == ATTR_INT) {
            attrib_vec.remove_elem<int>(idx);
        }
        else if (attrType == ATTR_FLOAT) {
            attrib_vec.remove_elem<float>(idx);
        }
        else if (attrType == ATTR_STRING) {
            attrib_vec.remove_elem < std::string >(idx);
        }
        else if (attrType == ATTR_VEC2) {
            attrib_vec.remove_elem<zeno::vec2f>(idx);
        }
        else if (attrType == ATTR_VEC3) {
            attrib_vec.remove_elem<zeno::vec3f>(idx);
        }
        else if (attrType == ATTR_VEC4) {
            attrib_vec.remove_elem<zeno::vec4f>(idx);
        }
    }

    ZENO_API std::map<std::string, AttributeVector>& GeometryObject::get_container(GeoAttrGroup grp) {
        if (grp == ATTR_GEO) {
            return m_geo_attrs;
        }
        else if (grp == ATTR_POINT) {
            return m_point_attrs;
        }
        else if (grp == ATTR_FACE) {
            return m_face_attrs;
        }
        else if (grp == ATTR_VERTEX) {
            return m_vert_attrs;
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }

    ZENO_API const std::map<std::string, AttributeVector>& GeometryObject::get_const_container(GeoAttrGroup grp) const {
        if (grp == ATTR_GEO) {
            return m_geo_attrs;
        }
        else if (grp == ATTR_POINT) {
            return m_point_attrs;
        }
        else if (grp == ATTR_FACE) {
            return m_face_attrs;
        }
        else if (grp == ATTR_VERTEX) {
            return m_vert_attrs;
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }


    ZENO_API int GeometryObject::create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& val_or_vec)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(attr_name);
        if (iter != container.end()) {
            return -1;   //already exist
        }

        int n = get_attr_size(grp);
        container.insert(std::make_pair(attr_name, AttributeVector(val_or_vec, n == 0 ? 1 : n)));

        //返回啥？
        return 0;
    }

    ZENO_API int GeometryObject::create_face_attr(std::string const& attr_name, const AttrVar& defl) {
        auto iter = m_face_attrs.find(attr_name);
        if (iter != m_face_attrs.end()) {
            return -1;   //already exist
        }
        int n = m_spTopology->nfaces();
        m_face_attrs.insert(std::make_pair(attr_name, AttributeVector(defl, n)));

        return 0;
    }

    ZENO_API int GeometryObject::create_point_attr(std::string const& attr_name, const AttrVar& defl) {
        auto iter = m_point_attrs.find(attr_name);
        if (iter != m_point_attrs.end()) {
            return -1;   //already exist
        }
        int n = m_spTopology->npoints();
        m_point_attrs.insert(std::make_pair(attr_name, AttributeVector(defl, n)));

        return 0;
    }

    ZENO_API int GeometryObject::create_vertex_attr(std::string const& attr_name, const AttrVar& defl) {
        if (attr_name == "Point Number") {
            throw makeError<UnimplError>("Point Number is an internal attribute");
        }
        auto iter = m_vert_attrs.find(attr_name);
        if (iter != m_vert_attrs.end()) {
            return -1;
        }
        int n = m_spTopology->nvertices();
        m_vert_attrs.insert(std::make_pair(attr_name, AttributeVector(defl, n)));

        return 0;
    }

    ZENO_API int GeometryObject::create_geometry_attr(std::string const& attr_name, const AttrVar& defl) {
        auto iter = m_geo_attrs.find(attr_name);
        if (iter != m_geo_attrs.end()) {
            return -1;   //already exist
        }
        m_geo_attrs.insert(std::make_pair(attr_name, AttributeVector(defl, 1)));
        return 0;
    }

    ZENO_API int GeometryObject::delete_attr(GeoAttrGroup grp, const std::string& attr_name)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(attr_name);
        if (iter == container.end())
            return 0;
        container.erase(iter);
        return 1;
    }

    ZENO_API int GeometryObject::delete_vertex_attr(std::string const& attr_name)
    {
        auto iter = m_vert_attrs.find(attr_name);
        if (iter == m_vert_attrs.end())
            return 0;
        m_vert_attrs.erase(iter);
        return 1;
    }

    ZENO_API int GeometryObject::delete_point_attr(std::string const& attr_name)
    {
        auto iter = m_point_attrs.find(attr_name);
        if (iter == m_point_attrs.end())
            return 0;
        m_point_attrs.erase(iter);
        return 1;
    }

    ZENO_API int GeometryObject::delete_face_attr(std::string const& attr_name)
    {
        auto iter = m_face_attrs.find(attr_name);
        if (iter == m_face_attrs.end())
            return 0;
        m_face_attrs.erase(iter);
        return 1;
    }

    ZENO_API int GeometryObject::delete_geometry_attr(std::string const& attr_name)
    {
        auto iter = m_geo_attrs.find(attr_name);
        if (iter == m_geo_attrs.end())
            return 0;
        m_geo_attrs.erase(iter);
        return 1;
    }

    ZENO_API bool GeometryObject::has_attr(GeoAttrGroup grp, std::string const& name)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        return container.find(name) != container.end();
    }

    ZENO_API bool GeometryObject::has_vertex_attr(std::string const& name) const
    {
        return m_vert_attrs.find(name) != m_vert_attrs.end();
    }

    ZENO_API bool GeometryObject::has_point_attr(std::string const& name) const
    {
        return m_point_attrs.find(name) != m_point_attrs.end();
    }

    ZENO_API bool GeometryObject::has_face_attr(std::string const& name) const
    {
        return m_face_attrs.find(name) != m_face_attrs.end();
    }

    ZENO_API bool GeometryObject::has_geometry_attr(std::string const& name) const
    {
        return m_geo_attrs.find(name) != m_geo_attrs.end();
    }

    ZENO_API GeoAttrType GeometryObject::get_attr_type(GeoAttrGroup grp, std::string const& name) {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(name);
        if (iter == container.end()) {
            std::string _name = name;
            if (name == "P" && grp == ATTR_POINT) {
                iter = container.find("pos");
                assert(iter != container.end());
                auto& attrVec = iter->second;
                return attrVec.type();
            }
            return ATTR_TYPE_UNKNOWN;
        }
        auto& attrVec = iter->second;
        return attrVec.type();
    }

    ZENO_API std::vector<std::string> GeometryObject::get_attr_names(GeoAttrGroup grp) {
        std::vector<std::string> names;
        switch (grp) {
        case ATTR_GEO:
            for (auto& [name, _] : m_geo_attrs) {
                names.push_back(name);
            }
            break;
        case ATTR_FACE:
            for (auto& [name, _] : m_face_attrs) {
                names.push_back(name);
            }
            break;
        case ATTR_POINT:
            for (auto& [name, _] : m_point_attrs) {
                names.push_back(name);
            }
            break;
        case ATTR_VERTEX:
            for (auto& [name, _] : m_vert_attrs) {
                names.push_back(name);
            }
            break;
        }
        return names;
    }

    ZENO_API int GeometryObject::set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(name);
        if (iter == container.end()) {
            return -1;
            //throw makeError<KeyError>(name, "not exist on point attr");
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(val);
        return 0;
    }

    ZENO_API int GeometryObject::set_vertex_attr(std::string const& attr_name, const AttrVar& defl)
    {
        auto iter = m_vert_attrs.find(attr_name);
        if (iter == m_vert_attrs.end()) {
            return -1;
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(defl);
        return 0;
    }

    ZENO_API int GeometryObject::set_point_attr(std::string const& attr_name, const AttrVar& defl)
    {
        auto iter = m_point_attrs.find(attr_name);
        if (iter == m_point_attrs.end()) {
            return -1;
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(defl);
        return 0;
    }

    ZENO_API int GeometryObject::set_face_attr(std::string const& attr_name, const AttrVar& defl)
    {
        auto iter = m_face_attrs.find(attr_name);
        if (iter == m_face_attrs.end()) {
            return -1;
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(defl);
        return 0;
    }

    ZENO_API int GeometryObject::set_geometry_attr(std::string const& attr_name, const AttrVar& defl)
    {
        auto iter = m_geo_attrs.find(attr_name);
        if (iter == m_geo_attrs.end()) {
            return -1;
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(defl);
        return 0;
    }

    void GeometryObject::initpoint(size_t point_id) {
        m_spTopology->initpoint(point_id);
    }

    ZENO_API void GeometryObject::initLineNextPoint(size_t point_id) {
        m_spTopology->initLineNextPoint(point_id);
    }

    ZENO_API int GeometryObject::add_point(zeno::vec3f pos) {
        copyTopologyAccordtoUseCount();

        auto pointAttrIter = m_point_attrs.find("pos");
        if (pointAttrIter == m_point_attrs.end()) {
            create_attr(ATTR_POINT, "pos", pos);
        } else {
            pointAttrIter->second.append(pos);
        }
        int ptnum = m_spTopology->add_point();
        CALLBACK_NOTIFY(add_point, ptnum)
        return ptnum;
    }
        
    ZENO_API int GeometryObject::add_vertex(int face_id, int point_id) {
        copyTopologyAccordtoUseCount();
        int ret = m_spTopology->add_vertex(face_id, point_id);
        if (ret != -1) {
            CALLBACK_NOTIFY(add_vertex, m_spTopology->vertex_index(face_id, ret))
            return ret;
        } else {
            return -1;
        }
    }

    /* Vertex相关 */

    /* linear_vertex_idx 这个概念的意思是，vertex在整个几何体所有vertex上的位置索引
      比如vertex排序如下：
        0:0
        0:1
        0:2
        1:0
        1:1     ->这个vertex对应的linear_idx就是4.
        1:2
        1:3     ->这个vertex对应的linear_idx就是6.
    */

    /*
      根据face_id和vertex_id(vertex在面内部的索引，起始值为0）
      返回linear_vertex_idx.
     */
    ZENO_API int GeometryObject::vertex_index(int face_id, int vertex_id) {
        return m_spTopology->vertex_index(face_id, vertex_id);
    }

    /*
     * 与linear_vertex_id共享一个point的下一个vertex的linear_vertex_id;
     */
    ZENO_API int GeometryObject::vertex_next(int linear_vertex_id) {
        return m_spTopology->vertex_next(linear_vertex_id);
    }

    /*
     * 与linear_vertex_id共享一个point的上一个vertex的linear_vertex_id;
     */
    ZENO_API int GeometryObject::vertex_prev(int linear_vertex_id) {
        return m_spTopology->vertex_prev(linear_vertex_id);
    }

    /*
     * 与linear_vertex_id关联的point的id;
     */
    ZENO_API int GeometryObject::vertex_point(int linear_vertex_id) {
        return m_spTopology->vertex_point(linear_vertex_id);
    }

    /*
     * 与linear_vertex_id关联的face的id;
     */
    ZENO_API int GeometryObject::vertex_face(int linear_vertex_id) {
        return m_spTopology->vertex_face(linear_vertex_id);
    }

    /*
     * 将linear_vertex_id转为它所在的那个面上的idx（就是2:3里面的3);
     */
    ZENO_API int GeometryObject::vertex_face_index(int linear_vertex_id) {
        return m_spTopology->vertex_face_index(linear_vertex_id);
    }

    ZENO_API std::tuple<int, int, int> GeometryObject::vertex_info(int linear_vertex_id) {
        return m_spTopology->vertex_info(linear_vertex_id);
    }

    ZENO_API void GeometryObject::fusePoints(std::vector<int>& fusedPoints) {
        int npoints = this->npoints();
        m_spTopology->fusePoints(fusedPoints);

        bool isline = m_spTopology->is_line();
        for (std::vector<int>::reverse_iterator it = fusedPoints.rbegin(); it != fusedPoints.rend(); ++it) {
            if ((*it) != -1) {
                int ptToRemove = npoints - 1 - (it - fusedPoints.rbegin());
                remove_point(ptToRemove);
                if (isline && (ptToRemove == npoints - 1)) {
                    m_spTopology->setLineNextPt(ptToRemove - 1, fusedPoints[ptToRemove]);
                }
            }
        }
    }

    ZENO_API int GeometryObject::add_face(const std::vector<int>& points) {
        copyTopologyAccordtoUseCount();
        int faceid = m_spTopology->addface(points);
        CALLBACK_NOTIFY(add_face, faceid);
        CALLBACK_NOTIFY(reset_vertices)
        return faceid;
    }

    /*
        从几何体中删除 face。
        andPoints = 0，不删除点。
        andPoints = 1，则还将删除与 face 关联但与任何其他 face 无关
        的任何点。
    */
    ZENO_API bool GeometryObject::remove_faces(const std::set<int>& faces, bool includePoints) {
        copyTopologyAccordtoUseCount();

        std::vector<int> removedPtnums;
        std::vector<int> removedVertices;
        for (auto& faceid: faces) {
            std::vector<int>& vertices = m_spTopology->face_vertices(faceid);
            std::copy(vertices.begin(), vertices.end(), std::back_inserter(removedVertices));
        }

        bool ret = m_spTopology->remove_faces(faces, includePoints, removedPtnums);
        if (ret) {
            if (includePoints) {
                for (auto& [name, attrib_vec] : m_point_attrs) {
                    for (int i = removedPtnums.size() - 1; i >= 0; i--) {
                        removeAttribElem(attrib_vec, removedPtnums[i]);
                    }
                }
            }
            for (auto& [name, attrib_vec] : m_vert_attrs) {
                for (int i = removedVertices.size() - 1; i >= 0; i--) {
                    removeAttribElem(attrib_vec, removedVertices[i]);
                }
            }
            for (auto& [name, attrib_vec] : m_face_attrs) {
                for (auto& it = faces.rbegin(); it != faces.rend(); ++it) {
                    removeAttribElem(attrib_vec, *it);
                }
            }
            CALLBACK_NOTIFY(reset_faces)
            CALLBACK_NOTIFY(reset_vertices)
            return true;
        } else {
            return false;
        }
    }

    ZENO_API int GeometryObject::npoints() const {
        return m_spTopology->npoints();
    }

    ZENO_API int GeometryObject::nfaces() const {
        return m_spTopology->nfaces();
    }

    ZENO_API int GeometryObject::nvertices() const {
        return m_spTopology->nvertices();
    }

    ZENO_API int GeometryObject::nvertices(int face_id) const {
        return m_spTopology->nvertices(face_id);
    }

    ZENO_API int GeometryObject::nattributes(GeoAttrGroup grp) const {
        switch (grp) {
        case ATTR_GEO: return m_geo_attrs.size();
        case ATTR_FACE: return m_face_attrs.size();
        case ATTR_POINT: return m_point_attrs.size();
        case ATTR_VERTEX: return m_vert_attrs.size();
        }
    }
}