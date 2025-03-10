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
    struct Face;
    struct Point;

    struct HEdge {
        std::string id;
        HEdge* pair = 0, * next = 0;
        size_t point = -1;
        size_t face = -1;

        size_t find_from() {
            if (pair) return pair->point;
            HEdge* h = this;
            while (h->next != this) {
                h = h->next;
            }
            return h->point;
        }
    };

    struct Face {
        int start_linearIdx;  //the vertex index of start vertex.
        HEdge* h = 0;      //any h-edge of this face.
    };

    struct Point {
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
        std::tuple<Point*, HEdge*, HEdge*> getPrev(HEdge* outEdge);
        size_t getNextOutEdge(size_t fromPoint, size_t currentOutEdge);
        size_t getPointTo(HEdge* hedge) const;

        void initpoint(size_t idxPoint);
        void initLineNextPoint(size_t point_id);   //对象是line时，init点的下一个点
        std::vector<vec3i> tri_indice() const;
        std::vector<int> edge_list() const;
        bool is_base_triangle() const;
        bool is_line() const;
        int npoints_in_face(Face* face) const;
        void geomTriangulate(zeno::TriangulateInfo& info);

        void setLineNextPt(int currPt, int nextPt); //对象是line时，修改当前点的下一个点为nextPt
        int getLineNextPt(int currPt); //对象是line时，获取当前pt的下一个点的编号

        /* 添加元素 */
        int addface(const std::vector<int>& points);
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
        std::vector<int> face_points(int face_id);
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
        std::vector<std::shared_ptr<Point>> m_points;
        std::vector<std::shared_ptr<Face>> m_faces;
        std::unordered_map<std::string, std::shared_ptr<HEdge>> m_hEdges;
        std::vector<zeno::vec2i> m_linesPt;
        bool m_bTriangle = true;    //所有面都是三角面
    };




}



#endif