//
// ShaderFinalize - INode2 implementation
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/types/MaterialObject.h>
#include <zeno/types/TextureObject.h>
#include <zeno/utils/format.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/utils/Exception.h>
#include <zeno/utils/interfaceutil.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <tinygltf/json.hpp>
#include <algorithm>
#include <cstring>

namespace zeno {

struct ShaderFinalize : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        EmissionPass em;

        if (nd->has_input("commonCode")) {
            char buf[4096];
            nd->get_input2_string("commonCode", buf, sizeof(buf));
            em.commonCode += buf;
        }

        auto code = em.finalizeCode({
            {1, "mat_base"},
            {3, "mat_basecolor"},
            {1, "mat_roughness"},
            {1, "mat_metallic"},
            {3, "mat_metalColor"},
            {1, "mat_specular"},
            {1, "mat_specularTint"},
            {1, "mat_anisotropic"},
            {1, "mat_anisoRotation"},

            {1, "mat_subsurface"},
            {3, "mat_sssParam"},
            {3, "mat_sssColor"},
            {1, "mat_scatterDistance"},
            {1, "mat_scatterStep"},

            {1, "mat_sheen"},
            {1, "mat_sheenTint"},

            {1, "mat_clearcoat"},
            {3, "mat_clearcoatColor"},
            {1, "mat_clearcoatRoughness"},
            {1, "mat_clearcoatIOR"},

            {1, "mat_specTrans"},
            {3, "mat_transColor"},
            {3, "mat_transTint"},
            {1, "mat_transTintDepth"},
            {1, "mat_transDistance"},
            {3, "mat_transScatterColor"},
            {1, "mat_ior"},

            {1, "mat_diffraction"},
            {3, "mat_diffractColor"},

            {1, "mat_flatness"},
            {1, "mat_shadowReceiver"},
            {1, "mat_shadowTerminatorOffset"},
            {1, "mat_thin"},
            {1, "mat_doubleSide"},
            {3, "mat_normal"},
            {1, "mat_displacement"},
            {1, "mat_smoothness"},
            {1, "mat_emissionIntensity"},
            {3, "mat_emission"},
            {3, "mat_reflectance"},
            {1, "mat_opacity"},
            {1, "mat_thickness"},
            {1, "mat_isHair"}
        }, {
            params->get_input_shader("base", zeno::reflect::Any(float(1.0f))),
            params->get_input_shader("basecolor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("roughness", zeno::reflect::Any(float(0.4f))),
            params->get_input_shader("metallic", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("metalColor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("specular", zeno::reflect::Any(float(1.0f))),
            params->get_input_shader("specularTint", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("anisotropic", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("anisoRotation", zeno::reflect::Any(float(0.0f))),

            params->get_input_shader("subsurface", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("sssParam", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("sssColor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("scatterDistance", zeno::reflect::Any(float(10000))),
            params->get_input_shader("scatterStep", zeno::reflect::Any(float(0))),

            params->get_input_shader("sheen", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("sheenTint", zeno::reflect::Any(float(0.5f))),

            params->get_input_shader("clearcoat", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("clearcoatColor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("clearcoatRoughness", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("clearcoatIOR", zeno::reflect::Any(float(1.5f))),

            params->get_input_shader("specTrans", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("transColor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("transTint", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("transTintDepth", zeno::reflect::Any(float(10000.0f))),
            params->get_input_shader("transDistance", zeno::reflect::Any(float(1.0f))),
            params->get_input_shader("transScatterColor", zeno::reflect::Any(vec3f(1.0f))),
            params->get_input_shader("ior", zeno::reflect::Any(float(1.5f))),

            params->get_input_shader("diffraction", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("diffractColor", zeno::reflect::Any(vec3f(0.0f))),

            params->get_input_shader("flatness", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("shadowReceiver", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("shadowTerminatorOffset", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("thin", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("doubleSide", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("normal", zeno::reflect::Any(vec3f(0, 0, 1))),
            params->get_input_shader("displacement", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("smoothness", zeno::reflect::Any(float(1.0f))),
            params->get_input_shader("emissionIntensity", zeno::reflect::Any(float(1))),
            params->get_input_shader("emission", zeno::reflect::Any(vec3f(0))),
            params->get_input_shader("reflectance", zeno::reflect::Any(vec3f(1))),
            params->get_input_shader("opacity", zeno::reflect::Any(float(0.0))),
            params->get_input_shader("thickness", zeno::reflect::Any(float(0.0f))),
            params->get_input_shader("isHair", zeno::reflect::Any(float(0.0f)))
        });
        auto commonCode = em.getCommonCode();

        char buf[4096];
        nd->get_input2_string("sssRadius", buf, sizeof(buf));
        std::string sssRadiusMethod(buf);
        if (sssRadiusMethod == "Fixed") {
            code += "bool sssFxiedRadius = true;\n";
        } else {
            code += "bool sssFxiedRadius = false;\n";
        }

        zeno::vec3i maskVi = zeno::toVec3i(nd->get_input2_vec3i("mask_value"));
        vec3f mask_value = (vec3f)maskVi / 255.0f;
        code += zeno::format("vec3 mask_value = vec3({}, {}, {});\n", mask_value[0], mask_value[1], mask_value[2]);

        auto mtl = std::make_unique<MaterialObject>();
        nd->get_input2_string("mtlid", buf, sizeof(buf));
        mtl->mtlidkey = buf;
        mtl->frag = std::move(code);

        nlohmann::json j;
        if (nd->has_input("opacity")) {
            const ShaderData& opa_data = params->get_input_shader("opacity", zeno::reflect::Any(float(0.0)));
            if (opa_data.data.index() == 0) {
                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {
                        float opacity = val;
                        opacity = std::max(0.0f, 1.0f - opacity);
                        j["opacity"] = opacity;
                    } else {
                        throw zeno::makeError<UnimplError>("the type of opacity is not int or float");
                    }
                }, std::get<NumericValue>(opa_data.data));
            }
        }
        mtl->parameters = j.dump();

        if (nd->has_input("extensionsCode")) {
            char buf[4096];
            nd->get_input2_string("extensionsCode", buf, sizeof(buf));
            mtl->extensions = buf;
        }

        if (nd->has_input("tex2dList")) {
            IListObject* tex2dList = nd->get_input_ListObject("tex2dList");
            if (tex2dList && !tex2dList->empty() && !em.tex2Ds.empty()) {
                nd->report_error("Can not use both way!");
                return ZErr_ParamError;
            }
            if (tex2dList) {
                for (size_t i = 0; i < tex2dList->size(); i++) {
                    IObject2* obj = tex2dList->get(i);
                    if (auto* tex = dynamic_cast<Texture2DObject*>(obj)) {
                        em.tex2Ds.push_back(safe_uniqueptr_cast<Texture2DObject>(zany2(tex->clone())));
                    }
                }
            }
        }
        if (!em.tex2Ds.empty()) {
            for (const auto& tex : em.tex2Ds) {
                mtl->tex2Ds.push_back(safe_uniqueptr_cast<Texture2DObject>(zany2(tex->clone())));
            }
            auto texCode = "uniform sampler2D zenotex[32]; \n";
            mtl->common.insert(0, texCode);
        }

        mtl->common = std::move(commonCode);
        nd->set_output_object("mtl", mtl.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(ShaderFinalize, {
    {
        {gParamType_Float, "base", "1"},
        {gParamType_Vec3f, "basecolor", "1,1,1", Socket_Primitve, ColorVec},
        {gParamType_Float, "roughness", "0.4"},
        {gParamType_Float, "metallic", "0.0"},
        {gParamType_Vec3f, "metalColor", "1.0,1.0,1.0", Socket_Primitve, ColorVec},
        {gParamType_Float, "specular", "1.0"},
        {gParamType_Float, "specularTint", "0.0"},
        {gParamType_Float, "anisotropic", "0.0"},
        {gParamType_Float, "anisoRotation", "0.0"},

        {gParamType_Float, "subsurface", "0.0"},
        {"enum Fixed Adaptive", "sssRadius", "Fixed"},
        {gParamType_Vec3f, "sssParam", "1.0,1.0,1.0"},
        {gParamType_Vec3f, "sssColor", "1.0,1.0,1.0", Socket_Primitve, ColorVec},
        {gParamType_Float, "scatterDistance", "10000"},
        {gParamType_Float, "scatterStep", "0"},

        {gParamType_Float, "sheen", "0.0"},
        {gParamType_Float, "sheenTint", "0.0"},

        {gParamType_Float, "clearcoat", "0.0"},
        {gParamType_Vec3f, "clearcoatColor", "1.0,1.0,1.0"},
        {gParamType_Float, "clearcoatRoughness", "0.0"},
        {gParamType_Float, "clearcoatIOR", "1.5"},

        {gParamType_Float, "specTrans", "0.0"},
        {gParamType_Vec3f, "transColor", "1.0,1.0,1.0"},
        {gParamType_Vec3f, "transTint", "1.0,1.0,1.0"},
        {gParamType_Float, "transTintDepth", "10000.0"},
        {gParamType_Float, "transDistance", "10.0"},
        {gParamType_Vec3f, "transScatterColor", "1.0,1.0,1.0"},
        {gParamType_Float, "ior", "1.3"},

        {gParamType_Float, "diffraction", "0.0"},
        {gParamType_Vec3f, "diffractColor", "0.0,0.0,0.0"},

        {gParamType_Float, "flatness", "0.0"},
        {gParamType_Float, "shadowReceiver", "0.0"},
        {gParamType_Float, "shadowTerminatorOffset", "0.0"},
        {gParamType_Float, "thin", "0.0"},
        {gParamType_Float, "doubleSide", "0.0"},
        {gParamType_Vec3f, "normal", "0,0,1"},
        {gParamType_Float, "displacement", "0"},
        {gParamType_Float, "smoothness", "1.0"},
        {gParamType_Float, "emissionIntensity", "1"},
        {gParamType_Vec3f, "emission", "0,0,0"},
        {gParamType_Vec3f, "reflectance", "1,1,1"},
        {gParamType_Float, "opacity", "0"},
        {gParamType_Float, "thickness", "0.0"},
        {gParamType_Float, "isHair", "0.0"},

        {gParamType_String, "commonCode"},
        {gParamType_String, "extensionsCode"},
        {gParamType_String, "mtlid", "Mat1"},
        {gParamType_List, "tex2dList"},
        {gParamType_Vec3i, "mask_value", "0,0,0"},
    },
    {
        {gParamType_Material, "mtl"},
    },
    {
        {"enum CUDA", "backend", "CUDA"},
    },
    {"shader"},
});

} // namespace zeno
