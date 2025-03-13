#include "geotopology.h"
#include <zeno/utils/format.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/vectorutil.h>


namespace zeno
{

    GeometryTopology::GeometryTopology(bool bTriangle, int nPoints, int nFaces) {
        m_bTriangle = bTriangle;
        m_points.resize(nPoints);
        m_faces.reserve(nFaces);    //面不好索引，只能逐个加
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
            m_faces[i]->start_linearIdx = rhs.m_faces[i]->start_linearIdx;
        }

        m_linesPt = rhs.m_linesPt;
#endif
    }

    void GeometryTopology::toPrimitive(std::shared_ptr<PrimitiveObject> spPrim) {
        int startIdx = 0;
        if (m_bTriangle) {
            spPrim->tris->resize(m_faces.size());
        }
        else {
            spPrim->polys->resize(m_faces.size());
        }

        for (int iFace = 0; iFace < m_faces.size(); iFace++) {
            auto face = m_faces[iFace].get();
            HEdge* firsth = face->h;
            HEdge* h = firsth;
            std::vector<int> points;
            do {
                int index = getPointTo(h);
                points.push_back(index);
                h = h->next;
            } while (firsth != h);

            if (m_bTriangle) {
                vec3i tri = { points[0],points[1], points[2] };
                spPrim->tris[iFace] = std::move(tri);
            }
            else {
                int sz = points.size();
                for (auto pt : points) {
                    spPrim->loops.push_back(pt);
                }
                spPrim->polys[iFace] = { startIdx, sz };
                startIdx += sz;
            }
        }

        if (is_line()) {
            spPrim->lines = m_linesPt;
        }
    }

    void GeometryTopology::initFromPrim(PrimitiveObject* prim) {
        int n = prim->verts->size();
        m_points.resize(n);
        for (int i = 0; i < m_points.size(); i++) {
            m_points[i] = std::make_shared<Point>();
        }

        int nFace = -1;
        m_bTriangle = prim->loops->empty() && !prim->tris->empty();
        if (m_bTriangle) {
            nFace = prim->tris->size();
            m_hEdges.reserve(nFace * 3);
        }
        else {
            assert(!prim->loops->empty() && !prim->polys->empty());
            nFace = prim->polys->size();
            //一般是四边形
            m_hEdges.reserve(nFace * 4);
        }
        m_faces.resize(nFace);

        for (int face = 0; face < nFace; face++) {
            std::vector<int> points;
            if (m_bTriangle) {
                auto const& ind = prim->tris[face];
                points = { ind[0], ind[1], ind[2] };
            }
            else {
                auto const& poly = prim->polys[face];
                int startIdx = poly[0], nPoints = poly[1];
                auto const& loops = prim->loops;
                for (int i = 0; i < nPoints; i++) {
                    points.push_back(loops[startIdx + i]);
                }
            }

            //TODO: init data for Face
            auto pFace = std::make_shared<Face>();

            HEdge* lastHedge = 0, * firstHedge = 0;
            for (int i = 0; i < points.size(); i++) {
                int vp = -1, vq = -1;
                if (i < points.size() - 1) {
                    vp = points[i];
                    vq = points[i + 1];
                }
                else {
                    vp = points[i];
                    vq = points[0];
                }

                //vp->vq
                auto hedge = std::make_shared<HEdge>();
                std::string id = zeno::format("{}->{}", vp, vq);
                hedge->id = id;
                m_hEdges.insert(std::make_pair(id, hedge));

                hedge->face = face;
                hedge->point = vq;

                if (lastHedge) {
                    lastHedge->next = hedge.get();
                }
                //TODO: 如果只有一条边会怎么样？
                if (i == points.size() - 1) {
                    hedge->next = firstHedge;
                }
                else if (i == 0) {
                    firstHedge = hedge.get();
                    int nF = m_faces.size();
                    if (m_faces.empty())
                        pFace->start_linearIdx = 0;
                    else {
                        pFace->start_linearIdx = m_faces[nF - 1]->start_linearIdx + nvertices(nF - 1);
                    }
                }

                //check whether the pair edge exist
                auto pairedge = checkHEdge(vq, vp);
                if (pairedge) {
                    hedge->pair = pairedge;
                    pairedge->pair = hedge.get();
                }

                m_points[vp]->edges.insert(hedge.get());

                //pFace->h = hedge.get();
                lastHedge = hedge.get();
            }
            //统一取第一条半边作为face的“起点边”
            pFace->h = firstHedge;

            m_faces[face] = std::move(pFace);
        }
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

    std::vector<vec3i> GeometryTopology::tri_indice() const {
        std::vector<vec3i> indice;
        for (auto spFace : m_faces) {
            HEdge* h = spFace->h;
            vec3i tri(h->point, h->next->point, h->next->next->point);
            indice.push_back(tri);
            indice.push_back(vec3i(h->next->next->next->point, h->point, h->next->next->point));
        }
        return indice;
    }

    std::vector<std::vector<int>> GeometryTopology::face_indice() const {
        std::vector<std::vector<int>> indice;
        for (int i = 0; i < m_faces.size(); i++) {
            std::vector<int> face = face_points(i);
            indice.emplace_back(face);
        }
        return indice;
    }

    std::vector<int> GeometryTopology::edge_list() const {
        std::vector<int> edges;
        for (auto spFace : m_faces) {
            HEdge* h = spFace->h;
            while (h->next != spFace->h) {
                edges.push_back(h->find_from());
                edges.push_back(h->point);
                h = h->next;
            }
            edges.push_back(h->find_from());
            edges.push_back(h->point);
        }
        return edges;
    }

    bool GeometryTopology::is_base_triangle() const {
        return m_bTriangle;
    }

    bool GeometryTopology::is_line() const {
        return !m_linesPt.empty();
    }

    int GeometryTopology::npoints_in_face(Face* face) const {
        auto h = face->h;
        int ncount = 0;
        do {
            ncount++;
            h = h->next;
        } while (face->h != h);
        return ncount;
    }

    void GeometryTopology::geomTriangulate(zeno::TriangulateInfo& info) {
        boolean_switch(info.has_lines, [&](auto has_lines) {
            std::vector<std::conditional_t<has_lines.value, vec2i, int>> scansum(m_faces.size());
            auto redsum = parallel_exclusive_scan_sum(m_faces.begin(), m_faces.end(),
                scansum.begin(), [&](auto& face) {
                    int npts = npoints_in_face(face.get());
                    if constexpr (has_lines.value) {
                        return vec2i(npts >= 3 ? npts - 2 : 0, npts == 2 ? 1 : 0);
                    }
                    else {
                        return npts >= 3 ? npts - 2 : 0;
                    }
                });
            std::vector<int> mapping;
            int tribase = 0;        //TODO: 即使是四边形构成的结合体，也有可能有一些三角形的面存在，这些三角形需要缓存起来
            int linebase = 0;
            if constexpr (has_lines.value) {
                info.tris.resize(tribase + redsum[0]);
                mapping.resize(tribase + redsum[0]);
                info.lines.resize(linebase + redsum[1]);
            }
            else {
                info.tris.resize(tribase + redsum);
                mapping.resize(tribase + redsum);
            }

            {
                parallel_for(m_faces.size(), [&](size_t i) {
                    Face* f = m_faces[i].get();
                    HEdge* hstart = f->h;
                    int start = hstart->find_from();
                    int len =  npoints_in_face(f);
                    if (len >= 3) {
                        int scanbase = 0;
                        if constexpr (has_lines.value) {
                            scanbase = scansum[i][0] + tribase;
                        }
                        else {
                            scanbase = scansum[i] + tribase;
                        }

                        HEdge* h = hstart;
                        while (h->next->point != start) {
                            info.tris[scanbase] = vec3i(start, h->point, h->next->point);
                            mapping[scanbase] = i;
                            h = h->next;
                            scanbase++;
                        }
                    }
                    if constexpr (has_lines.value) {
                        if (len == 2) {
                            int scanbase = scansum[i][1] + linebase;
                            info.lines[scanbase] = vec2i(start, f->h->point);
                        }
                    }
                });
            }
        });
    }

    void GeometryTopology::initLineNextPoint(size_t point_id) {
        if (m_linesPt.empty()) {
            m_linesPt.resize(m_points.size());
        }
        m_linesPt[point_id][0] = point_id;
        m_linesPt[point_id][1] = point_id + 1;
    }

    void GeometryTopology::setLineNextPt(int currPt, int nextPt) {
        if (currPt > -1 && currPt < m_linesPt.size()) {
            m_linesPt[currPt][1] = nextPt;
        }
    }

    int GeometryTopology::getLineNextPt(int currPt) {
        if (currPt < m_linesPt.size()) {
            return m_linesPt[currPt][1];
        }
    }

    int GeometryTopology::face_point(int face_id, int vert_id) const {
        if (face_id < 0 || face_id >= m_faces.size())
            return -1;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        do {
            if (vert_id-- == 0) {
                return h->point;
            }
            h = h->next;
        } while (h != firsth);
        return -1;
    }

    std::vector<int> GeometryTopology::face_points(int face_id) const {
        std::vector<int> pts;
        if (face_id < 0 || face_id >= m_faces.size())
            return pts;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        do {
            pts.push_back(h->point);
            h = h->next;
        } while (h != firsth);
        return pts;
    }

    std::vector<int> GeometryTopology::face_vertices(int face_id) {
        if (face_id < 0 || face_id >= m_faces.size())
            return std::vector<int>();

        auto start_linear_idx = m_faces[face_id]->start_linearIdx;
        int nVertex = nvertices(face_id);
        std::vector<int> vertices(nVertex);
        for (int i = 0; i < nVertex; i++) {
            vertices[i] = start_linear_idx + i;
        }
        return vertices;
    }

    std::vector<int> GeometryTopology::point_faces(int point_id) {
        std::vector<int> faces;
        if (point_id < 0 || point_id >= m_points.size())
            return faces;

        for (auto h : m_points[point_id]->edges) {
            faces.push_back(h->face);
        }
        return faces;
    }

    int GeometryTopology::point_vertex(int point_id) {
        std::vector<int> vertices = point_vertices(point_id);
        if (vertices.empty())
            return -1;
        return vertices[0];
    }

    std::vector<int> GeometryTopology::point_vertices(int point_id) {
        std::vector<int> vertices;
        if (point_id < 0 || point_id >= m_points.size())
            return vertices;

        for (auto pEdge : m_points[point_id]->edges) {
            auto spFace = m_faces[pEdge->face];
            spFace->start_linearIdx;
            auto pFirstH = spFace->h;
            auto h = pFirstH;
            int nCount = 1;

            if (point_id == pFirstH->find_from()) {
                vertices.push_back(spFace->start_linearIdx);
            }
            else {
                while (h->point != point_id) {
                    h = h->next;
                    nCount++;
                }
                vertices.push_back(spFace->start_linearIdx + nCount);
            }
        }
        std::sort(vertices.begin(), vertices.end());
        return vertices;
    }

    int GeometryTopology::npoints() const {
        return m_points.size();
    }

    int GeometryTopology::nfaces() const {
        return m_faces.size();
    }

    int GeometryTopology::nvertices() const {
        if (m_faces.empty()) {
            return 0;
        }
        else {
            int idxLast = m_faces.size() - 1;
            return m_faces[idxLast]->start_linearIdx + nvertices(idxLast);
        }
    }

    int GeometryTopology::nvertices(int face_id) const {
        if (face_id < 0 || face_id >= m_faces.size()) {
            return 0;
        }
        return npoints_in_face(m_faces[face_id].get());
    }

    int GeometryTopology::face_vertex(int face_id, int vert_id) {
        if (face_id < 0 || face_id >= m_faces.size() || vert_id < 0 || vert_id >= nvertices(face_id)) {
            return -1;
        }
        return m_faces[face_id]->start_linearIdx + vert_id;
    }

    int GeometryTopology::face_vertex_count(int face_id) {
        return nvertices(face_id);
    }

    int GeometryTopology::vertex_index(int face_id, int vertex_id) {
        return face_vertex(face_id, vertex_id);
    }

    /*
     * 与linear_vertex_id共享一个point的下一个vertex的linear_vertex_id;
     */
    int GeometryTopology::vertex_next(int linear_vertex_id) {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto& vertices = point_vertices(pointid);
        auto iter = std::find(vertices.cbegin(), vertices.cend(), linear_vertex_id);
        assert(iter != vertices.end());
        size_t idx = static_cast<size_t>(std::distance(vertices.cbegin(), iter));
        if (idx == vertices.size() - 1) {
            return vertices[0];
        }
        else {
            return vertices[idx + 1];
        }
    }

    int GeometryTopology::vertex_prev(int linear_vertex_id) {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto& vertices = point_vertices(pointid);
        auto iter = std::find(vertices.cbegin(), vertices.cend(), linear_vertex_id);
        assert(iter != vertices.end());
        size_t idx = static_cast<size_t>(std::distance(vertices.cbegin(), iter));
        if (idx == 0) {
            return vertices.back();
        }
        else {
            return vertices[idx - 1];
        }
    }

    int GeometryTopology::vertex_point(int linear_vertex_id) {
        return std::get<2>(vertex_info(linear_vertex_id));
    }

    std::tuple<int, int, int> GeometryTopology::vertex_info(int linear_vertex_id) {
        int faceid = vertex_face(linear_vertex_id);
        if (faceid == -1)
            return {-1,-1,-1};

        auto& spFace = m_faces[faceid];
        int offset = linear_vertex_id - spFace->start_linearIdx;
        if (offset == 0) {
            return { faceid, 0, spFace->h->find_from()};
        }
        if (offset >= nvertices(faceid))
            return { -1,-1,-1 };

        auto h = spFace->h;
        while (--offset) {
            h = h->next;
        }
        return { faceid, linear_vertex_id - spFace->start_linearIdx, h->point };
    }

    /*
     * 与linear_vertex_id关联的face的id;
     */
    int GeometryTopology::vertex_face(int linear_vertex_id) {
        int n = m_faces.size();
        if (n == 0)
            return -1;

        int left = 0, right = n - 1;
        while (right - left > 0) {
            int mid = (left + right) / 2;
            auto spFace = m_faces[mid];
            
            if (linear_vertex_id < spFace->start_linearIdx) {
                right = mid - 1;
            }
            else {
                int nVertices = nvertices(mid);
                if (linear_vertex_id >= spFace->start_linearIdx + nVertices) {
                    left = mid + 1;
                }
                else {
                    return mid;
                }
            }

        }
        auto& spFace = m_faces[left];
        if (linear_vertex_id >= spFace->start_linearIdx + nvertices(left))
            return -1;
        return left;
    }

    /*
     * 将linear_vertex_id转为它所在的那个面上的idx（就是2:3里面的3);
     */
    int GeometryTopology::vertex_face_index(int linear_vertex_id) {
        int idxFace = vertex_face(linear_vertex_id);
        if (idxFace == -1)
            return -1;

        auto spFace = m_faces[idxFace];
        auto pFirstH = spFace->h;
        auto h = pFirstH;
        int offset = linear_vertex_id - spFace->start_linearIdx;
        return offset;
    }

    void GeometryTopology::update_linear_vertex()
    {
        //更新linearIdx
        if (!m_faces.empty()) {
            m_faces[0]->start_linearIdx = 0;
            for (int i = 1; i < m_faces.size(); ++i) {
                m_faces[i]->start_linearIdx = m_faces[i - 1]->start_linearIdx + nvertices(i - 1);
            }
        }
    }

    void GeometryTopology::merge(const std::vector<GeometryTopology*>& topos) {
        int offset_pts = 0, offset_faces = 0;
        for (GeometryTopology* rhs : topos) {
            const std::string& uuid_base = generateUUID() + "/";
            int iFromPt = offset_pts;
            int iToPt = iFromPt + rhs->m_points.size() - 1;  //闭区间
            int iFromFace = offset_faces;
            int iToFace = iFromFace + rhs->m_faces.size() - 1;

            for (int i = offset_pts; i <= iToPt; i++) {
                auto p = std::make_shared<Point>();
                m_points[i] = std::move(p);
            }

            for (auto& [keyName, rEdge] : rhs->m_hEdges) {
                auto spEdge = std::make_shared<HEdge>();
                spEdge->id = uuid_base + rEdge->id;
                spEdge->point = rEdge->point + iFromPt;
                spEdge->face = rEdge->face + iFromFace;
                m_hEdges.insert(std::make_pair(spEdge->id, spEdge));
            }

            for (int i = iFromFace; i <= iToFace; i++) {
                //前面的faces只是reserve
                m_faces.emplace_back(std::make_shared<Face>());
            }

            //adjust all pointers
            for (int i = iFromPt; i <= iToPt; i++) {
                for (auto rEdge : rhs->m_points[i-iFromPt]->edges) {
                    auto& spEdge = m_hEdges[uuid_base + rEdge->id];
                    m_points[i]->edges.insert(spEdge.get());
                }
            }

            for (auto& [keyName, rhsptr] : rhs->m_hEdges)
            {
                auto p = m_hEdges[uuid_base + keyName];
                if (rhsptr->pair)
                    p->pair = m_hEdges[uuid_base + rhsptr->pair->id].get();
                p->next = m_hEdges[uuid_base + rhsptr->next->id].get();
            }

            for (int i = iFromFace; i <= iToFace; i++) {
                auto p = m_faces[i].get();
                auto spEdge = m_hEdges[uuid_base + rhs->m_faces[i - iFromFace]->h->id];
                m_faces[i]->h = spEdge.get();
                m_faces[i]->start_linearIdx = rhs->m_faces[i - iFromFace]->start_linearIdx + offset_faces;
            }

            offset_pts = iToPt + 1;
            offset_faces = iToFace + 1;

            //TODO
            //m_linesPt = rhs->m_linesPt;
        }
    }

    void GeometryTopology::fusePoints(std::vector<int>& fusedPoints) {
        std::set<int> facesToremove;
        std::vector<int> pointsToremove;

        for (std::vector<int>::reverse_iterator it = fusedPoints.rbegin(); it != fusedPoints.rend(); ++it) {
            int targetPt = *it;
            if (targetPt != -1) {
                int ptToRemove = npoints() - 1 - (it - fusedPoints.rbegin());

                const std::vector<int>& targetPtRelatedFacesvec = point_faces(targetPt);
                std::set<int> targetPtRelatedFaces(targetPtRelatedFacesvec.begin(), targetPtRelatedFacesvec.end());
                std::vector<int> ptToRemoveRelatedFaces = point_faces(ptToRemove);
                if (m_bTriangle) {
                    for (auto& face : ptToRemoveRelatedFaces) {
                        if (targetPtRelatedFaces.find(face) != targetPtRelatedFaces.end()) {//查询融合的面
                            facesToremove.insert(face);
                        }
                        else {
                            HEdge* inH,* outH;
                            HEdge* start = m_faces[face]->h;
                            do {
                                if (start->point == ptToRemove) {
                                    inH = start;
                                } else if (start->next->next->point == ptToRemove) {
                                    outH = start;
                                }
                                start = start->next;
                            } while (start != m_faces[face]->h);
                            //旧的点指向新的点
                            inH->point = targetPt;
                            m_points[ptToRemove]->edges.erase(outH);
                            m_points[targetPt]->edges.insert(outH);
                            //查询重复的面
                            bool duplicate = false;
                            for (std::set<HEdge*>::iterator it = m_points[targetPt]->edges.begin(); it != m_points[targetPt]->edges.end() && !duplicate; ++it) {
                                HEdge* start = (*it);
                                do {
                                    if (start != outH && start->point == outH->point && start->next->point == outH->next->point ||
                                        start->point == outH->next->point && start->next->point == outH->point) {
                                        facesToremove.insert(face);
                                        duplicate = true;
                                        break;
                                    }
                                    start = start->next;
                                } while (start != (*it));
                            }
                        }
                    }
                }
                else {

                }

                pointsToremove.push_back(ptToRemove);
            }
        }
        //移除面
        std::vector<int> removepts;
        remove_faces(facesToremove, false, removepts);
    }

    bool GeometryTopology::remove_point(int ptnum) {
        if (ptnum < 0 || ptnum >= m_points.size())
            return false;

        std::set<int> remFaces;
        std::set<std::string> remHEdges;

        for (auto outEdge : m_points[ptnum]->edges) {
            assert(outEdge);

            HEdge* firstEdge = outEdge;
            HEdge* nextEdge = firstEdge->next;
            assert(nextEdge);
            HEdge* nnextEdge = nextEdge->next;
            assert(nnextEdge);

            auto& [prevPoint, prevEdge, pprevEdge] = getPrev(outEdge);
            assert(prevEdge && pprevEdge);
            if (nextEdge && nnextEdge == prevEdge) {
                //triangle，整个面和所有隶属这个面的半边都要移除
                remFaces.insert(outEdge->face);

                HEdge* h = outEdge;
                HEdge* prev = nullptr;
                do {
                    remHEdges.insert(h->id);
                    //对面先置空自己
                    if (h->pair)
                        h->pair->pair = nullptr;
                    if (prev) {
                        //当前边的起点的edges也需要清除自己，第一条边的顶点除外（本身就是要被删除的点）
                        m_points[prev->point]->edges.erase(h);
                    }
                    prev = h;
                    h = h->next;
                } while (h != outEdge);
            }
            else {
                remHEdges.insert(outEdge->id);
                remHEdges.insert(prevEdge->id);

                auto newEdge = std::make_shared<HEdge>();

                std::string id = generateUUID();
                newEdge->id = id;
                m_hEdges.insert(std::make_pair(id, newEdge));

                //connect between outEdge->point and prevPoint.
                newEdge->point = outEdge->point;
                newEdge->pair = nullptr;
                newEdge->next = nextEdge;
                newEdge->face = outEdge->face;

                m_faces[newEdge->face]->h = newEdge.get();

                pprevEdge->next = newEdge.get();
                prevPoint->edges.erase(prevEdge);
                prevPoint->edges.insert(newEdge.get());
            }
        }

        m_points.erase(m_points.begin() + ptnum);

        for (auto keyname : remHEdges) {
            m_hEdges.erase(keyname);
        }

        //adjust face
        std::vector<int> _remFaces;
        for (int idx : remFaces)
            _remFaces.push_back(idx);
        std::sort(_remFaces.begin(), _remFaces.end());

        for (auto iter = _remFaces.rbegin(); iter != _remFaces.rend(); iter++) {
            int rmIdx = *iter;
            m_faces.erase(m_faces.begin() + rmIdx);
        }

        for (auto& [_, hedge] : m_hEdges) {
            if (hedge->point >= ptnum) {
                hedge->point--;
            }
            int nStep = 0;
            for (auto remFaceId : _remFaces) {
                if (hedge->face >= remFaceId)
                    nStep++;
                else
                    break;
            }
            hedge->face -= nStep;
            /*
            auto ph = hedge.get();
            auto fh = m_faces[hedge->face]->h;
            if (m_bTriangle) {
                assert(fh == ph || fh == ph->next || fh == ph->next->next);
            }
            */
        }

        if (!m_linesPt.empty()) {
            for (size_t pt = ptnum; pt < m_linesPt.size(); ++pt) {
                m_linesPt[pt][0] -= 1;
                m_linesPt[pt][1] -= 1;
            }
            m_linesPt.erase(m_linesPt.end() - 1);
        }

        update_linear_vertex();

        return true;
    }

    bool GeometryTopology::remove_vertex(int face_id, int vert_id) {
        if (face_id < 0 || face_id >= m_faces.size() || vert_id < 0 || vert_id >= nvertices(face_id)) {
            return false;
        }

        HEdge* first = m_faces[face_id]->h;
        HEdge* vertEdge = first;
        do 
        {
            if (vert_id-- == 0) {
                break;
            }
            vertEdge = vertEdge->next;
        } while (vertEdge != first);
        auto& [prepoint, prevedge, pprevedge] = getPrev(vertEdge);

        if (vertEdge->next == pprevedge) {
            size_t removeFaceid = vertEdge->face;
            HEdge* startRemove = prevedge;
            do 
            {
                if (startRemove->pair) {
                    startRemove->pair->pair = nullptr;
                }
                m_points[startRemove->point]->edges.erase(startRemove->next);
                std::string removeEdgeId = startRemove->id;
                startRemove = startRemove->next;
                m_hEdges.erase(removeEdgeId);
            } while (startRemove != prevedge);

            for (auto& [_, hedge] : m_hEdges) {
                if (hedge->face >= removeFaceid) {
                    hedge->face--;
                }
            }
            m_faces.erase(m_faces.begin() + removeFaceid);
        } else {
            std::shared_ptr<HEdge> newedge = std::make_shared<HEdge>();
            newedge->id = generateUUID();
            newedge->pair = nullptr;
            newedge->next = vertEdge->next;
            newedge->point = vertEdge->point;
            newedge->face = vertEdge->face;

            m_points[prevedge->point]->edges.erase(vertEdge);
            m_points[pprevedge->point]->edges.erase(prevedge);
            m_points[pprevedge->point]->edges.insert(newedge.get());
            pprevedge->next = newedge.get();

            m_faces[newedge->face]->h = newedge.get();

            m_hEdges.erase(vertEdge->id);
            m_hEdges.erase(prevedge->id);
            m_hEdges.insert({newedge->id, newedge});
        }
        update_linear_vertex();

        return true;
    }

    bool GeometryTopology::remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) {
        if (faces.empty()) {
            return false;
        }

        std::set<int> remPoints;
        std::set<Point*> remPtrPoints;
        for (auto face_id : faces) {
            if (face_id < 0 || face_id >= m_faces.size()) {
                return false;
            }

            auto pFace = m_faces[face_id];

            auto firsth = pFace->h;
            auto h = firsth;
            do
            {
                auto& spPoint = m_points[h->point];
                spPoint->edges.erase(h->next);
                if (includePoints) {
                    //考察face上所有的点，是否有点需要被删除
                    if (spPoint->edges.empty()) {
                        remPoints.insert(h->point);
                        remPtrPoints.insert(spPoint.get());
                        removedPtnum.push_back(h->point);
                    }
                }
                if (h->pair) {
                    //对面置空自己
                    h->pair->pair = nullptr;
                }
                h->pair = nullptr;
                auto nexth = h->next;
                //h的依赖都解除了，现在就可以删除了。
                m_hEdges.erase(h->id);
                h = nexth;
            } while (firsth != h);
        }

        //边都已经删除了，现在只要删除点和面即可。
        removeElements(m_points, remPoints);
        removeElements(m_faces, faces);

        //因为点被删除了，故需要调整所有未被删除的边（只要调整point和face的索引），至于face则不需要调整，因为删除的就是face，直接删了完事
        for (auto& [_, hedge] : m_hEdges) {
            int nStep = 0;
            for (auto remPointId : remPoints) {
                if (hedge->point >= remPointId)
                    nStep++;
                else
                    break;
            }
            hedge->point -= nStep;

            nStep = 0;
            for (auto remFaceId : faces) {
                if (hedge->face >= remFaceId)
                    nStep++;
                else
                    break;
            }
            hedge->face -= nStep;

            auto ph = hedge.get();
            auto fh = m_faces[hedge->face]->h;
            if (m_bTriangle) {
                assert(fh == ph || fh == ph->next || fh == ph->next->next);
            }
        }

        update_linear_vertex();

        return true;
    }

    int GeometryTopology::add_point() {
        auto spPoint = std::make_shared<Point>();
        m_points.emplace_back(spPoint);
        return m_points.size() - 1;
    }

    int GeometryTopology::add_vertex(int face_id, int point_id) {
        if (face_id < 0 || face_id >= m_faces.size() ||
            point_id < 0 || point_id >= m_points.size()) {
            return -1;
        }
        auto& spPoint = m_points[point_id];
        if (spPoint->edges.empty()) {
            auto pFace = m_faces[face_id];
            assert(pFace);
            auto firstH = pFace->h;
            auto& [prevPoint, prevEdge, pprevEdge] = getPrev(firstH);
            auto newedge = std::make_shared<HEdge>();
            newedge->point = prevEdge->point;
            newedge->next = firstH;
            newedge->face = face_id;
            newedge->id = generateUUID();
            prevEdge->next = newedge.get();
            prevEdge->point = point_id;
            m_hEdges.insert(std::make_pair(newedge->id, newedge));
            spPoint->edges.insert(newedge.get());   //需要将newedge加入newedge的edges中？
            m_bTriangle = false;    //有增加意味着不能当作三角形处理了

            update_linear_vertex();

            return npoints_in_face(m_faces[face_id].get());
        }
        else {
            return -1;
        }
    }

    int GeometryTopology::addface(const std::vector<int>& points) {
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
            if (from_point == 1 && to_point == 0 || from_point == 7 && to_point == 5) {
                int j;
                j = 0;
            }

            if (i == 0) {
                if (face_id == 0) {
                    spFace->start_linearIdx = 0;
                }
                else {
                    spFace->start_linearIdx = m_faces[face_id - 1]->start_linearIdx + 
                        nvertices(face_id - 1);
                }
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
        return m_faces.size() - 1;
    }
}