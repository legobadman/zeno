#include "halfedgetopo.h"
#include <zeno/utils/format.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/vectorutil.h>


namespace zeno
{
    struct IndiceCache {
        IndiceCache(HalfEdgeTopology* topo) : m_topo(topo) {}

        void clear_indice_cache() {
            m_temp_cache_point2idx.clear();
            m_temp_cache_edge2idx.clear();
            m_temp_cache_face2idx.clear();
            m_linearidx_in_face.clear();
        }

        void build_indice() {
            if (!m_temp_cache_point2idx.empty()) {
                return;
            }

            for (int i = 0; i < m_topo->m_points.size(); i++) {
                m_temp_cache_point2idx.insert(std::make_pair(m_topo->m_points[i], i));
            }
            for (int i = 0; i < m_topo->m_hEdges.size(); i++) {
                m_temp_cache_edge2idx.insert(std::make_pair(m_topo->m_hEdges[i], i));
            }
            for (int i = 0; i < m_topo->m_faces.size(); i++) {
                m_temp_cache_face2idx.insert(std::make_pair(m_topo->m_faces[i], i));
            }
        }

        int idx_of_point(HF_Point* point) const {
            if (!m_temp_cache_point2idx.empty())
                return m_temp_cache_point2idx.at(point);
            return std::find(m_topo->m_points.begin(), m_topo->m_points.end(), point) - m_topo->m_points.begin();
        }

        int idx_of_face(HF_Face* face) const {
            if (!m_temp_cache_face2idx.empty())
                return m_temp_cache_face2idx.at(face);
            return std::find(m_topo->m_faces.begin(), m_topo->m_faces.end(), face) - m_topo->m_faces.begin();
        }

        int idx_of_edge(HF_Edge* edge) const {
            if (!m_temp_cache_edge2idx.empty())
                return m_temp_cache_edge2idx.at(edge);
            return std::find(m_topo->m_hEdges.begin(), m_topo->m_hEdges.end(), edge) - m_topo->m_hEdges.begin();
        }

        int linear_idx_of_face(int faceid) {
            if (m_linearidx_in_face.empty()) {
                //建立缓存
                update_linear_vertex();
            }
            return m_linearidx_in_face[faceid];
        }

        void update_linear_vertex() {
            //更新linearIdx
            if (!m_topo->m_faces.empty()) {
                m_linearidx_in_face.resize(m_topo->m_faces.size());
                m_linearidx_in_face[0] = 0;
                for (int i = 1; i < m_topo->m_faces.size(); ++i) {
                    m_linearidx_in_face[i] = m_linearidx_in_face[i - 1] + m_topo->nvertices(i - 1);
                }
            }
        }

    private:
        std::unordered_map<HF_Point*, int> m_temp_cache_point2idx;
        std::unordered_map<HF_Edge*, int>  m_temp_cache_edge2idx;
        std::unordered_map<HF_Face*, int>  m_temp_cache_face2idx;

        //linear index就是vertex在全局所有vertex的排行，比如：
        //第一个面有四个顶点，那么第一个面的第一个vertex的linear_idx就是0
        //第二个面有三个顶点，那么第二个面的第一个vertex的linear_idx就是4（前面有第一个面的四个点，分别是0,1,2,3)
        // 
        //此外，一个面的linear index也就是这个面第一个vertex的linear idx
        std::vector<int> m_linearidx_in_face;
        HalfEdgeTopology* m_topo;
    };


    HalfEdgeTopology::HalfEdgeTopology(bool bTriangle, int nPoints, int nFaces, bool bInitFaces)
        : m_pCache(std::make_unique<IndiceCache>(this))
    {
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

    HalfEdgeTopology::HalfEdgeTopology()
        : m_pCache(std::make_unique<IndiceCache>(this))
    {
    }

    HalfEdgeTopology::~HalfEdgeTopology() {
    }

    HalfEdgeTopology::HalfEdgeTopology(const HalfEdgeTopology& rhs)
        : m_pCache(std::make_unique<IndiceCache>(this))
    {
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

    void HalfEdgeTopology::toPrimitive(PrimitiveObject* spPrim) {
        m_pCache->build_indice();

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

    void HalfEdgeTopology::initFromPrim(int n_points, PrimitiveObject* prim) {
        m_points.resize(n_points);
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
    }

    std::vector<HF_Edge*> HalfEdgeTopology::find_alledges_from(int fromPoint) const {
        return find_alledges_from(m_points[fromPoint]);
    }

    std::vector<HF_Edge*> HalfEdgeTopology::find_alledges_from(HF_Point* fromPoint) const {
        std::vector<HF_Edge*> edges;
        HF_Edge* p = fromPoint->edge_from_thispt;
        bool meet_boundary = false;
        if (!p)
            return edges;
        do {
            auto pair_p = p->pair;
            edges.push_back(p);
            if (pair_p == nullptr) {
                meet_boundary = true;
                break;
            }
            p = pair_p->next;
        } while (p != fromPoint->edge_from_thispt);

        if (meet_boundary) {
            p = fromPoint->edge_from_thispt;
            //往反方向找了
            auto prevEdge = getPrevEdge(p);
            do {
                p = prevEdge->pair;
                if (p) {
                    edges.push_back(p);
                    prevEdge = getPrevEdge(p);
                }
                else {
                    break;
                }
            } while (p && p != fromPoint->edge_from_thispt);
        }

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

    std::tuple<HF_Point*, HF_Edge*, HF_Edge*> HalfEdgeTopology::getPrev(HF_Edge* outEdge) const {
        HF_Edge* h = outEdge, * prev = nullptr;
        do {
            prev = h;
            h = h->next;
        } while (h && h->next != outEdge);
        return { outEdge->point_from, h, prev };
    }

    HF_Edge* HalfEdgeTopology::getPrevEdge(HF_Edge* outEdge) const {
        HF_Edge* h = outEdge, * prev = nullptr;
        do {
            prev = h;
            h = h->next;
        } while (h && h->next != outEdge);
        return h;
    }

    size_t HalfEdgeTopology::getNextOutEdge(size_t fromPoint, size_t currentOutEdge) {
        return -1;
    }

    size_t HalfEdgeTopology::getPointTo(HF_Edge* hedge) const {
        return m_pCache->idx_of_point(get_point_to(hedge));
    }

    std::vector<vec3i> HalfEdgeTopology::tri_indice() const {
        m_pCache->build_indice();

        //这有可能是针对四边形，分出两个三角形的索引
        std::vector<vec3i> indice;
        for (auto spFace : m_faces) {
            HF_Edge* h = spFace->h;

            int idx_of_h = m_pCache->idx_of_point(get_point_to(h));
            int idx_of_hnext = m_pCache->idx_of_point(get_point_to(h->next));
            int idx_of_hnextnext = m_pCache->idx_of_point(get_point_to(h->next->next));
            int idx_of_hnextnextnext = m_pCache->idx_of_point(get_point_to(h->next->next->next));

            vec3i tri(idx_of_h, idx_of_hnext, idx_of_hnextnext);
            indice.push_back(tri);
            indice.push_back(vec3i(idx_of_hnextnextnext, idx_of_h, idx_of_hnextnext));
        }
        return indice;
    }

    std::vector<std::vector<int>> HalfEdgeTopology::face_indice() const {
        m_pCache->build_indice();

        std::vector<std::vector<int>> indice;
        for (int i = 0; i < m_faces.size(); i++) {
            std::vector<int> face = face_points(i);
            indice.emplace_back(face);
        }
        return indice;
    }

    std::vector<int> HalfEdgeTopology::edge_list() const {
        m_pCache->build_indice();

        std::vector<int> edges;
        for (auto spFace : m_faces) {
            HF_Edge* h = spFace->h;
            while (h->next != spFace->h) {
                edges.push_back(m_pCache->idx_of_point(h->point_from));
                edges.push_back(m_pCache->idx_of_point(get_point_to(h)));
                h = h->next;
            }
            edges.push_back(m_pCache->idx_of_point(h->point_from));
            edges.push_back(m_pCache->idx_of_point(get_point_to(h)));
        }
        return edges;
    }

    bool HalfEdgeTopology::is_base_triangle() const {
        return m_bTriangle;
    }

    bool HalfEdgeTopology::is_line() const {
        return false;
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
        m_pCache->build_indice();

        if (face_id < 0 || face_id >= m_faces.size())
            return -1;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        do {
            if (vert_id-- == 0) {
                return m_pCache->idx_of_point(get_point_to(h));
            }
            h = h->next;
            if (!h)
                break;
        } while (h != firsth);
        return -1;
    }

    std::vector<int> HalfEdgeTopology::face_points(int face_id) const {
        m_pCache->build_indice();

        std::vector<int> pts;
        if (face_id < 0 || face_id >= m_faces.size())
            return pts;

        auto h = m_faces[face_id]->h;
        auto firsth = h;
        int point_from = m_pCache->idx_of_point(firsth->point_from);
        assert(point_from != -1);
        pts.push_back(point_from);
        do {
            int pointto = m_pCache->idx_of_point(get_point_to(h));
            pts.push_back(pointto);
            h = h->next;
            if (!h) {//line
                break;
            }
        } while (h->next != firsth);
        return pts;
    }

    std::vector<int> HalfEdgeTopology::face_vertices(int face_id) const {
        m_pCache->build_indice();

        if (face_id < 0 || face_id >= m_faces.size())
            return std::vector<int>();

        auto start_linear_idx = m_pCache->linear_idx_of_face(face_id);
        int nVertex = nvertices(face_id);
        std::vector<int> vertices(nVertex);
        for (int i = 0; i < nVertex; i++) {
            vertices[i] = start_linear_idx + i;
        }
        return vertices;
    }

    std::vector<int> HalfEdgeTopology::point_faces(int point_id) const {
        m_pCache->build_indice();

        std::vector<int> faces;
        if (point_id < 0 || point_id >= m_points.size())
            return faces;

        for (auto h : find_alledges_from(m_points[point_id])) {
            faces.push_back(m_pCache->idx_of_face(h->face));
        }
        return faces;
    }

    int HalfEdgeTopology::point_vertex(int point_id) const {
        m_pCache->build_indice();

        std::vector<int> vertices = point_vertices(point_id);
        if (vertices.empty())
            return -1;
        return vertices[0];
    }

    std::vector<int> HalfEdgeTopology::point_vertices(int point_id) const {
        m_pCache->build_indice();

        std::vector<int> vertices;
        if (point_id < 0 || point_id >= m_points.size())
            return vertices;

        for (auto pEdge : find_alledges_from(m_points[point_id])) {
            auto spFace = pEdge->face;
            int faceid = m_pCache->idx_of_face(spFace);
            auto pFirstH = spFace->h;
            auto h = pFirstH;
            int nCount = 1;

            if (m_points[point_id] == pFirstH->point_from) {
                vertices.push_back(faceid);
            }
            else {
                while (get_point_to(h) != m_points[point_id]) {
                    h = h->next;
                    nCount++;
                    if (!h) break;
                }
                vertices.push_back(m_pCache->linear_idx_of_face(faceid) + nCount);
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
        m_pCache->build_indice();

        if (m_faces.empty()) {
            return 0;
        }
        else {
            int idxLast = m_faces.size() - 1;
            return m_pCache->linear_idx_of_face(idxLast) + nvertices(idxLast);
        }
    }

    int HalfEdgeTopology::nvertices(int face_id) const {
        m_pCache->build_indice();
        if (face_id < 0 || face_id >= m_faces.size()) {
            return 0;
        }
        return npoints_in_face(m_faces[face_id]);
    }

    int HalfEdgeTopology::face_vertex(int face_id, int vert_id) const {
        m_pCache->build_indice();
        if (face_id < 0 || face_id >= m_faces.size() || vert_id < 0 || vert_id >= nvertices(face_id)) {
            return -1;
        }
        return m_pCache->linear_idx_of_face(face_id) + vert_id;
    }

    int HalfEdgeTopology::face_vertex_count(int face_id) const {
        m_pCache->build_indice();
        return nvertices(face_id);
    }

    int HalfEdgeTopology::vertex_index(int face_id, int vertex_id) const {
        m_pCache->build_indice();
        return face_vertex(face_id, vertex_id);
    }

    /*
     * 与linear_vertex_id共享一个point的下一个vertex的linear_vertex_id;
     */
    int HalfEdgeTopology::vertex_next(int linear_vertex_id) const {
        m_pCache->build_indice();

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

    int HalfEdgeTopology::vertex_prev(int linear_vertex_id) const {
        m_pCache->build_indice();

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

    int HalfEdgeTopology::vertex_point(int linear_vertex_id) const {
        m_pCache->build_indice();
        return std::get<2>(vertex_info(linear_vertex_id));
    }

    std::tuple<int, int, int> HalfEdgeTopology::vertex_info(int linear_vertex_id) const {
        m_pCache->build_indice();

        int faceid = vertex_face(linear_vertex_id);
        if (faceid == -1)
            return { -1,-1,-1 };

        auto& spFace = m_faces[faceid];
        int offset = linear_vertex_id - m_pCache->linear_idx_of_face(faceid);
        if (offset == 0) {
            return { faceid, 0, m_pCache->idx_of_point(spFace->h->point_from) };
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
        return { faceid, linear_vertex_id - m_pCache->linear_idx_of_face(faceid), m_pCache->idx_of_point(get_point_to(h)) };
    }

    /*
     * 与linear_vertex_id关联的face的id;
     */
    int HalfEdgeTopology::vertex_face(int linear_vertex_id) const {
        m_pCache->build_indice();

        int n = m_faces.size();
        if (n == 0)
            return -1;

        int left = 0, right = n - 1;
        while (right - left > 0) {
            int mid = (left + right) / 2;
            int face_linearidx = m_pCache->linear_idx_of_face(mid);
            auto spFace = m_faces[mid];

            if (linear_vertex_id < face_linearidx) {
                right = mid - 1;
            }
            else {
                int nVertices = nvertices(mid);
                if (linear_vertex_id >= face_linearidx + nVertices) {
                    left = mid + 1;
                }
                else {
                    return mid;
                }
            }

        }
        auto& spFace = m_faces[left];
        if (linear_vertex_id >= m_pCache->linear_idx_of_face(left) + nvertices(left))
            return -1;
        return left;
    }

    /*
     * 将linear_vertex_id转为它所在的那个面上的idx（就是2:3里面的3);
     */
    int HalfEdgeTopology::vertex_face_index(int linear_vertex_id) const {
        m_pCache->build_indice();

        int idxFace = vertex_face(linear_vertex_id);
        if (idxFace == -1)
            return -1;

        auto spFace = m_faces[idxFace];
        auto pFirstH = spFace->h;
        auto h = pFirstH;
        int offset = linear_vertex_id - m_pCache->linear_idx_of_face(idxFace);
        return offset;
    }

    void HalfEdgeTopology::_remove_edges(std::unordered_set<HF_Edge*>& remEdges) {
        for (auto iter = m_hEdges.begin(); iter != m_hEdges.end();) {
            if (remEdges.find(*iter) != remEdges.end()) {
                remEdges.erase(*iter);
                iter = m_hEdges.erase(iter);
            }
            else {
                iter++;
            }
        }
    }

    void HalfEdgeTopology::_remove_points(std::unordered_set<HF_Point*>& remPoints) {
        for (auto iter = m_points.begin(); iter != m_points.end();) {
            if (remPoints.find(*iter) != remPoints.end()) {
                remPoints.erase(*iter);
                iter = m_points.erase(iter);
            }
            else {
                iter++;
            }
        }
    }

    void HalfEdgeTopology::_remove_faces(std::unordered_set<HF_Face*>& remFaces) {
        for (auto iter = m_faces.begin(); iter != m_faces.end();) {
            if (remFaces.find(*iter) != remFaces.end()) {
                remFaces.erase(*iter);
                iter = m_faces.erase(iter);
            }
            else {
                iter++;
            }
        }
    }

    bool HalfEdgeTopology::remove_point(int ptnum) {
        if (ptnum < 0 || ptnum >= m_points.size())
            return false;

        std::unordered_set<HF_Face*> remFaces;
        std::unordered_set<HF_Edge*> remHEdges;

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

                int idxFace = m_pCache->idx_of_face(newEdge->face);
                m_faces[idxFace]->h = newEdge;

                pprevEdge->next = newEdge;
                prevPoint->edge_from_thispt = newEdge;
            }
        }

        m_points.erase(m_points.begin() + ptnum);

        _remove_edges(remHEdges);
        _remove_faces(remFaces);

        m_pCache->clear_indice_cache();
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

        auto [prepoint, prevedge, pprevedge] = getPrev(vertEdge);

        if (vertEdge->next == pprevedge) {
            std::unordered_set<HF_Edge*> toRemoveEdges;

            HF_Face* removeFace = vertEdge->face;
            HF_Edge* startRemove = prevedge;
            do {
                if (startRemove->pair) {
                    startRemove->pair->pair = nullptr;
                }
                HF_Point* remove_point_to = get_point_to(startRemove);
                //remove_point_to->edge_from_thispt要移除startRemove->next
                if (remove_point_to->edge_from_thispt == startRemove->next) {
                    std::vector<HF_Edge*> _edges_fromhere = find_alledges_from(remove_point_to);
                    remove_point_to->edge_from_thispt = nullptr;
                    for (auto _edge : _edges_fromhere) {
                        if (_edge != startRemove->next) {
                            remove_point_to->edge_from_thispt = _edge;
                            break;
                        }
                    }
                }
                toRemoveEdges.insert(startRemove);
            } while (startRemove != prevedge);

            _remove_edges(toRemoveEdges);
            std::unordered_set<HF_Face*> remFaces;
            remFaces.insert(removeFace);
            _remove_faces(remFaces);
        }
        else {
            auto newedge = new HF_Edge;
            newedge->pair = nullptr;
            newedge->next = vertEdge->next;
            newedge->face = vertEdge->face;
            newedge->point_from = prevedge->point_from;
            pprevedge->next = newedge;
            newedge->point_from->edge_from_thispt = newedge;
            newedge->face->h = newedge;
            m_hEdges.push_back(newedge);
        }

        m_pCache->clear_indice_cache();
        return true;
    }

    bool HalfEdgeTopology::remove_faces(const std::set<int>& faces, bool includePoints, std::vector<int>& removedPtnum) {
        if (faces.empty()) {
            return false;
        }

        std::unordered_set<HF_Point*> remPtrPoints;
        std::unordered_set<HF_Edge*> remEdges;

        for (auto face_id : faces) {
            if (face_id < 0 || face_id >= m_faces.size()) {
                return false;
            }

            auto pFace = m_faces[face_id];

            auto firsth = pFace->h;
            auto h = firsth;
            do
            {
                auto point = get_point_to(h);
                auto edges = find_alledges_from(point);
                edges.erase(std::find(edges.begin(), edges.end(), h->next));
                point->edge_from_thispt = 0;
                if (!edges.empty())
                    point->edge_from_thispt = edges[0];

                if (includePoints) {
                    //考察face上所有的点，是否有点需要被删除
                    if (edges.empty()) {
                        remPtrPoints.insert(point);
                        removedPtnum.push_back(m_pCache->idx_of_point(get_point_to(h)));
                    }
                }
                if (h->pair) {
                    //对面置空自己
                    h->pair->pair = nullptr;
                }
                h->pair = nullptr;
                auto nexth = h->next;
                //h的依赖都解除了，现在就可以删除了。
                remEdges.insert(h);
                h = nexth;
                if (!h)
                    break;
            } while (firsth != h);
        }

        std::sort(removedPtnum.begin(), removedPtnum.end());

        //边都已经删除了，现在只要删除点和面即可。
        _remove_edges(remEdges);
        _remove_points(remPtrPoints);

        std::unordered_set<HF_Face*> remFaces;
        for (auto faceid : faces) remFaces.insert(m_faces[faceid]);
        _remove_faces(remFaces);

        m_pCache->clear_indice_cache();
        return true;
    }

    int HalfEdgeTopology::add_point() {
        m_pCache->clear_indice_cache();
        m_points.push_back(new HF_Point);
        return m_points.size() - 1;
    }

    int HalfEdgeTopology::add_vertex(int face_id, int point_id) {
        if (face_id < 0 || face_id >= m_faces.size() ||
            point_id < 0 || point_id >= m_points.size()) {
            return -1;
        }
        auto& spPoint = m_points[point_id];
        if (find_alledges_from(spPoint).empty()) {
            auto pFace = m_faces[face_id];
            assert(pFace);
            auto firstH = pFace->h;
            auto [prevPoint, prevEdge, pprevEdge] = getPrev(firstH);
            auto newedge = new HF_Edge;
            newedge->point_from = spPoint;
            newedge->next = firstH;
            newedge->face = pFace;
            prevEdge->next = newedge;
            m_hEdges.push_back(newedge);
            spPoint->edge_from_thispt = newedge;
            m_bTriangle = false;    //有增加意味着不能当作三角形处理了


            m_pCache->clear_indice_cache();

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

            auto fromPoint = m_points[from_point];

            auto hedge = new HF_Edge;
            hedge->face = m_faces[face_id];
            hedge->next = 0;//next在下面会设置好
            hedge->point_from = fromPoint;

            assert(fromPoint);
            fromPoint->edge_from_thispt = hedge;

            //check whether the edge from `to_point` to `from_point` exists.
            auto toPoint = m_points[to_point];
            assert(toPoint);
            for (HF_Edge* outEdge : find_alledges_from(toPoint)) {
                if (get_point_to(outEdge) == m_points[from_point]) {
                    outEdge->pair = hedge;
                    hedge->pair = outEdge;
                    break;
                }
            }
            m_hEdges.push_back(hedge);
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

        m_pCache->clear_indice_cache();
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

            auto hedge = new HF_Edge;
            hedge->face = spFace;
            hedge->point_from = m_points[from_point];

            auto fromPoint = m_points[from_point];
            assert(fromPoint);
            fromPoint->edge_from_thispt = hedge;

            //check whether the edge from to_point to from_point.
            auto toPoint = m_points[to_point];
            assert(toPoint);
            for (HF_Edge* outEdge : find_alledges_from(toPoint)) {
                if (get_point_to(outEdge) == m_points[from_point]) {
                    outEdge->pair = hedge;
                    hedge->pair = outEdge;
                    break;
                }
            }

            m_hEdges.push_back(hedge);
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
        m_pCache->clear_indice_cache();
        
        return m_faces.size() - 1;
    }
}