#ifndef __GEOMETRY_TOPOLOGY_H__
#define __GEOMETRY_TOPOLOGY_H__

#include <vector>
#include <string>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/types/PrimitiveObject.h>

namespace zeno
{
    struct HEdge;
    struct HF_Face;
    struct HF_Point;

    struct HEdge {
        std::string id;
        HEdge* pair = 0, * next = 0;
        size_t point = -1;
        size_t point_from = -1;
        size_t face = -1;

        size_t find_from() {
            if (point_from != -1) return point_from;
            if (pair) return pair->point;
            HEdge* h = this;
            while (h->next != this) {
                h = h->next;
            }
            return h->point;
        }
    };

    struct HF_Face {
        int start_linearIdx;  //the vertex index of start vertex.
        HEdge* h = 0;      //应该是起始边
    };

    struct HF_Point {
        std::set<HEdge*> edges;    //all h-edge starting from this point.
    };

    struct GeometryTopology
    {
        GeometryTopology() = default;
        GeometryTopology(const GeometryTopology& rhs);
        GeometryTopology(bool bTriangle, int nPoints, int nFaces);

        void initFromPrim(PrimitiveObject* prim);
        void toPrimitive(std::shared_ptr<PrimitiveObject> spPrim);

        HEdge* checkHEdge(size_t fromPoint, size_t toPoint);
        std::tuple<HF_Point*, HEdge*, HEdge*> getPrev(HEdge* outEdge);
        size_t getNextOutEdge(size_t fromPoint, size_t currentOutEdge);
        size_t getPointTo(HEdge* hedge) const;

        std::vector<vec3i> tri_indice() const;
        std::vector<std::vector<int>> face_indice() const;
        std::vector<int> edge_list() const;
        bool is_base_triangle() const;
        bool is_line() const;
        int npoints_in_face(HF_Face* face) const;
        void geomTriangulate(zeno::TriangulateInfo& info);

        /* 添加元素 */
        int addface(const std::vector<int>& points, bool bClose);
        int add_point();
        int add_vertex(int face_id, int point_id);

        /* 移除元素相关 */
        bool remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum);
        bool remove_point(int ptnum);
        bool remove_vertex(int face_id, int vert_id);

        /* 返回元素个数 */
        int npoints() const;
        int nfaces() const;
        int nvertices() const;
        int nvertices(int face_id) const;

        /* 点相关 */
        std::vector<int> point_faces(int point_id);
        int point_vertex(int point_id);
        std::vector<int> point_vertices(int point_id);

        /* 面相关 */
        int face_point(int face_id, int vert_id) const;
        std::vector<int> face_points(int face_id) const;
        int face_vertex(int face_id, int vert_id);
        int face_vertex_count(int face_id);
        std::vector<int> face_vertices(int face_id);

        /* Vertex相关 */
        int vertex_index(int face_id, int vertex_id);
        int vertex_next(int linear_vertex_id);
        int vertex_prev(int linear_vertex_id);
        std::tuple<int, int, int>  vertex_info(int linear_vertex_id);
        int vertex_point(int linear_vertex_id);
        int vertex_face(int linear_vertex_id);
        int vertex_face_index(int linear_vertex_id);

        void update_linear_vertex();

        //特殊功能
        void fusePoints(std::vector<int>& fusedPoints);//将origin点合并到target点
        void merge(const std::vector<GeometryTopology*>& topos);

    private:
        bool isLineFace(HF_Face* f) const;

        std::vector<std::shared_ptr<HF_Point>> m_points;
        std::vector<std::shared_ptr<HF_Face>> m_faces;
        std::unordered_map<std::string, std::shared_ptr<HEdge>> m_hEdges;
        bool m_bTriangle = true;    //所有面都是三角面
    };
}



#endif