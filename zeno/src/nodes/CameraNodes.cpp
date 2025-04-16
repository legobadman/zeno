#include <zeno/zeno.h>
#include <zeno/types/CameraObject.h>
#include <zeno/types/LightObject.h>
#include <zeno/types/UserData.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/utils/eulerangle.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "zeno/extra/TempNode.h"
#include <regex>

namespace zeno {
struct CameraNode: zeno::INode{
    virtual void apply() override {
        auto camera = std::make_shared<zeno::CameraObject>();

        camera->pos = ZImpl(get_input2<zeno::vec3f>("pos"));
        camera->up = ZImpl(get_input2<zeno::vec3f>("up"));
        camera->view = ZImpl(get_input2<zeno::vec3f>("view"));
        camera->fov = ZImpl(get_input2<float>("fov"));
        camera->aperture = ZImpl(get_input2<float>("aperture"));
        camera->focalPlaneDistance = ZImpl(get_input2<float>("focalPlaneDistance"));
        camera->userData()->set_float("frame", ZImpl(get_input2<float>("frame")));

        auto other_props = ZImpl(get_input2<std::string>("other"));
        std::regex reg(",");
        std::sregex_token_iterator p(other_props.begin(), other_props.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<float> prop_vals;
        while (p != end) {
            prop_vals.push_back(std::stof(*p));
            p++;
        }
        if (prop_vals.size() == 6) {
            camera->pivot = {prop_vals[0], prop_vals[1], prop_vals[2]};
        }

        ZImpl(set_output("camera", std::move(camera)));
    }
};

ZENO_DEFNODE(CameraNode)({
     {
         {gParamType_Vec3f, "pos", "0,0,5"},
         {gParamType_Vec3f, "up", "0,1,0"},
         {gParamType_Vec3f, "view", "0,0,-1"},
         {gParamType_Float, "fov", "45"},
         {gParamType_Float, "aperture", "11"},
         {gParamType_Float, "focalPlaneDistance", "2.0"},
         {gParamType_String, "other", ""},
         {gParamType_Int, "frame", "0"},
     },
     {
         {gParamType_Camera, "camera"},
     },
     {
     },
     {"FBX"},
 });

struct MakeCamera : INode {
    virtual void apply() override {
        auto camera = std::make_shared<CameraObject>();

        camera->pos = ZImpl(get_input2<zeno::vec3f>("pos"));
        camera->up = ZImpl(get_input2<zeno::vec3f>("up"));
        camera->view = ZImpl(get_input2<zeno::vec3f>("view"));
        camera->ffar = ZImpl(get_input2<float>("far"));
        camera->fnear = ZImpl(get_input2<float>("near"));
        camera->fov = ZImpl(get_input2<float>("fov"));
        camera->aperture = ZImpl(get_input2<float>("aperture"));
        camera->focalPlaneDistance = ZImpl(get_input2<float>("focalPlaneDistance"));

        ZImpl(set_output("camera", std::move(camera)));
    }
};

ZENO_DEFNODE(MakeCamera)({
    {
        {gParamType_Vec3f, "pos", "0,0,5"},
        {gParamType_Vec3f, "up", "0,1,0"},
        {gParamType_Vec3f, "view", "0,0,-1"},
        {gParamType_Float, "near", "0.01"},
        {gParamType_Float, "far", "20000"},
        {gParamType_Float, "fov", "45"},
        {gParamType_Float, "aperture", "11"},
        {gParamType_Float, "focalPlaneDistance", "2.0"},
    },
    {
        {gParamType_Camera, "camera"},
    },
    {
    },
    {"shader"},
});

struct SetPhysicalCamera : INode {
    virtual void apply() override {
        auto camera = ZImpl(get_input("camera"));
        auto ud = camera->userData();
        ud->set_float("aperture", ZImpl(get_input2<float>("aperture")));
        ud->set_float("shutter_speed", ZImpl(get_input2<float>("shutter_speed")));
        ud->set_float("iso", ZImpl(get_input2<float>("iso")));
        ud->set_bool("aces", ZImpl(get_input2<bool>("aces")));
        ud->set_bool("exposure", ZImpl(get_input2<bool>("exposure")));

        ZImpl(set_output("camera", std::move(camera)));
    }
};

ZENO_DEFNODE(SetPhysicalCamera)({
    {
        {gParamType_Camera, "camera", "", zeno::Socket_ReadOnly},
        {gParamType_Float, "aperture", "2"},
        {gParamType_Float, "shutter_speed", "0.04"},
        {gParamType_Float, "iso", "150"},
        {gParamType_Bool, "aces", "0"},
        {gParamType_Bool, "exposure", "0"},
    },
    {
        {"CameraObject", "camera"},
    },
    {
    },
    {"shader"},
});

struct TargetCamera : INode {
    virtual void apply() override {
        auto camera = std::make_shared<CameraObject>();

        auto refUp = zeno::normalize(ZImpl(get_input2<zeno::vec3f>("refUp")));
        auto pos = ZImpl(get_input2<zeno::vec3f>("pos"));
        auto target = ZImpl(get_input2<zeno::vec3f>("target");
        auto AF = ZImpl(get_input2<bool>("AutoFocus")));
        vec3f view = zeno::normalize(target - pos);
        vec3f right = zeno::cross(view, refUp);
        vec3f up = zeno::cross(right, view);

        camera->pos = pos;
        camera->up = up;
        camera->view = view;
        camera->ffar = ZImpl(get_input2<float>("far"));
        camera->fnear = ZImpl(get_input2<float>("near"));
        camera->fov = ZImpl(get_input2<float>("fov"));
        camera->aperture = ZImpl(get_input2<float>("aperture"));
        if(AF){
            camera->focalPlaneDistance = zeno::length(target-pos);
        }else{
            camera->focalPlaneDistance = ZImpl(get_input2<float>("focalPlaneDistance"));
        }

        ZImpl(set_output("camera", std::move(camera)));
    }
};

ZENO_DEFNODE(TargetCamera)({
    {
        {gParamType_Vec3f, "pos", "0,0,5"},
        {gParamType_Vec3f, "refUp", "0,1,0"},
        {gParamType_Vec3f, "target", "0,0,0"},
        {gParamType_Float, "near", "0.01"},
        {gParamType_Float, "far", "20000"},
        {gParamType_Float, "fov", "45"},
        {gParamType_Float, "aperture", "11"},
        {gParamType_Bool,"AutoFocus","false"},
        {gParamType_Float, "focalPlaneDistance", "2.0"},
    },
    {
        {"CameraObject", "camera"},
    },
    {
    },
    {"shader"},
});

struct MakeLight : INode {
    virtual void apply() override {
        auto light = std::make_shared<LightObject>();
        light->lightDir = normalize(ZImpl(get_input2<zeno::vec3f>("lightDir")));
        light->intensity = ZImpl(get_input2<float>("intensity"));
        light->shadowTint = ZImpl(get_input2<zeno::vec3f>("shadowTint"));
        light->lightHight = ZImpl(get_input2<float>("lightHight"));
        light->shadowSoftness = ZImpl(get_input2<float>("shadowSoftness"));
        light->lightColor = ZImpl(get_input2<zeno::vec3f>("lightColor"));
        light->lightScale = ZImpl(get_input2<float>("lightScale"));
        light->isEnabled = ZImpl(get_input2<bool>("isEnabled"));
        ZImpl(set_output("light", std::move(light)));
    }
};

ZENO_DEFNODE(MakeLight)({
    {
        {gParamType_Vec3f, "lightDir", "1,1,0"},
        {gParamType_Float, "intensity", "10"},
        {gParamType_Vec3f, "shadowTint", "0.2,0.2,0.2"},
        {gParamType_Float, "lightHight", "1000.0"},
        {gParamType_Float, "shadowSoftness", "1.0"},
        {gParamType_Vec3f, "lightColor", "1,1,1"},
        {gParamType_Float, "lightScale", "1"},
        {gParamType_Bool, "isEnabled", "1"},
    },
    {
        {gParamType_Light, "light"},
    },
    {
    },
    {"shader"},
});

struct ScreenSpaceProjectedGrid : INode {
    float hitOnFloor(vec3f pos, vec3f dir, float sea_level) const {
        float t = (sea_level - pos[1]) / dir[1];
        return t;
    }
    virtual void apply() override {
        auto cam = ZImpl(get_input2<CameraObject>("cam"));
        auto prim = std::make_shared<PrimitiveObject>();
        auto raw_width = ZImpl(get_input2<int>("width"));
        auto raw_height = ZImpl(get_input2<int>("height"));
        auto u_padding = ZImpl(get_input2<int>("u_padding"));
        auto v_padding = ZImpl(get_input2<int>("v_padding"));
        auto sea_level = ZImpl(get_input2<float>("sea_level"));
        auto fov = glm::radians(cam->fov);
        auto pos = cam->pos;
        auto up = cam->up;
        auto view = cam->view;
        auto infinite = cam->ffar;

        auto width = raw_width + u_padding * 2;
        auto height = raw_height + v_padding * 2;

        auto right = zeno::cross(view, up);
        float ratio = float(raw_width) / float(raw_height);
        float right_scale = std::tan(fov / 2) * ratio * float(width - 1) / float(raw_width - 1);
        float up_scale = std::tan(fov / 2) * float(height - 1) / float(raw_height - 1);
        prim->verts.resize(width * height);
        for (auto j = 0; j <= height - 1; j++) {
            float v = float(j) / float(height - 1) * 2.0f - 1.0f;
            for (auto i = 0; i <= width - 1; i++) {
                float u = float(i) / float(width - 1) * 2.0f - 1.0f;
                auto dir = view + u * right * right_scale + v * up * up_scale;
                auto ndir = zeno::normalize(dir);
                auto t = hitOnFloor(pos, ndir, sea_level);
                if (t > 0 && t * zeno::dot(ndir, dir) < infinite) {
                    prim->verts[j * width + i] = pos + ndir * t;
                }
                else {
                    prim->verts[j * width + i] = pos + dir * infinite;
                }
            }
        }
        std::vector<vec3i> tris;
        tris.reserve((width - 1) * (height - 1) * 2);
        for (auto j = 0; j < height - 1; j++) {
            for (auto i = 0; i < width - 1; i++) {
                auto _0 = j * width + i;
                auto _1 = j * width + i + 1;
                auto _2 = j * width + i + 1 + width;
                auto _3 = j * width + i + width;
                tris.emplace_back(_0, _1, _2);
                tris.emplace_back(_0, _2, _3);
            }
        }
        prim->tris.values = tris;

        auto outs = zeno::TempNodeSimpleCaller("PrimitiveClip")
                .set("prim", std::move(prim))
                .set2<vec3f>("origin", pos)
                .set2<vec3f>("direction", view)
                .set2<float>("distance", infinite * 0.999)
                .set2<bool>("reverse:", false)
                .call();

        // Create nodes
        auto new_prim = std::dynamic_pointer_cast<PrimitiveObject>(outs.get("outPrim"));
        for (auto i = 0; i < new_prim->verts.size(); i++) {
            new_prim->verts[i][1] = sea_level;
        }
        ZImpl(set_output("prim", std::move(new_prim)));
    }
};

ZENO_DEFNODE(ScreenSpaceProjectedGrid)({
     {
         {gParamType_Camera, "cam"},
         {gParamType_Int, "width", "1920"},
         {gParamType_Int, "height", "1080"},
         {gParamType_Int, "u_padding", "0"},
         {gParamType_Int, "v_padding", "0"},
         {gParamType_Float, "sea_level", "0"},
     },
     {
{gParamType_Primitive, "prim"},
},
     {
     },
     {"shader"},
 });


struct CameraFrustum : INode {
    virtual void apply() override {
        auto cam = ZImpl(get_input2<CameraObject>("cam"));
        auto width = ZImpl(get_input2<int>("width"));
        auto height = ZImpl(get_input2<int>("height"));
        auto fov = glm::radians(cam->fov);
        auto pos = cam->pos;
        auto up = cam->up;
        auto view = cam->view;
        auto fnear = cam->fnear;
        auto ffar = cam->ffar;
        auto right = zeno::cross(view, up);
        float ratio = float(width) / float(height);
        auto prim = std::make_shared<PrimitiveObject>();
        prim->verts.resize(8);
        vec3f _near_left_up = pos + fnear * (view - right * std::tan(fov / 2) * ratio + up * std::tan(fov / 2));
        vec3f _near_left_down = pos + fnear * (view - right * std::tan(fov / 2) * ratio - up * std::tan(fov / 2));
        vec3f _near_right_up = pos + fnear * (view + right * std::tan(fov / 2) * ratio + up * std::tan(fov / 2));
        vec3f _near_right_down = pos + fnear * (view + right * std::tan(fov / 2) * ratio - up * std::tan(fov / 2));
        vec3f _far_left_up = pos + ffar * (view - right * std::tan(fov / 2) * ratio + up * std::tan(fov / 2));
        vec3f _far_left_down = pos + ffar * (view - right * std::tan(fov / 2) * ratio - up * std::tan(fov / 2));
        vec3f _far_right_up = pos + ffar * (view + right * std::tan(fov / 2) * ratio + up * std::tan(fov / 2));
        vec3f _far_right_down = pos + ffar * (view + right * std::tan(fov / 2) * ratio - up * std::tan(fov / 2));
        prim->verts[0] = _near_left_up;
        prim->verts[1] = _near_left_down;
        prim->verts[2] = _near_right_up;
        prim->verts[3] = _near_right_down;
        prim->verts[4] = _far_left_up;
        prim->verts[5] = _far_left_down;
        prim->verts[6] = _far_right_up;
        prim->verts[7] = _far_right_down;

        prim->lines.resize(12);
        prim->lines[0] = {0, 1};
        prim->lines[1] = {2, 3};
        prim->lines[2] = {0, 2};
        prim->lines[3] = {1, 3};
        prim->lines[0 + 4] = vec2i(0, 1) + 4;
        prim->lines[1 + 4] = vec2i(2, 3) + 4;
        prim->lines[2 + 4] = vec2i(0, 2) + 4;
        prim->lines[3 + 4] = vec2i(1, 3) + 4;
        prim->lines[0 + 8] = vec2i(0, 4);
        prim->lines[1 + 8] = vec2i(1, 5);
        prim->lines[2 + 8] = vec2i(2, 6);
        prim->lines[3 + 8] = vec2i(3, 7);

        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENO_DEFNODE(CameraFrustum)({
     {
         {gParamType_Camera, "cam"},
         {gParamType_Int, "width", "1920"},
         {gParamType_Int, "height", "1080"},
     },
     {
{gParamType_Primitive, "prim"},
},
     {
     },
     {"shader"},
 });

};
