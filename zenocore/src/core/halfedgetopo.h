#pragma once

#include "geotopology.h"

namespace zeno
{
    struct HF_Edge;
    struct HF_Face;
    struct HF_Point;

    struct HF_Edge {
        //std::string id;
        HF_Edge* pair = 0, *next = 0;
        //size_t point = -1;
        HF_Point* point_from = 0;
        HF_Face* face = 0;
    };

    struct HF_Face {
        //int start_linearIdx;  //the vertex index of start vertex.
        HF_Edge* h = 0;      //one h-edge from this face
    };

    struct HF_Point {
        HF_Edge* edge_from_thispt;    //one h-edge starting from this point.
    };


    struct HalfEdgeTopology : IGeomTopology
    {
        HalfEdgeTopology() = default;
        HalfEdgeTopology(const HalfEdgeTopology& rhs);
        HalfEdgeTopology(bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false);
        virtual ~HalfEdgeTopology();

        GeomTopoType type() const override;
        std::shared_ptr<IGeomTopology> clone() override;

        void build_indice();

        void initFromPrim(PrimitiveObject* prim);
        void toPrimitive(PrimitiveObject* spPrim);

        HF_Edge* checkHEdge(size_t fromPoint, size_t toPoint);
        std::tuple<HF_Point*, HF_Edge*, HF_Edge*> getPrev(HF_Edge* outEdge);
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
        std::vector<int> point_faces(int point_id) override;
        int point_vertex(int point_id) override;
        std::vector<int> point_vertices(int point_id) override;

        /* 面相关 */
        int face_point(int face_id, int vert_id) const override;
        std::vector<int> face_points(int face_id) const override;
        int face_vertex(int face_id, int vert_id) override;
        int face_vertex_count(int face_id) override;
        std::vector<int> face_vertices(int face_id) override;

        /* Vertex相关 */
        int vertex_index(int face_id, int vertex_id) override;
        int vertex_next(int linear_vertex_id) override;
        int vertex_prev(int linear_vertex_id) override;
        std::tuple<int, int, int>  vertex_info(int linear_vertex_id) override;
        int vertex_point(int linear_vertex_id) override;
        int vertex_face(int linear_vertex_id) override;
        int vertex_face_index(int linear_vertex_id) override;
        void update_linear_vertex();

    private:
        std::vector<HF_Edge*> find_alledges_from(int fromPoint);
        void clear_indice_cache() {
            m_temp_cache_point2idx.clear();
            m_temp_cache_edge2idx.clear();
            m_temp_cache_face2idx.clear();
        }

        int idx_of_point(HF_Point* point) const {
            if (!m_temp_cache_point2idx.empty())
                return m_temp_cache_point2idx.at(point);
            return std::find(m_points.begin(), m_points.end(), point) - m_points.begin();
        }

        int idx_of_face(HF_Face* face) const {
            if (!m_temp_cache_face2idx.empty())
                return m_temp_cache_face2idx.at(face);
            return std::find(m_faces.begin(), m_faces.end(), face) - m_faces.begin();
        }

        int idx_of_edge(HF_Edge* edge) const {
            if (!m_temp_cache_edge2idx.empty())
                return m_temp_cache_edge2idx.at(edge);
            return std::find(m_hEdges.begin(), m_hEdges.end(), edge) - m_hEdges.begin();
        }

        std::vector<HF_Point*> m_points;
        std::vector<HF_Face*> m_faces;
        std::vector<HF_Edge*> m_hEdges;
        bool m_bTriangle = true;    //所有面都是三角面

        //临时缓存索引，在修改拓扑以后自动失效（清空）
        std::map<HF_Point*, int> m_temp_cache_point2idx;
        std::map<HF_Edge*, int>  m_temp_cache_edge2idx;
        std::map<HF_Face*, int>  m_temp_cache_face2idx;
    };
}