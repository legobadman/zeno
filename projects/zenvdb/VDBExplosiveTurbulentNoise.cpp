#include <zeno/zeno.h>
#include <zeno/VDBGrid.h>
#include <zeno/types/NumericObject.h>
#include <openvdb/openvdb.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/points/PointAdvect.h>
#include <openvdb/tools/Interpolation.h>
#include <zeno/utils/interfaceutil.h>

namespace {

using namespace zeno;


template <class T0, class T1, class T2>
auto smoothstep(T0 edge0, T1 edge1, T2 x) {
  // Scale, bias and saturate x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0, 1);
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}


struct FBMTurbulent {
float UVScale= 			 0.4;
float Speed=				 0.6;

float FBM_WarpPrimary=		-0.24;
float FBM_WarpSecond=		 0.29;
float FBM_WarpPersist= 	 0.78;
float FBM_EvalPersist= 	 0.62;
float FBM_Persistence= 	 0.5;
float FBM_Lacunarity= 		 2.2;
int FBM_Octaves= 		 5;


//fork from Dave Hoskins
//https://www.shadertoy.com/view/4djSRW
zeno::vec4f hash43(zeno::vec3f p)
{
	zeno::vec4f p4 = fract(zeno::vec4f(p[0], p[1], p[2], p[0]) * zeno::vec4f(1031.f, .1030f, .0973f, .1099f));
    p4 += dot(p4, zeno::vec4f(p4[3],p4[2],p4[0],p4[1])+19.19f);
	return -1.0f + 2.0f * fract(zeno::vec4f(
        (p4[0] + p4[1])*p4[2], (p4[0] + p4[2])*p4[1],
        (p4[1] + p4[2])*p4[3], (p4[2] + p4[3])*p4[0])
    );
}

//offsets for noise
inline static const zeno::vec3f nbs[] = {
    zeno::vec3f(0.0, 0.0, 0.0),zeno::vec3f(0.0, 1.0, 0.0),zeno::vec3f(1.0, 0.0, 0.0),zeno::vec3f(1.0, 1.0, 0.0),
    zeno::vec3f(0.0, 0.0, 1.0),zeno::vec3f(0.0, 1.0, 1.0),zeno::vec3f(1.0, 0.0, 1.0),zeno::vec3f(1.0, 1.0, 1.0)
};

//'Simplex out of value noise', forked from: https://www.shadertoy.com/view/XltXRH
//not sure about performance, is this faster than classic simplex noise?
zeno::vec4f AchNoise3D(zeno::vec3f x)
{
    zeno::vec3f p = floor(x);
    zeno::vec3f fr = smoothstep(0.0f, 1.0f, fract(x));

    zeno::vec4f L1C1 = mix(hash43(p+nbs[0]), hash43(p+nbs[2]), fr[0]);
    zeno::vec4f L1C2 = mix(hash43(p+nbs[1]), hash43(p+nbs[3]), fr[0]);
    zeno::vec4f L1C3 = mix(hash43(p+nbs[4]), hash43(p+nbs[6]), fr[0]);
    zeno::vec4f L1C4 = mix(hash43(p+nbs[5]), hash43(p+nbs[7]), fr[0]);
    zeno::vec4f L2C1 = mix(L1C1, L1C2, fr[1]);
    zeno::vec4f L2C2 = mix(L1C3, L1C4, fr[1]);
    return mix(L2C1, L2C2, fr[2]);
}

zeno::vec4f ValueSimplex3D(zeno::vec3f p)
{
	zeno::vec4f a = AchNoise3D(p);
	zeno::vec4f b = AchNoise3D(p + 120.5f);
	return (a + b) * 0.5;
}

//my FBM
zeno::vec4f FBM(zeno::vec3f p)
{
    zeno::vec4f f(0), s(0), n(0);
    float a = 1.0f, w = 0.0f;
    for (int i=0; i<FBM_Octaves; i++)
    {
        n = ValueSimplex3D(p);
        f += (abs(n)) * a;	//billowed-like
        s += zeno::vec4f(n[2], n[3], n[0], n[1]) *a;
        a *= FBM_Persistence;
        w *= FBM_WarpPersist;
        p *= FBM_Lacunarity;
        p += zeno::vec3f(n[0], n[1], n[2]) * FBM_WarpPrimary *w;
        p += zeno::vec3f(s[0], s[1], s[2]) * FBM_WarpSecond;
        p[2] *= FBM_EvalPersist +(f[3] *0.5f+0.5f) *0.015f;
    }
    return f;
}

float operator()(float x, float y, float z) {  // https://www.shadertoy.com/view/3lsSR7
    auto fbm = FBM({x, y, z});
    float explosionGrad = (dot(fbm, zeno::vec4f(fbm[1], fbm[0], fbm[3], fbm[0]))) *0.5f;
    explosionGrad = pow(explosionGrad, 1.3f);
    explosionGrad = smoothstep(0.0f,1.0f,explosionGrad);
    return explosionGrad;
}

};

struct VDBExplosiveTurbulentNoise : INode {
  virtual void apply() override {
    auto inoutSDF = safe_dynamic_cast<VDBFloatGrid>(get_input("inoutSDF"));
    auto strength = get_input2_float("strength");
    auto scale = get_input2_float("scale");
    auto scaling = has_input("scaling") ?
        toVec3f(get_input2_vec3f("scaling"))
        : zeno::vec3f(1);
    auto translation = has_input("translation") ?
        toVec3f(get_input2_vec3f("translation"))
        : zeno::vec3f(0);
    auto inv_scale = 1.f / (scale * scaling);

    auto grid = inoutSDF->m_grid;
    float dx = grid->voxelSize()[0];
    strength *= dx;

    FBMTurbulent turbulent;
    turbulent.UVScale=m_pAdapter->get_param_float("UVScale");
    turbulent.Speed=m_pAdapter->get_param_float("Speed");
    turbulent.FBM_WarpPrimary=m_pAdapter->get_param_float("FBM_WarpPrimary");
    turbulent.FBM_WarpSecond=m_pAdapter->get_param_float("FBM_WarpSecond");
    turbulent.FBM_WarpPersist=m_pAdapter->get_param_float("FBM_WarpPersist");
    turbulent.FBM_EvalPersist=m_pAdapter->get_param_float("FBM_EvalPersist");
    turbulent.FBM_Persistence=m_pAdapter->get_param_float("FBM_Persistence");
    turbulent.FBM_Lacunarity=m_pAdapter->get_param_float("FBM_Lacunarity");
    turbulent.FBM_Octaves=m_pAdapter->get_param_int("FBM_Octaves");

    auto wrangler = [&](auto &leaf, openvdb::Index leafpos) {
        for (auto iter = leaf.beginValueOn(); iter != leaf.endValueOn(); ++iter) {
            auto coord = iter.getCoord();
            auto pos = (zeno::vec3i(coord[0], coord[1], coord[2]) + translation) * inv_scale;
            auto noise = strength * turbulent(pos[0], pos[1], pos[2]);
            iter.modifyValue([&] (auto &v) {
                v += noise;
            });
        }
    };
    auto velman = openvdb::tree::LeafManager<std::decay_t<decltype(grid->tree())>>(grid->tree());
    velman.foreach(wrangler);

    set_output("inoutSDF", get_input("inoutSDF"));
  }
};

ZENO_DEFNODE(VDBExplosiveTurbulentNoise)(
     { /* inputs: */ {
     {gParamType_VDBGrid,"inoutSDF"},
     {gParamType_Float, "strength", "1.0"},
     {gParamType_Float, "scale", "8.0"},
     {gParamType_Vec3f, "scaling", "1,1,1"},
     {gParamType_Vec3f, "translation", "0,0,0"},
     }, /* outputs: */ {
       {gParamType_VDBGrid,"inoutSDF"}
     }, /* params: */ {
    {gParamType_Float,"UVScale","0.4"},
    {gParamType_Float,"Speed","0.6"},
    {gParamType_Float,"FBM_WarpPrimary","-0.24"},
    {gParamType_Float,"FBM_WarpSecond","0.29"},
    {gParamType_Float,"FBM_WarpPersist","0.78"},
    {gParamType_Float,"FBM_EvalPersist","0.62"},
    {gParamType_Float,"FBM_Persistence","0.5"},
    {gParamType_Float,"FBM_Lacunarity","2.2"},
    {gParamType_Int,"FBM_Octaves","5"},
     }, /* category: */ {
     "openvdb",
     }});



}
