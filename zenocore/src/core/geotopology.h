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

        HEdge* checkHEdge(size_t fromPoint, size_t toPoint);
        std::tuple<Point*, HEdge*, HEdge*> getPrev(HEdge* outEdge);
        size_t getNextOutEdge(size_t fromPoint, size_t currentOutEdge);
        size_t getPointTo(HEdge* hedge) const;
        void addface(const std::vector<size_t>& points);
        void initpoint(size_t idxPoint);
        std::vector<vec3i> tri_indice() const;
        std::vector<int> edge_list() const;
        bool is_base_triangle() const;
        int npoints_in_face(Face* face) const;
        void geomTriangulate(zeno::TriangulateInfo& info);

        std::vector<std::shared_ptr<Face>> m_faces;
        std::unordered_map<std::string, std::shared_ptr<HEdge>> m_hEdges;
        std::vector<std::shared_ptr<Point>> m_points;
        bool m_bTriangle = true;
    };




}



#endif