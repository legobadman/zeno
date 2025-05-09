#include <zeno/types/IGeometryObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/helper.h>
#include <zeno/utils/interfaceutil.h>


namespace zeno
{
    zany GeometryObject_Adapter::clone() const {
        auto newGeom = std::make_shared<GeometryObject_Adapter>();
        newGeom->m_impl = new GeometryObject(*m_impl);
        return newGeom;
    }

    void GeometryObject_Adapter::Delete() {
        delete m_impl;
        m_impl = nullptr;
    }

    void GeometryObject_Adapter::inheritAttributes(
        GeometryObject_Adapter* rhs,
        int vtx_offset,
        int pt_offset,
        std::set<zeno::String> pt_nocopy,    //TODO abi problem
        int face_offset,
        std::set<zeno::String> face_nocopy
    ) {
        std::set<std::string> std_pt_nocopy, std_face_nocopy;
        for (auto zs : pt_nocopy) {
            std_pt_nocopy.insert(zeno::zsString2Std(zs));
        }
        for (auto zs : face_nocopy) {
            std_face_nocopy.insert(zeno::zsString2Std(zs));
        }
        m_impl->inheritAttributes(rhs->m_impl, vtx_offset, pt_offset, std_pt_nocopy, face_offset, std_face_nocopy);
    }

    Vector<Vec3f> GeometryObject_Adapter::points_pos() {
        const std::vector<vec3f>& vec = m_impl->points_pos();
        Vector<Vec3f> _vec(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            _vec[i] = toAbiVec3f(vec[i]);
        }
        return _vec;
    }

    Vector<Vec3i> GeometryObject_Adapter::tri_indice() const {
        const std::vector<vec3f>& vec = m_impl->points_pos();
        Vector<Vec3i> _vec(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            _vec[i] = toAbiVec3i(vec[i]);
        }
        return _vec;
    }

    Vector<int> GeometryObject_Adapter::edge_list() const {
        return stdVec2zeVec(m_impl->edge_list());
    }

    bool GeometryObject_Adapter::is_base_triangle() const {
        return m_impl->is_base_triangle();
    }

    bool GeometryObject_Adapter::is_Line() const {
        return m_impl->is_Line();
    }

    int GeometryObject_Adapter::get_group_count(GeoAttrGroup grp) const {
        return m_impl->get_group_count(grp);
    }

    GeoAttrType GeometryObject_Adapter::get_attr_type(GeoAttrGroup grp, const zeno::String& name) {
        return m_impl->get_attr_type(grp, zsString2Std(name));
    }

    zeno::Vector<zeno::String> GeometryObject_Adapter::get_attr_names(GeoAttrGroup grp) {
        const std::vector<std::string>& vec = m_impl->get_attr_names(grp);
        Vector<zeno::String> _vec(vec.size());
        for (int i = 0; i < vec.size(); i++) {
            _vec[i] = stdString2zs(vec[i]);
        }
        return _vec;
    }

    //��������
    int GeometryObject_Adapter::create_attr(GeoAttrGroup grp, const zeno::String& attr_name, const Any& defl) {
        return m_impl->create_attr(grp, zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::create_face_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->create_face_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::create_point_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->create_point_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::create_vertex_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->create_vertex_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::create_geometry_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->create_geometry_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::set_attr(GeoAttrGroup grp, const zeno::String& name, const Any& val) {
        return m_impl->set_attr(grp, zsString2Std(name), abiAnyToAttrVar(val));
    }

    int GeometryObject_Adapter::set_vertex_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->set_vertex_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::set_point_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->set_point_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::set_face_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->set_face_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    int GeometryObject_Adapter::set_geometry_attr(const zeno::String& attr_name, const Any& defl) {
        return m_impl->set_geometry_attr(zsString2Std(attr_name), abiAnyToAttrVar(defl));
    }

    bool GeometryObject_Adapter::has_attr(GeoAttrGroup grp, const zeno::String& name) {
        return m_impl->has_attr(grp, zsString2Std(name));
    }

    bool GeometryObject_Adapter::has_vertex_attr(const zeno::String& name) const {
        return m_impl->has_vertex_attr(zsString2Std(name));
    }

    bool GeometryObject_Adapter::has_point_attr(const zeno::String& name) const {
        return m_impl->has_point_attr(zsString2Std(name));
    }

    bool GeometryObject_Adapter::has_face_attr(const zeno::String& name) const {
        return m_impl->has_face_attr(zsString2Std(name));
    }

    bool GeometryObject_Adapter::has_geometry_attr(const zeno::String& name) const {
        return m_impl->has_geometry_attr(zsString2Std(name));
    }

    int GeometryObject_Adapter::delete_attr(GeoAttrGroup grp, const zeno::String& attr_name) {
        return m_impl->delete_attr(grp, zsString2Std(attr_name));
    }

    int GeometryObject_Adapter::delete_vertex_attr(const zeno::String& attr_name) {
        return m_impl->delete_vertex_attr(zsString2Std(attr_name));
    }

    int GeometryObject_Adapter::delete_point_attr(const zeno::String& attr_name) {
        return m_impl->delete_point_attr(zsString2Std(attr_name));
    }

    int GeometryObject_Adapter::delete_face_attr(const zeno::String& attr_name) {
        return m_impl->delete_face_attr(zsString2Std(attr_name));
    }

    int GeometryObject_Adapter::delete_geometry_attr(const zeno::String& attr_name) {
        return m_impl->delete_geometry_attr(zsString2Std(attr_name));
    }

    int GeometryObject_Adapter::add_vertex(int face_id, int point_id) {
        return m_impl->add_vertex(face_id, point_id);
    }

    int GeometryObject_Adapter::add_point(Vec3f pos) {
        return m_impl->add_point(toVec3f(pos));
    }

    int GeometryObject_Adapter::add_face(const zeno::Vector<int>& points, bool bClose) {
        return m_impl->add_face(zeVec2stdVec(points), bClose);
    }

    void GeometryObject_Adapter::set_face(int idx, const zeno::Vector<int>& points, bool bClose) {
        return m_impl->set_face(idx, zeVec2stdVec(points), bClose);
    }

    bool GeometryObject_Adapter::remove_faces(const std::set<int>& faces, bool includePoints) {
        return m_impl->remove_faces(faces, includePoints);
    }

    bool GeometryObject_Adapter::remove_point(int ptnum) {
        return m_impl->remove_point(ptnum);
    }

    bool GeometryObject_Adapter::remove_vertex(int face_id, int vert_id) {
        return m_impl->remove_vertex(face_id, vert_id);
    }

    int GeometryObject_Adapter::npoints() const {
        return m_impl->npoints();
    }

    int GeometryObject_Adapter::nfaces() const {
        return m_impl->nfaces();
    }

    int GeometryObject_Adapter::nvertices() const {
        return m_impl->nvertices();
    }

    int GeometryObject_Adapter::nvertices(int face_id) const {
        return m_impl->nvertices(face_id);
    }

    int GeometryObject_Adapter::nattributes(GeoAttrGroup grp) const {
        return m_impl->nattributes(grp);
    }

    zeno::Vector<int> GeometryObject_Adapter::point_faces(int point_id) {
        return stdVec2zeVec(m_impl->point_faces(point_id));
    }

    int GeometryObject_Adapter::point_vertex(int point_id) {
        return m_impl->point_vertex(point_id);
    }

    zeno::Vector<int> GeometryObject_Adapter::point_vertices(int point_id) {
        return stdVec2zeVec(m_impl->point_vertices(point_id));
    }

    int GeometryObject_Adapter::face_point(int face_id, int vert_id) const {
        return m_impl->face_point(face_id, vert_id);
    }

    zeno::Vector<int> GeometryObject_Adapter::face_points(int face_id) {
        return stdVec2zeVec(m_impl->face_points(face_id));
    }

    int GeometryObject_Adapter::face_vertex(int face_id, int vert_id) {
        return m_impl->face_vertex(face_id, vert_id);
    }

    int GeometryObject_Adapter::face_vertex_count(int face_id) {
        return m_impl->face_vertex_count(face_id);
    }

    zeno::Vector<int> GeometryObject_Adapter::face_vertices(int face_id) {
        return stdVec2zeVec(m_impl->face_vertices(face_id));
    }

    zeno::Vec3f GeometryObject_Adapter::face_normal(int face_id) {
        return toAbiVec3f(m_impl->face_normal(face_id));
    }

    int GeometryObject_Adapter::vertex_index(int face_id, int vertex_id) {
        return m_impl->vertex_index(face_id, vertex_id);
    }

    int GeometryObject_Adapter::vertex_next(int linear_vertex_id) {
        return m_impl->vertex_next(linear_vertex_id);
    }

    int GeometryObject_Adapter::vertex_prev(int linear_vertex_id) {
        return m_impl->vertex_prev(linear_vertex_id);
    }

    int GeometryObject_Adapter::vertex_point(int linear_vertex_id) {
        return m_impl->vertex_point(linear_vertex_id);
    }

    int GeometryObject_Adapter::vertex_face(int linear_vertex_id) {
        return m_impl->vertex_face(linear_vertex_id);
    }

    int GeometryObject_Adapter::vertex_face_index(int linear_vertex_id) {
        return m_impl->vertex_face_index(linear_vertex_id);
    }

    std::tuple<int, int, int> GeometryObject_Adapter::vertex_info(int linear_vertex_id) {
        return m_impl->vertex_info(linear_vertex_id);
    }

    zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject() {
        auto pGeom = std::make_shared<GeometryObject_Adapter>();
        pGeom->m_impl = new GeometryObject;
        return pGeom;
    }

    zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject(bool bTriangle, int nPoints, int nFaces, bool bInitFaces) {
        auto pGeom = std::make_shared<GeometryObject_Adapter>();
        pGeom->m_impl = new GeometryObject(bTriangle, nPoints, nFaces, bInitFaces);
        return pGeom;
    }

    zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject(PrimitiveObject* prim) {
        auto pGeom = std::make_shared<GeometryObject_Adapter>();
        pGeom->m_impl = new GeometryObject(prim);
        return pGeom;
    }

    zeno::SharedPtr<GeometryObject_Adapter> clone_GeometryObject(GeometryObject_Adapter* pGeom) {
        auto newGeom = std::make_shared<GeometryObject_Adapter>();
        newGeom->m_impl = new GeometryObject(*pGeom->m_impl);
        return newGeom;
    }

}