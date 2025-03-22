#include <zeno/geo/geometryutil.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/para/parallel_reduce.h>
#include "kdsearch.h"
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#include <random>
#include <zeno/utils/wangsrng.h>
#include <unordered_set>
#include <deque>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno
{
    bool prim_remove_point(GeometryObject* prim, int ptnum)
    {
        return false;
    }

    ZENO_API std::vector<vec3f> calc_point_normals(GeometryObject* geo, const std::vector<vec3i>& tris, float flip)
    {
        size_t n = geo->npoints();
        std::vector<vec3f> normals(n);

        std::vector<vec3f> pos(geo->points_pos());

        for (size_t i = 0; i < tris.size(); i++) {
            auto ind = tris[i];
            size_t i1 = ind[0], i2 = ind[1], i3 = ind[2];
            vec3f pos1(pos[i1]);
            vec3f pos2(pos[i2]);
            vec3f pos3(pos[i3]);
            auto n = cross(pos2 - pos1, pos3 - pos1);
            normals[i1] += n;
            normals[i2] += n;
            normals[i3] += n;
        }
        for (size_t i = 0; i < normals.size(); i++) {
            normals[i] = flip * normalizeSafe(normals[i]);
        }

        return normals;
    }

    ZENO_API std::vector<vec3f> calc_triangles_tangent(
        GeometryObject* geo,
        bool has_uv,
        const std::vector<vec3i>& tris,
        const std::vector<vec3f>& pos,
        const std::vector<vec3f>& uvarray,
        const std::vector<vec3f>& uv0data,
        const std::vector<vec3f>& uv1data,
        const std::vector<vec3f>& uv2data
    ) {
        //有可能tang已经计算出来的
        std::vector<vec3f> tang(tris.size());
        if (has_uv) {
#pragma omp parallel for
            for (auto i = 0; i < tris.size(); ++i) {
                const auto& pos0 = pos[tris[i][0]];
                const auto& pos1 = pos[tris[i][1]];
                const auto& pos2 = pos[tris[i][2]];
                zeno::vec3f uv0;
                zeno::vec3f uv1;
                zeno::vec3f uv2;

                uv0 = uv0data[i];
                uv1 = uv1data[i];
                uv2 = uv2data[i];

                auto edge0 = pos1 - pos0;
                auto edge1 = pos2 - pos0;
                auto deltaUV0 = uv1 - uv0;
                auto deltaUV1 = uv2 - uv0;

                auto f = 1.0f / (deltaUV0[0] * deltaUV1[1] - deltaUV1[0] * deltaUV0[1] + 1e-5);

                zeno::vec3f tangent;
                tangent[0] = f * (deltaUV1[1] * edge0[0] - deltaUV0[1] * edge1[0]);
                tangent[1] = f * (deltaUV1[1] * edge0[1] - deltaUV0[1] * edge1[1]);
                tangent[2] = f * (deltaUV1[1] * edge0[2] - deltaUV0[1] * edge1[2]);
                //printf("tangent:%f %f %f\n", tangent[0], tangent[1], tangent[2]);
                //zeno::log_info("tangent {} {} {}",tangent[0], tangent[1], tangent[2]);
                auto tanlen = zeno::length(tangent);
                tangent* (1.f / (tanlen + 1e-8));
                /*if (std::abs(tanlen) < 1e-8) {//fix by BATE
                    zeno::vec3f n = nrm[tris[i][0]], unused;
                    zeno::pixarONB(n, tang[i], unused);//TODO calc this in shader?
                } else {
                    tang[i] = tangent * (1.f / tanlen);
                }*/
                tang[i] = tangent;
            }
        }
        else {
#pragma omp parallel for
            for (auto i = 0; i < tris.size(); ++i) {
                const auto& pos0 = pos[tris[i][0]];
                const auto& pos1 = pos[tris[i][1]];
                const auto& pos2 = pos[tris[i][2]];
                zeno::vec3f uv0;
                zeno::vec3f uv1;
                zeno::vec3f uv2;

                uv0 = uvarray[tris[i][0]];
                uv1 = uvarray[tris[i][1]];
                uv2 = uvarray[tris[i][2]];

                auto edge0 = pos1 - pos0;
                auto edge1 = pos2 - pos0;
                auto deltaUV0 = uv1 - uv0;
                auto deltaUV1 = uv2 - uv0;

                auto f = 1.0f / (deltaUV0[0] * deltaUV1[1] - deltaUV1[0] * deltaUV0[1] + 1e-5);

                zeno::vec3f tangent;
                tangent[0] = f * (deltaUV1[1] * edge0[0] - deltaUV0[1] * edge1[0]);
                tangent[1] = f * (deltaUV1[1] * edge0[1] - deltaUV0[1] * edge1[1]);
                tangent[2] = f * (deltaUV1[1] * edge0[2] - deltaUV0[1] * edge1[2]);
                //printf("tangent:%f %f %f\n", tangent[0], tangent[1], tangent[2]);
                //zeno::log_info("tangent {} {} {}",tangent[0], tangent[1], tangent[2]);
                auto tanlen = zeno::length(tangent);
                tangent* (1.f / (tanlen + 1e-8));
                /*if (std::abs(tanlen) < 1e-8) {//fix by BATE
                    zeno::vec3f n = nrm[tris[i][0]], unused;
                    zeno::pixarONB(n, tang[i], unused);//TODO calc this in shader?
                    } else {
                    tang[i] = tangent * (1.f / tanlen);
                    }*/
                tang[i] = tangent;
            }
        }
        return tang;
    }

    ZENO_API std::vector<vec3f> compute_vertex_tangent(
        const std::vector<vec3i>& tris,
        const std::vector<vec3f>& tang,
        const std::vector<vec3f>& pos
    ) {
        std::vector<vec3f> atang;
        atang.assign(atang.size(), zeno::vec3f(0));
        for (size_t i = 0; i < tris.size(); ++i)
        {

            auto vidx = tris[i];
            zeno::vec3f v0 = pos[vidx[0]];
            zeno::vec3f v1 = pos[vidx[1]];
            zeno::vec3f v2 = pos[vidx[2]];
            auto e1 = v1 - v0, e2 = v2 - v0;
            float area = zeno::length(zeno::cross(e1, e2)) * 0.5;
            atang[vidx[0]] += area * tang[i];
            atang[vidx[1]] += area * tang[i];
            atang[vidx[2]] += area * tang[i];
        }
#pragma omp parallel for
        for (auto i = 0; i < atang.size(); i++)
        {
            atang[i] = atang[i] / (length(atang[i]) + 1e-6);
        }
        return atang;
    }

    ZENO_API std::pair<vec3f, vec3f> geomBoundingBox(GeometryObject* geo) {
        const std::vector<vec3f>& verts = geo->points_pos();
        if (!verts.size())
            return { {0, 0, 0}, {0, 0, 0} };
        return parallel_reduce_minmax(verts.begin(), verts.end());
    }

    ZENO_API std::shared_ptr<zeno::GeometryObject> mergeObjects(std::shared_ptr<zeno::ListObject> spList) {
        int nPoints = 0, nFaces = 0;
        const std::vector<std::shared_ptr<zeno::GeometryObject>>& geoobjs = spList->get<zeno::GeometryObject>();
        for (auto spObject : geoobjs) {
            nPoints += spObject->npoints();
            nFaces += spObject->nfaces();
        }
        std::shared_ptr<zeno::GeometryObject> mergedObj = std::make_shared<zeno::GeometryObject>(false, nPoints, nFaces);
        mergedObj->merge(geoobjs);
        return mergedObj;
    }

    ZENO_API bool dividePlane(
        const std::vector<vec3f>& face_pts, /*容器的顺序就是面点的逆序排序*/
        float A, float B, float C, float D,
        /*out*/std::vector<DivideFace>& split_faces,
        /*out*/std::map<int, DividePoint>& split_infos,
        /*out*/bool* pOnlyAbove
    ) {
        //TODO: 先用bbox过滤
        {
            bool hasbelow = false, hasabove = false;
            for (auto pos : face_pts) {
                float metric = A * pos[0] + B * pos[1] + C * pos[2] + D;
                if (metric < 0) {
                    hasbelow = true;
                }
                else if (metric > 0) {
                    hasabove = true;
                }
            }
            if (!(hasbelow && hasabove)) {
                if (pOnlyAbove) {
                    *pOnlyAbove = hasabove;
                }
                return false;
            }
        }

        int nCount = face_pts.size();
        std::vector<int> splitters;   //储存分割点的序号，注意，分割点也可以是顶点的序号
        std::vector<int> newface_pt_seq;  //原有的点加上分割点形成的点序号序列，后续要遍历这个序列得到分割面
        std::map<int, vec3f> splitter_pos;
        for (int i = 1; i <= face_pts.size(); i++) {
            //观察Pi-1和Pi之间是否有分割
            int from = -1, to = -1;
            if (i == face_pts.size()) {
                //TODO: 如果是线段，就不考虑最后的闭合线
                from = face_pts.size() - 1;
                to = 0;
            }
            else {
                from = i - 1;
                to = i;
            }
            vec3f p1 = face_pts[from], p2 = face_pts[to];
            float d1 = A * p1[0] + B * p1[1] + C * p1[2] + D;
            float d2 = A * p2[0] + B * p2[1] + C * p2[2] + D;
            newface_pt_seq.push_back(from);
            if (d1 * d2 < 0) {
                //新增一个分割点
                int new_splitter = nCount++;
                splitters.push_back(new_splitter);
                newface_pt_seq.push_back(new_splitter);
                float t = -(A*p1[0] + B*p1[1] + C*p1[2] + D) / (A*(p2[0]-p1[0]) + B*(p2[1]-p1[1]) + C * (p2[2]-p1[2]));
                float newx = p1[0] + t * (p2[0] - p1[0]);
                float newy = p1[1] + t * (p2[1] - p1[1]);
                float newz = p1[2] + t * (p2[2] - p1[2]);
                vec3f spliter_pos(newx, newy, newz);
                splitter_pos.insert(std::make_pair(new_splitter, spliter_pos));
                split_infos.insert(std::make_pair(new_splitter, DividePoint{ spliter_pos, from, to}));
            }
            else if (d1 * d2 == 0) {
                if (d1 == 0) {
                    splitters.push_back(from);
                    splitter_pos.insert(std::make_pair(from, p1));
                    split_infos.insert(std::make_pair(from, DividePoint{ p1, from, from }));
                }
                if (d2 == 0) {
                    splitters.push_back(to);
                    splitter_pos.insert(std::make_pair(to, p2));
                    split_infos.insert(std::make_pair(to, DividePoint{ p2, to, to }));
                }
            }
        }

        std::vector<int> next_pts(nCount);
        for (int i = 1; i < nCount; i++) {
            if (i == 0) {
                next_pts[newface_pt_seq[nCount - 1]] = newface_pt_seq[0];
            }
            next_pts[newface_pt_seq[i - 1]] = newface_pt_seq[i];
        }

        assert(splitters.size() > 1 && splitter_pos.size() > 1);

        //要对分割点进行排序，以便于在分割点之间“跳跃”
        {
            //先确定方向，
            vec3f sp1 = splitter_pos.begin()->second;
            vec3f sp2 = splitter_pos.rbegin()->second;
            vec3f d = sp2 - sp1;
            float _x = std::abs(d[0]), _y = std::abs(d[1]), _z = std::abs(d[2]);
            int proj = 0;
            if (_x < _y) {
                if (_y < _z) {
                    proj = 2;
                }
                else {
                    proj = 1;
                }
            }
            else {
                if (_x < _z) {
                    proj = 2;
                }
                else {
                    proj = 0;
                }
            }
            std::sort(splitters.begin(), splitters.end(), [&](int splitter1, int splitter2) {
                return splitter_pos[splitter1][proj] < splitter_pos[splitter2][proj];
            });
        }

        //找到要遍历的起点（这些起点不能是分割点，如果分割点和顶点重合，也不能作为起点）
        std::deque<int> start_pts;
        for (int i = 0; i < face_pts.size(); i++) {
            if (std::find(splitters.begin(), splitters.end(), i) == splitters.end()) {
                start_pts.push_back(i);
            }
        }

        std::function<bool(int, PointSide, std::vector<int>& )> search_ring = [&](
                int currpt, 
                PointSide side,
                std::vector<int>& exist_pts)->bool {
            //当前处在currpt点，已经收集到的点序（按逆时针排序）为exist_pts(不包含currpt)

            //首先判断是否闭环
            if (!exist_pts.empty() && next_pts[currpt] == exist_pts[0]) {
                exist_pts.push_back(currpt);
                return true;
            }

            //其次判断是否有重复路径
            if (std::find(exist_pts.begin(), exist_pts.end(), currpt) != exist_pts.end())
                return false;

            std::vector<int> temp_pts = exist_pts;
            temp_pts.push_back(currpt);
            int pt = next_pts[currpt];
            
            while (true) {
                //首先判断pt是否分割点
                vec3f ptpos;
                auto iterSpliter = splitter_pos.find(pt);
                if (iterSpliter != splitter_pos.end()) {
                    temp_pts.push_back(pt);
                    break;
                }
                else {
                    ptpos = face_pts[pt];
                }
                float metric = A * ptpos[0] + B * ptpos[1] + C * ptpos[2];
                if (metric < 0 && side == Above ||
                    metric > 0 && side == Below) {
                    //应该是先遇到分割点，而不是跳到另一侧
                    assert(false);
                    break;
                }
                //从currpt到pt-1，都是同一侧的点，可以加入
                temp_pts.push_back(pt);
                pt = next_pts[pt];      //取下一个点
            }

            //pt是一个分割点
            int split_pt = pt;
            //找pt的前一个分割点和后一个分割点，然后各自搜索下去，看能否形成闭环
            int split_idx = std::find(splitters.begin(), splitters.end(), split_pt) - splitters.begin();
            bool ret = false;
            if (split_idx > 0) {
                int prev_split_pt = splitters[split_idx - 1];
                ret = search_ring(prev_split_pt, side, temp_pts);
                if (ret) {
                    exist_pts = temp_pts;
                    return ret;
                }
            }
            if (split_idx < splitters.size() - 1) {
                int next_split_pt = splitters[split_idx + 1];
                ret = search_ring(next_split_pt, side, temp_pts);
                if (ret) {
                    exist_pts = temp_pts;
                    return true;
                }
            }
            return ret;
        };

        //遍历所有非分割点
        std::set<int> visited;
        assert(!start_pts.empty());
        while (!start_pts.empty())
        {
            int start_pt = start_pts.front();
            start_pts.pop_front();
            if (visited.find(start_pt) != visited.end()) {
                //这个非分割点已经被加到某一个面里了，不可能属于另一个面
                continue;
            }

            vec3f start_pos = face_pts[start_pt];

            float metric = A * start_pos[0] + B * start_pos[1] + C * start_pos[2] + D;
            assert(metric != 0);

            PointSide side = UnDecided;
            if (metric < 0) side = Below;
            else side = Above;

            std::vector<int> exist_face;
            bool ret = search_ring(start_pt, side, exist_face);
            if (ret) {
                DivideFace divideface;
                divideface.face_indice = exist_face;
                divideface.side = side;
                split_faces.push_back(divideface);
            }

            for (auto pid : exist_face) {
                visited.insert(pid);
            }
        }

#if 0
        int nPoints = face_pts.size();
        std::unordered_set<int> added_points;
        //开始循环遍历newface_pt_seq，不断收集点以形成新面
        while (added_points.size() < face_pts.size()) {
            PointSide side = UnDecided;
            std::vector<int> new_face;
            for (int i = 0; ; i++) {
                int curr_pt = newface_pt_seq[i];
                if (!new_face.empty() && curr_pt == new_face[0]) {
                    break;
                }
                vec3f curr_pos = face_pts[curr_pt];
                if (added_points.find(curr_pt) != added_points.end()) {
                    continue;
                }
                float metric = A * curr_pos[0] + B * curr_pos[1] + C * curr_pos[2] + D;
                if (side == UnDecided) {
                    if (metric < 0) {
                        side = Below;
                    }
                    else if (metric > 0) {
                        side = Above;
                    }
                    else {
                        //这里的顶点和分割点重合，仍未决定哪一边，等下一圈再跑一边
                        break;
                    }
                }
                if (metric < 0 && side == Below) {
                    new_face.push_back(curr_pt);
                    added_points.insert(curr_pt);
                }
                else if (metric > 0 && side == Above) {
                    new_face.push_back(curr_pt);
                    added_points.insert(curr_pt);
                }
                else if (metric == 0) {
                    //此时是分割点
                    new_face.push_back(curr_pt);
                }
            }
            split_faces.push_back(new_face);
        }
#endif
        return true;
    }

    ZENO_API std::shared_ptr<zeno::GeometryObject> scatter(std::shared_ptr<zeno::GeometryObject> input, const int nPointCount, int seed) {
        auto spOutput = std::make_shared<zeno::GeometryObject>(input->is_base_triangle(), nPointCount, 0);
        if (seed == -1) seed = std::random_device{}();

        const int nFaces = input->nfaces();
        std::vector<float> cdf(nFaces);
        std::vector<vec3f> newPts(nPointCount);

        const std::vector<vec3f>& pos = input->points_pos();
        std::vector<std::vector<int>> faces = input->face_indice();

        parallel_inclusive_scan_sum(faces.begin(), faces.end(), cdf.begin(), [&](std::vector<int> const& ind) {
            auto i1 = ind[0];
            auto i2 = ind[1];
            auto i3 = ind[2];
            auto i4 = ind[3];
            auto area = length(cross(pos[i3] - pos[i2], pos[i2] - pos[i1]));
            return area;
        });

        auto inv_total = 1 / cdf.back();
        parallel_for((size_t)0, cdf.size(), [&](size_t i) {
            cdf[i] *= inv_total;
        });

        if (false/*line*/) {
            //todo
        }
        else {
            parallel_for((size_t)0, (size_t)nPointCount, [&](size_t i) {
                wangsrng rng(seed, i);
                auto val = rng.next_float();
                auto it = std::lower_bound(cdf.begin(), cdf.end(), val);
                size_t index = it - cdf.begin();
                assert(index < nFaces);
                auto const& pointsIndice = faces[index];
                auto r1 = std::sqrt(rng.next_float());
                auto r2 = rng.next_float();
                if (pointsIndice.size() == 4) {
                    //目前只考虑直角四边形或平行四边形
                    vec3f v01 = pos[pointsIndice[1]] - pos[pointsIndice[0]];
                    vec3f v02 = pos[pointsIndice[2]] - pos[pointsIndice[1]];
                    vec3f new_pt = pos[pointsIndice[0]] + v01 * r1 + v02 * r2;
                    newPts[i] = new_pt;
                }
                else {
                    //TODO: 三角面
                }
            });
        }
        spOutput->create_point_attr("pos", newPts);
        return spOutput;
    }

    ZENO_API std::shared_ptr<zeno::GeometryObject> constructGeom(const std::vector<std::vector<zeno::vec3f>>& faces) {
        int nPoints = 0, nFaces = faces.size();
        for (auto& facePts : faces) {
            nPoints += facePts.size();
        }
        auto spOutput = std::make_shared<zeno::GeometryObject>(false, nPoints, nFaces);
        std::vector<vec3f> points;
        points.resize(nPoints);
        int nInitPoints = 0;
        for (int iFace = 0; iFace < faces.size(); iFace++) {
            const std::vector<zeno::vec3f>& facePts = faces[iFace];
            std::vector<int> ptIndice;
            for (int iPt = 0; iPt < facePts.size(); iPt++) {
                int currIdx = nInitPoints + iPt;
                points[currIdx] = facePts[iPt];
                ptIndice.push_back(currIdx);
            }
            nInitPoints += facePts.size();
            spOutput->add_face(ptIndice);
        }
        spOutput->create_attr(ATTR_POINT, "pos", points);
        return spOutput;
    }

    ZENO_API std::shared_ptr<zeno::GeometryObject> fuseGeometry(std::shared_ptr<zeno::GeometryObject> input, float threshold) {
        std::vector<vec3f> points = input->points_pos();

        const int N = points.size();
        zeno::KdTree kd(points, N);
        std::unordered_map<int, int> mp;
        std::unordered_map<int, vec3f> newpt_pos;

        int nPointsRemoved = 0, max_pt_idx = -1;
        for (int i = 0; i < N; i++) {
            zeno::vec3f pt = points[i];
            std::set<int> pts = kd.fix_radius_search(pt, threshold);
            int minIdx = *pts.begin();
            int reduceIdx = minIdx;
            if (reduceIdx == i) {
                //没有被映射的，说明是独立点，但是由于前面移除了一些点，所以索引要减
                reduceIdx -= nPointsRemoved;
            }
            else if (mp.find(reduceIdx) != mp.end()) {  //有可能reduceIdx本身要被减过的，所以要再检查一下
                reduceIdx = mp[reduceIdx];
            }
            mp.insert(std::make_pair(i, reduceIdx));
            max_pt_idx = std::max(max_pt_idx, reduceIdx);
            if (minIdx < i) {
                nPointsRemoved++;   //被映射的点算是“被移除”
            }
        }

        std::vector<std::vector<int>> faces = input->face_indice();
        for (std::vector<int>& face : faces) {
            for (int i = 0; i < face.size(); i++) {
                face[i] = mp[face[i]];
            }
        }

        int N_pts = max_pt_idx + 1;

        std::vector<vec3f> new_points;
        new_points.resize(N_pts);

        for (int i = 0; i < N; i++) {
            int map_idx = mp[i];
            vec3f pt = points[i];
            new_points[map_idx] = pt;
        }

        auto spOutput = std::make_shared<zeno::GeometryObject>(false, N_pts, faces.size());
        for (int iFace = 0; iFace < faces.size(); iFace++) {
            const std::vector<int>& facePts = faces[iFace];
            spOutput->add_face(facePts);
        }
        spOutput->create_attr(ATTR_POINT, "pos", new_points);
        return spOutput;
    }

    ZENO_API glm::mat4 calc_rotate_matrix(
        float xangle,
        float yangle,
        float zangle,
        Rotate_Orientaion orientaion
    ) {
        float rad_x = xangle * (M_PI / 180.0);
        float rad_y = yangle * (M_PI / 180.0);
        float rad_z = zangle * (M_PI / 180.0);
#if 0
        switch (orientaion)
        {
        case Orientaion_XY: //绕x轴旋转90
            rad_x = (xangle + 90) * (M_PI / 180.0);
            break;
        case Orientaion_YZ: //绕z轴旋转-90
            rad_z = (zangle - 90) * (M_PI / 180.0);
            break;
        case Orientaion_ZX:
            break;//默认都是基于ZX平面
        }
#endif
        //这里构造的方式是基于列，和公式上的一样，所以看起来反过来了
        glm::mat4 mx = glm::mat4(
            1, 0, 0, 0,
            0, cos(rad_x), sin(rad_x), 0,
            0, -sin(rad_x), cos(rad_x), 0,
            0, 0, 0, 1);
        glm::mat4 my = glm::mat4(
            cos(rad_y), 0, -sin(rad_y), 0,
            0, 1, 0, 0,
            sin(rad_y), 0, cos(rad_y), 0,
            0, 0, 0, 1);
        glm::mat4 mz = glm::mat4(
            cos(rad_z), sin(rad_z), 0, 0,
            -sin(rad_z), cos(rad_z), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        return mz * my * mx;
    }
}