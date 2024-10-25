#include <zeno/types/GeometryObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <assert.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/utils/vectorutil.h>
#include <zeno/utils/format.h>
#include "geotopology.h"
#include "../utils/zfxutil.h"
#include "zeno_types/reflect/reflection.generated.hpp"
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/utils/variantswitch.h>
#include <regex>


namespace zeno
{
    template <class T>
    static T get_zfxvar(zfxvariant value) {
        return std::visit([](auto const& val) -> T {
            using V = std::decay_t<decltype(val)>;
            if constexpr (!std::is_constructible_v<T, V>) {
                throw makeError<TypeError>(typeid(T), typeid(V), "get<zfxvariant>");
            }
            else {
                return T(val);
            }
            }, value);
    }

    ZENO_API GeometryObject::GeometryObject()
        : m_spTopology(std::make_shared<GeometryTopology>())
    {
    }

    ZENO_API GeometryObject::GeometryObject(bool bTriangle, int nPoints, int nFaces)
        : m_spTopology(std::make_shared<GeometryTopology>(bTriangle, nPoints, nFaces))
    {
    }

    ZENO_API GeometryObject::GeometryObject(const GeometryObject& rhs)
        : m_spTopology(rhs.m_spTopology)
    {
        m_point_attrs = rhs.m_point_attrs;
        m_face_attrs = rhs.m_face_attrs;
        m_geo_attrs = rhs.m_geo_attrs;
    }

    ZENO_API GeometryObject::~GeometryObject() {
    }

    ZENO_API GeometryObject::GeometryObject(PrimitiveObject* prim)
        : m_spTopology(std::make_shared<GeometryTopology>())
    {
        initFromPrim(prim);
    }

    ZENO_API std::shared_ptr<PrimitiveObject> GeometryObject::toPrimitive() {
        std::shared_ptr<PrimitiveObject> spPrim = std::make_shared<PrimitiveObject>();
        std::vector<vec3f>& vec_pos = points_pos();
        int nPoints = m_spTopology->m_points.size();
        assert(nPoints == vec_pos.size());
        spPrim->resize(nPoints);
        for (int i = 0; i < nPoints; i++) {
            spPrim->verts[i] = vec_pos[i];
        }

        //TODO: 导出属性
        std::set<std::string> export_attrs;
        for (auto& [name, sp_attr_data] : m_point_attrs) {
            if (name == "pos") {
                continue;
            }
            sp_attr_data.to_prim_attr(spPrim, name);
        }

        int startIdx = 0;
        if (m_spTopology->m_bTriangle) {
            spPrim->tris->resize(m_spTopology->m_faces.size());
        }
        else {
            spPrim->polys->resize(m_spTopology->m_faces.size());
        }

        for (int i = 0; i < m_spTopology->m_faces.size(); i++) {
            auto face = m_spTopology->m_faces[i].get();
            HEdge* firsth = face->h;
            HEdge* h = firsth;
            std::vector<int> points;
            do {
                int index = m_spTopology->getPointTo(h);
                points.push_back(index);
                h = h->next;
            } while (firsth != h);

            if (m_spTopology->m_bTriangle) {
                vec3i tri = { points[0],points[1], points[2] };
                spPrim->tris[i] = std::move(tri);
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
        return spPrim;
    }

    void GeometryObject::initFromPrim(PrimitiveObject* prim) {
        int n = prim->verts->size();
        m_spTopology->m_points.resize(n);
        std::vector<vec3f> pos;
        pos.resize(n);
        for (int i = 0; i < m_spTopology->m_points.size(); i++) {
            m_spTopology->m_points[i] = std::make_shared<Point>();
            pos[i] = prim->verts[i];
        }

        int nFace = -1;
        m_spTopology->m_bTriangle = prim->loops->empty() && !prim->tris->empty();
        if (m_spTopology->m_bTriangle) {
            nFace = prim->tris->size();
            m_spTopology->m_hEdges.reserve(nFace * 3);
        }
        else {
            assert(!prim->loops->empty() && !prim->polys->empty());
            nFace = prim->polys->size();
            //一般是四边形
            m_spTopology->m_hEdges.reserve(nFace * 4);
        }
        m_spTopology->m_faces.resize(nFace);

        for (int face = 0; face < nFace; face++) {
            std::vector<int> points;
            if (m_spTopology->m_bTriangle) {
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
                m_spTopology->m_hEdges.insert(std::make_pair(id, hedge));

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
                }

                //check whether the pair edge exist
                auto pairedge = m_spTopology->checkHEdge(vq, vp);
                if (pairedge) {
                    hedge->pair = pairedge;
                    pairedge->pair = hedge.get();
                }

                m_spTopology->m_points[vp]->edges.insert(hedge.get());

                //pFace->h = hedge.get();
                lastHedge = hedge.get();
            }
            //统一取第一条半边作为face的“起点边”
            pFace->h = firstHedge;

            m_spTopology->m_faces[face] = std::move(pFace);
        }

        create_attr(ATTR_POINT, "pos", pos);
    }

    int GeometryObject::get_point_count() const {
        return m_spTopology->m_points.size();
    }

    int GeometryObject::get_face_count() const {
        return m_spTopology->m_faces.size();
    }

    std::vector<vec3f> GeometryObject::points_pos() {
        std::map<std::string, AttributeVector>& container = get_container(ATTR_POINT);
        auto iter = container.find("pos");
        if (iter == container.end()) {
            throw makeError<KeyError>("pos", "not exist on point attr");
        }
        return iter->second.get_attrs<vec3f>();
    }

    ZENO_API std::vector<vec3i> GeometryObject::tri_indice() const {
        return m_spTopology->tri_indice();
    }

    ZENO_API std::vector<int> GeometryObject::edge_list() const {
        return m_spTopology->edge_list();
    }

    ZENO_API bool GeometryObject::is_base_triangle() const {
        return m_spTopology->is_base_triangle();
    }

    ZENO_API int GeometryObject::get_group_count(GeoAttrGroup grp) const {
        switch (grp) {
        case ATTR_POINT: return m_point_attrs.size();
        case ATTR_FACE: return m_face_attrs.size();
        case ATTR_GEO: return 1;
        default:
            return 0;
        }
    }

    ZENO_API void GeometryObject::geomTriangulate(zeno::TriangulateInfo& info) {
        if (is_base_triangle()) {
            //TODO
            return;
        }
        m_spTopology->geomTriangulate(info);
        //TODO: uv
    }

    bool GeometryObject::remove_point(int ptnum) {
        if (ptnum < 0 || ptnum >= m_spTopology->m_points.size())
            return false;

        std::set<int> remFaces;
        std::set<std::string> remHEdges;

        for (auto outEdge : m_spTopology->m_points[ptnum]->edges) {
            assert(outEdge);

            HEdge* firstEdge = outEdge;
            HEdge* nextEdge = firstEdge->next;
            assert(nextEdge);
            HEdge* nnextEdge = nextEdge->next;
            assert(nnextEdge);

            auto& [prevPoint, prevEdge, pprevEdge] = m_spTopology->getPrev(outEdge);
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
                        m_spTopology->m_points[prev->point]->edges.erase(h);
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
                m_spTopology->m_hEdges.insert(std::make_pair(id, newEdge));

                //connect between outEdge->point and prevPoint.
                newEdge->point = outEdge->point;
                newEdge->pair = nullptr;
                newEdge->next = nextEdge;
                newEdge->face = outEdge->face;

                m_spTopology->m_faces[newEdge->face]->h = newEdge.get();

                pprevEdge->next = newEdge.get();
                prevPoint->edges.erase(prevEdge);
                prevPoint->edges.insert(newEdge.get());
            }
        }

        m_spTopology->m_points.erase(m_spTopology->m_points.begin() + ptnum);

        for (auto keyname : remHEdges) {
            m_spTopology->m_hEdges.erase(keyname);
        }

        //adjust face
        std::vector<int> _remFaces;
        for (int idx : remFaces)
            _remFaces.push_back(idx);
        std::sort(_remFaces.begin(), _remFaces.end());

        for (auto iter = _remFaces.rbegin(); iter != _remFaces.rend(); iter++) {
            int rmIdx = *iter;
            m_spTopology->m_faces.erase(m_spTopology->m_faces.begin() + rmIdx);
        }

        for (auto& [_, hedge] : m_spTopology->m_hEdges) {
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
            auto fh = m_spTopology->m_faces[hedge->face]->h;
            if (m_spTopology->m_bTriangle) {
                assert(fh == ph || fh == ph->next || fh == ph->next->next);
            }
            */
        }
    }

    int GeometryObject::facepoint(int face_id, int vert_id) const
    {
        if (face_id < 0 || face_id >= m_spTopology->m_faces.size())
            return -1;

        auto h = m_spTopology->m_faces[face_id]->h;
        auto firsth = h;
        do {
            if (vert_id-- == 0) {
                return h->point;
            }
            h = h->next;
        } while (h != firsth);
        return -1;
    }

    zfxintarr GeometryObject::facepoints(int face_id)
    {
        zfxintarr pts;
        if (face_id < 0 || face_id >= m_spTopology->m_faces.size())
            return pts;

        auto h = m_spTopology->m_faces[face_id]->h;
        auto firsth = h;
        do {
            pts.push_back(h->point);
            h = h->next;
        } while (h != firsth);
        return pts;
    }

    zfxintarr GeometryObject::pointfaces(int point_id)
    {
        zfxintarr faces;
        if (point_id < 0 || point_id >= m_spTopology->m_points.size())
            return faces;

        for (auto h : m_spTopology->m_points[point_id]->edges) {
            faces.push_back(h->face);
        }
        return faces;
    }

    zfxintarr GeometryObject::pointvertex(int point_id)
    {
        throw makeError<UnimplError>();
    }

    ZENO_API size_t GeometryObject::get_attr_size(GeoAttrGroup grp) const {
        if (grp == ATTR_GEO) {
            return 1;
        }
        else if (grp == ATTR_POINT) {
            return m_spTopology->m_points.size();
        }
        else if (grp == ATTR_FACE) {
            return m_spTopology->m_faces.size();
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }

    ZENO_API std::map<std::string, AttributeVector>& GeometryObject::get_container(GeoAttrGroup grp) {
        if (grp == ATTR_GEO) {
            return m_geo_attrs;
        }
        else if (grp == ATTR_POINT) {
            return m_point_attrs;
        }
        else if (grp == ATTR_FACE) {
            return m_face_attrs;
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }

    ZENO_API const std::map<std::string, AttributeVector>& GeometryObject::get_const_container(GeoAttrGroup grp) const {
        if (grp == ATTR_GEO) {
            return m_geo_attrs;
        }
        else if (grp == ATTR_POINT) {
            return m_point_attrs;
        }
        else if (grp == ATTR_FACE) {
            return m_face_attrs;
        }
        else {
            throw makeError<UnimplError>("Unknown group on attr");
        }
    }

    bool GeometryObject::create_attr_by_zfx(GeoAttrGroup grp, const std::string& attr_name, const zfxvariant& defl)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(attr_name);
        if (iter != container.end()) {
            return false;   //already exist
        }

        AttrVar val = zeno::zfx::zfxvarToAttrvar(defl);
        int n = get_attr_size(grp);
        AttributeVector spAttr(val, n);

        container.insert(std::make_pair(attr_name, std::move(spAttr)));
        //需要同步到外侧的zfx manager
        return true;
    }

    bool GeometryObject::create_attr(GeoAttrGroup grp, const std::string& attr_name, const AttrVar& val_or_vec)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(attr_name);
        if (iter != container.end()) {
            return false;   //already exist
        }

        int n = get_attr_size(grp);
        container.insert(std::make_pair(attr_name, AttributeVector(val_or_vec, n)));
        //需要同步到外侧的zfx manager
        return true;
    }

    bool GeometryObject::delete_attr(GeoAttrGroup grp, const std::string& attr_name)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(attr_name);
        if (iter == container.end())
            return false;
        container.erase(iter);
        return true;
    }

    bool GeometryObject::has_attr(GeoAttrGroup grp, std::string const& name)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        return container.find(name) != container.end();
    }

    std::vector<zfxvariant> GeometryObject::get_attr_byzfx(GeoAttrGroup grp, std::string const& name)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(name);
        if (iter == container.end()) {
            throw makeError<KeyError>(name, "not exist on point attr");
        }

        auto& spAttrVec = iter->second;
        int n = spAttrVec.size();
        AttrVarVec& val = spAttrVec.get();
        return zeno::zfx::attrvarVecToZfxVec(val, n);
    }

    void GeometryObject::set_attr_byzfx(GeoAttrGroup grp, std::string const& name, const ZfxVariable& val, ZfxElemFilter& filter)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(name);
        if (iter == container.end()) {
            throw makeError<KeyError>(name, "not exist on point attr");
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        for (int i = 0; i < n; i++) {
            if (filter[i]) {
                const glm::vec3& vec = get_zfxvar<glm::vec3>(val.value[i]);
                spAttr.set_elem(i, zeno::vec3f(vec.x, vec.y, vec.z));
            }
        }
    }

    void GeometryObject::set_attr(GeoAttrGroup grp, std::string const& name, const AttrVar& val)
    {
        std::map<std::string, AttributeVector>& container = get_container(grp);
        auto iter = container.find(name);
        if (iter == container.end()) {
            throw makeError<KeyError>(name, "not exist on point attr");
        }
        AttributeVector& spAttr = iter->second;
        int n = spAttr.size();
        spAttr.set(val);
    }

    void GeometryObject::initpoint(size_t point_id) {
        m_spTopology->initpoint(point_id);
    }

    int GeometryObject::addpoint(zfxvariant var) {
        vec3f pos = std::visit([](auto&& val)->vec3f {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, glm::vec3>) {
                return vec3f({ val[0],val[1], val[2] });
            }
            else if constexpr (std::is_same_v<T, zfxfloatarr>) {
                return vec3f({ val[0],val[1], val[2] });
            }
            else {
                throw makeError<UnimplError>("type dismatch");
            }
        }, var);

        auto spPoint = std::make_shared<Point>();
        m_spTopology->m_points.emplace_back(spPoint);
        create_attr(ATTR_POINT, "pos", pos);
        return m_spTopology->m_points.size() - 1;
    }

    int GeometryObject::addvertex(size_t face_id, size_t point_id) {
        if (face_id < 0 || face_id >= m_spTopology->m_faces.size() ||
            point_id < 0 || point_id >= m_spTopology->m_points.size()) {
            return -1;
        }
        auto& spPoint = m_spTopology->m_points[point_id];
        if (spPoint->edges.empty()) {
            auto pFace = m_spTopology->m_faces[face_id];
            assert(pFace);
            auto firstH = pFace->h;
            auto& [prevPoint, prevEdge, pprevEdge] = m_spTopology->getPrev(firstH);
            auto newedge = std::make_shared<HEdge>();
            newedge->point = prevEdge->point;
            newedge->next = firstH;
            newedge->face = face_id;
            newedge->id = generateUUID();
            prevEdge->next = newedge.get();
            prevEdge->point = point_id;
            m_spTopology->m_hEdges.insert(std::make_pair(newedge->id, newedge));
            m_spTopology->m_bTriangle = false;    //有增加意味着不能当作三角形处理了
            return nvertices(face_id);
        }
        else {
            return -1;
        }
    }

    int GeometryObject::nvertices(int face_id) const {
        if (face_id < 0 || face_id >= m_spTopology->m_faces.size()) {
            return 0;
        }
        return m_spTopology->npoints_in_face(m_spTopology->m_faces[face_id].get());
    }

    void GeometryObject::addface(const std::vector<size_t>& points) {
        m_spTopology->addface(points);
    }

    void GeometryObject::setface(size_t face_id, const std::vector<size_t>& points) {
        m_spTopology->setface(face_id, points);
    }

    bool GeometryObject::remove_faces(const std::set<int>& faces, bool includePoints) {
        if (faces.empty()) {
            return false;
        }

        std::set<int> remPoints;
        std::set<Point*> remPtrPoints;
        for (auto face_id : faces) {
            auto pFace = m_spTopology->m_faces[face_id];
            if (includePoints) {
                //考察face上所有的点，是否有点需要被删除
                auto firsth = pFace->h;
                auto h = firsth;
                do
                {
                    auto& spPoint = m_spTopology->m_points[h->point];
                    spPoint->edges.erase(h);
                    if (spPoint->edges.empty()) {
                        remPoints.insert(h->point);
                        remPtrPoints.insert(spPoint.get());
                    }
                    if (h->pair) {
                        //对面置空自己
                        h->pair->pair = nullptr;
                    }
                    h->pair = nullptr;
                    auto nexth = h->next;
                    //h的依赖都解除了，现在就可以删除了。
                    m_spTopology->m_hEdges.erase(h->id);
                    h = nexth;
                } while (firsth != h);
            }
        }

        //边都已经删除了，现在只要删除点和面即可。
        removeElements(m_spTopology->m_points, remPoints);
        removeElements(m_spTopology->m_faces, faces);

        //因为点被删除了，故需要调整所有未被删除的边（只要调整point和face的索引），至于face则不需要调整，因为删除的就是face，直接删了完事
        for (auto& [_, hedge] : m_spTopology->m_hEdges) {
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
            auto fh = m_spTopology->m_faces[hedge->face]->h;
            if (m_spTopology->m_bTriangle) {
                assert(fh == ph || fh == ph->next || fh == ph->next->next);
            }
        }
        return true;
    }

    int GeometryObject::npoints() const {
        return m_spTopology->m_points.size();
    }

    int GeometryObject::nfaces() const {
        return m_spTopology->m_faces.size();
    }

    int GeometryObject::nvertices() const {
        //TODO:
        return -1;
    }

}