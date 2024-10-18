#include <zeno/geo/geometryutil.h>
#include <zeno/types/GeometryObject.h>
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

        std::vector<std::vector<float>*> vec_pos = geo->get_vec_attr_value<float>(ATTR_POINT, "pos");
        assert(vec_pos.size() == 3);
        auto& xvec = *vec_pos[0];
        auto& yvec = *vec_pos[1];
        auto& zvec = *vec_pos[2];

        for (size_t i = 0; i < tris.size(); i++) {
            auto ind = tris[i];
            size_t i1 = ind[0], i2 = ind[1], i3 = ind[2];
            vec3f pos1(xvec[i1], yvec[i1], zvec[i1]);
            vec3f pos2(xvec[i2], yvec[i2], zvec[i2]);
            vec3f pos3(xvec[i3], yvec[i3], zvec[i3]);
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

}