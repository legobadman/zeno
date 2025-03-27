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

        static const int Divide_X = 0;
        static const int Divide_Y = 1;
        static const int Divide_Z = 2;

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input")},
                {"remove_shared_edge", ParamPrimitive("Remove Shared Edge")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        zeno::vec3f getInsetByInter(zeno::vec3f Pa, zeno::vec3f Pb, float axis_val, int axis) {
            float t1 = (axis_val - Pa[axis]) / (Pb[axis] - Pa[axis]);
            //如果t==0或者t==1，说明交点就是两侧顶点之一，或者都是（垂直的情况）
            zeno::vec3f P1_inset = Pa + t1 * (Pb - Pa);
            return P1_inset;
        }

        void removeRepeat(std::vector<zeno::vec3f>& vec) {
            zeno::vec3f lastPos = vec[0];
            for (auto iter = vec.begin(); iter != vec.end(); ) {
                if (iter == vec.begin()) {
                    iter++;
                    continue;
                }
                if (*iter == lastPos) {
                    //当前pos是重复的点
                    lastPos = *iter;
                    iter = vec.erase(iter);
                }
                else {
                    lastPos = *iter;
                    iter++;
                }
            }
        }

        bool splitFace(
            const std::vector<zeno::vec3f>& facepts,        //当前面所有的点坐标，元素之间形成边，假定没有孤立点或者洞
            int axis,    //分割轴
            float axis_val,     //对应于axis分割轴下的分割面
            /*out*/std::vector<zeno::vec3f>& left_face,     //分割出来“左”边的面，后续可能要继续分割
            /*out*/std::vector<zeno::vec3f>& right_face    //分割出来“右”边的面，后续可能要继续分割
            )
        {
            float axis_min = 0, axis_max = 0;
            std::pair<vec3f, vec3f> bbox_info = parallel_reduce_minmax(facepts.begin(), facepts.end());
            axis_min = bbox_info.first[axis];
            axis_max = bbox_info.second[axis];

            if (axis_val <= axis_min || axis_val >= axis_max) {
                return false;
            }

            std::vector<int> left_points, right_points;
            const int N = facepts.size();
            for (int i = 0; i < N; i++) {
                zeno::vec3f currPos = facepts[i];
                if (currPos[axis] <= axis_val) {
                    left_points.push_back(i);
                }
                if (currPos[axis] >= axis_val) {
                    right_points.push_back(i);
                }
            }

            std::sort(left_points.begin(), left_points.end());
            std::sort(right_points.begin(), right_points.end());

            const int leftN = left_points.size();
            const int rightN = right_points.size();

            //两个相交点的左右顶点的索引
            //这个焦点信息要这么记录：
            //inset的first值<=axis_val, second值>=axis_val
            //inset的左右可以是同一个值，即，顶点就是交点
            std::pair<int, int> inset1 = { -1,-1 }, inset2 = { -1,-1 };

            //遍历左右两边的points，找到交点信息
            bool bHasHole = false;
            for (int i = 1; i < left_points.size(); i++) {
                int Pi_1 = left_points[i - 1];
                int Pi = left_points[i];
                if (Pi - Pi_1 > 1) {
                    //出现了洞，这样就可以直接提炼出两个交点信息
                    bHasHole = true;
                    //这时候要看right_points的首尾，因为可能出现重合点
                    inset1 = { Pi_1, right_points.front() };
                    inset2 = { Pi, right_points.back() };
                    break;
                }
            }
            for (int i = 1; i < right_points.size() && !bHasHole; i++) {
                int Pi_1 = right_points[i - 1];
                int Pi = right_points[i];
                if (Pi - Pi_1 > 1) {
                    bHasHole = true;
                    inset1 = { left_points.front(), Pi_1};
                    inset2 = { left_points.back(), Pi};
                    break;
                }
            }
            if (!bHasHole) {
                //两个分割面的点序列都是顺序的，首尾相连
                int left_first = left_points.front(), left_last = left_points.back();
                int right_first = right_points.front(), right_last = right_points.back();

                if ((left_first == right_last + 1 || left_first == right_last) && 
                    left_last == N - 1 && right_first == 0) {
                    //右边的尾巴接在左边的开头
                    inset1 = { left_last, right_first };
                    inset2 = { left_first, right_last };
                }
                else if ((right_first == left_last + 1 || right_first == left_last) && 
                    right_last == N - 1 && left_first == 0) {
                    inset1 = { left_first, right_last };
                    inset2 = { left_last, right_first };
                }
            }

            assert(inset1.first != -1 && inset1.second != -1 && inset2.first != -1 && inset2.second != -1);

            //开始加点坐标，如果点旁边有交点，还得插值求交点
            for (int i = 0; i < left_points.size(); i++) {
                //当前点可能是交点边界
                int Pi = left_points[i];
                if (Pi == inset1.first || Pi == inset2.first) {
                    //有可能inset1和inset2也重合了。。。
                    int left = Pi, right = -1;
                    if (Pi == inset1.first) {
                        right = inset1.second;
                    }
                    else if (Pi == inset2.first) {
                        right = inset2.second;
                    }
                    assert(right >= 0);

                    if (left == right) {
                        //顶点和交点重合
                        left_face.push_back(facepts[left]);
                    }
                    else {
                        vec3f P_inset = getInsetByInter(facepts[left], facepts[right], axis_val, axis);
                        //先加交点还是先加点，根本取决于当前的点下一点，是否在本序列里
                        if ((left + 1) % N == left_points[(i + 1) % leftN]) {
                            //先加交点，再加点
                            left_face.push_back(P_inset);
                            left_face.push_back(facepts[left]);
                        }
                        else {
                            left_face.push_back(facepts[left]);
                            left_face.push_back(P_inset);
                        }
                    }
                }
                else if (Pi == inset1.second) {
                    //Pi是左边面的点，只有重合点才能出现在inset右边，但这种情况在前面已经考虑了
                    assert(false);
                }
                else {
                    left_face.push_back(facepts[Pi]);
                }
            }

            for (int i = 0; i < right_points.size(); i++) {
                //当前点可能是交点边界
                int Pi = right_points[i];
                if (Pi == inset1.second || Pi == inset2.second) {
                    //有可能inset1和inset2也重合了。。。
                    int left = -1, right = Pi;
                    if (Pi == inset1.second) {
                        left = inset1.first;
                    }
                    else if (Pi == inset2.second) {
                        left = inset2.first;
                    }
                    assert(left >= 0);

                    if (left == right) {
                        //顶点和交点重合
                        right_face.push_back(facepts[right]);
                    }
                    else {
                        vec3f P_inset = getInsetByInter(facepts[left], facepts[right], axis_val, axis);

                        if ((right + 1) % N == right_points[(i + 1) % rightN]) {
                            right_face.push_back(P_inset);
                            right_face.push_back(facepts[right]);
                        }
                        else {
                            right_face.push_back(facepts[right]);
                            right_face.push_back(P_inset);
                        }
                    }
                }
                else if (Pi == inset1.second) {
                    //Pi是左边面的点，只有重合点才能出现在inset右边，但这种情况在前面已经考虑了
                    assert(false);
                }
                else {
                    right_face.push_back(facepts[Pi]);
                }
            }
            return true;
        }

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

            std::vector<vec3f> pos = input_object->points_pos();
            std::vector<std::vector<zeno::vec3f>> exist_faces;

            int nFace = input_object->nfaces();
            for (int f = 0; f < nFace; f++)
            {
                std::vector<vec3f> currFace;    //当前面所有点的坐标
                std::vector<int> pts = input_object->face_points(f);
                for (auto pt : pts) {
                    currFace.push_back(pos[pt]);
                }
                exist_faces.push_back(currFace);

                //直接开始遍历x切割平面，从“左”往“右”
                for (int i = 0; i <= nx; i++) {
                    float xi = xmin + i * dx;

                    std::vector<std::vector<zeno::vec3f>> new_faces;
                    for (auto iter = exist_faces.begin(); iter != exist_faces.end();) {
                        std::vector<vec3f> leftFace, rightFace;
                        bool bSuccess = splitFace(*iter, Divide_X, xi, leftFace, rightFace);
                        if (bSuccess) {
                            //移除当前迭代器，并且把leftFace和rightFace加到最前面。
                            iter = exist_faces.erase(iter);
                            new_faces.push_back(leftFace);
                            new_faces.push_back(rightFace);
                        }
                        else {
                            //当前无法拆分了(可能只是面恰好没法被平面切割，也可能是平面最小化），保留
                            iter++;
                        }
                    }
                    if (!new_faces.empty()) {
                        exist_faces.insert(exist_faces.end(), new_faces.begin(), new_faces.end());
                    }
                }

                for (int i = 0; i <= ny; i++) {
                    float yi = ymin + i * dy;

                    std::vector<std::vector<zeno::vec3f>> new_faces;
                    for (auto iter = exist_faces.begin(); iter != exist_faces.end();) {
                        std::vector<vec3f> leftFace, rightFace;
                        bool bSuccess = splitFace(*iter, Divide_Y, yi, leftFace, rightFace);
                        if (bSuccess) {
                            //移除当前迭代器，并且把leftFace和rightFace加到最前面。
                            iter = exist_faces.erase(iter);
                            new_faces.push_back(leftFace);
                            new_faces.push_back(rightFace);
                        }
                        else {
                            //当前无法拆分了，保留
                            iter++;
                        }
                    }
                    if (!new_faces.empty()) {
                        exist_faces.insert(exist_faces.end(), new_faces.begin(), new_faces.end());
                    }
                }

                for (int i = 0; i <= nz; i++) {
                    float zi = zmin + i * dz;

                    std::vector<std::vector<zeno::vec3f>> new_faces;
                    for (auto iter = exist_faces.begin(); iter != exist_faces.end();) {
                        std::vector<vec3f> leftFace, rightFace;
                        bool bSuccess = splitFace(*iter, Divide_Z, zi, leftFace, rightFace);
                        if (bSuccess) {
                            //移除当前迭代器，并且把leftFace和rightFace加到最前面。
                            iter = exist_faces.erase(iter);
                            new_faces.push_back(leftFace);
                            new_faces.push_back(rightFace);
                        }
                        else {
                            //当前无法拆分了，保留
                            iter++;
                        }
                    }
                    if (!new_faces.empty()) {
                        exist_faces.insert(exist_faces.end(), new_faces.begin(), new_faces.end());
                    }
                }
            }

            assert(exist_faces.size() > 0);
            if (exist_faces.size() == 1) {
                return input_object;
            }
            else {
                auto spOutput = constructGeom(exist_faces);
                //auto spOutput = zeno::fuseGeometry(spOutput, 0.05);   //fuse有bug，导致出现了重复点
                return spOutput;
            }
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
            const auto& pos = input_object->points_pos();
            const int nface = input_object->nfaces();
            std::vector<std::vector<int>> newFaces;
            std::vector<vec3f> new_pos = pos;

            float A = direction[0], B = direction[1], C = direction[2], 
                D = -(A * center_pos[0] + B * center_pos[1] + C * center_pos[2]);

            int nPoints = pos.size();

            std::map<std::string, int> split_cache;

            for (int iFace = 0; iFace < nface; iFace++) {
                std::vector<int> face_indice = input_object->face_points(iFace);
                std::vector<vec3f> face_pos(face_indice.size());
                for (int i = 0; i < face_indice.size(); i++)
                {
                    face_pos[i] = pos[face_indice[i]];
                }
                std::vector<DivideFace> split_faces;
                std::map<int, DividePoint> split_infos;
                bool onlyAbove = false;
                bool bSuccess = dividePlane(face_pos, A, B, C, D, split_faces, split_infos, &onlyAbove);
                if (!bSuccess) {
                    if (Keep == "All") {
                        newFaces.push_back(face_indice);
                    }
                    else if (Keep == "Part Below The Plane" && !onlyAbove) {
                        //随便取一个点，观察是否在平面的下方
                        newFaces.push_back(face_indice);
                        //被抛弃的面，就不加进去，让以前的点成为孤立点，然后再判断所处平面位置从而删掉
                        //这样就不影响后续新面各个点的索引
                        //也许需要记录这些点号，方便统一删除
                    }
                    else if (Keep == "Part Above The Plane" && onlyAbove) {
                        newFaces.push_back(face_indice);
                    }
                    continue;
                }

                for (const DivideFace& divideFace : split_faces)
                {
                    //new_face_indice是“相对索引”，现在要转为基于全局points的索引。
                    std::vector<int> new_face_abs_indice;
                    for (int relidx : divideFace.face_indice) {
                        int final_idx = -1;
                        if (relidx < face_indice.size()) {
                            final_idx = face_indice[relidx];
                        }
                        else {
                            //新增的分割点，需要查表看是不是缓存了
                            DividePoint& split_info = split_infos[relidx];
                            int rel_p1 = split_info.from, rel_p2 = split_info.to;
                            int abs_p1 = face_indice[rel_p1], abs_p2 = face_indice[rel_p2];
                            std::string key = zeno::format("{}->{}", std::min(abs_p1, abs_p2), std::max(abs_p1, abs_p2));
                            auto iter = split_cache.find(key);
                            if (iter != split_cache.end()) {
                                final_idx = iter->second;
                            }
                            else {
                                final_idx = new_pos.size();
                                new_pos.push_back(split_info.pos);
                                split_cache.insert(std::make_pair(key, final_idx));
                            }
                        }
                        new_face_abs_indice.push_back(final_idx);
                    }
                    newFaces.emplace_back(std::move(new_face_abs_indice));
                }
            }

            auto spOutput = std::make_shared<GeometryObject>(false, new_pos.size(), newFaces.size());
            for (const std::vector<int>& face_indice : newFaces) {
                spOutput->add_face(face_indice, true);  //todo: 考虑线
            }
            spOutput->create_point_attr("pos", new_pos);
            //TODO: 剩下位于平面之下的孤立点都要被去掉
            return spOutput;
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
            std::vector<vec3f> vertex_normals;
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
                        vertex_normals.push_back(nrm);
                    }
                }
            }
            auto spOutput = constructGeom(newFaces);
            if (bPostComputeNormals)
                spOutput->create_vertex_attr("nrm", vertex_normals);
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
            //Too difficult to hold the index.

            int npts = input_object->npoints();
            int nfaces = input_object->nfaces();
            const auto& pos = input_object->points_pos();

            if (pointSort == "By Vertex Order") {
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
