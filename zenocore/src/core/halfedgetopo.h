#pragma once

#include "geotopology.h"
#include <unordered_set>

namespace zeno
{
    struct HF_Edge;

    struct HF_Face {
        HF_Edge* h = 0;      //one h-edge from this face
    };

    struct HF_Point {
        HF_Edge* edge_from_thispt = 0;    //one h-edge starting from this point.
    };

    struct HF_Edge {
        HF_Edge* pair = 0, *next = 0;
        HF_Point* point_from = 0;
        HF_Face* face = 0;
    };

    struct IndiceCache;

    struct HalfEdgeTopology : IGeomTopology
    {
        HalfEdgeTopology();
        HalfEdgeTopology(const HalfEdgeTopology& rhs);
        HalfEdgeTopology(bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false);
        virtual ~HalfEdgeTopology();

        GeomTopoType type() const override;
        std::shared_ptr<IGeomTopology> clone() override;

        void initFromPrim(int n_points, PrimitiveObject* prim);
        void toPrimitive(PrimitiveObject* spPrim);

        HF_Edge* checkHEdge(size_t fromPoint, size_t toPoint);
        std::tuple<HF_Point*, HF_Edge*, HF_Edge*> getPrev(HF_Edge* outEdge) const;
        HF_Edge* getPrevEdge(HF_Edge* outEdge) const;
        size_t getNextOutEdge(size_t fromPoint, size_t currentOutEdge);
        size_t getPointTo(HF_Edge* hedge) const;

        std::vector<vec3i> tri_indice() const override;
        std::vector<std::vector<int>> face_indice() const override;
        std::vector<int> edge_list() const override;
        bool is_base_triangle() const override;
        bool is_line() const override;

        int npoints_in_face(HF_Face* face) const;
        void geomTriangulate(zeno::TriangulateInfo& info);

        /* 添加元素 */
        int add_face(const std::vector<int>& points, bool bClose) override;
        void set_face(int idx, const std::vector<int>& points, bool bClose) override;
        int add_point() override;
        int add_vertex(int face_id, int point_id) override;

        /* 移除元素相关 */
        bool remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) override;
        bool remove_point(int ptnum) override;
        bool remove_vertex(int face_id, int vert_id) override;

        /* 返回元素个数 */
        int npoints() const override;
        int nfaces() const override;
        int nvertices() const override;
        int nvertices(int face_id) const override;

        /* 点相关 */
        std::vector<int> point_faces(int point_id) const override;
        int point_vertex(int point_id) const override;
        std::vector<int> point_vertices(int point_id) const override;

        /* 面相关 */
        int face_point(int face_id, int vert_id) const override;
        std::vector<int> face_points(int face_id) const override;
        int face_vertex(int face_id, int vert_id) const override;
        int face_vertex_count(int face_id) const override;
        std::vector<int> face_vertices(int face_id) const override;

        /* Vertex相关 */
        int vertex_index(int face_id, int vertex_id) const override;
        int vertex_next(int linear_vertex_id) const override;
        int vertex_prev(int linear_vertex_id) const override;
        std::tuple<int, int, int>  vertex_info(int linear_vertex_id) const override;
        int vertex_point(int linear_vertex_id) const override;
        int vertex_face(int linear_vertex_id) const override;
        int vertex_face_index(int linear_vertex_id) const override;

    private:
        std::vector<HF_Edge*> find_alledges_from(int fromPoint) const;
        std::vector<HF_Edge*> find_alledges_from(HF_Point* fromPoint) const;
        HF_Point* get_point_to(HF_Edge* pEdge) const {
            return pEdge->next->point_from;
        }
        void _remove_edges(std::unordered_set<HF_Edge*>& remEdges);
        void _remove_faces(std::unordered_set<HF_Face*>& remFaces);
        void _remove_points(std::unordered_set<HF_Point*>& remPoints);

        std::vector<HF_Point*> m_points;
        std::vector<HF_Face*> m_faces;
        std::vector<HF_Edge*> m_hEdges;
        bool m_bTriangle = true;    //所有面都是三角面

        //临时缓存索引，在修改拓扑以后自动失效（清空）
        std::unique_ptr<IndiceCache> m_pCache;
        friend struct IndiceCache;
    };
}