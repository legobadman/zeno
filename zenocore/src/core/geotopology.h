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
        int point = -1;
        int face = -1;
    };

    struct Face {
        HEdge* h = 0;      //any h-edge of this face.
    };

    struct Point {
        std::set<HEdge*> edges;    //all h-edge starting from this point.
    };

    struct GeometryTopology
    {
        GeometryTopology(const GeometryTopology& rhs);

        HEdge* checkHEdge(int fromPoint, int toPoint);
        std::tuple<Point*, HEdge*, HEdge*> getPrev(HEdge* outEdge);
        int getNextOutEdge(int fromPoint, int currentOutEdge);
        int getPointTo(HEdge* hedge) const;

        std::vector<std::shared_ptr<Face>> m_faces;
        std::unordered_map<std::string, std::shared_ptr<HEdge>> m_hEdges;
        std::vector<std::shared_ptr<Point>> m_points;
        bool m_bTriangle = true;
    };




}



#endif