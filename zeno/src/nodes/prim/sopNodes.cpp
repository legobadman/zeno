#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/geo/geometryutil.h>
#include "glm/gtc/matrix_transform.hpp"
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <sstream>
#include <zeno/formula/zfxexecute.h>
#include <zeno/geo/geometryutil.h>
#include <zeno/utils/format.h>
#include <unordered_set>
#include <zeno/para/parallel_reduce.h>
#include "zeno_types/reflect/reflection.generated.hpp"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {
    using namespace zeno::reflect;

    struct ZDEFNODE() CopyToPoints : INode {
        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Geometry")},
                {"target_Obj", ParamObject("Target Geometry")},
                {"alignTo", ParamPrimitive("Align To", "Align To Point Center", Combobox, std::vector<std::string>{"Align To Point Center", "Align To Min"})}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::shared_ptr<zeno::GeometryObject> target_Obj,
            const std::string& alignTo = "Align To Point Center"
        ) {
            if (!input_object) {
                throw makeError<UnimplError>("empty input object.");
            }
            else if (!target_Obj) {
                throw makeError<UnimplError>("empty target object.");
            }
            if (!input_object->has_point_attr("pos")) {
                throw makeError<UnimplError>("invalid input object.");
            }
            else if (!target_Obj->has_point_attr("pos")) {
                throw makeError<UnimplError>("invalid target object.");
            }

            std::vector<zeno::vec3f> inputPos = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "pos");
            std::vector <std::tuple< bool, std::vector<int >> > inputFacesPoints(input_object->nfaces(), std::tuple<bool, std::vector<int >>({false, std::vector<int>()}));
            for (int i = 0; i < input_object->nfaces(); ++i) {
                inputFacesPoints[i] = std::tuple(input_object->isLineFace(i), input_object->face_points(i));
            }


            std::vector<zeno::vec3f> inputNrm;
            std::vector<zeno::vec2f> inputLines;
            bool hasNrm = input_object->has_point_attr("nrm");
            bool isLine = input_object->is_Line();
            if (hasNrm) {
                inputNrm = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "nrm");
            }

            zeno::vec3f originCenter, _min, _max;
            std::tie(_min, _max) = geomBoundingBox(input_object.get());
            if (alignTo == "Align To Point Center") {
                originCenter = (_min + _max) / 2;
            }
            else {
                originCenter = _min;
            }

            size_t targetObjPointsCount = target_Obj->npoints();
            size_t inputObjPointsCount = input_object->npoints();
            size_t inputObjFacesCount = input_object->nfaces();
            size_t newObjPointsCount = targetObjPointsCount * inputObjPointsCount;
            size_t newObjFacesCount = targetObjPointsCount * inputObjFacesCount;

            std::vector<zeno::vec3f> newObjPos(newObjPointsCount, zeno::vec3f());
            std::vector<zeno::vec3f> newObjNrm;
            if (hasNrm) {
                newObjNrm.resize(newObjPointsCount);
            }

            std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(input_object->is_base_triangle(), newObjPointsCount, newObjFacesCount, true);
            for (size_t i = 0; i < targetObjPointsCount; ++i) {
                zeno::vec3f targetPos = target_Obj->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, i);
                zeno::vec3f dx = targetPos - originCenter;
                glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(dx[0], dx[1], dx[2]));

                const size_t pt_offset = i * inputObjPointsCount;
                const size_t face_offset = i * inputObjFacesCount;
                for (size_t j = 0; j < inputObjPointsCount; j++)
                {
                    auto idx = pt_offset + j;

                    auto& pt = inputPos[j];
                    glm::vec4 gp = translate * glm::vec4(pt[0], pt[1], pt[2], 1);
                    newObjPos[idx] = zeno::vec3f(gp.x, gp.y, gp.z);

                    if (hasNrm) {
                        newObjNrm[idx] = inputNrm[j];
                    }
                }
                for (size_t j = 0; j < inputObjFacesCount; ++j)
                {
                    std::vector<int> facePoints = std::get<1>(inputFacesPoints[j]);
                    for (int k = 0; k < facePoints.size(); ++k) {
                        facePoints[k] += pt_offset;
                    }
                    spgeo->set_face(face_offset + j, facePoints, !std::get<0>(inputFacesPoints[j]));
                }
                spgeo->inheritAttributes(input_object, -1, pt_offset, {"pos", "nrm"}, face_offset, {});
            }

            spgeo->create_attr(ATTR_POINT, "pos", newObjPos);
            if (hasNrm) {
                spgeo->create_attr(ATTR_POINT, "nrm", newObjNrm);
            }

            return spgeo;
        }

    };

    struct ZDEFNODE() Sweep : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"snapDistance", ParamPrimitive("Snap Distance")},
                {"surface_shape", ParamPrimitive("Surface Shape", "Square Tube", Combobox, std::vector<std::string>{"Second Input", "Square Tube"})},
                {"surface_width", ParamPrimitive("Width", 0.2, Slider, std::vector<float>{0.0, 1.0, 0.001}, "", "visible = parameter('Surface Shape').value == 'Square Tube';")},
                {"surface_column", ParamPrimitive("Columns", 2, Slider, std::vector<int>{1, 20, 1}, "", "visible = parameter('Surface Shape').value == 'Square Tube';")},
                {"input_object", ParamObject("Input")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::string surface_shape = "Square Tube",
            float surface_width = 0.2f,
            int surface_column = 2
        )
        {
            return nullptr;
#if 0
            const float w = surface_width;
            if (surface_shape == "Square Tube") {
                if (input_object->is_Line()) {
                    //先针对单位长度的竖线，以及给定的width和column,建模出基本的tube模型
                    std::vector<vec3f> basepts;
                    for (int iy = 0; iy <= 0/*只须考虑底部的面*/; iy++) {
                        for (int iz = 0; iz <= surface_column; iz++) {
                            for (int ix = 0; ix <= surface_column; ix++) {
                                if (0 < ix && ix < surface_column &&
                                    0 < iz && iz < surface_column) {
                                    continue;
                                }
                                float step = surface_width / surface_column;
                                float start = -surface_width / 2;
                                basepts.push_back(vec3f(start + ix * step, iy, start + iz * step));
                            }
                        }
                    }

                    const int npts = basepts.size();
                    std::vector<vec3f> pts = input_object->points_pos();
                    std::vector<vec3f> lastendface(npts);
                    vec3f originVec(0, 1, 0);

                    std::vector<vec3f> newpts;
                    for (int currPt = 0; currPt < pts.size(); currPt++) {
                        int nextPt = input_object->getLineNextPt(currPt);
                        if (currPt != nextPt) {
                            //能进到这里，就不是最后一个点
                            vec3f p1 = pts[currPt];
                            vec3f p2 = pts[nextPt];

                            float vx = p2[0] - p1[0];
                            float vy = p2[1] - p1[1];
                            float vz = p2[2] - p1[2];
                            float LB = zeno::sqrt(vx * vx + vy * vy + vz * vz);

                            vec3f vpt(vx, vy, vz);
                            vpt = zeno::normalize(vpt);
#if 1
                            glm::vec3 vxz(vpt[0], 0, vpt[2]);

                            //步骤一：绕x轴，将y轴旋转至z轴原来位置
                            glm::mat4 M_toz(1.0);
                            {
                                M_toz = glm::rotate(M_toz, glm::pi<float>() / 2, glm::vec3(1, 0, 0));
                            }

                            //步骤二：旋转至xz平面的投影
                            glm::mat4 M_toxz(1.0);
                            {
                                float theta = zeno::atan(vxz[0] / vxz[2]);
                                glm::vec3 rotate_u = glm::vec3(0, 1, 0);
                                M_toxz = glm::rotate(M_toxz, theta, rotate_u);   //绕y轴旋转theta度，将向量投影至x轴
                            }

                            //步骤三：往上提
                            glm::mat4 M_up(1.0);
                            if (vpt[1] != 0)
                            {
                                glm::vec3 vdir(vpt[0], vpt[1], vpt[2]);
                                float tantheta = vpt[1] / zeno::sqrt(vpt[0] * vpt[0] + vpt[2] * vpt[2]);
                                float theta = zeno::atan(tantheta);
                                glm::vec3 rotate_u = glm::cross(glm::vec3(vpt[0], 0, vpt[2]), vdir);
                                M_up = glm::rotate(M_up, theta, rotate_u);
                            }

                            //步骤四：平移
                            glm::mat4 M_translate(1.0);
                            M_translate = glm::translate(M_translate, glm::vec3(p1[0], p1[1], p1[2]));

                            glm::mat4 transM = M_translate * M_up * M_toxz * M_toz;
#endif

#if 0
                            //步骤一：先平移至原点
                            glm::mat4 M_translate(1.0);
                            M_translate = glm::translate(M_translate, -glm::vec3(p1[0], p1[1], p1[2]));

                            //步骤二：投影至xz平面
                            glm::mat4 M_toxz(1.0);
                            {
                                glm::vec3 vdir(vpt[0], vpt[1], vpt[2]);
                                float tantheta = vdir[1] / zeno::sqrt(vdir[0] * vdir[0] + vdir[2] * vdir[2]);
                                float theta = zeno::atan(tantheta);
                                
                                glm::vec3 rotate_u = glm::cross(vdir, glm::vec3(vpt[0], 0, vpt[2]));
                                M_toxz = glm::rotate(M_toxz, theta, rotate_u);
                            }

                            //步骤三：投影至z轴
                            glm::mat4 M_z(1.0);
                            {
                                glm::vec3 vxz(vpt[0], 0, vpt[2]);
                                float theta = zeno::atan(vxz[0] / vxz[2]);
                                glm::vec3 rotate_u = glm::vec3(0, 1, 0);
                                M_z = glm::rotate(M_z, theta, rotate_u);   //绕y轴逆向旋转theta度，将向量投影至x轴
                            }

                            //步骤四：旋转至y轴
                            glm::mat4 M_y(1.0);
                            {
                                M_y = glm::rotate(M_y, -glm::pi<float>()/2, glm::vec3(1, 0, 0));
                            }
                            glm::mat4 rev_transM = M_y * M_z * M_toxz * M_translate;
                            glm::mat4 transM = glm::inverse(rev_transM);
#endif


#if 0
                            //计算两个向量的叉积，得到旋转轴
                            vec3f rot = zeno::cross(originVec, vpt);
                            auto len = zeno::length(rot);
                            glm::mat4 Rx(1.0);
                            if (len == 0) {
                                //考虑originVec和vpt平行的情况
                                //只要单位矩阵即可
                            }
                            else {
                                rot = zeno::normalize(rot);

                                float cos_theta = zeno::dot(originVec, vpt);
                                float sin_theta = zeno::sqrt(1 - cos_theta * cos_theta);
                                float rx = rot[0], ry = rot[1], rz = rot[2];

                                glm::mat3 K = glm::mat3(
                                    0, rz, -ry,
                                    -rz, 0, rx, 
                                    ry, -rx, 0
                                );
                                glm::mat3 R_ = glm::mat3(1.0) + sin_theta * K + (1 - cos_theta) * K * K;
                                Rx = glm::mat4(R_);
                            }

                            glm::mat4 transM(
                                Rx[0][0], Rx[0][1], Rx[0][2], 0,
                                Rx[1][0], Rx[1][1], Rx[1][2], 0,
                                Rx[2][0], Rx[2][1], Rx[2][2], 0,
                                p1[0], p1[1], p1[2], 1);
#endif

                            std::vector<vec3f> istartface;
                            {
                                istartface.resize(npts);
                                for (int i = 0; i < npts; i++) {
                                    vec3f basept = basepts[i];
                                    glm::vec4 v(basept[0], basept[1], basept[2], 1.0);

#if 1
                                    //M_translate* M_up* M_toxz* M_toz;
                                    glm::vec4 _v1 = M_toz * v;
                                    glm::vec4 _v2 = M_toxz * _v1;
                                    glm::vec4 _v3 = M_up * _v2;
                                    glm::vec4 _v4 = M_translate * _v3;
#endif

                                    glm::vec4 newpt = transM* v;
                                    istartface[i] = vec3f(newpt[0], newpt[1], newpt[2]);
                                }
                            }
                            if (currPt >= 0) {
                                if (false) {
                                    //此时收集到istartface是当前点，需要考虑到上一线段的末尾点形成面与当前面的不一致情况
                                    int _npoints = newpts.size();
                                    assert(_npoints >= npts);
                                    for (int i = 0; i < npts; i++) {
                                        //修正当前面
                                        istartface[i] = (istartface[i] + lastendface[i]) / 2;
                                    }
                                }
                                //istartface = lastendface;
                            }

                            //先收集这些确定的点
                            if (currPt >= 0) {
                                for (auto pt : istartface) {
                                    newpts.push_back(pt);
                                }
                            }

                            std::vector<vec3f> iendface(npts);
                            for (int i = 0; i < npts; i++) {
                                iendface[i] = istartface[i] + LB * vpt;
                                lastendface[i] = iendface[i];
                            }

                            //提前收集endface
                            for (auto pt : iendface) {
                                newpts.push_back(pt);
                            }
                        }
                        else {
                            //for (auto pt : lastendface) {
                            //    newpts.push_back(pt);
                            //}
                        }
                    }
                    std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(false, newpts.size(), 0);
                    spgeo->create_attr(ATTR_POINT, "pos", newpts);
                    return spgeo;
#if 0
                    std::vector<vec3f> pts = input_object->points_pos();
                    for (int i = 0; i < pts.size(); i++) {
                        if (i > 0 && i < pts.size() - 1) {
                            vec3f pt = pts[i];
                            vec3f pt_1 = pts[i - 1];
                            vec3f pt1 = pts[i + 1];
                            vec3f v1 = zeno::normalize(pt1 - pt);
                            vec3f v2 = zeno::normalize(pt_1 - pt);
                            while (zeno::dot(v1, v2) == -1) {
                                v1 += vec3f(0.0001, 0, 0);
                                v2 += vec3f(0.0001, 0, 0);
                            }

                            vec3f hor_v = zeno::normalize(v1 + v2);
                            vec3f ver_v = zeno::normalize(zeno::cross(v1, hor_v));

                            //这个start_pt留到最后加
                            vec3f start_pt = pt + w / 2.f * (hor_v + ver_v);
                            //逆时针转一圈
                            vec3f curr_start = start_pt;
                            vec3f curr_pt;
                            for (int i = 1; i <= surface_column; i++) {
                                curr_pt = curr_start + w / surface_column * i * (-hor_v);
                                int j;
                                j = 0;
                            }
                            curr_start = curr_pt; //上一次遍历的终点是下一个线段的起点
                            for (int i = 1; i <= surface_column; i++) {
                                curr_pt = curr_start + w / surface_column * i * (-ver_v);
                                int j;
                                j = 0;
                            }
                            curr_start = curr_pt;
                            for (int i = 1; i <= surface_column; i++) {
                                curr_pt = curr_start + w / surface_column * i * hor_v;
                                int j;
                                j = 0;
                            }
                            curr_start = curr_pt;
                            for (int i = 1; i <= surface_column; i++) {
                                curr_pt = curr_start + w / surface_column * i * ver_v;
                                int j;
                                j = 0;
                            }
                        }
                    }
#endif
                }
            }
            return input_object;
#endif
        }
    };

    struct ZDEFNODE() Merge : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"list_object", ParamObject("Input Of Objects")},
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::ListObject> list_object
        ) {
            std::shared_ptr<zeno::GeometryObject> mergedObj = zeno::mergeObjects(list_object);
            return mergedObj;
        }
    };

    struct ZDEFNODE() Divide : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input")},
                {"remove_shared_edge", ParamPrimitive("Remove Shared Edge")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            zeno::vec3f Size,
            bool remove_shared_edge
        ) {
            auto bbox = geomBoundingBox(input_object.get());
            float xmin = bbox.first[0], ymin = bbox.first[1], zmin = bbox.first[2],
                xmax = bbox.second[0], ymax = bbox.second[1], zmax = bbox.second[2];
            float dx = Size[0], dy = Size[1], dz = Size[2];
            int nx = (dx == 0) ? 0 : (xmax - xmin) / dx;
            int ny = (dy == 0) ? 0 : (ymax - ymin) / dy;
            int nz = (dz == 0) ? 0 : (zmax - zmin) / dz;

            for (int i = 0; i <= nx; i++) {
                float xi = xmin + i * dx;
                input_object = divideObject(input_object, Keep_Both, vec3f(xi, 0, 0), vec3f(1, 0, 0));
            }
            for (int i = 0; i <= ny; i++) {
                float yi = ymin + i * dy;
                input_object = divideObject(input_object, Keep_Both, vec3f(0, yi, 0), vec3f(0, 1, 0));
            }
            for (int i = 0; i <= nz; i++) {
                float zi = zmin + i * dz;
                input_object = divideObject(input_object, Keep_Both, vec3f(0, 0, zi), vec3f(0, 0, 1));
            }
            return input_object;
        }
    };

    struct ZDEFNODE() Resample : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input Object")}
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float Length = 0.1)
        {
            if (Length == 0)
                return input_object;

            const int nface = input_object->nfaces();
            const auto& pos = input_object->points_pos();

            std::vector<vec3f> newpos;
            std::vector<std::vector<int>> newfaces;
            for (int iFace = 0; iFace < nface; iFace++) {
                std::vector<int> face_indice = input_object->face_points(iFace);
                std::vector<int> newface;

                for (int i = 0; i < face_indice.size(); i++) {
                    int startPt = -1, endPt = -1;
                    if (i == 0) {
                        startPt = face_indice[face_indice.size() - 1];
                        endPt = face_indice[0];
                    }
                    else {
                        startPt = face_indice[i - 1];
                        endPt = face_indice[i];
                    }

                    vec3f startPos = pos[startPt], endPos = pos[endPt];
                    vec3f dir = zeno::normalize(endPos - startPos);
                    int k = zeno::length(endPos - startPos) / Length;

                    for (int j = 0; j < k; j++) {
                        vec3f middlePos = startPos + j * Length * dir;
                        newface.push_back(newpos.size());
                        newpos.push_back(middlePos);
                        //if (j == k) {
                        //    if (!(middlePos == endPos)) {
                        //        newface.push_back(newpos.size());
                        //        newpos.push_back(endPos);
                        //    }
                        //}
                    }
                }
                newfaces.push_back(newface);
            }

            auto spOutput = std::make_shared<GeometryObject>(input_object->is_base_triangle(), newpos.size(), newfaces.size());
            for (auto faceindice : newfaces) {
                spOutput->add_face(faceindice);
            }
            spOutput->create_point_attr("pos", newpos);
            return spOutput;
        }
    };

    struct ZDEFNODE() Reverse : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input Object")}
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(std::shared_ptr<zeno::GeometryObject> input_object)
        {
            const int nface = input_object->nfaces();
            const auto& pos = input_object->points_pos();
            std::shared_ptr<zeno::GeometryObject> spOutput = std::make_shared<zeno::GeometryObject>(input_object->is_base_triangle(), pos.size(), nface);

            for (int iFace = 0; iFace < nface; iFace++) {
                std::vector<int> face_indice = input_object->face_points(iFace);
                std::reverse(face_indice.begin() + 1, face_indice.end());
                spOutput->add_face(face_indice); //TODO: line
            }
            spOutput->create_point_attr("pos", pos);    //暂时不考虑其他属性
            return spOutput;
        }
    };

    struct ZDEFNODE() Clip : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input Object")},
                {"Keep", ParamPrimitive("Keep", "All", Combobox, std::vector<std::string>{"All", "Part Below The Plane", "Part Above The Plane"})},
                {"center_pos", ParamPrimitive("Center Position")},
                {"direction", ParamPrimitive("Direction")}
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            const std::string& Keep = "All",
            zeno::vec3f center_pos = zeno::vec3f(0, 0, 0),
            zeno::vec3f direction = zeno::vec3f(0, 1, 0))
        {
            DivideKeep keep;
            if (Keep == "All") {
                keep = Keep_Both;
            }
            else if (Keep == "Part Below The Plane") {
                keep = Keep_Above;
            }
            else if (Keep == "Part Above The Plane") {
                keep = Keep_Below;
            }
            else {
                throw makeError<UnimplError>("Keep Error");
            }
            return divideObject(input_object, keep, center_pos, direction);
        }
    };

    struct ZDEFNODE() RemoveUnusedPoints : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Object")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(std::shared_ptr<zeno::GeometryObject> input_object) {
            std::set<int> unused;
            for (int i = 0; i < input_object->npoints(); i++) {
                if (input_object->point_faces(i).empty()) {
                    unused.insert(i);
                }
            }
            for (auto iter = unused.rbegin(); iter != unused.rend(); iter++) {
                //TODO: impl remove_points
                input_object->remove_point(*iter);
            }
            return input_object;
        }
    };

    struct ZDEFNODE() RemoveInlinePoints : INode {
        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Object")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(std::shared_ptr<zeno::GeometryObject> input_object)
        {
            const std::vector<vec3f> pos = input_object->points_pos();
            std::set<int> unusedPoints;
            for (int iFace = 0; iFace < input_object->nfaces(); iFace++) {
                const std::vector<int>& pts = input_object->face_points(iFace);
                for (int i = 1; i < pts.size() - 1; i++) {
                    //观察pts[i]是否只有一个面
                    const auto& _faces = input_object->point_faces(pts[i]);
                    if (_faces.size() != 1) {
                        continue;
                    }
                    assert(_faces[0] == iFace);

                    vec3f pa = pos[pts[i - 1]];
                    vec3f pb = pos[pts[i]];
                    vec3f pc = pos[pts[i + 1]];
                    //检查pa,pb,pc是否共线
                    if (zeno::dot(zeno::normalize(pb - pa), zeno::normalize(pc - pa)) == 1) {
                        unusedPoints.insert(pts[i]);
                    }
                }
            }
            for (auto iter = unusedPoints.rbegin(); iter != unusedPoints.rend(); iter++) {
                //TODO: impl remove_points
                input_object->remove_point(*iter);
            }
            return input_object;
        }
    };

    struct ZDEFNODE() Measure : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Object")},
                {"measure", ParamPrimitive("Measure", "Area", Combobox, std::vector<std::string>{"Area", "Length"})},
                {"outputAttrName", ParamPrimitive("Output Attribute Name", "area")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object, 
            const std::string& measure = "Area",
            const std::string& outputAttrName = "area")
        {
            const auto& pos = input_object->points_pos();
            int nFace = input_object->nfaces();
            std::vector<float> measurements(nFace, 0.f);

            for (int iFace = 0; iFace < nFace; iFace++) {
                const std::vector<int>& pts = input_object->face_points(iFace);
                int n = pts.size();
                std::vector<vec3f> face_ptpos(n);
                for (int i = 0; i < n; i++) {
                    face_ptpos[i] = pos[pts[i]];
                }
                if (measure == "Area") {
                    if (n < 3) continue;        //线段
                    vec3f P0 = face_ptpos[0];
                    float area = 0.f;
                    for (int i = 1; i < pts.size() - 1; i++) {
                        vec3f P1 = face_ptpos[i];
                        vec3f P2 = face_ptpos[i + 1];

                        vec3f v1 = P1 - P0;
                        vec3f v2 = P2 - P0;

                        vec3f cross_product = zeno::cross(v1, v2);
                        area += 0.5f * zeno::length(cross_product);
                    }
                    measurements[iFace] = area;
                }
                else if (measure == "Length") {
                    //TODO
                }
            }
            input_object->create_face_attr(outputAttrName, measurements);
            return input_object;
        }
    };

    struct ZDEFNODE() Mirror : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Object")},
                {"bKeepOriginal", ParamPrimitive("Keep Original")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            zeno::vec3f Position = zeno::vec3f(0, 0, 0),
            zeno::vec3f Direction = zeno::vec3f(1, 0, 0),
            float Distance = 1.f,
            bool bKeepOriginal = true
            )
        {
            const auto& pos = input_object->points_pos();
            int npos = pos.size();
            int nface = input_object->nfaces();

            int Npos = bKeepOriginal ? 2 * npos : npos;
            int Nface = bKeepOriginal ? 2 * nface : nface;

            std::vector<vec3f> new_pos(Npos);

            auto spOutput = std::make_shared<zeno::GeometryObject>(input_object->is_base_triangle(), Npos, Nface);
            std::copy(pos.begin(), pos.end(), new_pos.begin());
            if (bKeepOriginal) {
                std::copy(pos.begin(), pos.end(), new_pos.begin() + npos);
            }

            if (bKeepOriginal) {
                for (int iFace = 0; iFace < nface; iFace++) {
                    std::vector<int> pts = input_object->face_points(iFace);
                    spOutput->add_face(pts);
                }
            }

            for (int iFace = 0; iFace < nface; iFace++) {
                std::vector<int> pts = input_object->face_points(iFace);
                std::reverse(pts.begin(), pts.end());
                std::vector<int> offset_pts;
                for (int pt : pts) {
                    zeno::vec3f P = pos[pt];

                    float A = Direction[0], B = Direction[1], C = Direction[2];
                    float px = Position[0], py = Position[1], pz = Position[2];
                    float D = -(A * px + B * py + C * pz);
                    zeno::vec3f normal_vec(A, B, C);
                    float normal_squared = zeno::dot(normal_vec, normal_vec);
                    float dist = (zeno::dot(P, normal_vec) + D) / normal_squared;
                    zeno::vec3f P_mirror = P - 2 * dist * normal_vec;

                    pt += (bKeepOriginal ? npos : 0);
                    new_pos[pt] = P_mirror;
                    offset_pts.push_back(pt);
                }
                spOutput->add_face(offset_pts);
            }
            spOutput->create_point_attr("pos", new_pos);
            return spOutput;
        }
    };

    struct ZDEFNODE() MatchSize : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Object")},
                {"match_object", ParamObject("Match Object")},
                {"TargetPosition", ParamPrimitive("Target Position", zeno::vec3f(0,0,0), Vec3edit, Any(), "", "visible = parameter('Match Object').connected == false;")},
                {"TargetSize", ParamPrimitive("Target Size", zeno::vec3f(1,1,1), Vec3edit, Any(), "", "visible = parameter('Match Object').connected == false;")},
                {"bTranslate", ParamPrimitive("Translate")},
                {"TranslateXTo", ParamPrimitive("Translate X To", "Center", Combobox, std::vector<std::string>{"Min", "Center", "Max"})},
                {"TranslateYTo", ParamPrimitive("Translate Y To", "Center", Combobox, std::vector<std::string>{"Min", "Center", "Max"})},
                {"TranslateZTo", ParamPrimitive("Translate Z To", "Center", Combobox, std::vector<std::string>{"Min", "Center", "Max"})},
                {"bScaleToFit", ParamPrimitive("Scale To Fit")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        static glm::vec3 mapplypos(glm::mat4 const& matrix, zeno::vec3f const& vector) {
            auto vec = zeno::vec_to_other<glm::vec3>(vector);
            auto vector4 = matrix * glm::vec4(vec, 1.0f);
            return glm::vec3(vector4) / vector4.w;
        }

        static glm::vec3 mapplynrm(glm::mat4 const& matrix, zeno::vec3f const& vector) {
            auto vec = zeno::vec_to_other<glm::vec3>(vector);
            glm::mat3 normMatrix(matrix);
            normMatrix = glm::transpose(glm::inverse(normMatrix));
            auto vector3 = normMatrix * vec;
            return glm::normalize(vector3);
        }

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::shared_ptr<zeno::GeometryObject> match_object,
            zeno::vec3f TargetPosition = zeno::vec3f(0,0,0),
            zeno::vec3f TargetSize = zeno::vec3f(1,1,1),
            bool bTranslate = true,
            std::string TranslateXTo = "Center",
            std::string TranslateYTo = "Center",
            std::string TranslateZTo = "Center",
            bool bScaleToFit = false
        ) {
            if (!bTranslate && !bScaleToFit) {
                return input_object;
            }

            const auto& currbbox = geomBoundingBox(input_object.get());

            zeno::vec3f boxmin, boxmax;
            if (match_object) {
                const auto& bbox = geomBoundingBox(match_object.get());
                boxmin = bbox.first;
                boxmax = bbox.second;
            }
            else {
                boxmin = TargetPosition - 0.5 * TargetSize;
                boxmax = TargetPosition + 0.5 * TargetSize;
            }

            vec3f boxsize = boxmax - boxmin;
            if (TranslateXTo == "Min") {
                boxmin[0] += boxsize[0] / 2.;
                boxmax[0] += boxsize[0] / 2.;
            }
            else if (TranslateXTo == "Max") {
                boxmin[0] -= boxsize[0] / 2.;
                boxmax[0] -= boxsize[0] / 2.;
            }

            if (TranslateYTo == "Min") {
                boxmin[1] += boxsize[1] / 2.;
                boxmax[1] += boxsize[1] / 2.;
            }
            else if (TranslateYTo == "Max") {
                boxmin[1] -= boxsize[1] / 2.;
                boxmax[1] -= boxsize[1] / 2.;
            }

            if (TranslateZTo == "Min") {
                boxmin[2] += boxsize[2] / 2.;
                boxmax[2] += boxsize[2] / 2.;
            }
            else if (TranslateZTo == "Max") {
                boxmin[2] -= boxsize[2] / 2.;
                boxmax[2] -= boxsize[2] / 2.;
            }

            glm::mat4 matrix(1.);
            if (bScaleToFit) {
                vec3f boxsize = (boxmax - boxmin);
                matrix = glm::scale(matrix, glm::vec3(boxsize[0], boxsize[1], boxsize[2]));
            }
            if (bTranslate) {
                vec3f boxcenter = (boxmin + boxmax) / 2.;
                vec3f center_offset = boxcenter - (currbbox.first + currbbox.second) / 2.;
                matrix = glm::translate(matrix, glm::vec3(center_offset[0], center_offset[1], center_offset[2]));
            }

            if (input_object->has_attr(ATTR_POINT, "pos"))
            {
                input_object->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", 0, [&](int idx, zeno::vec3f old_pos)->zeno::vec3f {
                    auto p = mapplypos(matrix, old_pos);
                    auto newpos = zeno::other_to_vec<3>(p);
                    return newpos;
                    });
            }

            if (input_object->has_attr(ATTR_POINT, "nrm"))
            {
                input_object->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "nrm", 0, [&](int idx, zeno::vec3f old_nrm)->zeno::vec3f {
                    auto n = mapplynrm(matrix, old_nrm);
                    auto newnrm = zeno::other_to_vec<3>(n);
                    return newnrm;
                    });
            }

            return input_object;
        }
    };

    struct ZDEFNODE() Peak : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float Distance = 0.2f
        ) {
            const std::vector<vec3f>& nrms = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "nrm");
            input_object->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", 0, [&](int idx, zeno::vec3f old_pos)->zeno::vec3f {
                const vec3f& nrm = nrms[idx];
                if (idx == 3) {
                    int j;
                    j = 0;
                }
                return old_pos + nrm * Distance;
                });
            return input_object;
        }
    };

    struct ZDEFNODE() Scatter : INode {
        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"Count", ParamPrimitive("Points Count", 10, Slider, std::vector<int>{0, 1000, 1})},
                {"Seed", ParamPrimitive("Random Seed", 0, Slider, std::vector<int>{0, 100, 1})},
                {"sampleRegion", ParamPrimitive("Sample Regin", "Face", Combobox, std::vector<std::string>{"Face", "Volumn"})}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            const std::string& sampleRegion = "Face",
            int Count = 10,
            int Seed = 0
        ) {
            auto spOutput = scatter(input_object, sampleRegion, Count, Seed);
            return spOutput;
        }
    };

    struct ZDEFNODE() ConvertLine : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"bKeepOrder", ParamPrimitive("Keep Order")},
                {"lengthAttr", ParamPrimitive("Length Attribute")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            bool bKeepOrder = true,
            std::string lengthAttr = ""
        ) {
            int nPts = input_object->npoints();
            const std::vector<std::vector<int>>& faces = input_object->face_indice();

            std::unordered_set<uint64_t> hash;
            std::vector<std::vector<int>> lines;
            for (auto ind : faces) {
                for (int i = 1; i < ind.size(); i++) {
                    int from = ind[i - 1];
                    int to = ind[i];
                    std::pair<int, int> p = { from, to };
                    uint64_t minidx = std::min(from, to), maxidx = std::max(from, to);
                    uint64_t id = minidx + (maxidx << 32);
                    if (hash.find(id) == hash.end()) {
                        lines.push_back({ from, to });
                        hash.insert(id);
                    }
                }
            }

            std::shared_ptr<zeno::GeometryObject> line_object = std::make_shared<zeno::GeometryObject>(false, nPts, lines.size());
            line_object->create_point_attr("pos", input_object->points_pos());
            for (auto line : lines) {
                line_object->add_face(line, false);
            }
            return line_object;
        }
    };

    struct ZDEFNODE() UniquePoints : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"bPostComputeNormals", ParamPrimitive("Post-Compute Normals")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            bool bPostComputeNormals = false
        ) {
            std::vector<std::vector<vec3f>> newFaces;
            const std::vector<zeno::vec3f>& pos = input_object->points_pos();
            std::vector<vec3f> point_normals;

            for (int iFace = 0; iFace < input_object->nfaces(); iFace++) {
                std::vector<int> pts = input_object->face_points(iFace);
                std::vector<vec3f> new_face(pts.size());
                for (int i = 0; i < pts.size(); i++) {
                    new_face[i] = pos[pts[i]];
                }
                newFaces.push_back(new_face);

                if (bPostComputeNormals && new_face.size() > 2) {
                    vec3f p01 = new_face[1] - new_face[0];
                    vec3f p12 = new_face[2] - new_face[1];
                    vec3f nrm = zeno::normalize(zeno::cross(p01, p12));
                    for (int i = 0; i < new_face.size(); i++) {
                        point_normals.push_back(nrm);
                    }
                }
            }
            auto spOutput = constructGeom(newFaces);
            if (bPostComputeNormals)
                spOutput->create_point_attr("nrm", point_normals);
            return spOutput;
        }
    };

    struct ZDEFNODE() Sort : INode {
        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"pointSort", ParamPrimitive("Point Sort", "No Change", Combobox, std::vector<std::string>{"No Change", "By Vertex Order", "Random", "Along Vector", "Near To Point"})},
                {"alongVector", ParamPrimitive("Vector", zeno::vec3f(0,1,0), Vec3edit, Any(), "", "visible = parameter('Point Sort').value == 'Along Vector';")},
                {"nearPoint", ParamPrimitive("Point", zeno::vec3f(0,1,0), Vec3edit, Any(), "", "visible = parameter('Point Sort').value == 'Near To Point';")},
                {"bReversePointSort", ParamPrimitive("Reverse Point Sort")},
                {"faceSort", ParamPrimitive("Face Sort", "No Change", Combobox, std::vector<std::string>{"No Change", "Random", "By Attribute"})},
                {"byAttrName", ParamPrimitive("Attribute Name", "", Lineedit, Any(), "", "visible = parameter('Face Sort').value == 'By Attribute';")},
                {"bReverseFaceSort", ParamPrimitive("Reverse Face Sort")},
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            const std::string & pointSort = "No Change",
            zeno::vec3f alongVector = zeno::vec3f(0, 1, 0),
            zeno::vec3f nearPoint = zeno::vec3f(0, 0, 0),
            bool bReversePointSort = false,
            const std::string & faceSort = "No Change",
            const std::string& byAttrName = "",
            bool bReverseFaceSort = false
        ) {
            int npts = input_object->npoints();
            int nfaces = input_object->nfaces();
            const auto& pos = input_object->points_pos();

            if (pointSort == "By Vertex Order") {
                std::set<std::string> edges;
                std::vector<vec3f> new_pos(pos.size());
                int nSortPoints = 0;
                std::map<int, int> old2new;
                auto spOutput = std::make_shared<GeometryObject>(false, npts, nfaces, true);
                for (int iFace = 0; iFace < nfaces; iFace++) {
                    std::vector<int> pts = input_object->face_points(iFace);
                    std::vector<int> newface;
                    for (int vertex = 0; vertex < pts.size(); vertex++) {
                        int fromPt, toPt;

                        fromPt = pts[vertex];

                        if (vertex == pts.size() - 1) {
                            toPt = pts[0];
                        }
                        else {
                            toPt = pts[vertex + 1];
                        }

                        int newFrom, newTo;
                        if (old2new.find(fromPt) != old2new.end()) {
                            newFrom = old2new[fromPt];
                        }
                        else {
                            newFrom = nSortPoints++;
                        }
                        if (old2new.find(toPt) != old2new.end()) {
                            newTo = old2new[toPt];
                        }
                        else {
                            newTo = nSortPoints++;
                        }

                        old2new.insert(std::make_pair(fromPt, newFrom));
                        old2new.insert(std::make_pair(toPt, newTo));
                        newface.push_back(newFrom);
                    }
                    spOutput->set_face(iFace, newface);
                }
                for (int oldpt = 0; oldpt < pos.size(); oldpt++) {
                    int newpt = old2new[oldpt];
                    new_pos[newpt] = pos[oldpt];
                }
                spOutput->create_point_attr("pos", new_pos);
                return spOutput;
            }
            return nullptr;
        }
    };

    struct ZDEFNODE() PolyExpand : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"bOutputInside", ParamPrimitive("Output Inside")},
                {"bOutputOutside", ParamPrimitive("Output Outside")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        void findOffsetPoints(
            vec3f p1,
            vec3f p2,
            vec3f p3,
            float offset,
            vec3f& p1_inside,
            vec3f& p1_outside,
            vec3f& p2_inside,
            vec3f& p2_outside)
        {
            vec3f v12 = p2 - p1, v23 = p3 - p2;
            vec3f n1 = zeno::cross(v12, v23);
            vec3f n2 = zeno::cross(v23, v12);
            vec3f p1_inside_dir = zeno::normalize(zeno::cross(n1, v12));
            vec3f p1_outside_dir = -p1_inside_dir;
            vec3f p2_outside_dir = zeno::normalize(zeno::cross(n2, v23));
            vec3f p2_inside_dir = -p2_outside_dir;
            p1_inside = p1 + p1_inside_dir * offset;
            p1_outside = p1 + p1_outside_dir * offset;
            p2_inside = p2 + p2_inside_dir * offset;
            p2_outside = p2 + p2_outside_dir * offset;
        }

        vec3f findIntersect(vec3f p1, vec3f v1, vec3f p2, vec3f v2)
        {
            float x1 = p1[0], y1 = p1[1], z1 = p1[2], x2 = p2[0], y2 = p2[1], z2 = p2[2];
            float a1 = v1[0], b1 = v1[1], c1 = v1[2], a2 = v2[0], b2 = v2[1], c2 = v2[2];

            float v1_dist = zeno::lengthSquared(v1);
            float v2_dist = zeno::lengthSquared(v2);
            float v1_v2 = zeno::dot(v1, v2);

            float divider = v1_dist * v2_dist - zeno::pow(v1_v2, 2);

            float t = (zeno::dot(p2-p1,v1) * v2_dist - zeno::dot(p2-p1,v2) * v1_v2) / divider;
            float s = (zeno::dot(p2 - p1, v2) * v1_dist - zeno::dot(p2 - p1, v1) * v1_v2) / divider;

            vec3f result = p1 + t * v1;
            return result;
        }

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float Offset = 0.1,
            bool bOutputInside = true,
            bool bOutputOutside = true
        ) {
            const std::vector<vec3f>& pos = input_object->points_pos();
            std::vector<std::vector<vec3f>> newFaces;
            for (int iFace = 0; iFace < input_object->nfaces(); iFace++) {
                std::vector<int> pts = input_object->face_points(iFace);
                std::vector<vec3f> inside_face(pts.size()), outside_face(pts.size());
                for (int i = 0; i < pts.size(); i++) {
                    int p1 = pts[i];
                    int p2 = pts[(i + 1) % pts.size()];
                    int p3 = pts[(i + 2) % pts.size()];

                    vec3f p1_inside, p1_outside, p2_inside, p2_outside;

                    findOffsetPoints(pos[p1], pos[p2], pos[p3], Offset,
                        p1_inside, p1_outside, p2_inside, p2_outside);
                    vec3f v1 = zeno::normalize(pos[p2] - pos[p1]);
                    vec3f v2 = zeno::normalize(pos[p3] - pos[p2]);
                    vec3f pi_inside = findIntersect(p1_inside, v1, p2_inside, v2);
                    vec3f pi_outside = findIntersect(p1_outside, v1, p2_outside, v2);
                    inside_face[i] = pi_inside;
                    outside_face[i] = pi_outside;
                }
                if (bOutputInside)
                    newFaces.push_back(inside_face);
                if (bOutputOutside)
                    newFaces.push_back(outside_face);
            }
            auto spOutput = constructGeom(newFaces);
            auto spFinal = zeno::fuseGeometry(spOutput, 0.005);
            return spFinal;
        }
    };

    struct ZDEFNODE() Extrude : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"Distance", ParamPrimitive("Distance", 0.f)},//, Slider, std::vector<float>{0.0, 1.0, 0.01})},
                {"Inset", ParamPrimitive("Inset", 0.f)},//, Slider, std::vector<float>{0.0, 1.0, 0.01})},
                {"bOutputFrontAttr", ParamPrimitive("Output Front Attribute", false, Checkbox)},
                {"front_attr", ParamPrimitive("Front Attribute", "extrudeFront", Lineedit, zeno::reflect::Any(), "", "enable = parameter('Output Front Attribute').value == 1;")},
                {"bOutputBackAttr", ParamPrimitive("Output Back Attribute", false, Checkbox)},
                {"back_attr", ParamPrimitive("Back Attribute", "extrudeBack", Lineedit, zeno::reflect::Any(), "", "enable = parameter('Output Back Attribute').value == 1;")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        zeno::vec3f getSpreadDirector(std::shared_ptr<GeometryObject> input_obj, int idxPt)
        {
            /* 根据给定的输入obj，以及顶点，得到这个点的挤出延长线方向向量 */
            //找出这个点关联的所有面，然后得到各个面的法线，然后求和即可
            std::vector<int> faces = input_obj->point_faces(idxPt);
            zeno::vec3f nrm_sum(0,0,0);
            for (auto f : faces) {
                //TODO: 后续可以考虑提前缓存法线，目前基于实现方便先直接调用
                zeno::vec3f face_nrm = input_obj->face_normal(f);
                nrm_sum += face_nrm;
            }
            return zeno::normalize(nrm_sum);
        }

        std::pair<zeno::vec3f, zeno::vec3f> getExtrudeDest(
            std::shared_ptr<GeometryObject> input_obj,
            int lastPt,
            int currPt,
            int back_face,
            float dist,
            bool& needFace
            )
        {
            /* 给定基准的back平面，以及这个平面上相邻的两个点lastPt,currPt，以dist的挤出距离，求出:
            1. 这两个点延长至front平面时的终点位置
            2. 这两个点的线在延申时形成的面，是否需要产生。（如果是两个面的共线，则不需要产生）
             */
            zeno::vec3f nrm = input_obj->face_normal(back_face);
            const auto& pts = input_obj->points_pos();

            zeno::vec3f last_pos = pts[lastPt];
            zeno::vec3f last_spread_dir = getSpreadDirector(input_obj, lastPt);
            float costheta = zeno::dot(last_spread_dir, nrm);
            float k = 1. / costheta;
            zeno::vec3f dest1 = k * dist * last_spread_dir + last_pos;

            zeno::vec3f curr_pos = pts[currPt];
            zeno::vec3f curr_spread_dir = getSpreadDirector(input_obj, currPt);
            costheta = zeno::dot(curr_spread_dir, nrm);
            k = 1. / costheta;
            zeno::vec3f dest2 = k * dist * curr_spread_dir + curr_pos;

            std::vector<int> faces1 = input_obj->point_faces(lastPt);
            std::vector<int> faces2 = input_obj->point_faces(currPt);
            //两个点关联的面，除了back_face，观察是否还有别的面。
            needFace = true;
            for (auto f1 : faces1) {
                for (auto f2 : faces2) {
                    if (f1 == f2 && f1 != back_face) {
                        needFace = false;
                        break;
                    }
                }
            }
            return { dest1, dest2 };
        }

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float Distance = 0.f,
            float Inset = 0.f,
            bool bOutputFrontAttr = false,
            const std::string& front_attr = "",
            bool bOutputBackAttr = false,
            const std::string& back_attr = ""
        ) {
            if (Distance > 0) {
                std::vector<std::vector<vec3f>> newFaces;
                const std::vector<zeno::vec3f>& pos = input_object->points_pos();
                for (int iFace = 0; iFace < input_object->nfaces(); iFace++) {
                    std::vector<int> pts = input_object->face_points(iFace);
                    std::vector<vec3f> dest_pts;

                    std::vector<vec3f> front_pts;
                    for (int i = 0; i < pts.size(); i++) {
                        int lastPt = -1, currPt = -1;
                        if (i == 0) {
                            lastPt = pts[pts.size() - 1];
                            currPt = pts[0];
                        }
                        else {
                            lastPt = pts[i - 1];
                            currPt = pts[i];
                        }
                        bool needFace = false;
                        const auto& spread = getExtrudeDest(input_object, lastPt, currPt, iFace, Distance, needFace);
                        zeno::vec3f last_spread = spread.first;
                        zeno::vec3f curr_spread = spread.second;

                        front_pts.push_back(last_spread);

                        if (needFace) {
                            //dest_pts[i], dest_pts[i-1], pts[i], pts[i-1]将构成一个平面
                            std::vector<zeno::vec3f> new_face = { curr_spread, last_spread, pos[lastPt], pos[currPt] };
                            newFaces.push_back(new_face);
                        }
                    }
                    //front face
                    newFaces.push_back(front_pts);  //todo: 需要传一个front的字段标记为front面。
                }
                auto spOutput = constructGeom(newFaces);
                auto spFinal = zeno::fuseGeometry(spOutput, 0.005);
                return spFinal;
            }
            else {
                return input_object;
            }
        }
    };

    struct ZDEFNODE() Blast : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input")},
                {"zfx", ParamPrimitive("Zfx Expression")},
                {"group", ParamPrimitive("Group", "Points", Combobox, std::vector<std::string>{"Points", "Faces"})},
                {"deleteNonSelected", ParamPrimitive("Delete Non Selected")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::string zfx = "",
            std::string group="Points",
            bool deleteNonSelected = false
        ) {
            zeno::ZfxContext ctx;
            std::string rem_what;
            std::string idnum;
            std::string nor = deleteNonSelected ? "!" : "";
            if (group == "Points") {
                rem_what = "point";
                idnum = "@ptnum";
                ctx.runover = ATTR_POINT;
            }
            else if (group == "Faces") {
                rem_what = "face";
                idnum = "@facenum";
                ctx.runover = ATTR_FACE;
            }
            else {
                throw UnimplError("Unknown group on blast");
            }

            std::string finalZfx;
            finalZfx += "if (" + nor + "(" + zfx + ")" + ") {\n";
            finalZfx += "    remove_" + rem_what + "(" + idnum + ");\n";
            finalZfx += "}";

            std::shared_ptr<zeno::GeometryObject> spOutput(input_object);
            
            ctx.spNode = shared_from_this();
            ctx.spObject = spOutput;
            ctx.code = finalZfx;

            zeno::ZfxExecute zfxexe(finalZfx, &ctx);
            zfxexe.execute();
            return spOutput;
        }

        zeno::CustomUI export_customui() const override {
            zeno::CustomUI ui = zeno::INode::export_customui();
            ui.uistyle.background = "#DF7C1B";
            return ui;
        }
    };

    struct ZDEFNODE() AverageFuse : INode {
        //houdini fuse节点的average模式

        ReflectCustomUI m_uilayout = {
            _Group{
                {"snapDistance", ParamPrimitive("Snap Distance")},
                {"input_object", ParamObject("Input Geometry")},
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float snapDistance = 0.01
        ) {

            if (!input_object) {
                throw makeError<UnimplError>("empty input object.");
            }
#if 1
            return fuseGeometry(input_object, snapDistance);
#else
            int ptnumber = input_object->npoints();
            std::map < int, std::vector<int> > pointsToFuse;
            std::vector<int> fusedPoints(ptnumber, -1);
            const std::vector<zeno::vec3f>& inputPos = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "pos");
            for (int i = 0; i < inputPos.size(); ++i) {
                if (fusedPoints[i] != -1) {
                    continue;
                }
                for (int j = i + 1; j < inputPos.size(); ++j) {
                    if (fusedPoints[j] != -1) {
                        continue;
                    }
                    if (glm::distance(glm::vec3(inputPos[i][0], inputPos[i][1], inputPos[i][2]), glm::vec3(inputPos[j][0], inputPos[j][1], inputPos[j][2])) <= snapDistance) {
                        pointsToFuse[i].push_back(j);
                        fusedPoints[j] = i;
                    }
                }
            }

            for (auto& [targetPt, fusePoints] : pointsToFuse) {
                zeno::vec3f pos = input_object->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, targetPt);
                for (auto& pt : fusePoints) {
                    pos += input_object->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, pt);
                }
                pos /= (fusePoints.size() + 1);
                input_object->set_elem<zeno::vec3f>(ATTR_POINT, "pos", targetPt, pos);
            }
            input_object->fusePoints(fusedPoints);

            return input_object;
#endif
        }
    };
}
