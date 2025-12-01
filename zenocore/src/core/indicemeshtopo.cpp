#include "indicemeshtopo.h"
#include <zeno/para/parallel_reduce.h>

namespace zeno
{
    IndiceMeshTopology::IndiceMeshTopology(PrimitiveObject* prim)
        : m_indiceMesh_topo(std::make_unique<PrimitiveObject>(*prim))
    {
        //不需要顶点数属性，是记录在外面的
        //m_indiceMesh_topo->verts.clear();   //需要记录顶点，因为可以需要这几个点作后续的拓扑处理
        m_indiceMesh_topo->verts.attrs.clear();
    }

    IndiceMeshTopology::IndiceMeshTopology(bool bTriangle, int nPoints, int nFaces, bool bInitFaces)
    {
        m_indiceMesh_topo = std::make_unique<PrimitiveObject>();
        if (bTriangle) {
            m_indiceMesh_topo->tris->resize(nFaces);
        }
        else {
            //loops目前还没初始化，需要知道拓扑
            m_indiceMesh_topo->polys->resize(nFaces);
        }
    }

    IndiceMeshTopology::IndiceMeshTopology(bool bTriangle, int nPoints, const std::vector<std::vector<int>>& faces)
    {
        m_indiceMesh_topo = std::make_unique<PrimitiveObject>();
        m_indiceMesh_topo->verts.resize(nPoints);
        int nFace = faces.size();
        if (bTriangle) {
            m_indiceMesh_topo->tris->resize(nFace);
            for (auto i = 0; i < nFace; i++) {
                const auto& face = faces[i];
                if (face.size() != 3) throw makeError<UnimplError>("");
                m_indiceMesh_topo->tris[i] = { face[0], face[1], face[2] };
            }
        }
        else {
            m_indiceMesh_topo->polys->resize(nFace);
            int start_offset_in_loop = 0;
            for (auto i = 0; i < nFace; i++) {
                const auto& face = faces[i];
                for (auto pt : face) {
                    m_indiceMesh_topo->loops->push_back(pt);
                }
                m_indiceMesh_topo->polys[i] = { start_offset_in_loop, (int)face.size() };
                start_offset_in_loop += (int)face.size();
            }
        }
    }

    GeomTopoType IndiceMeshTopology::type() const {
        return Topo_IndiceMesh;
    }

    std::shared_ptr<IGeomTopology> IndiceMeshTopology::clone() {
        return std::make_shared<IndiceMeshTopology>(m_indiceMesh_topo.get());
    }

    std::unique_ptr<PrimitiveObject> IndiceMeshTopology::toPrimitiveObject() const {
        return std::make_unique<PrimitiveObject>(*m_indiceMesh_topo);
    }

    std::vector<vec3i> IndiceMeshTopology::tri_indice() const {
        throw makeError<UnimplError>("");
    }

    std::vector<std::vector<int>> IndiceMeshTopology::face_indice() const {
        throw makeError<UnimplError>("");
    }

    std::vector<int> IndiceMeshTopology::edge_list() const {
        throw makeError<UnimplError>("");
    }

    bool IndiceMeshTopology::is_base_triangle() const {
        return !m_indiceMesh_topo->tris->empty();
    }

    bool IndiceMeshTopology::is_line() const {
        return false;
    }

    /* 添加元素 */
    int IndiceMeshTopology::add_face(const std::vector<int>& points, bool bClose) {
        if (points.empty()) {
            return -1;
        }
        for (int point_id : points) {
            if (point_id < 0 || point_id >= m_indiceMesh_topo->verts.size()) {
                throw makeError<UnimplError>("Invalid point index in add_face");
            }
        }

        int face_id = nfaces();
        if (!bClose) {
            if (points.size() < 2) {
                throw makeError<UnimplError>("Open face must have at least 2 points");
            }
            if (points.size() == 2) {
                vec2i line = { points[0], points[1] };
                m_indiceMesh_topo->lines.push_back(line);
            }
            else {
                for (size_t i = 0; i < points.size() - 1; i++) {
                    vec2i line = { points[i], points[i + 1] };
                    m_indiceMesh_topo->lines.push_back(line);
                }
            }
        }
        else {
            if (is_base_triangle()) {
                if (points.size() != 3) {
                    throw makeError<UnimplError>("Triangle mesh can only add triangle faces");
                }
                vec3i tri = { points[0], points[1], points[2] };
                m_indiceMesh_topo->tris.push_back(tri);
            }
            else {
                if (points.size() < 3) {
                    throw makeError<UnimplError>("Closed face must have at least 3 points");
                }
                int start_offset = m_indiceMesh_topo->loops.size();
                for (int point_id : points) {
                    m_indiceMesh_topo->loops.push_back(point_id);
                }
                vec2i poly = { start_offset, (int)points.size() };
                m_indiceMesh_topo->polys.push_back(poly);
            }
        }

        return face_id;
    }

    void IndiceMeshTopology::set_face(int idx, const std::vector<int>& points, bool bClose) {
        throw makeError<UnimplError>("");
    }

    int IndiceMeshTopology::add_point() {
        throw makeError<UnimplError>("");
    }

    int IndiceMeshTopology::add_vertex(int face_id, int point_id) {
        throw makeError<UnimplError>("");
    }

    /* 移除元素相关 */
    bool IndiceMeshTopology::remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) {
        throw makeError<UnimplError>("");
    }

    bool IndiceMeshTopology::remove_point(int ptnum) {
        throw makeError<UnimplError>("");
    }

    bool IndiceMeshTopology::remove_vertex(int face_id, int vert_id) {
        throw makeError<UnimplError>("");
    }

    /* 返回元素个数 */
    int IndiceMeshTopology::npoints() const {
        //verts其实只存在Geom层面，拓扑层面后续会自定义，而不是沿用PrimitiveObject
        return m_indiceMesh_topo->verts.size();
    }

    int IndiceMeshTopology::nfaces() const {
        if (is_base_triangle()) {
            return m_indiceMesh_topo->tris->size();
        }
        else {
            return m_indiceMesh_topo->polys->size();
        }
    }

    int IndiceMeshTopology::nvertices() const {
        if (is_base_triangle()) {
            return m_indiceMesh_topo->tris->size() * 3;
        }
        else {
            return parallel_reduce_sum(m_indiceMesh_topo->polys->begin(), m_indiceMesh_topo->polys->end(), [&](vec2i const& ind) {
                return ind[1];
            });
        }
    }

    int IndiceMeshTopology::nvertices(int face_id) const {
        if (is_base_triangle()) {
            return 3;
        }
        else {
            if (face_id < 0 || face_id >= m_indiceMesh_topo->polys->size()) {
                throw makeError<UnimplError>("face_id invalid");
            }
            return m_indiceMesh_topo->polys[face_id][1];
        }
    }

    /* 点相关 */
    std::vector<int> IndiceMeshTopology::point_faces(int point_id) const {
        std::vector<int> faces;
        if (point_id < 0 || point_id >= m_indiceMesh_topo->verts.size())
            return faces;

        if (is_base_triangle()) {
            for (int i = 0; i < m_indiceMesh_topo->tris.size(); i++) {
                const auto& tri = m_indiceMesh_topo->tris[i];
                if (tri[0] == point_id || tri[1] == point_id || tri[2] == point_id) {
                    faces.push_back(i);
                }
            }
        } else {
            for (int i = 0; i < m_indiceMesh_topo->polys.size(); i++) {
                const auto& poly = m_indiceMesh_topo->polys[i];
                int startIdx = poly[0];
                int count = poly[1];

                for (int j = 0; j < count; j++) {
                    if (m_indiceMesh_topo->loops[startIdx + j] == point_id) {
                        faces.push_back(i);
                        break;
                    }
                }
            }
        }
        return faces;
    }

    int IndiceMeshTopology::point_vertex(int point_id) const {
        if (point_id < 0 || point_id >= m_indiceMesh_topo->verts.size())
            return -1;

        if (is_base_triangle()) {
            int face_id = 0;
            for (const auto& tri : m_indiceMesh_topo->tris) {
                for (int vert_id = 0; vert_id < 3; vert_id++) {
                    if (tri[vert_id] == point_id) {
                        return face_id * 3 + vert_id;
                    }
                }
                face_id++;
            }
        } else {
            for (int linear_vertex_id = 0; linear_vertex_id < m_indiceMesh_topo->loops->size(); linear_vertex_id++) {
                if (m_indiceMesh_topo->loops[linear_vertex_id] == point_id) {
                    return linear_vertex_id;
                }
            }
        }
        return -1;
    }

    std::vector<int> IndiceMeshTopology::point_vertices(int point_id) const {
        std::vector<int> vertices;
        if (point_id < 0 || point_id >= m_indiceMesh_topo->verts.size())
            return vertices;

        if (is_base_triangle()) {
            int face_id = 0;
            for (const auto& tri : m_indiceMesh_topo->tris) {
                for (int vert_id = 0; vert_id < 3; vert_id++) {
                    if (tri[vert_id] == point_id) {
                        vertices.push_back(face_id * 3 + vert_id);
                        break;
                    }
                }
                face_id++;
            }
        } else {
            for (int linear_vertex_id = 0; linear_vertex_id < m_indiceMesh_topo->loops->size(); linear_vertex_id++) {
                if (m_indiceMesh_topo->loops[linear_vertex_id] == point_id) {
                    vertices.push_back(linear_vertex_id);
                }
            }
        }
        return vertices;
    }

    /* 面相关 */
    int IndiceMeshTopology::face_point(int face_id, int vert_id) const {
        if (face_id < 0 || vert_id < 0)
            return -1;

        if (is_base_triangle()) {
            if (face_id >= m_indiceMesh_topo->tris->size() || vert_id >= 3)
                return -1;
            return m_indiceMesh_topo->tris[face_id][vert_id];
        } else {
            if (face_id >= m_indiceMesh_topo->polys->size() || vert_id >= nvertices(face_id))
                return -1;

            const auto& poly = m_indiceMesh_topo->polys[face_id];
            int startIdx = poly[0];
            return m_indiceMesh_topo->loops[startIdx + vert_id];
        }
    }

    std::vector<int> IndiceMeshTopology::face_points(int face_id) const {
        const auto& face_summary = m_indiceMesh_topo->polys[face_id];
        int startIdx = face_summary[0];
        int count = face_summary[1];
        std::vector<int> face;
        for (int vert = startIdx; vert < startIdx + count; vert++) {
            face.push_back(m_indiceMesh_topo->loops[vert]);
        }
        return face;
    }

    int IndiceMeshTopology::face_vertex(int face_id, int vert_id) const {
        if (face_id < 0 || vert_id < 0)
            return -1;

        if (is_base_triangle()) {
            if (face_id >= m_indiceMesh_topo->tris->size() || vert_id >= 3)
                return -1;
            return face_id * 3 + vert_id;
        } else {
            if (face_id >= m_indiceMesh_topo->polys->size() || vert_id >= nvertices(face_id))
                return -1;

            return m_indiceMesh_topo->polys[face_id][0] + vert_id;
        }
    }

    int IndiceMeshTopology::face_vertex_count(int face_id) const {
        return nvertices(face_id);
    }

    std::vector<int> IndiceMeshTopology::face_vertices(int face_id) const {
        std::vector<int> vertices;
        if (face_id < 0 || face_id >= nfaces())
            return vertices;

        int nVertex = nvertices(face_id);
        vertices.reserve(nVertex);

        if (is_base_triangle()) {
            int start_vertex = face_id * 3;
            for (int vert_id = 0; vert_id < nVertex; vert_id++) {
                vertices.push_back(start_vertex + vert_id);
            }
        } else {
            int start_offset = m_indiceMesh_topo->polys[face_id][0];
            for (int vert_id = 0; vert_id < nVertex; vert_id++) {
                vertices.push_back(start_offset + vert_id);
            }
        }

        return vertices;
    }

    /* Vertex相关 */
    int IndiceMeshTopology::vertex_index(int face_id, int vertex_id) const {
        if (face_id < 0 || vertex_id < 0)
            return -1;

        if (is_base_triangle()) {
            if (face_id >= m_indiceMesh_topo->tris->size() || vertex_id >= 3)
                return -1;
            return face_id * 3 + vertex_id;
        } else {
            if (face_id >= m_indiceMesh_topo->polys->size() || vertex_id >= nvertices(face_id))
                return -1;

            return m_indiceMesh_topo->polys[face_id][0] + vertex_id;
        }
    }

    int IndiceMeshTopology::vertex_next(int linear_vertex_id) const {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto vertices = point_vertices(pointid);
        auto iter = std::find(vertices.cbegin(), vertices.cend(), linear_vertex_id);
        if (iter == vertices.end()) {
            return -1;
        }

        size_t idx = static_cast<size_t>(std::distance(vertices.cbegin(), iter));
        if (idx == vertices.size() - 1) {
            return vertices[0];
        }
        else {
            return vertices[idx + 1];
        }
    }

    int IndiceMeshTopology::vertex_prev(int linear_vertex_id) const {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto vertices = point_vertices(pointid);
        auto iter = std::find(vertices.cbegin(), vertices.cend(), linear_vertex_id);
        if (iter == vertices.end()) {
            return -1;
        }

        size_t idx = static_cast<size_t>(std::distance(vertices.cbegin(), iter));
        if (idx == 0) {
            return vertices.back();
        }
        else {
            return vertices[idx - 1];
        }
    }

    std::tuple<int, int, int> IndiceMeshTopology::vertex_info(int linear_vertex_id) const {
        if (linear_vertex_id < 0) {
            return { -1, -1, -1 };
        }

        int face_id = vertex_face(linear_vertex_id);
        if (face_id == -1) {
            return { -1, -1, -1 };
        }

        int vertex_index = vertex_face_index(linear_vertex_id);
        if (vertex_index == -1) {
            return { -1, -1, -1 };
        }

        int point_id = vertex_point(linear_vertex_id);
        if (point_id == -1) {
            return { -1, -1, -1 };
        }

        return { face_id, vertex_index, point_id };
    }

    int IndiceMeshTopology::vertex_point(int linear_vertex_id) const {
        if (linear_vertex_id < 0) {
            return -1;
        }

        if (is_base_triangle()) {
            int face_id = linear_vertex_id / 3;
            int vert_id = linear_vertex_id % 3;
            if (face_id < 0 || face_id >= m_indiceMesh_topo->tris->size() ||
                vert_id < 0 || vert_id >= 3) {
                return -1;
            }
            return m_indiceMesh_topo->tris[face_id][vert_id];
        } else {
            if (linear_vertex_id >= m_indiceMesh_topo->loops->size()) {
                return -1;
            }
            return m_indiceMesh_topo->loops[linear_vertex_id];
        }
    }

    int IndiceMeshTopology::vertex_face(int linear_vertex_id) const {
        if (linear_vertex_id < 0) {
            return -1;
        }

        if (is_base_triangle()) {
            int face_id = linear_vertex_id / 3;
            if (face_id < 0 || face_id >= m_indiceMesh_topo->tris->size()) {
                return -1;
            }
            return face_id;
        } else {
            int left = 0, right = m_indiceMesh_topo->polys->size() - 1;
            while (right - left > 0) {
                int mid = left + (right - left) / 2;
                int start_offset = m_indiceMesh_topo->polys[mid][0];
                int face_size = m_indiceMesh_topo->polys[mid][1];

                if (linear_vertex_id < start_offset) {
                    right = mid - 1;
                } else if (linear_vertex_id >= start_offset + face_size) {
                    left = mid + 1;
                } else {
                    return mid;
                }
            }
            if (left >= 0 && left < m_indiceMesh_topo->polys->size()) {
                int start_offset = m_indiceMesh_topo->polys[left][0];
                int face_size = m_indiceMesh_topo->polys[left][1];
                if (linear_vertex_id >= start_offset && linear_vertex_id < start_offset + face_size) {
                    return left;
                }
            }
        }
        return -1;
    }

    int IndiceMeshTopology::vertex_face_index(int linear_vertex_id) const {
        if (linear_vertex_id < 0) {
            return -1;
        }

        if (is_base_triangle()) {
            int face_id = linear_vertex_id / 3;
            if (face_id < 0 || face_id >= m_indiceMesh_topo->tris->size()) {
                return -1;
            }
            return linear_vertex_id % 3;
        } else {
            int face_id = vertex_face(linear_vertex_id);
            if (face_id == -1) {
                return -1;
            }
            int start_offset = m_indiceMesh_topo->polys[face_id][0];
            int face_size = m_indiceMesh_topo->polys[face_id][1];
            int vertex_index = linear_vertex_id - start_offset;
            if (vertex_index < 0 || vertex_index >= face_size) {
                return -1;
            }
            return vertex_index;
        }
    }
}