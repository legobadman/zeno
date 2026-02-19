/* note: this module is implemented by Cursor */

#include "indicemeshtopo2.h"
#include <zeno/geo/commonutil.h>
#include <zeno/utils/Error.h>
#include <zeno/core/common.h>
#include <algorithm>
#include <set>

namespace zeno
{
    IndiceMeshTopology2::IndiceMeshTopology2(PrimitiveObject* prim)
    {
        int np = (int)prim->verts.size();
        m_points.resize(np);
        for (int i = 0; i < np; i++) {
            m_points[i] = prim->verts[i];
        }

        auto add_face_from_points = [this](const std::vector<int>& pts) {
            if (pts.empty()) return;
            int start = (int)m_loops.size();
            for (int p : pts) {
                m_loops.push_back(p);
            }
            m_polys.push_back(vec2i{start, (int)pts.size()});
        };

        for (const auto& tri : prim->tris) {
            add_face_from_points({tri[0], tri[1], tri[2]});
        }
        for (const auto& quad : prim->quads) {
            add_face_from_points({quad[0], quad[1], quad[2], quad[3]});
        }
        for (const auto& line : prim->lines) {
            add_face_from_points({line[0], line[1]});
        }
        if (prim->polys.size() > 0) {
            for (const auto& poly : prim->polys) {
                int start = poly[0], count = poly[1];
                std::vector<int> pts;
                for (int j = 0; j < count; j++) {
                    pts.push_back(prim->loops[start + j]);
                }
                add_face_from_points(pts);
            }
        }
    }

    IndiceMeshTopology2::IndiceMeshTopology2(int nPoints)
    {
        m_points.resize(nPoints, vec3f(0, 0, 0));
    }

    IndiceMeshTopology2::IndiceMeshTopology2(int nPoints, const std::vector<std::vector<int>>& faces)
    {
        m_points.resize(nPoints, vec3f(0, 0, 0));
        for (const auto& face : faces) {
            add_face(face, true);
        }
    }

    GeomTopoType IndiceMeshTopology2::type() const
    {
        return Topo_IndiceMesh2;
    }

    std::shared_ptr<IGeomTopology> IndiceMeshTopology2::clone()
    {
        auto topo = std::make_shared<IndiceMeshTopology2>();
        topo->m_points = m_points;
        topo->m_polys = m_polys;
        topo->m_loops = m_loops;
        return topo;
    }

    std::unique_ptr<PrimitiveObject> IndiceMeshTopology2::toPrimitiveObject() const
    {
        auto prim = std::make_unique<PrimitiveObject>();
        prim->verts.values = m_points;
        prim->verts.attrs.clear();

        prim->tris.values.clear();
        prim->quads.values.clear();
        prim->lines.values.clear();
        prim->polys.values = m_polys;
        prim->loops.values = m_loops;
        prim->polys.attrs.clear();
        prim->loops.attrs.clear();
        return prim;
    }

    std::vector<vec3i> IndiceMeshTopology2::tri_indice() const
    {
        if (nfaces() == 0) return {};
        auto prim = toPrimitiveObject();
        primTriangulate(prim.get(), false, false, false);
        return prim->tris.values;
    }

    std::vector<std::vector<int>> IndiceMeshTopology2::face_indice() const
    {
        std::vector<std::vector<int>> result;
        for (int i = 0; i < nfaces(); i++) {
            result.push_back(face_points(i));
        }
        return result;
    }

    std::vector<int> IndiceMeshTopology2::edge_list() const
    {
        std::set<std::pair<int, int>> edgeSet;
        for (int face_id = 0; face_id < nfaces(); face_id++) {
            auto pts = face_points(face_id);
            int n = (int)pts.size();
            if (n < 2) continue;
            for (int i = 0; i < n; i++) {
                int a = pts[i];
                int b = pts[(i + 1) % n];
                if (a > b) std::swap(a, b);
                edgeSet.insert({a, b});
            }
        }
        std::vector<int> edges;
        for (const auto& e : edgeSet) {
            edges.push_back(e.first);
            edges.push_back(e.second);
        }
        return edges;
    }

    bool IndiceMeshTopology2::is_base_triangle() const
    {
        if (nfaces() == 0) return false;
        for (int i = 0; i < nfaces(); i++) {
            if (nvertices(i) != 3) return false;
        }
        return true;
    }

    bool IndiceMeshTopology2::is_line() const
    {
        if (nfaces() == 0) return false;
        for (int i = 0; i < nfaces(); i++) {
            if (nvertices(i) != 2) return false;
        }
        return true;
    }

    int IndiceMeshTopology2::add_face(const std::vector<int>& points, bool bClose)
    {
        if (points.empty()) return -1;
        for (int p : points) {
            if (p < 0 || p >= (int)m_points.size()) {
                throw makeError<UnimplError>("Invalid point index in add_face");
            }
        }

        std::vector<int> face_pts;
        if (!bClose) {
            if (points.size() < 2) {
                throw makeError<UnimplError>("Open face must have at least 2 points");
            }
            for (size_t i = 0; i < points.size() - 1; i++) {
                face_pts = {points[i], points[i + 1]};
                int start = (int)m_loops.size();
                for (int p : face_pts) m_loops.push_back(p);
                m_polys.push_back(vec2i{start, 2});
            }
            return nfaces() - 1;
        }
        else {
            face_pts = points;
            if (face_pts.size() < 1) return -1;
            int start = (int)m_loops.size();
            for (int p : face_pts) m_loops.push_back(p);
            m_polys.push_back(vec2i{start, (int)face_pts.size()});
            return nfaces() - 1;
        }
    }

    void IndiceMeshTopology2::set_face(int idx, const std::vector<int>& points, bool bClose)
    {
        if (idx < 0 || idx >= nfaces()) {
            throw makeError<UnimplError>("Invalid face index in set_face");
        }
        for (int p : points) {
            if (p < 0 || p >= (int)m_points.size()) {
                throw makeError<UnimplError>("Invalid point index in set_face");
            }
        }

        if (!bClose) {
            if (points.size() < 2) {
                throw makeError<UnimplError>("Open face must have at least 2 points");
            }
            remove_faces({idx}, false, std::vector<int>{});
            for (size_t i = 0; i < points.size() - 1; i++) {
                add_face({points[i], points[i + 1]}, true);
            }
            return;
        }

        if (points.size() < 1) {
            throw makeError<UnimplError>("Face must have at least 1 point");
        }

        int start = m_polys[idx][0];
        int old_count = m_polys[idx][1];
        int new_count = (int)points.size();
        int diff = new_count - old_count;

        if (diff > 0) {
            m_loops.insert(m_loops.begin() + start + old_count, points.begin() + old_count, points.end());
        }
        else if (diff < 0) {
            m_loops.erase(m_loops.begin() + start + new_count, m_loops.begin() + start + old_count);
        }
        for (int i = 0; i < std::min(old_count, new_count); i++) {
            m_loops[start + i] = points[i];
        }
        m_polys[idx][1] = new_count;
        rebuild_poly_offsets();
    }

    int IndiceMeshTopology2::add_point()
    {
        int id = (int)m_points.size();
        m_points.push_back(vec3f(0, 0, 0));
        return id;
    }

    int IndiceMeshTopology2::add_vertex(int face_id, int point_id)
    {
        if (face_id < 0 || face_id >= nfaces()) {
            throw makeError<UnimplError>("Invalid face index in add_vertex");
        }
        if (point_id < 0 || point_id >= (int)m_points.size()) {
            throw makeError<UnimplError>("Invalid point index in add_vertex");
        }

        int start = m_polys[face_id][0];
        int count = m_polys[face_id][1];
        int insert_pos = start + count;
        m_loops.insert(m_loops.begin() + insert_pos, point_id);
        m_polys[face_id][1] = count + 1;
        rebuild_poly_offsets();
        return count;
    }

    bool IndiceMeshTopology2::remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum)
    {
        if (faces.empty()) return false;

        std::set<int> pts_used;
        for (int fid = 0; fid < nfaces(); fid++) {
            if (faces.count(fid)) continue;
            for (int p : face_points(fid)) pts_used.insert(p);
        }

        std::vector<vec2i> new_polys;
        std::vector<int> new_loops;
        int offset = 0;
        for (int fid = 0; fid < nfaces(); fid++) {
            if (faces.count(fid)) {
                if (includePoints) {
                    for (int p : face_points(fid)) {
                        if (pts_used.count(p) == 0) removedPtnum.push_back(p);
                    }
                }
                continue;
            }
            int start = m_polys[fid][0];
            int count = m_polys[fid][1];
            new_polys.push_back(vec2i{offset, count});
            for (int j = 0; j < count; j++) new_loops.push_back(m_loops[start + j]);
            offset += count;
        }
        m_polys = std::move(new_polys);
        m_loops = std::move(new_loops);

        if (includePoints && !removedPtnum.empty()) {
            std::sort(removedPtnum.begin(), removedPtnum.end());
            removedPtnum.erase(std::unique(removedPtnum.begin(), removedPtnum.end()), removedPtnum.end());
            for (int i = (int)removedPtnum.size() - 1; i >= 0; i--) {
                remove_point(removedPtnum[i]);
            }
        }
        return true;
    }

    bool IndiceMeshTopology2::remove_point(int ptnum)
    {
        if (ptnum < 0 || ptnum >= (int)m_points.size()) return false;

        std::set<int> faces_to_remove;
        for (int fid = 0; fid < nfaces(); fid++) {
            for (int p : face_points(fid)) {
                if (p == ptnum) { faces_to_remove.insert(fid); break; }
            }
        }
        std::vector<int> dummy;
        remove_faces(faces_to_remove, false, dummy);

        m_points.erase(m_points.begin() + ptnum);
        for (int& p : m_loops) {
            if (p > ptnum) p--;
        }
        rebuild_poly_offsets();
        return true;
    }

    bool IndiceMeshTopology2::remove_vertex(int face_id, int vert_id)
    {
        if (face_id < 0 || face_id >= nfaces()) return false;
        int nv = nvertices(face_id);
        if (vert_id < 0 || vert_id >= nv) return false;

        int start = m_polys[face_id][0];
        if (nv <= 1) {
            remove_faces({face_id}, false, std::vector<int>{});
            return true;
        }
        m_loops.erase(m_loops.begin() + start + vert_id);
        m_polys[face_id][1] = nv - 1;
        rebuild_poly_offsets();
        return true;
    }

    int IndiceMeshTopology2::npoints() const { return (int)m_points.size(); }
    int IndiceMeshTopology2::nfaces() const { return (int)m_polys.size(); }
    int IndiceMeshTopology2::nvertices() const { return (int)m_loops.size(); }
    int IndiceMeshTopology2::nvertices(int face_id) const
    {
        if (face_id < 0 || face_id >= nfaces()) return 0;
        return m_polys[face_id][1];
    }

    std::vector<int> IndiceMeshTopology2::point_faces(int point_id) const
    {
        std::vector<int> result;
        if (point_id < 0 || point_id >= npoints()) return result;
        for (int fid = 0; fid < nfaces(); fid++) {
            for (int p : face_points(fid)) {
                if (p == point_id) { result.push_back(fid); break; }
            }
        }
        return result;
    }

    int IndiceMeshTopology2::point_vertex(int point_id) const
    {
        auto verts = point_vertices(point_id);
        return verts.empty() ? -1 : verts[0];
    }

    std::vector<int> IndiceMeshTopology2::point_vertices(int point_id) const
    {
        std::vector<int> result;
        if (point_id < 0 || point_id >= npoints()) return result;
        for (size_t i = 0; i < m_loops.size(); i++) {
            if (m_loops[i] == point_id) result.push_back((int)i);
        }
        return result;
    }

    int IndiceMeshTopology2::face_point(int face_id, int vert_id) const
    {
        if (face_id < 0 || face_id >= nfaces()) return -1;
        int nv = nvertices(face_id);
        if (vert_id < 0 || vert_id >= nv) return -1;
        return m_loops[m_polys[face_id][0] + vert_id];
    }

    std::vector<int> IndiceMeshTopology2::face_points(int face_id) const
    {
        std::vector<int> result;
        if (face_id < 0 || face_id >= nfaces()) return result;
        int start = m_polys[face_id][0];
        int count = m_polys[face_id][1];
        for (int j = 0; j < count; j++) result.push_back(m_loops[start + j]);
        return result;
    }

    int IndiceMeshTopology2::face_vertex(int face_id, int vert_id) const
    {
        if (face_id < 0 || face_id >= nfaces()) return -1;
        int nv = nvertices(face_id);
        if (vert_id < 0 || vert_id >= nv) return -1;
        return m_polys[face_id][0] + vert_id;
    }

    int IndiceMeshTopology2::face_vertex_count(int face_id) const
    {
        return nvertices(face_id);
    }

    std::vector<int> IndiceMeshTopology2::face_vertices(int face_id) const
    {
        std::vector<int> result;
        if (face_id < 0 || face_id >= nfaces()) return result;
        int start = m_polys[face_id][0];
        int count = m_polys[face_id][1];
        for (int j = 0; j < count; j++) result.push_back(start + j);
        return result;
    }

    int IndiceMeshTopology2::vertex_index(int face_id, int vertex_id) const
    {
        return face_vertex(face_id, vertex_id);
    }

    int IndiceMeshTopology2::vertex_next(int linear_vertex_id) const
    {
        int pt = vertex_point(linear_vertex_id);
        if (pt < 0) return -1;
        auto verts = point_vertices(pt);
        if (verts.size() <= 1) return verts.empty() ? -1 : verts[0];
        auto it = std::find(verts.begin(), verts.end(), linear_vertex_id);
        if (it == verts.end()) return -1;
        size_t idx = (size_t)(it - verts.begin());
        return verts[(idx + 1) % verts.size()];
    }

    int IndiceMeshTopology2::vertex_prev(int linear_vertex_id) const
    {
        int pt = vertex_point(linear_vertex_id);
        if (pt < 0) return -1;
        auto verts = point_vertices(pt);
        if (verts.size() <= 1) return verts.empty() ? -1 : verts[0];
        auto it = std::find(verts.begin(), verts.end(), linear_vertex_id);
        if (it == verts.end()) return -1;
        size_t idx = (size_t)(it - verts.begin());
        return verts[(idx + verts.size() - 1) % verts.size()];
    }

    std::tuple<int, int, int> IndiceMeshTopology2::vertex_info(int linear_vertex_id) const
    {
        int fid = vertex_face(linear_vertex_id);
        if (fid < 0) return {-1, -1, -1};
        int vid = vertex_face_index(linear_vertex_id);
        if (vid < 0) return {-1, -1, -1};
        int pt = vertex_point(linear_vertex_id);
        if (pt < 0) return {-1, -1, -1};
        return {fid, vid, pt};
    }

    int IndiceMeshTopology2::vertex_point(int linear_vertex_id) const
    {
        if (linear_vertex_id < 0 || linear_vertex_id >= (int)m_loops.size()) return -1;
        return m_loops[linear_vertex_id];
    }

    int IndiceMeshTopology2::vertex_face(int linear_vertex_id) const
    {
        return linear_vertex_to_face(linear_vertex_id);
    }

    int IndiceMeshTopology2::vertex_face_index(int linear_vertex_id) const
    {
        int fid = vertex_face(linear_vertex_id);
        if (fid < 0) return -1;
        int start = m_polys[fid][0];
        return linear_vertex_id - start;
    }

    int IndiceMeshTopology2::linear_vertex_to_face(int linear_vertex_id) const
    {
        if (linear_vertex_id < 0 || linear_vertex_id >= (int)m_loops.size()) return -1;
        int lo = 0, hi = (int)m_polys.size() - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            int start = m_polys[mid][0];
            int end = start + m_polys[mid][1];
            if (linear_vertex_id < start) hi = mid - 1;
            else if (linear_vertex_id >= end) lo = mid + 1;
            else return mid;
        }
        return -1;
    }

    void IndiceMeshTopology2::rebuild_poly_offsets()
    {
        int offset = 0;
        for (auto& p : m_polys) {
            p[0] = offset;
            offset += p[1];
        }
    }
}
