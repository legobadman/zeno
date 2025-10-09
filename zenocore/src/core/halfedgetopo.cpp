#include "halfedgetopo.h"
#include <zeno/utils/format.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/vectorutil.h>


namespace zeno
{
    HalfEdgeTopology::HalfEdgeTopology(bool bTriangle, int nPoints, int nFaces, bool bInitFaces) {
        m_bTriangle = bTriangle;
        m_points.resize(nPoints);
        if (bInitFaces)
            m_faces.resize(nFaces);
        else
            m_faces.reserve(nFaces);
        for (int i = 0; i < nPoints; i++) {
            m_points[i] = new HF_Point;
        }
    }

    HalfEdgeTopology::~HalfEdgeTopology() {
    }

    HalfEdgeTopology::HalfEdgeTopology(const HalfEdgeTopology& rhs) {
        m_bTriangle = rhs.m_bTriangle;
#if 0
        auto spPrim = rhs.toPrimitive();
        initFromPrim(spPrim);
#else
        m_points.resize(rhs.m_points.size());
        m_faces.resize(rhs.m_faces.size());

        std::map<HF_Point*, int> rhs_point2idx;
        std::map<HF_Edge*, int> rhs_edge2idx;
        std::map<HF_Face*, int> rhs_face2idx;
        for (int i = 0; i < rhs.m_points.size(); i++) {
            rhs_point2idx.insert(std::make_pair(rhs.m_points[i], i));
        }
        for (int i = 0; i < rhs.m_hEdges.size(); i++) {
            rhs_edge2idx.insert(std::make_pair(rhs.m_hEdges[i], i));
        }
        for (int i = 0; i < rhs.m_faces.size(); i++) {
            rhs_face2idx.insert(std::make_pair(rhs.m_faces[i], i));
        }

        for (int i = 0; i < m_points.size(); i++) {
            m_points[i] = new HF_Point;
        }
        for (int i = 0; i < m_faces.size(); i++) {
            m_faces[i] = new HF_Face;
        }
        for (int i = 0; i < m_hEdges.size(); i++) {
            m_hEdges[i] = new HF_Edge;
        }


        for (int i = 0; i < m_points.size(); i++) {
            m_points[i]->edge_from_thispt = m_hEdges[rhs_edge2idx[rhs.m_points[i]->edge_from_thispt]];
        }

        for (int i = 0; i < m_hEdges.size(); i++) {
            auto rhs_pEdge = rhs.m_hEdges[i];
            m_hEdges[i]->point_from = m_points[rhs_point2idx[rhs_pEdge->point_from]];
            m_hEdges[i]->face = m_faces[rhs_face2idx[rhs_pEdge->face]];
            m_hEdges[i]->pair = m_hEdges[rhs_edge2idx[rhs_pEdge->pair]];
            m_hEdges[i]->next = m_hEdges[rhs_edge2idx[rhs_pEdge->next]];
        }

        for (int i = 0; i < m_faces.size(); i++) {
            m_faces[i]->h = m_hEdges[rhs_edge2idx[rhs.m_faces[i]->h]];
        }
#endif
    }

    GeomTopoType HalfEdgeTopology::type() const { return Topo_HalfEdge; }

    std::shared_ptr<IGeomTopology> HalfEdgeTopology::clone() {
        return std::make_shared<HalfEdgeTopology>(*this);
    }

    void HalfEdgeTopology::build_indice() {
        for (int i = 0; i < m_points.size(); i++) {
            m_temp_cache_point2idx.insert(std::make_pair(m_points[i], i));
        }
        for (int i = 0; i < m_hEdges.size(); i++) {
            m_temp_cache_edge2idx.insert(std::make_pair(m_hEdges[i], i));
        }
        for (int i = 0; i < m_faces.size(); i++) {
            m_temp_cache_face2idx.insert(std::make_pair(m_faces[i], i));
        }
    }

    void HalfEdgeTopology::toPrimitive(PrimitiveObject* spPrim) {
        int startIdx = 0;
        AttrVector<vec2i> lines;
        for (int iFace = 0; iFace < m_faces.size(); iFace++) {
            auto face = m_faces[iFace];
            HF_Edge* firsth = face->h;
            HF_Edge* h = firsth;
            assert(h != nullptr);
            std::vector<int> points;
            do {
                if (h == nullptr) {
                    break;
                }
                int index = getPointTo(h);
                points.push_back(index);
                h = h->next;
            } while (firsth != h);

            if (m_bTriangle) {
                vec3i tri = { points[0],points[1], points[2] };
                spPrim->tris.push_back(std::move(tri));
            }
            else {
                int sz = points.size();
                for (auto pt : points) {
                    spPrim->loops.push_back(pt);
                }
                spPrim->polys.push_back({ startIdx, sz });
                startIdx += sz;
            }
        }

        if (lines.size() > 0) {
            spPrim->lines = lines;
        }
    }

    void HalfEdgeTopology::initFromPrim(PrimitiveObject* prim) {
        int n = prim->verts->size();
        m_points.resize(n);
        for (int i = 0; i < m_points.size(); i++) {
            m_points[i] = new HF_Point;
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
            std::vector<int> point_indice;
            if (m_bTriangle) {
                auto const& ind = prim->tris[face];
                point_indice = { ind[0], ind[1], ind[2] };
            }
            else {
                auto const& poly = prim->polys[face];
                int startIdx = poly[0], nPoints = poly[1];
                auto const& loops = prim->loops;
                for (int i = 0; i < nPoints; i++) {
                    point_indice.push_back(loops[startIdx + i]);
                }
            }

            //TODO: init data for Face
            auto pFace = new HF_Face;

            HF_Edge* lastHedge = 0, * firstHedge = 0;
            for (int i = 0; i < point_indice.size(); i++) {
                int vp = -1, vq = -1;
                if (i < point_indice.size() - 1) {
                    vp = point_indice[i];
                    vq = point_indice[i + 1];
                }
                else {
                    vp = point_indice[i];
                    vq = point_indice[0];
                }

                //vp->vq
                auto hedge = new HF_Edge;
                m_hEdges.push_back(hedge);

                hedge->face = pFace;
                hedge->point_from = m_points[vp];

                if (lastHedge) {
                    lastHedge->next = hedge;
                }
                //TODO: 如果只有一条边会怎么样？
                if (i == point_indice.size() - 1) {
                    hedge->next = firstHedge;
                }
                else if (i == 0) {
                    firstHedge = hedge;
                    /*
                    int nF = m_faces.size();
                    if (m_faces.empty())
                        pFace->start_linearIdx = 0;
                    else {
                        pFace->start_linearIdx = m_faces[nF - 1]->start_linearIdx + nvertices(nF - 1);
                    }
                    */
                }

                //check whether the pair edge exist
                auto pairedge = checkHEdge(vq, vp);
                if (pairedge) {
                    hedge->pair = pairedge;
                    pairedge->pair = hedge;
                }

                m_points[vp]->edge_from_thispt = hedge;

                lastHedge = hedge;
            }
            //统一取第一条半边作为face的“起点边”
            pFace->h = firstHedge;

            m_faces[face] = pFace;
        }

        //update_linear_vertex();
    }

    std::vector<HF_Edge*> HalfEdgeTopology:: find_alledges_from(int fromPoint) {
        std::vector<HF_Edge*> edges;
        HF_Point* pt = this->m_points[fromPoint];
        HF_Edge* p = pt->edge_from_thispt;
        do {
            auto pair_p = p->pair;
            edges.push_back(p);
            p = pair_p->next;
        } while (p != pt->edge_from_thispt);
        return edges;
    }

    HF_Edge* HalfEdgeTopology::checkHEdge(size_t fromPoint, size_t toPoint) {
        if (!(fromPoint < m_points.size() && 
            toPoint < m_points.size() &&
            fromPoint >= 0 &&
            toPoint >= 0)) {
            throw makeError<UnimplError>("invalid index");
        }
        HF_Point* fromP = m_points[fromPoint];
        HF_Point* toP = m_points[toPoint];

        auto edges1 = find_alledges_from(fromPoint);
        auto edges2 = find_alledges_from(toPoint);
        for (auto hedge : edges1) {
            if (hedge->next->point_from == toP) {
                return hedge;
            }
        }
        return nullptr;
    }

    std::tuple<HF_Point*, HF_Edge*, HF_Edge*> HalfEdgeTopology::getPrev(HF_Edge* outEdge) {
        HF_Edge* h = outEdge, * prev = nullptr;
        do {
            prev = h;
            h = h->next;
        } while (h && h->next != outEdge);
        return { outEdge->point_from, h, prev };
    }

    size_t HalfEdgeTopology::getNextOutEdge(size_t fromPoint, size_t currentOutEdge) {
        return -1;
    }

    size_t HalfEdgeTopology::getPointTo(HF_Edge* hedge) const {
        return hedge->point;
    }

    std::vector<vec3i> HalfEdgeTopology::tri_indice() const {
        std::vector<vec3i> indice;
        for (auto spFace : m_faces) {
            HF_Edge* h = spFace->h;
            vec3i tri(h->point, h->next->point, h->next->next->point);
            indice.push_back(tri);
            indice.push_back(vec3i(h->next->next->next->point, h->point, h->next->next->point));
        }
        return indice;
    }

    std::vector<std::vector<int>> HalfEdgeTopology::face_indice() const {
        std::vector<std::vector<int>> indice;
        for (int i = 0; i < m_faces.size(); i++) {
            std::vector<int> face = face_points(i);
            indice.emplace_back(face);
        }
        return indice;
    }

    std::vector<int> HalfEdgeTopology::edge_list() const {
        std::vector<int> edges;
        for (auto spFace : m_faces) {
            HF_Edge* h = spFace->h;
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

    bool HalfEdgeTopology::is_base_triangle() const {
        return m_bTriangle;
    }

    bool HalfEdgeTopology::is_line() const {
        //可能是线和面混用，所以这个api的意义是什么？
        return false;
        //return !m_linesPt.empty();
    }

    int HalfEdgeTopology::npoints_in_face(HF_Face* face) const {
        auto h = face->h;
        int ncount = 0;
        do {
            ncount++;
            if (h == nullptr) {
                //line
                break;
            }
            h = h->next;
        } while (face->h != h);
        return ncount;
    }

    void HalfEdgeTopology::geomTriangulate(zeno::TriangulateInfo& info) {
        //TODO: no use right now
#if 0
        boolean_switch(info.has_lines, [&](auto has_lines) {
            std::vector<std::conditional_t<has_lines.value, vec2i, int>> scansum(m_faces.size());
            auto redsum = parallel_exclusive_scan_sum(m_faces.begin(), m_faces.end(),
                scansum.begin(), [&](auto& face) {
                    int npts = npoints_in_face(face);
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
                    HF_Face* f = m_faces[i];
                    HF_Edge* hstart = f->h;
                    int start = hstart->find_from();
                    int len = npoints_in_face(f);
                    if (len >= 3) {
                        int scanbase = 0;
                        if constexpr (has_lines.value) {
                            scanbase = scansum[i][0] + tribase;
                        }
                        else {
                            scanbase = scansum[i] + tribase;
                        }

                        HF_Edge* h = hstart;
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
#endif
    }

    int HalfEdgeTopology::face_point(int face_id, int vert_id) const {
        if (face_id < 0 || face_id >= m_faces.size())
            return -1;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        do {
            if (vert_id-- == 0) {
                return h->point;
            }
            h = h->next;
            if (!h)
                break;
        } while (h != firsth);
        return -1;
    }

    std::vector<int> HalfEdgeTopology::face_points(int face_id) const {
        std::vector<int> pts;
        if (face_id < 0 || face_id >= m_faces.size())
            return pts;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        assert(firsth->point_from != -1);
        pts.push_back(h->point_from);
        do {
            pts.push_back(h->point);
            h = h->next;
            if (!h) {//line
                break;
            }
        } while (h->next != firsth);
        return pts;
    }

    std::vector<int> HalfEdgeTopology::face_vertices(int face_id) {
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

    std::vector<int> HalfEdgeTopology::point_faces(int point_id) {
        std::vector<int> faces;
        if (point_id < 0 || point_id >= m_points.size())
            return faces;

        for (auto h : m_points[point_id]->edges) {
            faces.push_back(h->face);
        }
        return faces;
    }

    int HalfEdgeTopology::point_vertex(int point_id) {
        std::vector<int> vertices = point_vertices(point_id);
        if (vertices.empty())
            return -1;
        return vertices[0];
    }

    std::vector<int> HalfEdgeTopology::point_vertices(int point_id) {
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
                    if (!h) break;
                }
                vertices.push_back(spFace->start_linearIdx + nCount);
            }
        }
        std::sort(vertices.begin(), vertices.end());
        return vertices;
    }

    int HalfEdgeTopology::npoints() const {
        return m_points.size();
    }

    int HalfEdgeTopology::nfaces() const {
        return m_faces.size();
    }

    int HalfEdgeTopology::nvertices() const {
        if (m_faces.empty()) {
            return 0;
        }
        else {
            int idxLast = m_faces.size() - 1;
            return m_faces[idxLast]->start_linearIdx + nvertices(idxLast);
        }
    }

    int HalfEdgeTopology::nvertices(int face_id) const {
        if (face_id < 0 || face_id >= m_faces.size()) {
            return 0;
        }
        return npoints_in_face(m_faces[face_id]);
    }

    int HalfEdgeTopology::face_vertex(int face_id, int vert_id) {
        if (face_id < 0 || face_id >= m_faces.size() || vert_id < 0 || vert_id >= nvertices(face_id)) {
            return -1;
        }
        return m_faces[face_id]->start_linearIdx + vert_id;
    }

    int HalfEdgeTopology::face_vertex_count(int face_id) {
        return nvertices(face_id);
    }

    int HalfEdgeTopology::vertex_index(int face_id, int vertex_id) {
        return face_vertex(face_id, vertex_id);
    }

    /*
     * 与linear_vertex_id共享一个point的下一个vertex的linear_vertex_id;
     */
    int HalfEdgeTopology::vertex_next(int linear_vertex_id) {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto vertices = point_vertices(pointid);
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

    int HalfEdgeTopology::vertex_prev(int linear_vertex_id) {
        int pointid = vertex_point(linear_vertex_id);
        if (pointid == -1) {
            return -1;
        }

        auto vertices = point_vertices(pointid);
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

    int HalfEdgeTopology::vertex_point(int linear_vertex_id) {
        return std::get<2>(vertex_info(linear_vertex_id));
    }

    std::tuple<int, int, int> HalfEdgeTopology::vertex_info(int linear_vertex_id) {
        int faceid = vertex_face(linear_vertex_id);
        if (faceid == -1)
            return { -1,-1,-1 };

        auto& spFace = m_faces[faceid];
        int offset = linear_vertex_id - spFace->start_linearIdx;
        if (offset == 0) {
            return { faceid, 0, spFace->h->find_from() };
        }
        if (offset >= nvertices(faceid))
            return { -1,-1,-1 };

        auto h = spFace->h;
        while (--offset) {
            if (!h) {
                break;
            }
            h = h->next;
        }
        return { faceid, linear_vertex_id - spFace->start_linearIdx, h->point };
    }

    /*
     * 与linear_vertex_id关联的face的id;
     */
    int HalfEdgeTopology::vertex_face(int linear_vertex_id) {
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
    int HalfEdgeTopology::vertex_face_index(int linear_vertex_id) {
        int idxFace = vertex_face(linear_vertex_id);
        if (idxFace == -1)
            return -1;

        auto spFace = m_faces[idxFace];
        auto pFirstH = spFace->h;
        auto h = pFirstH;
        int offset = linear_vertex_id - spFace->start_linearIdx;
        return offset;
    }

    void HalfEdgeTopology::update_linear_vertex()
    {
        //更新linearIdx
        if (!m_faces.empty()) {
            m_faces[0]->start_linearIdx = 0;
            for (int i = 1; i < m_faces.size(); ++i) {
                m_faces[i]->start_linearIdx = m_faces[i - 1]->start_linearIdx + nvertices(i - 1);
            }
        }
    }

    bool HalfEdgeTopology::remove_point(int ptnum) {
        if (ptnum < 0 || ptnum >= m_points.size())
            return false;

        std::set<HF_Face*> remFaces;
        std::set<HF_Edge*> remHEdges;

        const std::vector<HF_Edge*> outedges_in_rem_pt = find_alledges_from(ptnum);

        for (auto outEdge : outedges_in_rem_pt) {
            assert(outEdge);

            HF_Edge* nextEdge = outEdge->next;
            assert(nextEdge);
            HF_Edge* nnextEdge = nextEdge->next;
            assert(nnextEdge);

            auto [prevPoint, prevEdge, pprevEdge] = getPrev(outEdge);
            assert(prevEdge && pprevEdge);
            if (nextEdge && nnextEdge == prevEdge) {
                //triangle，整个面和所有隶属这个面的半边都要移除
                remFaces.insert(outEdge->face);

                HF_Edge* h = outEdge;
                HF_Edge* prev = nullptr;
                do {
                    remHEdges.insert(h);
                    //对面先置空自己
                    if (h->pair)
                        h->pair->pair = nullptr;
                    if (prev) {
                        if (h->point_from->edge_from_thispt == h) {
                            //估计不用这么操作，后续应该会对edge_from_thispt统一整理
                            h->point_from->edge_from_thispt = 0;
                        }
                    }
                    prev = h;
                    h = h->next;
                } while (h != outEdge);
            }
            else {
                remHEdges.insert(outEdge);
                remHEdges.insert(prevEdge);

                auto newEdge = new HF_Edge;
                m_hEdges.push_back(newEdge);

                //connect between outEdge->point and prevPoint.
                newEdge->point_from = prevEdge->point_from;
                newEdge->pair = nullptr;
                newEdge->next = nextEdge;
                newEdge->face = outEdge->face;

                int idxFace = idx_of_face(newEdge->face);
                m_faces[idxFace]->h = newEdge;

                pprevEdge->next = newEdge;
                prevPoint->edge_from_thispt = newEdge;
            }
        }

        m_points.erase(m_points.begin() + ptnum);

        for (auto iter = m_hEdges.begin(); iter != m_hEdges.end();) {
            if (remHEdges.find(*iter) != remHEdges.end()) {
                remHEdges.erase(*iter);
                iter = m_hEdges.erase(iter);
            }
            else {
                iter++;
            }
        }

        for (auto iter = m_faces.begin(); iter != m_faces.end();) {
            if (remFaces.find(*iter) != remFaces.end()) {
                remFaces.erase(*iter);
                iter = m_faces.erase(iter);
            }
            else {
                iter++;
            }
        }

        //删除半边以后，HF_Face::h和HF_Point::edge_from_thispt可能失效
        //不过上述循环可能已经解决了
        update_linear_vertex();
        return true;
    }

    bool HalfEdgeTopology::remove_vertex(int face_id, int vert_id) {
        if (face_id < 0 || face_id >= m_faces.size() || vert_id < 0 || vert_id >= nvertices(face_id)) {
            return false;
        }

        HF_Edge* first = m_faces[face_id]->h;
        HF_Edge* vertEdge = first;
        do
        {
            if (vert_id-- == 0) {
                break;
            }
            vertEdge = vertEdge->next;
            if (!vertEdge)
                break;
        } while (vertEdge != first);

        bool isline = false;//line
        first = vertEdge;
        do
        {
            if (first)
                first = first->next;
            if (!first) {
                isline = true;
                break;
            }
        } while (vertEdge != first);
        if (isline) {
            if (m_hEdges.size() == 1) {//one edge line case
                HF_Edge* e = m_hEdges.begin()->second;
                m_points[e->point_from]->edges.erase(e);
                m_faces.erase(m_faces.begin() + e->face);
                m_hEdges.clear();
            }
            else {
                m_points[m_points.size() - 2]->edges.erase(vertEdge);

                HF_Point* prevPoint = nullptr;
                HF_Edge* prevEdge = nullptr, * pprevEdge = nullptr;
                for (auto& [_, e] : m_hEdges) {
                    if (e->next == vertEdge) {
                        prevEdge = e;
                        prevPoint = m_points[prevEdge->point_from];
                    }
                    else if (e->next && e->next->next == vertEdge) {
                        pprevEdge = e;
                    }
                }
                if (!vertEdge) {//rm last vertex
                    prevPoint->edges.erase(prevEdge);
                    if (pprevEdge) {
                        pprevEdge->next = nullptr;
                    }
                    if (prevEdge) {
                        m_hEdges.erase(prevEdge->id);
                    }
                }
                else {
                    if (prevEdge) {
                        auto newedge = new HF_Edge;
                        newedge->id = generateUUID();
                        newedge->pair = nullptr;
                        newedge->next = vertEdge->next;
                        newedge->point = vertEdge->point;
                        newedge->point_from = prevEdge->point_from;
                        newedge->face = vertEdge->face;

                        m_points[prevEdge->point]->edges.erase(vertEdge);
                        prevPoint->edges.erase(prevEdge);
                        prevPoint->edges.insert(newedge);
                        if (pprevEdge) {
                            pprevEdge->next = newedge;
                        }
                        else {
                            m_faces[newedge->face]->h = newedge;
                        }
                        m_hEdges.erase(vertEdge->id);
                        m_hEdges.erase(prevEdge->id);
                        m_hEdges.insert({ newedge->id, newedge });
                    }
                    else {
                        m_faces[vertEdge->face]->h = vertEdge->next;
                        m_points[vertEdge->point_from]->edges.erase(vertEdge);
                        m_hEdges.erase(vertEdge->id);
                    }
                }
            }
        }
        else {
            auto [prepoint, prevedge, pprevedge] = getPrev(vertEdge);

            if (vertEdge->next == pprevedge) {
                size_t removeFaceid = vertEdge->face;
                HF_Edge* startRemove = prevedge;
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
            }
            else {
                auto newedge = new HF_Edge;
                newedge->id = generateUUID();
                newedge->pair = nullptr;
                newedge->next = vertEdge->next;
                newedge->point = vertEdge->point;
                //TODO: newedge->point_from = ?
                newedge->face = vertEdge->face;

                m_points[prevedge->point]->edges.erase(vertEdge);
                m_points[pprevedge->point]->edges.erase(prevedge);
                m_points[pprevedge->point]->edges.insert(newedge);
                pprevedge->next = newedge;

                m_faces[newedge->face]->h = newedge;

                m_hEdges.erase(vertEdge->id);
                m_hEdges.erase(prevedge->id);
                m_hEdges.insert({ newedge->id, newedge });
            }
        }
        update_linear_vertex();

        return true;
    }

    bool HalfEdgeTopology::remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) {
        if (faces.empty()) {
            return false;
        }

        std::set<int> remPoints;
        std::set<HF_Point*> remPtrPoints;
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
                        remPtrPoints.insert(spPoint);
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
                if (!h)
                    break;
            } while (firsth != h);
        }

        std::sort(removedPtnum.begin(), removedPtnum.end());

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
            for (auto remPointId : remPoints) {
                if (hedge->point_from >= remPointId)
                    nStep++;
                else
                    break;
            }
            hedge->point_from -= nStep;

            nStep = 0;
            for (auto remFaceId : faces) {
                if (hedge->face >= remFaceId)
                    nStep++;
                else
                    break;
            }
            hedge->face -= nStep;

            auto ph = hedge;
            auto fh = m_faces[hedge->face]->h;
            if (m_bTriangle) {
                assert(fh == ph || fh == ph->next || fh == ph->next->next);
            }
        }

        update_linear_vertex();

        return true;
    }

    int HalfEdgeTopology::add_point() {
        m_points.push_back(new HF_Point);
        return m_points.size() - 1;
    }

    int HalfEdgeTopology::add_vertex(int face_id, int point_id) {
        if (face_id < 0 || face_id >= m_faces.size() ||
            point_id < 0 || point_id >= m_points.size()) {
            return -1;
        }
        auto& spPoint = m_points[point_id];
        if (spPoint->edges.empty()) {
            auto pFace = m_faces[face_id];
            assert(pFace);
            auto firstH = pFace->h;
            auto [prevPoint, prevEdge, pprevEdge] = getPrev(firstH);
            auto newedge = new HF_Edge;
            newedge->point = prevEdge->point;
            newedge->next = firstH;
            newedge->face = face_id;
            newedge->id = generateUUID();
            prevEdge->next = newedge;
            prevEdge->point = point_id;
            m_hEdges.insert(std::make_pair(newedge->id, newedge));
            spPoint->edges.insert(newedge);   //需要将newedge加入newedge的edges中？
            m_bTriangle = false;    //有增加意味着不能当作三角形处理了

            update_linear_vertex();

            return npoints_in_face(m_faces[face_id]);
        }
        else {
            return -1;
        }
    }

    void HalfEdgeTopology::set_face(int idx, const std::vector<int>& points, bool bClose) {
        //points要按照逆时针方向
        HF_Face* spFace = new HF_Face;
        size_t face_id = idx;

        std::vector<HF_Edge*> edges;
        for (size_t i = 0; i < points.size(); i++) {
            size_t from_point = -1, to_point = -1;
            if (i == points.size() - 1) {
                if (!bClose)
                    continue;   //line
                from_point = points[i];
                to_point = points[0];
            }
            else {
                //edge: from i to i+1
                from_point = points[i];
                to_point = points[i + 1];
            }

            //DEBUG:
            if (from_point == 1 && to_point == 0 || from_point == 7 && to_point == 5) {
                int j;
                j = 0;
            }

            if ((bClose && i == 0) || (!bClose && i == 1)) {
                if (face_id == 0) {
                    spFace->start_linearIdx = 0;
                }
                else {
                    spFace->start_linearIdx = m_faces[face_id - 1]->start_linearIdx +
                        nvertices(face_id - 1);
                }
            }

            auto hedge = new HF_Edge;
            hedge->face = face_id;
            hedge->point = to_point;
            hedge->point_from = from_point;
            std::string id = zeno::format("{}->{}", from_point, to_point);
            hedge->id = id;

            if (hedge->point == hedge->point_from) {
                int j;
                j = 0;
            }

            auto fromPoint = m_points[from_point];
            assert(fromPoint);
            fromPoint->edges.insert(hedge);

            //check whether the edge from to_point to from_point.
            auto toPoint = m_points[to_point];
            assert(toPoint);
            for (HF_Edge* outEdge : toPoint->edges) {
                if (outEdge->point == from_point) {
                    outEdge->pair = hedge;
                    hedge->pair = outEdge;
                    break;
                }
            }

            m_hEdges.insert(std::make_pair(id, hedge));
            edges.push_back(hedge);
        }

        for (size_t i = 0; i < edges.size(); i++) {
            if (i == edges.size() - 1) {
                if (bClose) {
                    edges[i]->next = edges[0];
                }
            }
            else {
                edges[i]->next = edges[i + 1];
            }
        }

        spFace->h = edges[0];
        m_faces[idx] = spFace;
    }

    int HalfEdgeTopology::add_face(const std::vector<int>& points, bool bClose) {
        //points要按照逆时针方向
        HF_Face* spFace = new HF_Face;
        size_t face_id = m_faces.size();

        std::vector<HF_Edge*> edges;
        for (size_t i = 0; i < points.size(); i++) {
            size_t from_point = -1, to_point = -1;
            if (i == points.size() - 1) {
                if (!bClose)
                    continue;   //line
                from_point = points[i];
                to_point = points[0];
            }
            else {
                //edge: from i to i+1
                from_point = points[i];
                to_point = points[i + 1];
            }

            //DEBUG:
            if (from_point == 1 && to_point == 0 || from_point == 7 && to_point == 5) {
                int j;
                j = 0;
            }

            if ((bClose && i == 0) || (!bClose && i == 1)) {
                if (face_id == 0) {
                    spFace->start_linearIdx = 0;
                }
                else {
                    spFace->start_linearIdx = m_faces[face_id - 1]->start_linearIdx +
                        nvertices(face_id - 1);
                }
            }

            auto hedge = new HF_Edge;
            hedge->face = face_id;
            hedge->point = to_point;
            hedge->point_from = from_point;
            std::string id = zeno::format("{}->{}", from_point, to_point);
            hedge->id = id;

            if (hedge->point == hedge->point_from) {
                int j;
                j = 0;
            }

            auto fromPoint = m_points[from_point];
            assert(fromPoint);
            fromPoint->edges.insert(hedge);

            //check whether the edge from to_point to from_point.
            auto toPoint = m_points[to_point];
            assert(toPoint);
            for (HF_Edge* outEdge : toPoint->edges) {
                if (outEdge->point == from_point) {
                    outEdge->pair = hedge;
                    hedge->pair = outEdge;
                    break;
                }
            }

            m_hEdges.insert(std::make_pair(id, hedge));
            edges.push_back(hedge);
        }

        for (size_t i = 0; i < edges.size(); i++) {
            if (i == edges.size() - 1) {
                if (bClose) {
                    edges[i]->next = edges[0];
                }
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