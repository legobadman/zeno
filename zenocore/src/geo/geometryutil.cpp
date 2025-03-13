#include <zeno/geo/geometryutil.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/para/parallel_reduce.h>
#include "kdsearch.h"
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

    ZENO_API std::shared_ptr<zeno::GeometryObject> fuseGeometry(std::shared_ptr<zeno::GeometryObject> input, float threshold) {
        std::vector<vec3f> points = input->points_pos();

        const int N = points.size();
        zeno::KdTree kd(points, N);
        std::unordered_map<int, int> mp;
        std::unordered_map<int, vec3f> newpt_pos;

        int nPointsRemoved = 0, max_pt_idx = -1;
        for (int i = 0; i < N; i++) {
            zeno::vec3f pt = points[i];
            std::set<int> pts = kd.fix_radius_search(pt, 0);
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
        for (int i = 0; i < N_pts; i++) {
            spOutput->initpoint(i);
        }
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