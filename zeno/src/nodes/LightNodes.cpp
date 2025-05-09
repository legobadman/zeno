#include <zeno/zeno.h>
#include <zeno/extra/TempNode.h>
#include <zeno/utils/eulerangle.h>

#include <zeno/types/UserData.h>
#include <zeno/types/LightObject.h>
#include <zeno/types/PrimitiveObject.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#ifndef M_PIf
#define M_PIf (float)M_PI
#endif

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtc/matrix_transform.hpp"

namespace zeno {

struct LightNode : INode {
    virtual void apply() override {
        auto isL = true; //ZImpl(get_input2<int>("islight");
        auto invertdir = ZImpl(get_input2<int>("invertdir"));
        
        auto scale = ZImpl(get_input2<zeno::vec3f>("scale"));
        auto rotate = ZImpl(get_input2<zeno::vec3f>("rotate"));
        auto position = ZImpl(get_input2<zeno::vec3f>("position"));
        auto quaternion = ZImpl(get_input2<zeno::vec4f>("quaternion"));

        auto color = ZImpl(get_input2<zeno::vec3f>("color"));
        auto exposure = ZImpl(get_input2<float>("exposure"));
        auto intensity = ZImpl(get_input2<float>("intensity"));

        auto scaler = powf(2.0f, exposure);

        if (std::isnan(scaler) || std::isinf(scaler) || scaler < 0.0f) {
            scaler = 1.0f;
            printf("Light exposure = %f is invalid, fallback to 0.0 \n", exposure);
        }

        intensity *= scaler;

        auto ccc = color * intensity;
        for (size_t i=0; i<ccc.size(); ++i) {
            if (std::isnan(ccc[i]) || std::isinf(ccc[i]) || ccc[i] < 0.0f) {
                ccc[i] = 1.0f;
                printf("Light color component %lu is invalid, fallback to 1.0 \n", i);
            }
        }

        auto mask = ZImpl(get_input2<int>("mask"));
        auto spread = ZImpl(get_input2<zeno::vec2f>("spread"));
        auto visible = ZImpl(get_input2<int>("visible"));
        auto doubleside = ZImpl(get_input2<int>("doubleside"));
        
        if (doubleside) { invertdir = false; }

        std::string type = ZImpl(get_input2<std::string>(lightTypeKey));
        auto typeEnum = magic_enum::enum_cast<LightType>(type).value_or(LightType::Diffuse);
        auto typeOrder = magic_enum::enum_integer(typeEnum);

        std::string shapeString = ZImpl(get_input2<std::string>(lightShapeKey));
        auto shapeEnum = magic_enum::enum_cast<LightShape>(shapeString).value_or(LightShape::Plane);
        auto shapeOrder = magic_enum::enum_integer(shapeEnum);

        auto prim = std::make_shared<zeno::PrimitiveObject>();
        auto &VERTS = prim->verts;
        auto &LINES = prim->lines;
        auto &TRIS = prim->tris;

        if (ZImpl(has_input("prim"))) {
            auto mesh = ZImpl(get_input<PrimitiveObject>("prim"));

            if (mesh->tris->size() > 0) {
                prim = mesh;
                shapeEnum = LightShape::TriangleMesh;
                shapeOrder = magic_enum::enum_integer(shapeEnum);
            }
        } else {

            auto order = ZImpl(get_input2<std::string>("EulerRotationOrder"));
            auto orderTyped = magic_enum::enum_cast<EulerAngle::RotationOrder>(order).value_or(EulerAngle::RotationOrder::YXZ);

            auto measure = ZImpl(get_input2<std::string>("EulerAngleMeasure"));
            auto measureTyped = magic_enum::enum_cast<EulerAngle::Measure>(measure).value_or(EulerAngle::Measure::Radians);

            glm::vec3 eularAngleXYZ = glm::vec3(rotate[0], rotate[1], rotate[2]);
            glm::mat4 rotation = EulerAngle::rotate(orderTyped, measureTyped, eularAngleXYZ);

            if (shapeEnum == LightShape::Point) {
                scale = {0 ,scale[1], 0};

                if (typeEnum == LightType::Diffuse) {
                    spread = {1, 1};
                }
            }

            const auto transformWithoutScale = [&]() { 
                glm::quat rawQuat(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
                glm::mat4 matQuat  = glm::toMat4(rawQuat);

                glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], position[2]));
                transform = transform * rotation * matQuat;
                return transform;
            } ();

            VERTS->push_back(zeno::vec3f(+0.5, 0, +0.5));
            VERTS->push_back(zeno::vec3f(+0.5, 0, -0.5));
            VERTS->push_back(zeno::vec3f(-0.5, 0, +0.5));
            VERTS->push_back(zeno::vec3f(-0.5, 0, -0.5));

            auto pscale = std::max(scale[0], scale[2]);
            pscale = std::max(pscale, scale[1]);

            if (shapeEnum == LightShape::Sphere) {

                auto tmpPrim = zeno::TempNodeSimpleCaller("CreateSphere")
                        .set2<zeno::vec3f>("position", {0,0,0})
                        .set2<zeno::vec3f>("scaleSize", {1,1,1})
                        .set2<float>("radius", 0.5f)
                        .set2<zeno::vec3f>("rotate", {0,0,0})
                        .set2<bool>("hasNormal", false)
                        .set2<bool>("hasVertUV", false)
                        .set2<bool>("isFlipFace", false)
                        .set2<int>("rows", 180)
                        .set2<int>("columns", 360)
                        .set2<bool>("quads", false)
                        .set2<bool>("SphereRT", false)
                        .set2<std::string>("EulerRotationOrder:", "XYZ")
                        .set2<std::string>("EulerAngleMeasure:", "Degree")
                        .call().get<zeno::PrimitiveObject>("prim");

                        VERTS->reserve(tmpPrim->verts->size());
                        TRIS.reserve(tmpPrim->tris->size());

                VERTS->insert(VERTS.end(), tmpPrim->verts->begin(), tmpPrim->verts->end());
                for (size_t i=0; i<tmpPrim->tris.size(); ++i) {
                    auto tri = tmpPrim->tris[i];
                    TRIS.push_back(tri+4);
                }

                scale = zeno::vec3f(min(scale[0], scale[2]));
                pscale = 0.0;
            } 
            else if (shapeEnum == LightShape::Ellipse) {

                auto tmpPrim = zeno::TempNodeSimpleCaller("CreateDisk")
                        .set2<zeno::vec3f>("position", {0,0,0})
                        .set2<zeno::vec3f>("scaleSize", {1,1,1})
                        .set2<zeno::vec3f>("rotate", {0,0,0})
                        .set2<float>("radius", 0.5f)
                        .set2<float>("divisions", 360)
                        .set2<bool>("hasNormal", false)
                        .set2<bool>("hasVertUV", false)
                        .set2<bool>("isFlipFace", false)
                        .call().get<zeno::PrimitiveObject>("prim");

                        VERTS->reserve(tmpPrim->verts->size());
                        TRIS.reserve(tmpPrim->tris->size());

                VERTS->insert(VERTS.end(), tmpPrim->verts->begin(), tmpPrim->verts->end());
                for (size_t i=0; i<tmpPrim->tris.size(); ++i) {
                    auto tri = tmpPrim->tris[i];
                    TRIS.push_back(tri+4);
                }
            }

            if (shapeEnum != LightShape::Point) {
                // Plane Indices
                if (TRIS->size() == 0) {
                    TRIS.emplace_back(zeno::vec3i(0, 3, 1));
                    TRIS.emplace_back(zeno::vec3i(3, 0, 2));
                }

                for (auto& v : VERTS) {
                    v = scale * v;
                }
            }

            auto line_spread = spread;
            if (typeEnum != LightType::Projector) {
                line_spread = {spread[0], spread[0]};
            } 

            int lut[] = {+1, +3, -1, -3};

            int vertex_offset = VERTS->size();            
            for (size_t i=0; i<4; ++i) {

                auto info = lut[i];

                auto axis = glm::vec3(0, 0, 0);
                auto pick = 2-(abs(info)-1);
                axis[pick] = std::copysign(1, info);

                if (pick == 0) { // inverse axis
                    axis[pick] *= -1;
                }

                glm::mat4 sub_rotate = glm::rotate(glm::mat4(1.0), line_spread[i%2] * M_PIf/2.0f, axis);
                auto end_point = sub_rotate * glm::vec4(0, -0.3, 0, 1.0f);

                glm::vec4 p0 = glm::vec4(0,0,0,1);
                glm::vec4 p1 = glm::vec4(pscale, pscale, pscale, 1.0f) * (end_point);

                auto delta = glm::vec4(0.0);
                delta[abs(info)-1] = 0.5f * scale[abs(info)-1];

                if ( info < 0 ) { // negative
                    delta = -delta;
                }

                p0 += delta;
                p1 += delta;

                if (line_spread[i%2] < line_spread[(i+1)%2]) { // spread at the same surface
                    p1 *= cos( line_spread[(i+1)%2] * M_PIf/2.0f ) / cos( line_spread[(i)%2] * M_PIf/2.0f );
                }

                VERTS->push_back(zeno::vec3f(p0[0], p0[1], p0[2]));
                VERTS->push_back(zeno::vec3f(p1[0], p1[1], p1[2]));

                LINES->push_back({vertex_offset, vertex_offset+1});
                vertex_offset +=2;
            }

            if (shapeEnum == LightShape::Point) {

                int anchor_offset = VERTS->size();
                VERTS->push_back({0,0,0});

                if (typeEnum != LightType::Projector){

                    glm::mat4 sub_trans = glm::rotate(glm::mat4(1.0), M_PIf/4, glm::vec3(0,1,0));

                    for (size_t i=4; i<=(anchor_offset-1); ++i) {
                        auto& v = VERTS.at(i);
                        auto p = sub_trans * glm::vec4(v[0], v[1], v[2], 1);

                        VERTS->push_back( {p.x, p.y, p.z} );
                        LINES->push_back({anchor_offset, (int)VERTS.size()-1});
                    }
                } else {
                    auto vertical_distance = VERTS[anchor_offset-1][1];
                    float x_max=-FLT_MAX, x_min=FLT_MAX;
                    float z_max=-FLT_MAX, z_min=FLT_MAX;

                    for (int i=0; i<4; ++i) {
                        auto idx = anchor_offset - 1 - i * 2;
                        auto& tmp = VERTS[idx];

                        x_max = max(tmp[0], x_max);
                        x_min = min(tmp[0], x_min);    
                        z_max = max(tmp[2], z_max);
                        z_min = min(tmp[2], z_min);
                    }

                    VERTS->push_back({ x_max, vertical_distance, z_max} );
                    VERTS->push_back({ x_max, vertical_distance, z_min} );
                    VERTS->push_back({ x_min, vertical_distance, z_min} );
                    VERTS->push_back({ x_min, vertical_distance, z_max} );

                    LINES->push_back({anchor_offset+1, anchor_offset+2});
                    LINES->push_back({anchor_offset+2, anchor_offset+3});
                    LINES->push_back({anchor_offset+3, anchor_offset+4});
                    LINES->push_back({anchor_offset+4, anchor_offset+1});
                }

                if (typeEnum == LightType::Diffuse) {
                
                    int vertex_offset = VERTS->size();
                    
                    for (auto i : {-1, 0, 1}) {
                        for (auto j : {-1, 0, 1}) {

                            auto sub_trans = glm::rotate(glm::mat4(1.0), M_PIf/4, glm::vec3(i,0,j));
                            if (i == 0 && j == 0) { sub_trans = glm::mat4(1.0); }

                            sub_trans = glm::scale(sub_trans, {0, scale[1], 0});

                            auto p1 = sub_trans * glm::vec4(0, +.3, 0, 1);
                            auto p2 = sub_trans * glm::vec4(0, -.3, 0, 1);  

                            VERTS->push_back(zeno::vec3f(p1[0], p1[1], p1[2]));
                            VERTS->push_back(zeno::vec3f(p2[0], p2[1], p2[2]));

                            LINES->push_back({anchor_offset, vertex_offset+0});
                            LINES->push_back({anchor_offset, vertex_offset+1});

                            vertex_offset += 2;
                        } // j
                    } // i 
                }   
            }

            if ( (shapeEnum != LightShape::Sphere) && (invertdir || doubleside) ) {

                auto sub_trans = glm::rotate(glm::mat4(1.0), M_PIf, glm::vec3(1,0,0));
                auto vertices_offset = VERTS.size();

                if (doubleside) {
                    
                    VERTS.reserve(VERTS->size()*2);
                    LINES.reserve(LINES->size()*2);

                    std::remove_reference<decltype(LINES)>::type tmp(LINES->size());

                    std::transform(LINES.begin(), LINES.end(), tmp->begin(), 
                    [&](auto ele){ return ele + vertices_offset; });

                    LINES->insert(LINES.end(), tmp->begin(), tmp->end());
                }

                for (size_t i=0; i<vertices_offset; ++i) {
                    auto& v = VERTS.at(i);
                    auto p = sub_trans * glm::vec4(v[0], v[1], v[2], 1.0f);
                    if (invertdir) {
                        v = zeno::vec3f(p[0], p[1], p[2]);
                    }
                    if (doubleside) {
                        VERTS->push_back(zeno::vec3f(p[0], p[1], p[2]));
                    }
                }
            }

            auto &clr = VERTS.add_attr<zeno::vec3f>("clr");
            for (size_t i=0; i<VERTS.size(); ++i) {
                auto& v = VERTS.at(i);
                auto p = transformWithoutScale * glm::vec4(v[0], v[1], v[2], 1.0f);
                v = zeno::vec3f(p[0], p[1], p[2]);
                clr[i] = ccc;
            } 
        }

        auto ud = prim->userData();

        ud->set_bool("isRealTimeObject", std::move(isL));

        ud->set_bool("isL", isL);
        ud->set_int("ivD", invertdir);
        ud->set_vec3f("pos", toAbiVec3f(position));
        ud->set_vec3f("scale", toAbiVec3f(scale));
        ud->set_vec3f("rotate", toAbiVec3f(rotate));
        ud->set_vec4f("quaternion", toAbiVec4f(quaternion));
        ud->set_vec3f("color", toAbiVec3f(color));
        ud->set_float("intensity", intensity);

        float fluxFixed = ZImpl(get_input2<float>("fluxFixed"));
        ud->set_float("fluxFixed", fluxFixed);
        auto maxDistance = ZImpl(get_input2<float>("maxDistance"));
        ud->set_float("maxDistance", std::move(maxDistance));
        auto falloffExponent = ZImpl(get_input2<float>("falloffExponent"));
        ud->set_float("falloffExponent", std::move(falloffExponent));

        if (ZImpl(has_input2<std::string>("profile"))) {
            auto profile = ZImpl(get_input2<std::string>("profile"));
            ud->set_string("lightProfile", stdString2zs(profile));
        }
        if (ZImpl(has_input2<std::string>("texturePath"))) {
            auto texture = ZImpl(get_input2<std::string>("texturePath"));
            ud->set_string("lightTexture", stdString2zs(texture));

            auto gamma = ZImpl(get_input2<float>("textureGamma"));
            ud->set_float("lightGamma", gamma);
        }

        ud->set_int("type", typeOrder);
        ud->set_int("shape", shapeOrder);

        ud->set_int("mask", mask);
        ud->set_vec2f("spread", toAbiVec2f(spread));
        ud->set_int("visible", visible);
        ud->set_int("doubleside", doubleside);

        auto visibleIntensity = ZImpl(get_input2<float>("visibleIntensity"));
        ud->set_float("visibleIntensity", std::move(visibleIntensity));

        ZImpl(set_output("prim", std::move(prim)));
    }

    const static inline std::string lightShapeKey = "shape";

    static std::string lightShapeDefaultString() {
        auto name = magic_enum::enum_name(LightShape::Plane);
        return std::string(name);
    }

    static std::string lightShapeListString() {
        auto list = magic_enum::enum_names<LightShape>();

        std::string result;
        for (auto& ele : list) {
            result += " ";
            result += ele;
        }
        return result;
    }

    const static inline std::string lightTypeKey = "type";

    static std::string lightTypeDefaultString() {
        auto name = magic_enum::enum_name(LightType::Diffuse);
        return std::string(name);
    }

    static std::string lightTypeListString() {
        auto list = magic_enum::enum_names<LightType>();

        std::string result;
        for (auto& ele : list) {
            result += " ";
            result += ele;
        }
        return result;
    }
};

ZENO_DEFNODE(LightNode)({
    {
        {gParamType_Vec3f, "position", "0, 0, 0"},
        {gParamType_Vec3f, "scale", "1, 1, 1"},
        {gParamType_Vec3f, "rotate", "0, 0, 0"},
        {gParamType_Vec4f, "quaternion", "1, 0, 0, 0"},

        {gParamType_Vec3f, "color", "1, 1, 1"},
        {gParamType_Float, "exposure", "0"},
        {gParamType_Float, "intensity", "1"},
        {gParamType_Float, "fluxFixed", "-1.0"},

        {gParamType_Vec2f, "spread", "1.0, 0.0"},
        {gParamType_Float, "maxDistance", "-1.0" },
        {gParamType_Float, "falloffExponent", "2.0"},
        {gParamType_Int, "mask", "255"},
        {gParamType_Bool, "visible", "0"},
        {gParamType_Bool, "invertdir", "0"},
        {gParamType_Bool, "doubleside", "0"},

        {gParamType_String,"profile", "", Socket_Primitve, ReadPathEdit},
        {gParamType_String,"texturePath", "", Socket_Primitve, ReadPathEdit},
        {gParamType_Float,  "textureGamma", "1.0"},
        {gParamType_Float, "visibleIntensity", "-1.0"},

        {"enum " + LightNode::lightShapeListString(), LightNode::lightShapeKey, LightNode::lightShapeDefaultString()},
        {"enum " + LightNode::lightTypeListString(), LightNode::lightTypeKey, LightNode::lightTypeDefaultString()},
        {gParamType_Primitive, "prim"},
    },
    {
        {gParamType_Primitive, "prim"}
    },
    {
        {"enum " + EulerAngle::RotationOrderListString(), "EulerRotationOrder", EulerAngle::RotationOrderDefaultString()},
        {"enum " + EulerAngle::MeasureListString(), "EulerAngleMeasure", EulerAngle::MeasureDefaultString()}
    },
    {"shader"},
});

} // namespace