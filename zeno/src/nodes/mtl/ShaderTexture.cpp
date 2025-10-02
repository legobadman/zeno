#include <zeno/zeno.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/ShaderObject.h>
#include <zeno/types/TextureObject.h>
#include "zeno/types/HeatmapObject.h"
#include <zeno/utils/string.h>
#include <algorithm>
#include "zeno/utils/format.h"
#include "zeno/utils/logger.h"
#include <tinygltf/stb_image_write.h>
#include <filesystem>

#include <string>
#include "magic_enum.hpp"

namespace zeno
{
struct ShaderTexture2D : ShaderNodeClone<ShaderTexture2D>
{
    virtual int determineType(EmissionPass *em) override {
        auto texId = ZImpl(get_input2<int>("texId"));
        auto uvtiling = em->determineType(ZImpl(get_input_shader("uvtiling")));
        if (ZImpl(has_input("coord"))) {
            auto coord = em->determineType(ZImpl(get_input_shader("coord")));
            if (coord < 2)
                throw zeno::Exception("ShaderTexture2D expect coord to be at least vec2");
        }

        auto type = zsString2Std(get_input2_string("type"));
        if (type == "float")
            return 1;
        else if (type == "vec2")
            return 2;
        else if (type == "vec3")
            return 3;
        else if (type == "vec4")
            return 4;
        else
            throw zeno::Exception("ShaderTexture2D got bad type: " + type);
    }

    virtual void emitCode(EmissionPass *em) override {
        auto texId = ZImpl(get_input2<int>("texId"));
        auto type = ZImpl(get_input2<std::string>("type"));
        auto uvtiling = em->determineExpr(ZImpl(get_input_shader("uvtiling")));
        std::string coord = "att_uv";
        if (ZImpl(has_input("coord"))) {
            coord = em->determineExpr(ZImpl(get_input_shader("coord")));
        }
        em->emitCode(zeno::format("{}(texture2D(zenotex[{}], vec2({}) * {}))", type, texId, coord, uvtiling));
    }
};



struct ShaderTexture3D : ShaderNodeClone<ShaderTexture3D>
{
    enum struct SamplingMethod {
        Closest, Trilinear, Triquadratic, Tricubic, Stochastic
    };

    static std::string methodDefaultString() {
        auto name = magic_enum::enum_name(SamplingMethod::Trilinear);
        return std::string(name);
    }

    static std::string methodListString() {
        auto list = magic_enum::enum_names<SamplingMethod>();

        std::string result;
        for (auto& ele : list) {
            result += " ";
            result += ele;
        }
        return result;
    }

    virtual int determineType(EmissionPass *em) override {
        auto texId = ZImpl(get_input2<int>("texId"));
        auto coord = em->determineType(ZImpl(get_input_shader("coord")));
        if (coord != 3)
            throw zeno::Exception("ShaderTexture3D expect coord to be vec3");

        auto type = zsString2Std(get_input2_string("type"));

        static const std::map<std::string, int> type_map {
            //{"half", 0},
            //{"float", 1},
            {"vec2", 2},
        };

        if (type_map.count(type) == 0) {
            throw zeno::Exception("ShaderTexture3D got bad type: " + type);
        }

        return type_map.at(type);            
    }

    virtual void emitCode(EmissionPass *em) override {
        auto texId = ZImpl(get_input2<int>("texId"));
        auto type = ZImpl(get_input2<std::string>("type"));
        auto coord = em->determineExpr(ZImpl(get_input_shader("coord")));

        auto cihou = get_input2_bool("cihou");
        
        auto space = zsString2Std(get_input2_string("space"));
        auto world_space = (space == "World")? "true":"false";

        auto dim = em->determineType(ZImpl(get_input_shader("coord")));
        auto method = ZImpl(get_input2<std::string>("method"));

	    auto casted = magic_enum::enum_cast<SamplingMethod>(method).value_or(SamplingMethod::Trilinear);

        auto order = magic_enum::enum_integer(casted);
        std::string ORDER = std::to_string( order );

        //using DataTypeNVDB0 = float; //nanovdb::Fp32;
        //using GridTypeNVDB0 = nanovdb::NanoGrid<DataTypeNVDB0>;
        std::string sid = std::to_string(texId);

        std::string DataTypeNVDB = "DataTypeNVDB" + sid;
        std::string GridTypeNVDB = "GridTypeNVDB" + sid;

        em->emitCode(type + "(samplingVDB<"+ ORDER +","+ world_space + "," + DataTypeNVDB +">(vdb_grids[" + sid + "], vec3(" + coord + "), attrs, " + std::to_string(cihou) + "))");
    }
};

    ZENDEFNODE(ShaderTexture2D, {
    {
        {gParamType_Int, "texId", "0"},
        {gParamType_IObject, "coord"},
        {gParamType_Vec2f, "uvtiling", "1,1"},
        {"enum float vec2 vec3 vec4", "type", "vec3"},
    },
    {
        {gParamType_Shader, "out"},
    },
    {},
    {
        "shader",
    },
});

ZENDEFNODE(ShaderTexture3D, {
    {
        {"int", "texId", "0"},
        {"vec3f", "coord", "0,0,0"},
        {"bool", "cihou", "0"},
        {"enum World Local", "space", "World"},
        {"enum vec2", "type", "vec2"},
        
        {"enum " + ShaderTexture3D::methodListString(), "method", ShaderTexture3D::methodDefaultString()} 
    },
    {
        {gParamType_Shader, "out"},
    },
    {},
    {
        "shader",
    },
});

struct SmartTexture2D : ShaderNodeClone<SmartTexture2D>
{
    static constexpr char
        texWrapping[] = "REPEAT MIRRORED_REPEAT CLAMP_TO_EDGE CLAMP_TO_BORDER",
        texFiltering[] = "NEAREST LINEAR NEAREST_MIPMAP_NEAREST LINEAR_MIPMAP_NEAREST NEAREST_MIPMAP_LINEAR LINEAR_MIPMAP_LINEAR";
    virtual int determineType(EmissionPass *em) override {
        auto uvtiling = em->determineType(ZImpl(get_input_shader("uvtiling")));
        auto normalScale = em->determineType(ZImpl(get_input_shader("normalScale")));
        auto h_scale = em->determineType(ZImpl(get_input_shader("heightScale")));
        if (ZImpl(has_input("coord"))) {
            auto coord = em->determineType(ZImpl(get_input_shader("coord")));
            if (coord < 2)
                throw zeno::Exception("ShaderTexture2D expect coord to be at least vec2");
        }

        auto type = zsString2Std(get_input2_string("type"));
        if (type == "float" || type == "R" || type == "G" || type == "B" || type == "A")
            return 1;
        else if (type == "vec2")
            return 2;
        else if (type == "vec3")
            return 3;
        else if (type == "vec4")
            return 4;
        else
            throw zeno::Exception("ShaderTexture2D got bad type: " + type);
    }

    virtual void emitCode(EmissionPass *em) override {
        auto texId = em->tex2Ds.size();
        auto tex = std::make_unique<zeno::Texture2DObject>();
        auto texture_path = zsString2Std(get_input2_string("path"));
        if (has_input("heatmap")) {
            if (texture_path.empty()) {
                std::srand(std::time(0));
                texture_path = std::filesystem::temp_directory_path().string() + '/' + "heatmap-" + std::to_string(std::rand()) + ".png";
            }
            auto heatmap = ZImpl(get_input<zeno::HeatmapObject>("heatmap"));
            if (heatmap && !heatmap->colors.empty()) {
            std::vector<uint8_t> col;
            int width = heatmap->colors.size();
            int height = width;
            col.reserve(width * height * 3);
            for (auto i = 0; i < height; i++) {
                for (auto & color : heatmap->colors) {
                    col.push_back(zeno::clamp(int(color[0] * 255.99), 0, 255));
                    col.push_back(zeno::clamp(int(color[1] * 255.99), 0, 255));
                    col.push_back(zeno::clamp(int(color[2] * 255.99), 0, 255));
                }
            }
            stbi_flip_vertically_on_write(false);
            stbi_write_png(texture_path.c_str(), width, height, 3, col.data(), 0);
        }
        }
        if(!std::filesystem::exists(std::filesystem::u8path(texture_path))){
            //zeno::log_warn("texture file not found!");
            auto type = zsString2Std(get_input2_string("type"));
            vec4f number= vec4f(0,0,0,0);
            if(ZImpl(has_input2<float>("value")))
            {
              number[0] = ZImpl(get_input2<float>("value"));
            }
            if(ZImpl(has_input2<zeno::vec2f>("value")))
            {
              auto in = ZImpl(get_input2<zeno::vec2f>("value"));
              number[0] = in[0];
              number[1] = in[1];
            }
            if(ZImpl(has_input2<zeno::vec3f>("value")))
            {
              auto in = ZImpl(get_input2<zeno::vec3f>("value"));
              number[0] = in[0];
              number[1] = in[1];
              number[2] = in[2];
            }
            if(ZImpl(has_input2<zeno::vec4f>("value")))
            {
              auto in = ZImpl(get_input2<zeno::vec4f>("value"));
              number[0] = in[0];
              number[1] = in[1];
              number[2] = in[2];
              number[3] = in[3];
            }

            if (type == "float" || type == "R")
                em->emitCode(zeno::format("{}",number[0]));
            else if (type == "G")
                em->emitCode(zeno::format("{}",number[1]));
            else if (type == "B")
                em->emitCode(zeno::format("{}",number[2]));
            else if (type == "A")
                em->emitCode(zeno::format("{}",number[3]));
            else if (type == "vec2")
                em->emitCode(zeno::format("vec2({},{})",number[0],number[1]));
            else if (type == "vec3")
                em->emitCode(zeno::format("vec3({},{},{})",number[0],number[1],number[2]));
            else if (type == "vec4")
                em->emitCode(zeno::format("vec4({},{},{},{})",number[0],number[1],number[2],number[3]));
            else
                throw zeno::Exception("ShaderTexture2D got bad type: " + type);
            
            return;
            
        }

        tex->path = texture_path;
        tex->blockCompression = get_input2_bool("blockCompression");

    #define SET_TEX_WRAP(TEX, WRAP)                                    \
        if (WRAP == "REPEAT")                                          \
            TEX->WRAP = Texture2DObject::TexWrapEnum::REPEAT;          \
        else if (WRAP == "MIRRORED_REPEAT")                            \
            TEX->WRAP = Texture2DObject::TexWrapEnum::MIRRORED_REPEAT; \
        else if (WRAP == "CLAMP_TO_EDGE")                              \
            TEX->WRAP = Texture2DObject::TexWrapEnum::CLAMP_TO_EDGE;   \
        else if (WRAP == "CLAMP_TO_BORDER")                            \
            TEX->WRAP = Texture2DObject::TexWrapEnum::CLAMP_TO_BORDER; \
        else                                                           \
            throw zeno::Exception(#WRAP + WRAP);

        auto wrapS = ZImpl(get_input<zeno::StringObject>("wrapT"))->get();
        SET_TEX_WRAP(tex, wrapS)
        auto wrapT = ZImpl(get_input<zeno::StringObject>("wrapS"))->get();
        SET_TEX_WRAP(tex, wrapT)

    #undef SET_TEX_WRAP

    #define SET_TEX_FILTER(TEX, FILTER)                                           \
        if (FILTER == "NEAREST")                                                  \
            TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST;                \
        else if (FILTER == "LINEAR")                                              \
            TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR;                 \
        else if (FILTER == "NEAREST_MIPMAP_NEAREST")                              \
            TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST_MIPMAP_NEAREST; \
        else if (FILTER == "LINEAR_MIPMAP_NEAREST")                               \
            TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR_MIPMAP_NEAREST;  \
        else if (FILTER == "NEAREST_MIPMAP_LINEAR")                               \
            TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST_MIPMAP_LINEAR;  \
        else if (FILTER == "LINEAR_MIPMAP_LINEAR")                                \
            TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR_MIPMAP_LINEAR;   \
        else                                                                      \
            throw zeno::Exception(#FILTER + FILTER);

        auto minFilter = ZImpl(get_input<zeno::StringObject>("minFilter")->get());
        SET_TEX_FILTER(tex, minFilter)
        auto magFilter = ZImpl(get_input<zeno::StringObject>("magFilter")->get());
        SET_TEX_FILTER(tex, magFilter)

    #undef SET_TEX_FILTER
        em->tex2Ds.push_back(tex);
        auto type = zsString2Std(get_input2_string("type"));
        std::string suffix;
        if (type == "R") {
            suffix = ".x";
            type = "";
        }
        else if (type == "G") {
            suffix = ".y";
            type = "";
        }
        else if (type == "B") {
            suffix = ".z";
            type = "";
        }
        else if (type == "A") {
            suffix = ".w";
            type = "";
        }
        auto uvtiling = em->determineExpr(ZImpl(get_input_shader("uvtiling")));
        auto nscale = em->determineExpr(ZImpl(get_input_shader("normalScale")));
        auto hscale = em->determineExpr(ZImpl(get_input_shader("heightScale")));
        std::string coord = "att_uv";
        if (ZImpl(has_input("coord"))) {
            coord = em->determineExpr(ZImpl(get_input_shader("coord")));
        }
        auto postprocess = zsString2Std(get_input2_string("post_process"));
        
        if (type == "float" && postprocess == "1-x") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format("1.0f - texture2D<float,float>(zenotex[{}], saturate( vec2({}) * {}) )", texId, coord, uvtiling));
            }
            else {
                em->emitCode(zeno::format("1.0f - texture2D<float,float>(zenotex[{}], vec2({}) * {} )", texId, coord, uvtiling));
            }
            return;
        }
        if(postprocess == "raw"){
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format("{}(texture2D(zenotex[{}], saturate( vec2({}) * {}) )){}", type, texId, coord, uvtiling, suffix));
            }
            else {
            em->emitCode(zeno::format("{}(texture2D(zenotex[{}], vec2({}) * {})){}", type, texId, coord, uvtiling, suffix));
            }
        }else if (postprocess == "srgb"){
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format("pow({}(texture2D(zenotex[{}], saturate(vec2({}) * {}))),2.2f){}", type, texId, coord, uvtiling, suffix));
            }
            else {
            em->emitCode(zeno::format("pow({}(texture2D(zenotex[{}], vec2({}) * {})),2.2f){}", type, texId, coord, uvtiling, suffix));
            }
        }else if (postprocess == "normal_map"){
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format("normalize({}(texture2D(zenotex[{}], saturate(vec2({}) * {}))) * vec3({},{},1.0) - vec3(0.5*{},0.5*{},0.0)){}", type, texId, coord, uvtiling, nscale,nscale,nscale,nscale,suffix));
            }
            else {
                em->emitCode(zeno::format("normalize({}(texture2D(zenotex[{}], vec2({}) * {})) * vec3({},{},1.0) - vec3(0.5*{},0.5*{},0.0)){}", type, texId, coord, uvtiling, nscale,nscale,nscale,nscale,suffix));
            }
        }else if (postprocess == "1-x"){
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format("{}(1.0) - {}(texture2D(zenotex[{}], saturate(vec2({}) * {}))){}", type, type, texId, coord, uvtiling, suffix));
            }
            else {
            em->emitCode(zeno::format("{}(1.0) - {}(texture2D(zenotex[{}], vec2({}) * {})){}", type, type, texId, coord, uvtiling, suffix));
        }
        }else if (postprocess == "displacement"){
            em->emitCode(zeno::format("{}(parallaxCall(*(TriangleInput*)&attrs, zenotex[{}], vec2({}), {}, {})){}", type, texId, coord, uvtiling, hscale, suffix));
    }
    }
};

ZENDEFNODE(SmartTexture2D, {
    {
        {gParamType_String, "path", "", Socket_Primitve, ReadPathEdit},
        {gParamType_Heatmap, "heatmap"},
        {(std::string) "enum " + SmartTexture2D::texWrapping, "wrapS", "REPEAT"},
        {(std::string) "enum " + SmartTexture2D::texWrapping, "wrapT", "REPEAT"},
        {(std::string) "enum " + SmartTexture2D::texFiltering, "minFilter", "LINEAR"},
        {(std::string) "enum " + SmartTexture2D::texFiltering, "magFilter", "LINEAR"},
        {gParamType_Vec2f, "coord"},
        {gParamType_Vec2f, "uvtiling", "1,1"},
        {gParamType_Vec4f, "value", "0,0,0,0"},
        {gParamType_Float, "normalScale", "1.0"},
        {gParamType_Vec4f, "heightScale", "1.0,1.0,0.0,1.0"},
        {"enum float vec2 vec3 vec4 R G B A", "type", "vec3"},
        {"enum raw srgb normal_map 1-x displacement", "post_process", "raw"},
        {gParamType_Bool, "blockCompression", "false"}
    },
    {
        {gParamType_Shader, "out"},
    },
    {},
    {
        "shader",
    },
});

} // namespace zeno

