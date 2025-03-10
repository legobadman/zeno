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
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::shared_ptr<zeno::GeometryObject> target_Obj
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
            std::vector<std::vector<int>> inputFacesPoints(input_object->nfaces(), std::vector<int>());
            for (int i = 0; i < input_object->nfaces(); ++i) {
                inputFacesPoints[i] = input_object->face_points(i);
            }


            std::vector<zeno::vec3f> inputNrm;
            std::vector<zeno::vec2f> inputLines;
            bool hasNrm = input_object->has_point_attr("nrm");
            bool isLine = input_object->is_Line();
            if (hasNrm) {
                inputNrm = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "nrm");
            }
            if (isLine) {
                inputLines.resize(input_object->npoints());
                for (size_t i = 0; i < input_object->npoints(); ++i) {
                    inputLines[i][0] = i;
                    inputLines[i][1] = input_object->getLineNextPt(i);
                }
            }

            zeno::vec3f originCenter, _min, _max;
            std::tie(_min, _max) = geomBoundingBox(input_object.get());
            originCenter = (_min + _max) / 2;

            size_t targetObjPointsCount = target_Obj->npoints();
            size_t inputObjPointsCount = input_object->npoints();
            size_t inputObjFacesCount = input_object->nfaces();
            size_t newObjPointsCount = targetObjPointsCount * inputObjPointsCount;

            std::vector<zeno::vec3f> newObjPos(newObjPointsCount, zeno::vec3f());
            std::vector<zeno::vec3f> newObjNrm;
            if (hasNrm) {
                newObjNrm.resize(newObjPointsCount);
            }

            std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(input_object->is_base_triangle(), newObjPointsCount, 0);
            for (size_t i = 0; i < targetObjPointsCount; ++i) {
                zeno::vec3f targetPos = target_Obj->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, i);
                zeno::vec3f dx = targetPos - originCenter;
                glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(dx[0], dx[1], dx[2]));

                size_t offset = i * inputObjPointsCount;
                for (size_t j = 0; j < inputObjPointsCount; j++)
                {
                    auto idx = offset + j;

                    auto& pt = inputPos[j];
                    glm::vec4 gp = translate * glm::vec4(pt[0], pt[1], pt[2], 1);
                    newObjPos[idx] = zeno::vec3f(gp.x, gp.y, gp.z);

                    if (hasNrm) {
                        newObjNrm[idx] = inputNrm[j];
                    }

                    spgeo->initpoint(idx);

                    if (isLine) {
                        spgeo->initLineNextPoint(idx);
                        if (j == inputObjPointsCount - 1) {
                            spgeo->setLineNextPt(idx, idx);
                        }
                    }
                }
                for (size_t j = 0; j < inputObjFacesCount; ++j)
                {
                    std::vector<int> facePoints = inputFacesPoints[j];
                    for (int k = 0; k < facePoints.size(); ++k) {
                        facePoints[k] += offset;
                    }
                    spgeo->add_face(facePoints);
                }
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
            std::ostringstream ss;
            ss << "if (" << (deleteNonSelected ? "!" : "") <<
                "(" << zfx << ")) { " << (group=="Points" ? "remove_point" : "remove_face")
                << "(@ptnum); }";

            std::shared_ptr<zeno::GeometryObject> spOutput(input_object);

            const std::string& finalZfx = ss.str();
            zeno::ZfxContext ctx;
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
        }
    };
}
