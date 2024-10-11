#include "geotopology.h"
#include <zeno/utils/format.h>


namespace zeno
{
    GeometryTopology::GeometryTopology(bool bTriangle, int nPoints, int nFaces) {
        m_bTriangle = bTriangle;
        m_points.resize(nPoints);
        m_faces.reserve(nFaces);        //面不好索引，只能逐个加
    }

    GeometryTopology::GeometryTopology(const GeometryTopology& rhs) {
        m_bTriangle = rhs.m_bTriangle;
#if 0
        auto spPrim = rhs.toPrimitive();
        initFromPrim(spPrim.get());
#else
        m_points.resize(rhs.m_points.size());
        m_faces.resize(rhs.m_faces.size());

        for (int i = 0; i < m_points.size(); i++) {
            auto p = std::make_shared<Point>();
            auto& rp = rhs.m_points[i];
            m_points[i] = std::move(p);
        }

        for (auto& [keyName, rEdge] : rhs.m_hEdges) {
            auto spEdge = std::make_shared<HEdge>();
            spEdge->id = rEdge->id;
            spEdge->point = rEdge->point;
            spEdge->face = rEdge->face;
            m_hEdges.insert(std::make_pair(spEdge->id, spEdge));
        }

        for (int i = 0; i < m_faces.size(); i++) {
            m_faces[i] = std::make_shared<Face>();
        }

        //adjust all pointers
        for (int i = 0; i < m_points.size(); i++) {
            for (auto rEdge : rhs.m_points[i]->edges) {
                auto& spEdge = m_hEdges[rEdge->id];
                m_points[i]->edges.insert(spEdge.get());
            }
        }

        for (auto& [keyName, rhsptr] : rhs.m_hEdges)
        {
            auto p = m_hEdges[keyName];
            p->face = rhsptr->face;
            p->point = rhsptr->point;
            if (rhsptr->pair)
                p->pair = m_hEdges[rhsptr->pair->id].get();
            p->next = m_hEdges[rhsptr->next->id].get();
        }

        for (int i = 0; i < m_faces.size(); i++) {
            auto p = m_faces[i].get();
            auto spEdge = m_hEdges[rhs.m_faces[i]->h->id];
            m_faces[i]->h = spEdge.get();
        }
#endif
    }

    HEdge* GeometryTopology::checkHEdge(size_t fromPoint, size_t toPoint) {
        assert(fromPoint < m_points.size() && toPoint < m_points.size() &&
            fromPoint >= 0 && toPoint >= 0);
        for (auto hedge : m_points[fromPoint]->edges) {
            if (hedge->point == toPoint) {
                return hedge;
            }
        }
        return nullptr;
    }

    std::tuple<Point*, HEdge*, HEdge*> GeometryTopology::getPrev(HEdge* outEdge) {
        HEdge* h = outEdge, * prev = nullptr;
        Point* point = nullptr;
        do {
            prev = h;
            point = m_points[h->point].get();
            h = h->next;
        } while (h && h->next != outEdge);
        return { point, h, prev };
    }

    size_t GeometryTopology::getNextOutEdge(size_t fromPoint, size_t currentOutEdge) {
        return -1;
    }

    size_t GeometryTopology::getPointTo(HEdge* hedge) const {
        return hedge->point;
    }

    void GeometryTopology::initpoint(size_t idxPoint) {
        if (!m_points[idxPoint])
            m_points[idxPoint] = std::make_shared<Point>();
    }

    void GeometryTopology::addface(const std::vector<size_t>& points) {
        //points要按照逆时针方向
        std::shared_ptr<Face> spFace = std::make_shared<Face>();
        size_t face_id = m_faces.size();

        std::vector<HEdge*> edges;
        for (size_t i = 0; i < points.size(); i++) {
            size_t from_point = -1, to_point = -1;
            if (i == 0) {
                //edge: from N-1 to 0
                from_point = points[points.size() - 1];
                to_point = points[i];
            }
            else {
                //edge: from i-1 to i
                from_point = points[i - 1];
                to_point = points[i];
            }

            //DEBUG:
            if (from_point == 5 && to_point == 7 || from_point == 7 && to_point == 5) {
                int j;
                j = 0;
            }

            std::shared_ptr<HEdge> hedge = std::make_shared<HEdge>();
            hedge->face = face_id;
            hedge->point = to_point;
            std::string id = zeno::format("{}->{}", from_point, to_point);
            hedge->id = id;

            auto fromPoint = m_points[from_point];
            assert(fromPoint);
            fromPoint->edges.insert(hedge.get());

            //check whether the edge from to_point to from_point.
            auto toPoint = m_points[to_point];
            assert(toPoint);
            for (HEdge* outEdge : toPoint->edges) {
                if (outEdge->point == from_point) {
                    outEdge->pair = hedge.get();
                    hedge->pair = outEdge;
                    break;
                }
            }

            m_hEdges.insert(std::make_pair(id, hedge));
            edges.push_back(hedge.get());
        }

        for (size_t i = 0; i < edges.size(); i++) {
            if (i == edges.size() - 1) {
                edges[i]->next = edges[0];
            }
            else {
                edges[i]->next = edges[i + 1];
            }
        }
        spFace->h = edges[0];
        m_faces.emplace_back(spFace);
    }
}