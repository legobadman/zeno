//
// CameraEval - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/types/CameraObject.h>
#include <zeno/types/CurveObject.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <algorithm>

namespace zeno {

namespace {

CurveData createCurvePoint(std::vector<float>& t, std::vector<float>& y) {
    std::vector<zeno::vec2f> c1;
    std::vector<zeno::vec2f> c2;
    size_t N = t.size();
    c1.resize(N);
    c2.resize(N);
    c1[0] = zeno::vec2f(t[0] + (t[1] - t[0]) / 3.0f, y[0]);
    c2[0] = zeno::vec2f(t[0], y[0]);
    c1[N - 1] = zeno::vec2f(t[N - 1], y[N - 1]);
    c2[N - 1] = zeno::vec2f(t[N - 1] + (t[N - 2] - t[N - 1]) / 3.0f, y[N - 1]);
    for (size_t i = 1; i <= N - 2; i++) {
        float extrap = (t[i] - t[i - 1]) > 0 ? (t[i + 1] - t[i]) / (t[i] - t[i - 1]) : 0;
        zeno::vec2f R = zeno::vec2f(t[i], y[i]) + extrap * (zeno::vec2f(t[i], y[i]) - zeno::vec2f(t[i - 1], y[i - 1]));
        zeno::vec2f T = 0.5f * (R + zeno::vec2f(t[i + 1], y[i + 1]));
        c1[i] = (2.0f * zeno::vec2f(t[i], y[i]) + 1.0f * T) / 3.0f;
    }
    for (size_t i = 1; i <= N - 2; i++) {
        zeno::vec2f dir = zeno::vec2f(t[i], y[i]) - c1[i];
        float l = (t[i] - t[i - 1]) / 3.0f;
        float amp = std::abs(dir[0]) > 0.0f ? l / std::abs(dir[0]) : 0.0f;
        c2[i] = zeno::vec2f(t[i], y[i]) + amp * dir;
    }
    CurveData dat;
    for (size_t i = 0; i < N; i++) {
        dat.addPoint(t[i], y[i], zeno::CurveData::PointType::kBezier,
                     c2[i] - zeno::vec2f(t[i], y[i]), c1[i] - zeno::vec2f(t[i], y[i]),
                     CurveData::HDL_ALIGNED);
    }
    return dat;
}

} // namespace

struct CameraEval : INode2 {
    CurveData curve_x, curve_y, curve_z;
    CurveData curve_tx, curve_ty, curve_tz;
    CurveData curve_vx, curve_vy, curve_vz;
    CurveData curve_ux, curve_uy, curve_uz;
    CurveData curve_fov, curve_apertures, curve_fPD;

    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        int frameid = nd->has_input("frameid") ? (int)std::lround(nd->get_input2_float("frameid")) : nd->GetFrameId();

        IListObject* listObj = nd->get_input_ListObject("nodelist");
        if (!listObj) {
            nd->report_error("CameraEval: no nodelist input");
            return ZErr_ParamError;
        }

        std::vector<std::unique_ptr<CameraObject>> nodelist;
        for (size_t i = 0; i < listObj->size(); i++) {
            IObject2* obj = listObj->get(i);
            auto* cam = dynamic_cast<CameraObject*>(obj);
            if (cam)
                nodelist.push_back(safe_uniqueptr_cast<CameraObject>(zany2(cam->clone())));
        }

        std::sort(nodelist.begin(), nodelist.end(), [](const auto& a, const auto& b) {
            return a->userData()->get_float("frame") < b->userData()->get_float("frame");
        });

        int target_camera_count = 0;
        for (const auto& cam : nodelist) {
            target_camera_count += cam->userData()->get_int("is_target", 0);
        }

        if (nodelist.size() == 1) {
            nd->set_output_object("camera", nodelist[0].release());
            return ZErr_OK;
        }
        if (nodelist.empty()) {
            nd->report_error("CameraEval: empty nodelist");
            return ZErr_ParamError;
        }
        if (frameid <= std::lround(nodelist[0]->userData()->get_float("frame"))) {
            nd->set_output_object("camera", nodelist[0].release());
            return ZErr_OK;
        }
        if (frameid >= std::lround(nodelist.back()->userData()->get_float("frame"))) {
            nd->set_output_object("camera", nodelist.back().release());
            return ZErr_OK;
        }

        std::vector<float> ts(nodelist.size()), xs(nodelist.size()), ys(nodelist.size()), zs(nodelist.size());
        std::vector<float> xus(nodelist.size()), yus(nodelist.size()), zus(nodelist.size());
        std::vector<float> fovs(nodelist.size()), apertures(nodelist.size()), focalPlaneDistances(nodelist.size());
        std::vector<float> txs(nodelist.size()), tys(nodelist.size()), tzs(nodelist.size());

        if ((int)nodelist.size() == target_camera_count) {
            for (size_t i = 0; i < nodelist.size(); i++) {
                auto const& cur = nodelist[i];
                float f = cur->userData()->get_float("frame");
                auto target = cur->userData()->get_vec3f("target");
                ts[i] = f;
                xs[i] = cur->pos[0]; ys[i] = cur->pos[1]; zs[i] = cur->pos[2];
                txs[i] = target[0]; tys[i] = target[1]; tzs[i] = target[2];
                xus[i] = cur->up[0]; yus[i] = cur->up[1]; zus[i] = cur->up[2];
                fovs[i] = cur->fov;
                apertures[i] = cur->aperture;
                focalPlaneDistances[i] = cur->focalPlaneDistance;
            }
            curve_x = createCurvePoint(ts, xs);
            curve_y = createCurvePoint(ts, ys);
            curve_z = createCurvePoint(ts, zs);
            curve_tx = createCurvePoint(ts, txs);
            curve_ty = createCurvePoint(ts, tys);
            curve_tz = createCurvePoint(ts, tzs);
            curve_ux = createCurvePoint(ts, xus);
            curve_uy = createCurvePoint(ts, yus);
            curve_uz = createCurvePoint(ts, zus);
            curve_fov = createCurvePoint(ts, fovs);
            curve_apertures = createCurvePoint(ts, apertures);
            curve_fPD = createCurvePoint(ts, focalPlaneDistances);
        } else {
            std::vector<float> vxs(nodelist.size()), vys(nodelist.size()), vzs(nodelist.size());
            float totalLength = 0;
            for (size_t i = 1; i < nodelist.size(); i++) {
                totalLength += zeno::length(nodelist[i]->pos - nodelist[i - 1]->pos);
            }
            float h = totalLength > 0 ? 0.001f * totalLength : 1.0f;
            for (size_t i = 0; i < nodelist.size(); i++) {
                auto const& cur = nodelist[i];
                float f = cur->userData()->get_float("frame");
                ts[i] = f;
                xs[i] = cur->pos[0]; ys[i] = cur->pos[1]; zs[i] = cur->pos[2];
                vxs[i] = cur->view[0]; vys[i] = cur->view[1]; vzs[i] = cur->view[2];
                xus[i] = cur->pos[0] + h * cur->up[0];
                yus[i] = cur->pos[1] + h * cur->up[1];
                zus[i] = cur->pos[2] + h * cur->up[2];
                fovs[i] = cur->fov;
                apertures[i] = cur->aperture;
                focalPlaneDistances[i] = cur->focalPlaneDistance;
            }
            curve_x = createCurvePoint(ts, xs);
            curve_y = createCurvePoint(ts, ys);
            curve_z = createCurvePoint(ts, zs);
            curve_vx = createCurvePoint(ts, vxs);
            curve_vy = createCurvePoint(ts, vys);
            curve_vz = createCurvePoint(ts, vzs);
            curve_ux = createCurvePoint(ts, xus);
            curve_uy = createCurvePoint(ts, yus);
            curve_uz = createCurvePoint(ts, zus);
            curve_fov = createCurvePoint(ts, fovs);
            curve_apertures = createCurvePoint(ts, apertures);
            curve_fPD = createCurvePoint(ts, focalPlaneDistances);
        }

        auto camera = std::make_unique<CameraObject>();
        camera->pos[0] = curve_x.eval(frameid);
        camera->pos[1] = curve_y.eval(frameid);
        camera->pos[2] = curve_z.eval(frameid);
        camera->fov = curve_fov.eval(frameid);
        camera->aperture = curve_apertures.eval(frameid);

        if ((int)nodelist.size() == target_camera_count) {
            zeno::vec3f refUp(curve_ux.eval(frameid), curve_uy.eval(frameid), curve_uz.eval(frameid));
            refUp = zeno::normalize(refUp);
            zeno::vec3f tarPos(curve_tx.eval(frameid), curve_ty.eval(frameid), curve_tz.eval(frameid));
            camera->view = zeno::normalize(tarPos - camera->pos);
            auto cur_right = zeno::normalize(zeno::cross(camera->view, refUp));
            camera->up = zeno::normalize(zeno::cross(cur_right, camera->view));
            int af = nodelist[0]->userData()->get_int("AutoFocus", 1);
            if (af)
                camera->focalPlaneDistance = zeno::distance(camera->pos, tarPos);
        } else {
            zeno::vec3f refUp(curve_ux.eval(frameid), curve_uy.eval(frameid), curve_uz.eval(frameid));
            refUp = refUp - camera->pos;
            refUp = zeno::normalize(refUp);
            camera->view = zeno::vec3f(curve_vx.eval(frameid), curve_vy.eval(frameid), curve_vz.eval(frameid));
            camera->view = zeno::normalize(camera->view);
            auto cur_right = zeno::normalize(zeno::cross(camera->view, refUp));
            camera->up = zeno::normalize(zeno::cross(cur_right, camera->view));
            camera->focalPlaneDistance = curve_fPD.eval(frameid);
        }

        nd->set_output_object("camera", camera.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(CameraEval, {
    {{gParamType_Int, "frameid"},
     {gParamType_List, "nodelist"}},
    {{gParamType_Camera, "camera"}},
    {},
    {"FBX"},
});

} // namespace zeno
