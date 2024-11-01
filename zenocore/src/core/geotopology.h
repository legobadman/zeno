#ifndef __GEOMETRY_TOPOLOGY_H__
#define __GEOMETRY_TOPOLOGY_H__

#include <vector>
#include <string>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/core/FunctionManager.h>

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
        int  addface(const std::vector<int>& points);
        void setface(size_t idx, const std::vector<size_t>& points);
        int add_point();
        int add_vertex(int face_id, int point_id);
        bool remove_point(int ptnum);
        bool remove_faces(const std::set<int>& faces, bool includePoints);
        void initpoint(size_t idxPoint);
        std::vector<vec3i> tri_indice() const;
        std::vector<int> edge_list() const;
        bool is_base_triangle() const;
        int npoints_in_face(Face* face) const;
        void geomTriangulate(zeno::TriangulateInfo& info);

        int face_point(int face_id, int vert_id) const;
        std::vector<int> face_points(int face_id);
        std::vector<int> point_faces(int point_id);

        int npoints() const;
        int nfaces() const;
        int nvertices() const;
        int nvertices(int face_id) const;

    private:
        std::vector<std::shared_ptr<Point>> m_points;
        std::vector<std::shared_ptr<Face>> m_faces;
        std::unordered_map<std::string, std::shared_ptr<HEdge>> m_hEdges;
        bool m_bTriangle = true;    //所有面都是三角面
    };




}



#endif