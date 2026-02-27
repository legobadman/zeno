#include "simple_geometry_common.h"

#include <array>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

// Modulo 289, optimizes to code without divisions.
static glm::vec3 mod289(glm::vec3 x) {
    glm::vec3 ret{};
    ret.x = x.x - std::floor(x.x * (1.0f / 289.0f)) * 289.0f;
    ret.y = x.y - std::floor(x.y * (1.0f / 289.0f)) * 289.0f;
    ret.z = x.z - std::floor(x.z * (1.0f / 289.0f)) * 289.0f;
    return ret;
}

static double mod289(double x) {
    return x - std::floor(x * (1.0 / 289.0)) * 289.0;
}

// Permutation polynomial (ring size 289 = 17*17).
static glm::vec3 permute(glm::vec3 x) {
    glm::vec3 ret{};
    ret.x = static_cast<float>(mod289(((x.x * 34.0f) + 10.0f) * x.x));
    ret.y = static_cast<float>(mod289(((x.y * 34.0f) + 10.0f) * x.y));
    ret.z = static_cast<float>(mod289(((x.z * 34.0f) + 10.0f) * x.z));
    return ret;
}

static double permute(double x) {
    return mod289(((x * 34.0) + 10.0) * x);
}

// Hashed 2-D gradients with an extra rotation.
// (The constant 0.0243902439 is 1/41)
static glm::vec2 rgrad2(glm::vec2 p, double rot) {
    double u = permute(permute(p.x) + p.y) * 0.0243902439 + rot;
    u = 4.0 * glm::fract(u) - 2.0;
    return glm::vec2(static_cast<float>(std::abs(u) - 1.0),
        static_cast<float>(std::abs(std::abs(u + 1.0) - 2.0) - 1.0));
}

// 2-D non-tiling simplex noise with rotating gradients and analytical derivative.
// Returns (noise, d/dx, d/dy).
static glm::vec3 srdnoise(glm::vec2 pos, double rot) {
    pos.y += 0.001f;
    glm::vec2 uv = glm::vec2(pos.x + pos.y * 0.5f, pos.y);

    glm::vec2 i0 = glm::floor(uv);
    glm::vec2 f0 = glm::fract(uv);
    glm::vec2 i1 = (f0.x > f0.y) ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f);

    glm::vec2 p0 = glm::vec2(i0.x - i0.y * 0.5f, i0.y);
    glm::vec2 p1 = glm::vec2(p0.x + i1.x - i1.y * 0.5f, p0.y + i1.y);
    glm::vec2 p2 = glm::vec2(p0.x + 0.5f, p0.y + 1.0f);

    i1 = i0 + i1;

    glm::vec2 d0 = pos - p0;
    glm::vec2 d1 = pos - p1;
    glm::vec2 d2 = pos - p2;

    glm::vec3 x = glm::vec3(p0.x, p1.x, p2.x);
    glm::vec3 y = glm::vec3(p0.y, p1.y, p2.y);
    glm::vec3 iuw = x + glm::vec3(0.5f) * y;
    glm::vec3 ivw = y;

    iuw = mod289(iuw);
    ivw = mod289(ivw);

    glm::vec2 g0 = rgrad2(glm::vec2(iuw.x, ivw.x), rot);
    glm::vec2 g1 = rgrad2(glm::vec2(iuw.y, ivw.y), rot);
    glm::vec2 g2 = rgrad2(glm::vec2(iuw.z, ivw.z), rot);

    glm::vec3 w = glm::vec3(glm::dot(g0, d0), glm::dot(g1, d1), glm::dot(g2, d2));
    glm::vec3 t = glm::vec3(0.8f) - glm::vec3(glm::dot(d0, d0), glm::dot(d1, d1), glm::dot(d2, d2));

    glm::vec3 dtdx = glm::vec3(-2.0f) * glm::vec3(d0.x, d1.x, d2.x);
    glm::vec3 dtdy = glm::vec3(-2.0f) * glm::vec3(d0.y, d1.y, d2.y);

    if (t.x < 0.0f) { dtdx.x = 0.0f; dtdy.x = 0.0f; t.x = 0.0f; }
    if (t.y < 0.0f) { dtdx.y = 0.0f; dtdy.y = 0.0f; t.y = 0.0f; }
    if (t.z < 0.0f) { dtdx.z = 0.0f; dtdy.z = 0.0f; t.z = 0.0f; }

    glm::vec3 t2 = t * t;
    glm::vec3 t4 = t2 * t2;
    glm::vec3 t3 = t2 * t;

    const float n = glm::dot(t4, w);

    glm::vec2 dt0 = glm::vec2(dtdx.x, dtdy.x) * glm::vec2(4.0f) * t3.x;
    glm::vec2 dn0 = t4.x * g0 + dt0 * w.x;
    glm::vec2 dt1 = glm::vec2(dtdx.y, dtdy.y) * glm::vec2(4.0f) * t3.y;
    glm::vec2 dn1 = t4.y * g1 + dt1 * w.y;
    glm::vec2 dt2 = glm::vec2(dtdx.z, dtdy.z) * glm::vec2(4.0f) * t3.z;
    glm::vec2 dn2 = t4.z * g2 + dt2 * w.z;

    glm::vec2 grad = dn0 + dn1 + dn2;
    return glm::vec3(11.0f) * glm::vec3(n, grad.x, grad.y);
}

static glm::vec3 sdnoise(glm::vec2 pos) {
    return srdnoise(pos, 0.0);
}

static int noise_fastfloor(double x) {
    return x > 0.0 ? static_cast<int>(x) : static_cast<int>(x) - 1;
}

static float noise_sgrad3(int hash, float x, float y, float z) {
    switch (hash & 0xF) {
    case 0x0: return x + y;
    case 0x1: return -x + y;
    case 0x2: return x - y;
    case 0x3: return -x - y;
    case 0x4: return x + z;
    case 0x5: return -x + z;
    case 0x6: return x - z;
    case 0x7: return -x - z;
    case 0x8: return y + z;
    case 0x9: return -y + z;
    case 0xA: return y - z;
    case 0xB: return -y - z;
    case 0xC: return y + x;
    case 0xD: return -y + z;
    case 0xE: return y - x;
    case 0xF: return -y - z;
    default: return 0.0f;
    }
}

static const int noise_permutation_256[256] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,
    136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,
    159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,
    47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,
    253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
    81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,
    205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

static int noise_perm(int idx) {
    return noise_permutation_256[idx & 255];
}

static float noise_fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float noise_grad(int hash, float x, float y, float z) {
    switch (hash & 0xF) {
    case 0x0: return x + y;
    case 0x1: return -x + y;
    case 0x2: return x - y;
    case 0x3: return -x - y;
    case 0x4: return x + z;
    case 0x5: return -x + z;
    case 0x6: return x - z;
    case 0x7: return -x - z;
    case 0x8: return y + z;
    case 0x9: return -y + z;
    case 0xA: return y - z;
    case 0xB: return -y - z;
    case 0xC: return y + x;
    case 0xD: return -y + z;
    case 0xE: return y - x;
    case 0xF: return -y - z;
    default: return 0.0f;
    }
}

static float noise_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float noise_perlin(float x, float y, float z) {
    const int xi = static_cast<int>(std::floor(x)) & 255;
    const int yi = static_cast<int>(std::floor(y)) & 255;
    const int zi = static_cast<int>(std::floor(z)) & 255;

    const float xf = x - std::floor(x);
    const float yf = y - std::floor(y);
    const float zf = z - std::floor(z);
    const float u = noise_fade(xf);
    const float v = noise_fade(yf);
    const float w = noise_fade(zf);

    const int aaa = noise_perm(noise_perm(noise_perm(xi) + yi) + zi);
    const int aba = noise_perm(noise_perm(noise_perm(xi) + yi + 1) + zi);
    const int aab = noise_perm(noise_perm(noise_perm(xi) + yi) + zi + 1);
    const int abb = noise_perm(noise_perm(noise_perm(xi) + yi + 1) + zi + 1);
    const int baa = noise_perm(noise_perm(noise_perm(xi + 1) + yi) + zi);
    const int bba = noise_perm(noise_perm(noise_perm(xi + 1) + yi + 1) + zi);
    const int bab = noise_perm(noise_perm(noise_perm(xi + 1) + yi) + zi + 1);
    const int bbb = noise_perm(noise_perm(noise_perm(xi + 1) + yi + 1) + zi + 1);

    const float x1 = noise_lerp(noise_grad(aaa, xf, yf, zf), noise_grad(baa, xf - 1.0f, yf, zf), u);
    const float x2 = noise_lerp(noise_grad(aba, xf, yf - 1.0f, zf), noise_grad(bba, xf - 1.0f, yf - 1.0f, zf), u);
    const float y1 = noise_lerp(x1, x2, v);
    const float x3 = noise_lerp(noise_grad(aab, xf, yf, zf - 1.0f), noise_grad(bab, xf - 1.0f, yf, zf - 1.0f), u);
    const float x4 = noise_lerp(noise_grad(abb, xf, yf - 1.0f, zf - 1.0f), noise_grad(bbb, xf - 1.0f, yf - 1.0f, zf - 1.0f), u);
    const float y2 = noise_lerp(x3, x4, v);
    return noise_lerp(y1, y2, w);
}

// 3D Perlin simplex noise.
static float noise_simplex3(float x, float y, float z) {
    float n0, n1, n2, n3;
    static const float F3 = 1.0f / 3.0f;
    static const float G3 = 1.0f / 6.0f;

    const float s = (x + y + z) * F3;
    const int i = noise_fastfloor(x + static_cast<double>(s));
    const int j = noise_fastfloor(y + static_cast<double>(s));
    const int k = noise_fastfloor(z + static_cast<double>(s));
    const float t = static_cast<float>(i + j + k) * G3;
    const float X0 = static_cast<float>(i) - t;
    const float Y0 = static_cast<float>(j) - t;
    const float Z0 = static_cast<float>(k) - t;
    const float x0 = x - X0;
    const float y0 = y - Y0;
    const float z0 = z - Z0;

    int i1, j1, k1;
    int i2, j2, k2;
    if (x0 >= y0) {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
    } else {
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
    }

    const float x1 = x0 - static_cast<float>(i1) + G3;
    const float y1 = y0 - static_cast<float>(j1) + G3;
    const float z1 = z0 - static_cast<float>(k1) + G3;
    const float x2 = x0 - static_cast<float>(i2) + 2.0f * G3;
    const float y2 = y0 - static_cast<float>(j2) + 2.0f * G3;
    const float z2 = z0 - static_cast<float>(k2) + 2.0f * G3;
    const float x3 = x0 - 1.0f + 3.0f * G3;
    const float y3 = y0 - 1.0f + 3.0f * G3;
    const float z3 = z0 - 1.0f + 3.0f * G3;

    const int ii = i & 0xff;
    const int jj = j & 0xff;
    const int kk = k & 0xff;

    const int gi0 = noise_perm(ii + noise_perm(jj + noise_perm(kk)));
    const int gi1 = noise_perm(ii + i1 + noise_perm(jj + j1 + noise_perm(kk + k1)));
    const int gi2 = noise_perm(ii + i2 + noise_perm(jj + j2 + noise_perm(kk + k2)));
    const int gi3 = noise_perm(ii + 1 + noise_perm(jj + 1 + noise_perm(kk + 1)));

    float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0.0f) n0 = 0.0f;
    else { t0 *= t0; n0 = t0 * t0 * noise_sgrad3(gi0, x0, y0, z0); }
    float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0.0f) n1 = 0.0f;
    else { t1 *= t1; n1 = t1 * t1 * noise_sgrad3(gi1, x1, y1, z1); }
    float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0.0f) n2 = 0.0f;
    else { t2 *= t2; n2 = t2 * t2 * noise_sgrad3(gi2, x2, y2, z2); }
    float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0.0f) n3 = 0.0f;
    else { t3 *= t3; n3 = t3 * t3 * noise_sgrad3(gi3, x3, y3, z3); }
    return 32.0f * (n0 + n1 + n2 + n3);
}

static glm::vec3 noise_random3(glm::vec3 p) {
    glm::vec3 val = glm::sin(glm::vec3(
        glm::dot(p, glm::vec3(127.1f, 311.7f, 74.7f)),
        glm::dot(p, glm::vec3(269.5f, 183.3f, 246.1f)),
        glm::dot(p, glm::vec3(113.5f, 271.9f, 124.6f))
    ));
    val *= 43758.5453123f;
    return glm::fract(val);
}

static float noise_mydistance(glm::vec3 a, glm::vec3 b, int t) {
    if (t == 0) {
        const float d = glm::length(a - b);
        return d * d;
    } else if (t == 1) {
        const glm::vec3 x = glm::abs(a - b);
        return std::max(std::max(x.x, x.y), x.z);
    } else {
        const glm::vec3 x = glm::abs(a - b);
        return x.x + x.y + x.z;
    }
}

static float noise_worley3(
    float px, float py, float pz, int f_type, int dist_type,
    float offset_x, float offset_y, float offset_z, float jitter
) {
    const glm::vec3 pos(px, py, pz);
    const glm::vec3 offset(offset_x, offset_y, offset_z);
    const glm::vec3 i_pos = glm::floor(pos);
    const glm::vec3 f_pos = glm::fract(pos);

    float f1 = 9e9f;
    float f2 = f1;
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                const glm::vec3 neighbor(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                glm::vec3 point = noise_random3(i_pos + neighbor);
                point = 0.5f + 0.5f * glm::sin(offset + 6.2831f * point);
                point *= jitter;
                const glm::vec3 feature = neighbor + point;
                const float dist = noise_mydistance(feature, f_pos, dist_type);
                if (dist < f1) {
                    f2 = f1;
                    f1 = dist;
                } else if (dist < f2) {
                    f2 = dist;
                }
            }
        }
    }
    return (f_type == 0) ? f1 : (f2 - f1);
}

static void impulse_tab_init(int seed, std::array<float, 256 * 4>& tab) {
    std::default_random_engine engine(seed);
    std::uniform_real_distribution<float> d(0.0f, 1.0f);
    for (int i = 0; i < 256; ++i) {
        tab[i * 4 + 0] = d(engine);
        tab[i * 4 + 1] = d(engine);
        tab[i * 4 + 2] = d(engine);
        tab[i * 4 + 3] = 1.0f - 2.0f * d(engine);
    }
}

static float quintic(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float scnoise(float x, float y, float z, int pulsenum, int seed) {
    std::array<float, 256 * 4> tab{};
    impulse_tab_init(seed, tab);

    const int ix = static_cast<int>(std::floor(x));
    const int iy = static_cast<int>(std::floor(y));
    const int iz = static_cast<int>(std::floor(z));
    const float fx = x - static_cast<float>(ix);
    const float fy = y - static_cast<float>(iy);
    const float fz = z - static_cast<float>(iz);

    float sum = 0.0f;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                int h = noise_perm(ix + i + noise_perm(iy + j + noise_perm(iz + k)));
                for (int n = pulsenum; n > 0; --n, h = (h + 1) & 255) {
                    const float* fp = &tab[h * 4];
                    const float dx = (static_cast<float>(i) - fx + fp[0]);
                    const float dy = (static_cast<float>(j) - fy + fp[1]);
                    const float dz = (static_cast<float>(k) - fz + fp[2]);
                    const float dist = dx * dx + dy * dy + dz * dz;
                    if (dist < 1.0f) sum += quintic(1.0f - std::sqrt(dist)) * fp[3];
                }
            }
        }
    }
    return (pulsenum > 0) ? (sum / static_cast<float>(pulsenum)) : 0.0f;
}

} // namespace

struct erode_noise_perlin : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim_2DGrid");
        const std::string attr_name = get_input2_string(nd, "attrName");
        const std::string attr_type = get_input2_string(nd, "attrType");
        const std::string vec3_attr_name = get_input2_string(nd, "vec3fAttrName");
        if (!terrain->has_attr(ATTR_POINT, vec3_attr_name.c_str(), ATTR_VEC3)) {
            throw std::runtime_error("no such data named `" + vec3_attr_name + "`.");
        }
        const int n = terrain->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        terrain->get_vec3f_attr(ATTR_POINT, vec3_attr_name.c_str(), pos.data(), pos.size());

        if (attr_type == "float3") {
            std::vector<Vec3f> out(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = Vec3f{
                    noise_perlin(p.x, p.y, p.z),
                    noise_perlin(p.y, p.z, p.x),
                    noise_perlin(p.z, p.x, p.y)
                };
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_vec3(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        } else {
            std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = noise_perlin(p.x, p.y, p.z);
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_float(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        }
        nd->set_output_object("prim_2DGrid", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(erode_noise_perlin,
    Z_INPUTS(
        {"prim_2DGrid", _gParamType_Geometry},
        {"vec3fAttrName", _gParamType_String, ZString("pos")},
        {"attrName", _gParamType_String, ZString("noise")},
        {"attrType", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("float", "float3")}
    ),
    Z_OUTPUTS(
        {"prim_2DGrid", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

struct erode_noise_simplex : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim_2DGrid");
        const std::string attr_name = get_input2_string(nd, "attrName");
        const std::string attr_type = get_input2_string(nd, "attrType");
        const std::string pos_like_attr_name = get_input2_string(nd, "posLikeAttrName");

        if (!terrain->has_attr(ATTR_POINT, pos_like_attr_name.c_str(), ATTR_VEC3)) {
            throw std::runtime_error("no such data name `" + pos_like_attr_name + "`.");
        }

        const int n = terrain->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        terrain->get_vec3f_attr(ATTR_POINT, pos_like_attr_name.c_str(), pos.data(), pos.size());

        if (attr_type == "float3") {
            std::vector<Vec3f> out(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                const float x = noise_simplex3(p.x, p.y, p.z);
                const float y = noise_simplex3(p.y, p.z, p.x);
                const float z = noise_simplex3(p.z, p.x, p.y);
                out[static_cast<std::size_t>(i)] = Vec3f{x, y, z};
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_vec3(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        } else {
            std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = noise_simplex3(p.x, p.y, p.z);
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_float(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        }

        nd->set_output_object("prim_2DGrid", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(erode_noise_simplex,
    Z_INPUTS(
        {"prim_2DGrid", _gParamType_Geometry},
        {"posLikeAttrName", _gParamType_String, ZString("pos")},
        {"attrName", _gParamType_String, ZString("noise")},
        {"attrType", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("float", "float3")}
    ),
    Z_OUTPUTS(
        {"prim_2DGrid", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

struct erode_noise_sparse_convolution : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim_2DGrid");
        const int pulsenum = nd->get_input2_int("pulsenum");
        const int seed = nd->get_input2_int("seed");
        const std::string attr_name = get_input2_string(nd, "attrName");
        const std::string attr_type = get_input2_string(nd, "attrType");
        const std::string pos_like_attr = get_input2_string(nd, "posLikeAttrName");
        if (!terrain->has_attr(ATTR_POINT, pos_like_attr.c_str(), ATTR_VEC3)) {
            throw std::runtime_error("no such data named `" + pos_like_attr + "`");
        }
        const int n = terrain->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        terrain->get_vec3f_attr(ATTR_POINT, pos_like_attr.c_str(), pos.data(), pos.size());

        if (attr_type == "float3") {
            std::vector<Vec3f> out(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = Vec3f{
                    scnoise(p.x, p.y, p.z, pulsenum, seed),
                    scnoise(p.y, p.z, p.x, pulsenum, seed),
                    scnoise(p.z, p.x, p.y, pulsenum, seed)
                };
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_vec3(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        } else {
            std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = scnoise(p.x, p.y, p.z, pulsenum, seed);
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_float(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        }
        nd->set_output_object("prim_2DGrid", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(erode_noise_sparse_convolution,
    Z_INPUTS(
        {"prim_2DGrid", _gParamType_Geometry},
        {"posLikeAttrName", _gParamType_String, ZString("pos")},
        {"pulsenum", _gParamType_Int, ZInt(3)},
        {"seed", _gParamType_Int, ZInt(1)},
        {"attrName", _gParamType_String, ZString("noise")},
        {"attrType", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("float", "float3")}
    ),
    Z_OUTPUTS(
        {"prim_2DGrid", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

struct erode_noise_worley : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim_2DGrid");
        const std::string pos_like_attr = get_input2_string(nd, "posLikeAttrName");
        if (!terrain->has_attr(ATTR_POINT, pos_like_attr.c_str(), ATTR_VEC3)) {
            throw std::runtime_error("no such data named `" + pos_like_attr + "`");
        }
        const std::string attr_name = get_input2_string(nd, "attrName");
        const std::string attr_type = get_input2_string(nd, "attrType");
        const float jitter = nd->get_input2_float("celljitter");
        const Vec3f seed = nd->get_input2_vec3f("seed");
        const std::string f_type_s = get_input2_string(nd, "fType");
        const std::string dist_type_s = get_input2_string(nd, "distType");
        const int f_type = (f_type_s == "F2-F1") ? 1 : 0;
        int dist_type = 0;
        if (dist_type_s == "Chebyshev") dist_type = 1;
        else if (dist_type_s == "Manhattan") dist_type = 2;

        const int n = terrain->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        terrain->get_vec3f_attr(ATTR_POINT, pos_like_attr.c_str(), pos.data(), pos.size());

        if (attr_type == "float3") {
            std::vector<Vec3f> out(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] = Vec3f{
                    noise_worley3(p.x, p.y, p.z, f_type, dist_type, seed.x, seed.y, seed.z, jitter),
                    noise_worley3(p.y, p.z, p.x, f_type, dist_type, seed.x, seed.y, seed.z, jitter),
                    noise_worley3(p.z, p.x, p.y, f_type, dist_type, seed.x, seed.y, seed.z, jitter)
                };
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_vec3(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        } else {
            std::vector<float> out(static_cast<std::size_t>(n), 0.0f);
            for (int i = 0; i < n; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                out[static_cast<std::size_t>(i)] =
                    noise_worley3(p.x, p.y, p.z, f_type, dist_type, seed.x, seed.y, seed.z, jitter);
            }
            terrain->delete_attr(ATTR_POINT, attr_name.c_str());
            terrain->create_attr_by_float(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        }
        nd->set_output_object("prim_2DGrid", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(erode_noise_worley,
    Z_INPUTS(
        {"prim_2DGrid", _gParamType_Geometry},
        {"seed", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"posLikeAttrName", _gParamType_String, ZString("pos")},
        {"celljitter", _gParamType_Float, ZFloat(1.0f)},
        {"distType", _gParamType_String, ZString("Euclidean"), Combobox, Z_STRING_ARRAY("Euclidean", "Chebyshev", "Manhattan")},
        {"fType", _gParamType_String, ZString("F1"), Combobox, Z_STRING_ARRAY("F1", "F2-F1")},
        {"attrName", _gParamType_String, ZString("noise")},
        {"attrType", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("float", "float3")}
    ),
    Z_OUTPUTS(
        {"prim_2DGrid", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

struct erode_noise_analytic_simplex_2d : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim_2DGrid");

        const std::string attr_name = get_input2_string(nd, "attrName");
        const std::string pos_like_attr_name = get_input2_string(nd, "posLikeAttrName");
        if (!terrain->has_attr(ATTR_POINT, pos_like_attr_name.c_str(), ATTR_VEC3)) {
            throw std::runtime_error("no such attr named `" + pos_like_attr_name + "`");
        }

        const int n = terrain->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        terrain->get_vec3f_attr(ATTR_POINT, pos_like_attr_name.c_str(), pos.data(), pos.size());

        std::vector<Vec3f> out(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) {
            const glm::vec3 ret = sdnoise(glm::vec2(pos[static_cast<std::size_t>(i)].x, pos[static_cast<std::size_t>(i)].z));
            out[static_cast<std::size_t>(i)] = Vec3f{ret.x, ret.y, ret.z};
        }

        terrain->delete_attr(ATTR_POINT, attr_name.c_str());
        terrain->create_attr_by_vec3(ATTR_POINT, attr_name.c_str(), out.data(), out.size());
        nd->set_output_object("prim_2DGrid", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(erode_noise_analytic_simplex_2d,
    Z_INPUTS(
        {"prim_2DGrid", _gParamType_Geometry},
        {"posLikeAttrName", _gParamType_String, ZString("pos")},
        {"attrName", _gParamType_String, ZString("analyticNoise")}
    ),
    Z_OUTPUTS(
        {"prim_2DGrid", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

} // namespace zeno

