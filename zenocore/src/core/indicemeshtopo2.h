#pragma once

/* note: this module is implemented by Cursor */

#include "geotopology.h"

namespace zeno
{
    /**
     * IndiceMeshTopology2 - Index mesh based topology (no half-edge).
     * Supports multiple geometry types per Houdini SOP concepts:
     * - Points: discrete points with no faces (nfaces=0, nvertices=0)
     * - Lines: each line segment is a face with 2 vertices
     * - Polygons: triangles, quads, or arbitrary n-gons (3+ vertices per face)
     * - Mixed: points + lines + polygons in one mesh
     *
     * Point: unique spatial position, can be shared by multiple faces.
     * Vertex: belongs to exactly one face; one point can have multiple vertices
     *         (one per face that references that point).
     * Face: polygon with N vertices (N=1 point, N=2 line, N>=3 polygon).
     */
    class IndiceMeshTopology2 : public IGeomTopology
    {
    public:
        IndiceMeshTopology2() = default;
        IndiceMeshTopology2(const IndiceMeshTopology2& rhs) = delete;
        IndiceMeshTopology2(PrimitiveObject* prim);
        IndiceMeshTopology2(int nPoints, const std::vector<std::vector<int>>& faces);
        IndiceMeshTopology2(int nPoints);

        GeomTopoType type() const override;
        std::shared_ptr<IGeomTopology> clone() override;
        std::unique_ptr<PrimitiveObject> toPrimitiveObject() const;

        std::vector<vec3i> tri_indice() const override;
        std::vector<std::vector<int>> face_indice() const override;
        std::vector<int> edge_list() const override;
        bool is_base_triangle() const override;
        bool is_line() const override;

        /* Add elements */
        int add_face(const std::vector<int>& points, bool bClose) override;
        void set_face(int idx, const std::vector<int>& points, bool bClose) override;
        int add_point() override;
        int add_vertex(int face_id, int point_id) override;

        /* Remove elements */
        bool remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) override;
        bool remove_point(int ptnum) override;
        bool remove_vertex(int face_id, int vert_id) override;

        /* Counts */
        int npoints() const override;
        int nfaces() const override;
        int nvertices() const override;
        int nvertices(int face_id) const override;

        /* Point queries */
        std::vector<int> point_faces(int point_id) const override;
        int point_vertex(int point_id) const override;
        std::vector<int> point_vertices(int point_id) const override;

        /* Face queries */
        int face_point(int face_id, int vert_id) const override;
        std::vector<int> face_points(int face_id) const override;
        int face_vertex(int face_id, int vert_id) const override;
        int face_vertex_count(int face_id) const override;
        std::vector<int> face_vertices(int face_id) const override;

        /* Vertex queries */
        int vertex_index(int face_id, int vertex_id) const override;
        int vertex_next(int linear_vertex_id) const override;
        int vertex_prev(int linear_vertex_id) const override;
        std::tuple<int, int, int> vertex_info(int linear_vertex_id) const override;
        int vertex_point(int linear_vertex_id) const override;
        int vertex_face(int linear_vertex_id) const override;
        int vertex_face_index(int linear_vertex_id) const override;

    private:
        std::vector<vec3f> m_points;
        std::vector<vec2i> m_polys;  // (start_offset, count) into m_loops
        std::vector<int> m_loops;    // point index per vertex

        int linear_vertex_to_face(int linear_vertex_id) const;
        void rebuild_poly_offsets();
    };
}
