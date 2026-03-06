#if 0
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include "ABCCommon.h"
#include "ABCTree.h"
#include "api_zs_alembic.h"
#include "Alembic/Abc/IObject.h"
#include <filesystem>
#include <iobject2.h>
#include <zenum.h>
#include <zvec.h>
#include <zcommon.h>
#include <inodeimpl.h>
#include <inodedata.h>
#include <glm/glm.hpp>

namespace zeno {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[1024] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

/** Min/max over vector<Vec3f> without zenocore parallel_reduce. */
static std::pair<zeno::Vec3f, zeno::Vec3f> minmax_vec3f(
    const std::vector<zeno::Vec3f>& vec) {
    if (vec.empty()) return {zeno::Vec3f(), zeno::Vec3f()};
    zeno::Vec3f lo(vec[0].x, vec[0].y, vec[0].z);
    zeno::Vec3f hi(lo);
    for (size_t i = 1; i < vec.size(); i++) {
        for (int c = 0; c < 3; c++) {
            float v = vec[i][c];
            if (v < lo[c]) lo[c] = v;
            if (v > hi[c]) hi[c] = v;
        }
    }
    return {lo, hi};
}

struct ListDeleter { void operator()(IListObject* p) const { if (p) p->Delete(); } };

bool SaveEXR(const float* rgb, size_t width, size_t height, const char* outfilename) {
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    // Split RGBRGBRGB... into R, G and B layer
    for (int i = 0; i < width * height; i++) {
        images[0][i] = rgb[3*i+0];
        images[1][i] = rgb[3*i+1];
        images[2][i] = rgb[3*i+2];
    }

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0)); // B
    image_ptr[1] = &(images[1].at(0)); // G
    image_ptr[2] = &(images[0].at(0)); // R

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err = nullptr; // or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
        fprintf(stderr, "Save EXR err: %s\n", err);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return ret;
    }
    printf("Saved exr file. [ %s ] \n", outfilename);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

static glm::vec3 normalized_vec3(glm::vec3 vec, glm::vec3 _min, glm::vec3 _max) {
    return (vec - _min) / (_max - _min);
}

int align_to(int count, int align) {
    int remainder = count % align;
    if (remainder == 0) {
        return count;
    }
    else {
        return count + (align - remainder);
    }
}

static void geom_calc_normal(zeno::IGeometryObject* geom) {
    int np = geom->npoints();
    int nf = geom->nfaces();
    if (np <= 0 || nf <= 0) return;
    std::vector<zeno::Vec3f> nrm((size_t)np, zeno::Vec3f(0, 0, 0));
    std::vector<int> pts(4);
    for (int f = 0; f < nf; f++) {
        size_t n = geom->face_points(f, pts.data(), 4);
        if (n < 3) continue;
        zeno::Vec3f fn = geom->face_nrm(f);
        for (size_t i = 0; i < n; i++)
            nrm[(size_t)pts[i]] = nrm[(size_t)pts[i]] + fn;
    }
    for (auto& v : nrm) {
        float len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        if (len > 1e-8f) { v[0] /= len; v[1] /= len; v[2] /= len; }
    }
    geom->create_attr_by_vec3(zeno::ATTR_POINT, "nrm", nrm.data(), (size_t)np);
}

void writeObjFile(
    const zeno::IGeometryObject* geom,
    const char *path,
    int32_t frameNum,
    const std::pair<zeno::Vec3f, zeno::Vec3f>& bbox
)
{
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        abort();
    }

    fprintf(fp, "# Zeno generated from an alembic file.\n");

    int npts = geom->npoints();
    int nf = geom->nfaces();
    std::vector<zeno::Vec3f> vertices((size_t)npts);
    geom->points_pos(vertices.data(), (size_t)npts);

    size_t vatWidth = std::min((size_t)npts, (size_t)8192);
    auto rowsPerFrame = static_cast<int32_t>(std::ceil((float)npts / (float)vatWidth));
    size_t vatHeight = rowsPerFrame * frameNum;
    fprintf(fp, "# metadata VATWidth %zu\n", vatWidth);
    fprintf(fp, "# metadata RowsPerFrame %d\n", rowsPerFrame);
    fprintf(fp, "# metadata FrameNum %d\n", frameNum);
    fprintf(fp, "# metadata VATHeight %zu\n", vatHeight);
    fprintf(fp, "# metadata BMin %f %f %f\n", bbox.first[0], bbox.first[1], bbox.first[2]);
    fprintf(fp, "# metadata BMax %f %f %f\n", bbox.second[0], bbox.second[1], bbox.second[2]);

    const auto map_into_bbox = [&bbox](const zeno::Vec3f& v) {
        return zeno::Vec3f(v[0], v[1], v[2]);
    };

    for (int i = 0; i < npts; i++) {
        const auto v = map_into_bbox(vertices[(size_t)i]);
        fprintf(fp, "v %f %f %f\n", v[0], v[1], v[2]);
    }

    std::vector<std::pair<float, float>> vatUvMap;
    for (size_t i = 0; i < (size_t)npts; i++) {
        float u1 = (float(i % vatWidth)) / (float)vatWidth;
        float u2 = (float((i + 1) % vatWidth)) / (float)vatWidth;
        if (u1 > 1.0f) { u1 -= 1.0f; u2 -= 1.0f; }
        if (u1 > u2) u2 = 1.0f;
        float v1 = std::floor((float)i / (float)vatWidth) / (float)vatHeight;
        float v2 = std::floor(float(i + vatWidth) / (float)vatWidth) / (float)vatHeight;
        vatUvMap.emplace_back((u1 + u2) * 0.5f, std::min((v1 + v2) * 0.5f, 1.0f));
        fprintf(fp, "vn %.10f %.10f %.10f\n", vatUvMap[i].first, vatUvMap[i].second, 0.0f);
    }

    bool has_uv = geom->has_attr(zeno::ATTR_FACE, "uv0", zeno::ATTR_VEC3) &&
                  geom->has_attr(zeno::ATTR_FACE, "uv1", zeno::ATTR_VEC3) &&
                  geom->has_attr(zeno::ATTR_FACE, "uv2", zeno::ATTR_VEC3);
    std::vector<zeno::Vec3f> uv0_f, uv1_f, uv2_f;
    if (has_uv && nf > 0) {
        uv0_f.resize((size_t)nf);
        uv1_f.resize((size_t)nf);
        uv2_f.resize((size_t)nf);
        geom->get_vec3f_attr(zeno::ATTR_FACE, "uv0", uv0_f.data(), (size_t)nf);
        geom->get_vec3f_attr(zeno::ATTR_FACE, "uv1", uv1_f.data(), (size_t)nf);
        geom->get_vec3f_attr(zeno::ATTR_FACE, "uv2", uv2_f.data(), (size_t)nf);
    }

    std::vector<int> pts(4);
    for (int32_t count = 0; count < nf; count++) {
        size_t n = geom->face_points(count, pts.data(), 4);
        if (n < 3) continue;
        const int32_t v0 = pts[0], v1 = pts[1], v2 = pts[2];
        const int32_t ui0 = count * 3 + 1, ui1 = count * 3 + 2, ui2 = count * 3 + 3;
        if (has_uv && (size_t)count < uv0_f.size()) {
            fprintf(fp, "vt %.10f %.10f\n", uv0_f[(size_t)count][0], uv0_f[(size_t)count][1]);
            fprintf(fp, "vt %.10f %.10f\n", uv1_f[(size_t)count][0], uv1_f[(size_t)count][1]);
            fprintf(fp, "vt %.10f %.10f\n", uv2_f[(size_t)count][0], uv2_f[(size_t)count][1]);
        } else {
            fprintf(fp, "vt 0 0\nvt 0 0\nvt 0 0\n");
        }
        fprintf(fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
            v0 + 1, ui0, v0 + 1, v1 + 1, ui1, v1 + 1, v2 + 1, ui2, v2 + 1);
    }
    fclose(fp);
}

struct AlembicToSoftBodyVAT : INode2 {
    DEF_OVERRIDE_FOR_INODE
    Alembic::Abc::v12::IArchive archive;
    bool read_done = false;

    ZErrorCode apply(INodeData* nd) override {
        int frameStart = nd->get_input2_int("frameStart");
        int frameEnd = nd->get_input2_int("frameEnd");
        if (frameEnd < frameStart) std::swap(frameEnd, frameStart);
        int frameNum = frameEnd - frameStart;
        if (frameNum <= 0) {
            nd->report_error("AlembicToSoftBodyVAT: invalid frame range");
            return ZErr_ParamError;
        }
        bool use_xform = nd->get_input2_bool("useXForm");
        std::string writePath = get_input2_string(nd, "outputPath");
        std::string path = get_input2_string(nd, "path");
        size_t vatWidth;
        int32_t rowsPerFrame;
        size_t spaceToAlign;
        std::vector<zeno::Vec3f> temp_bboxs;
        size_t vatHeight;
        {
            if (!read_done) {
                archive = readABC(path);
            }
            double _start, _end;
            GetArchiveStartAndEndTime(archive, _start, _end);
            TimeAndSamplesMap timeMap;
            Alembic::Util::uint32_t numSamplings = archive.getNumTimeSamplings();
            for (Alembic::Util::uint32_t s = 0; s < numSamplings; ++s) {
                timeMap.add(archive.getTimeSampling(s),
                            archive.getMaxNumSamplesForTimeSamplingIndex(s));
            }
            auto obj = archive.getTop();
            std::vector<float> pos_f32;
            std::vector<float> nrm_f32;
            IListObject* frameListRaw = zeno::zs_alembic::createList();
            std::unique_ptr<IListObject, ListDeleter> frameList(frameListRaw);
            if (!frameList) {
                nd->report_error("AlembicToSoftBodyVAT: createList failed");
                return ZErr_ParamError;
            }
            for (int32_t idx = frameStart; idx < frameEnd; ++idx) {
                const int32_t frameIndex = frameEnd - idx - 1;
                auto abctree = std::make_unique<ABCTree>();
                traverseABC(obj, *abctree, idx, read_done, false, "", timeMap, ObjectVisibility::kVisibilityDeferred, false, false, 0);
                std::vector<zeno::IGeometryObject*> geomlst;
                if (use_xform) {
                    IListObject* prims = get_xformed_prims_igeom(abctree.get());
                    if (prims) {
                        for (size_t i = 0; i < prims->size(); i++)
                            geomlst.push_back(dynamic_cast<zeno::IGeometryObject*>(prims->get(i)));
                        prims->Delete();
                    }
                } else {
                    abctree->visitPrims([&](zeno::IGeometryObject* p) {
                        geomlst.push_back(p);
                    });
                }
                zeno::IGeometryObject* mergedRaw = zeno::zs_alembic::PrimMerge(geomlst);
                std::unique_ptr<zeno::IGeometryObject, zeno::ABCTreeGeomDeleter> mergedGeom(mergedRaw);
                if (!mergedGeom) continue;
                if (nd->get_input2_bool("flipFrontBack")) {
                    auto* g = zeno::zs_alembic::primFlipFaces(mergedGeom.get());
                    if (g != mergedGeom.get()) mergedGeom.reset(g);
                }
                { auto* g = zeno::zs_alembic::primTriangulate(mergedGeom.get()); if (g != mergedGeom.get()) mergedGeom.reset(g); }
                frameList->push_back(mergedGeom->clone());
                std::vector<zeno::Vec3f> pos((size_t)mergedGeom->npoints());
                mergedGeom->points_pos(pos.data(), pos.size());
                auto bbox = minmax_vec3f(pos);
                temp_bboxs.push_back(bbox.first);
                temp_bboxs.push_back(bbox.second);
                nd->set_output_object("primitive", mergedGeom.release());
            }
            // reduce bbox_temp to actual bbox
            auto bbox = minmax_vec3f(temp_bboxs);
            read_done = true;
            for (int32_t idx = frameStart; idx < frameEnd; ++idx) {
                const int32_t frameIndex = idx - frameStart;
                auto* mergedGeom = dynamic_cast<zeno::IGeometryObject*>(frameList->get(frameIndex));
                if (!mergedGeom) continue;
                int npts = mergedGeom->npoints();
                // Save first frame mesh to obj
                if (frameIndex == 0) {
                    vatWidth = std::min((size_t)npts, (size_t)8192);
                    rowsPerFrame = static_cast<int32_t>(std::ceil((float)npts / (float)vatWidth));
                    vatHeight = rowsPerFrame * frameNum;
                    spaceToAlign = vatWidth * rowsPerFrame - (size_t)npts;
                    std::string objPath = writePath + ".obj";
                    if (std::filesystem::exists(objPath)) {
                        std::filesystem::remove(objPath);
                    }
                    writeObjFile(mergedGeom, objPath.c_str(), frameNum, bbox);
                }
                // Save other frames to vat - Position
                std::vector<zeno::Vec3f> verts((size_t)npts);
                mergedGeom->points_pos(verts.data(), (size_t)npts);
                for (int i = 0; i < npts; i++) {
                    glm::vec3 v(verts[(size_t)i][0], verts[(size_t)i][1], verts[(size_t)i][2]);
                    glm::vec3 gmin(bbox.first[0], bbox.first[1], bbox.first[2]);
                    glm::vec3 gmax(bbox.second[0], bbox.second[1], bbox.second[2]);
                    glm::vec3 vec = normalized_vec3(v, gmin, gmax);
                    pos_f32.push_back(vec.x);
                    pos_f32.push_back(vec.y);
                    pos_f32.push_back(vec.z);
                }
                for (size_t i = 0; i < spaceToAlign; ++i) {
                    pos_f32.push_back(0.0f);
                    pos_f32.push_back(0.0f);
                    pos_f32.push_back(0.0f);
                }
                geom_calc_normal(mergedGeom);
                std::vector<zeno::Vec3f> nrm_ref((size_t)npts);
                mergedGeom->get_vec3f_attr(zeno::ATTR_POINT, "nrm", nrm_ref.data(), (size_t)npts);
                for (int i = 0; i < npts; i++) {
                    nrm_f32.push_back(nrm_ref[(size_t)i][0]);
                    nrm_f32.push_back(nrm_ref[(size_t)i][1]);
                    nrm_f32.push_back(nrm_ref[(size_t)i][2]);
                }
                for (size_t i = 0; i < spaceToAlign; ++i) {
                    nrm_f32.push_back(0.0f);
                    nrm_f32.push_back(0.0f);
                    nrm_f32.push_back(0.0f);
                }
            }
            std::string posPath = writePath + "-position-texture.exr";
            if (std::filesystem::exists(posPath)) {
                std::filesystem::remove(posPath);
            }
            std::string nrmPath = writePath + "-normal-texture.exr";
            if (std::filesystem::exists(nrmPath)) {
                std::filesystem::remove(nrmPath);
            }
            SaveEXR(pos_f32.data(), vatWidth, vatHeight, posPath.c_str());
            SaveEXR(nrm_f32.data(), vatWidth, vatHeight, nrmPath.c_str());
        }
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(AlembicToSoftBodyVAT,
    Z_INPUTS(
        {"path", _gParamType_String, ZString(""), ReadPathEdit},
        {"outputPath", _gParamType_String, ZString(""), WritePathEdit},
        {"useXForm", _gParamType_Bool, ZInt(1)},
        {"flipFrontBack", _gParamType_Bool, ZInt(1)},
        {"frameStart", _gParamType_Int, ZInt(0)},
        {"frameEnd", _gParamType_Int, ZInt(1)}
    ),
    Z_OUTPUTS({"primitive", _gParamType_Geometry}),
    "alembic",
    "primitive",
    "",
    ""
);

void writeDynamicRemeshObjFile(
  const char *path,
  int32_t frameNum,
  const std::pair<zeno::Vec3f, zeno::Vec3f>& bbox,
  size_t triNum
) {

  FILE *fp = fopen(path, "w");
  if (!fp) {
    perror(path);
    abort();
  }

  fprintf(fp, "# Zeno generated from an alembic file.\n");

  const size_t vertNum = triNum * 3;

  size_t vatWidth = std::min(vertNum, (size_t)8192);
  auto rowsPerFrame = static_cast<int32_t>(std::ceil((float)vertNum / (float)vatWidth));
  size_t vatHeight = rowsPerFrame * frameNum;
  fprintf(fp, "# metadata VATWidth %d\n", vatWidth);
  fprintf(fp, "# metadata RowsPerFrame %d\n", rowsPerFrame);
  fprintf(fp, "# metadata FrameNum %d\n", frameNum);
  fprintf(fp, "# metadata VATHeight %d\n", vatHeight);
  fprintf(fp, "# metadata BMin %f %f %f\n", bbox.first[0], bbox.first[1], bbox.first[2]);
  fprintf(fp, "# metadata BMax %f %f %f\n", bbox.second[0], bbox.second[1], bbox.second[2]);

  constexpr float scale = 0.01f;

  const float center = float(triNum) * scale * 0.5f;

  for (size_t idx = 0; idx < triNum; ++idx) {
    float x = float(idx) * scale - center;
    size_t idxBase = idx * 3 + 1;


    auto outputUV = [&](size_t vertId) {
      float u1 = float(vertId % vatWidth) / float(vatWidth);
      float u2 = float((vertId + 1) % vatWidth) / float(vatWidth);
      if (u1 > 1.0f) {
        u1 -= 1.0f;
        u2 -= 1.0f;
      }
      if (u1 > u2) u2 = 1.0f;
      float u = (u1 + u2) * 0.5f;
      float v1 = std::floor((float)vertId / (float)vatWidth) / (float)vatHeight;
      float v2 = std::floor(float(vertId + vatWidth) / (float)vatWidth) / (float)vatHeight;
      float v = std::min((v1 + v2) * 0.5f, 1.0f);

      fprintf(fp, "vt %.10f %.10f\n", u, v);
    };

    outputUV(idxBase - 1);
    outputUV(idxBase);
    outputUV(idxBase + 1);

    fprintf(fp, "v %f %f %f\n", x, x, x);
    fprintf(fp, "v %f %f %f\n", x + 0.25, x, x + 0.25);
    fprintf(fp, "v %f %f %f\n", x - 0.25, x, x - 0.25);
    fprintf(fp, "f %d/%d %d/%d %d/%d\n", idxBase, idxBase, idxBase + 1, idxBase + 1, idxBase + 2, idxBase + 2);
  }
  fclose(fp);
}

struct AlembicToDynamicRemeshVAT : INode2 {
    DEF_OVERRIDE_FOR_INODE
    Alembic::Abc::v12::IArchive archive;
    bool read_done = false;

    ZErrorCode apply(INodeData* nd) override {
      int frameStart = nd->get_input2_int("frameStart");
      int frameEnd = nd->get_input2_int("frameEnd");
      if (frameEnd < frameStart) std::swap(frameEnd, frameStart);
      int frameNum = frameEnd - frameStart;
      if (frameNum <= 0) {
        nd->report_error("AlembicToDynamicRemeshVAT: invalid frame range");
        return ZErr_ParamError;
      }
      bool use_xform = nd->get_input2_bool("useXForm");
      std::string writePath = get_input2_string(nd, "outputPath");
      std::string path = get_input2_string(nd, "path");
      bool shouldFlipFrontBack = nd->get_input2_bool("flipFrontBack");

      std::vector<float> pos_f32;
      std::vector<float> nrm_f32;
      std::vector<zeno::Vec3f> temp_bboxs;

      if (!read_done) archive = readABC(path);
      TimeAndSamplesMap timeMap;
      Alembic::Util::uint32_t numSamplings = archive.getNumTimeSamplings();
      for (Alembic::Util::uint32_t s = 0; s < numSamplings; ++s) {
        timeMap.add(archive.getTimeSampling(s),
                    archive.getMaxNumSamplesForTimeSamplingIndex(s));
      }

      auto obj = archive.getTop();
      IListObject* frameListRaw = zeno::zs_alembic::createList();
      std::unique_ptr<IListObject, ListDeleter> frameList(frameListRaw);
      if (!frameList) {
        nd->report_error("AlembicToDynamicRemeshVAT: createList failed");
        return ZErr_ParamError;
      }
      size_t maxTriNum = 0;
      for (int32_t idx = frameStart; idx < frameEnd; ++idx) {
        const int32_t frameIndex = frameEnd - idx - 1;
        auto abctree = std::make_unique<ABCTree>();
        traverseABC(obj, *abctree, idx, read_done, false, "", timeMap, ObjectVisibility::kVisibilityDeferred, false, false, 0);
        std::vector<zeno::IGeometryObject*> geomlst;
        if (use_xform) {
          IListObject* prims = get_xformed_prims_igeom(abctree.get());
          if (prims) {
            for (size_t i = 0; i < prims->size(); i++)
              geomlst.push_back(dynamic_cast<zeno::IGeometryObject*>(prims->get(i)));
            prims->Delete();
          }
        } else {
          abctree->visitPrims([&](zeno::IGeometryObject* p) { geomlst.push_back(p); });
        }
        zeno::IGeometryObject* mergedRaw = zeno::zs_alembic::PrimMerge(geomlst);
        std::unique_ptr<zeno::IGeometryObject, zeno::ABCTreeGeomDeleter> mergedGeom(mergedRaw);
        if (!mergedGeom) continue;
        if (shouldFlipFrontBack) { auto* g = zeno::zs_alembic::primFlipFaces(mergedGeom.get()); if (g != mergedGeom.get()) mergedGeom.reset(g); }
        { auto* g = zeno::zs_alembic::primTriangulate(mergedGeom.get()); if (g != mergedGeom.get()) mergedGeom.reset(g); }
        maxTriNum = std::max((size_t)mergedGeom->nfaces(), maxTriNum);
        frameList->push_back(mergedGeom->clone());
        std::vector<zeno::Vec3f> pos((size_t)mergedGeom->npoints());
        mergedGeom->points_pos(pos.data(), pos.size());
        auto bbox = minmax_vec3f(pos);
        temp_bboxs.push_back(bbox.first);
        temp_bboxs.push_back(bbox.second);
        nd->set_output_object("primitive", mergedGeom.release());
      }
      auto bbox = minmax_vec3f(temp_bboxs);
      read_done = true;

      writeDynamicRemeshObjFile(writePath.c_str(), frameNum, bbox, maxTriNum);

      const size_t vertNum = maxTriNum * 3;
      size_t vatWidth = std::min(vertNum, (size_t)8192);
      auto rowsPerFrame = static_cast<int32_t>(std::ceil((float)vertNum / (float)vatWidth));
      size_t vatHeight = rowsPerFrame * frameNum;
      size_t spaceToAlign = -1;

      for (int32_t idx = frameStart; idx < frameEnd; ++idx) {
        const int32_t frameIndex = idx - frameStart;
        auto* mergedGeom = dynamic_cast<zeno::IGeometryObject*>(frameList->get(frameIndex));
        if (!mergedGeom) continue;
        int nf = mergedGeom->nfaces();
        int npts = mergedGeom->npoints();
        spaceToAlign = vatWidth * rowsPerFrame - (size_t)nf * 3;
        geom_calc_normal(mergedGeom);
        std::vector<zeno::Vec3f> verts((size_t)npts);
        std::vector<zeno::Vec3f> nrm_ref((size_t)npts);
        mergedGeom->points_pos(verts.data(), (size_t)npts);
        mergedGeom->get_vec3f_attr(zeno::ATTR_POINT, "nrm", nrm_ref.data(), (size_t)npts);
        std::vector<int> pts(4);
        for (int t = 0; t < nf; t++) {
          size_t n = mergedGeom->face_points(t, pts.data(), 4);
          if (n < 3) continue;
          for (int i = 0; i < 3; i++) {
            int vi = pts[i];
            glm::vec3 v(verts[(size_t)vi][0], verts[(size_t)vi][1], verts[(size_t)vi][2]);
            glm::vec3 gmin(bbox.first[0], bbox.first[1], bbox.first[2]);
            glm::vec3 gmax(bbox.second[0], bbox.second[1], bbox.second[2]);
            glm::vec3 vec = normalized_vec3(v, gmin, gmax);
            pos_f32.push_back(vec.x);
            pos_f32.push_back(vec.y);
            pos_f32.push_back(vec.z);
            nrm_f32.push_back(nrm_ref[(size_t)vi][0]);
            nrm_f32.push_back(nrm_ref[(size_t)vi][1]);
            nrm_f32.push_back(nrm_ref[(size_t)vi][2]);
          }
        }
        for (size_t i = 0; i < spaceToAlign; ++i) {
          pos_f32.push_back(0.0f);
          pos_f32.push_back(0.0f);
          pos_f32.push_back(0.0f);
          nrm_f32.push_back(0.0f);
          nrm_f32.push_back(0.0f);
          nrm_f32.push_back(0.0f);
        }
      }

      std::string posPath = writePath + "-position-texture.exr";
      if (std::filesystem::exists(posPath)) {
        std::filesystem::remove(posPath);
      }
      std::string nrmPath = writePath + "-normal-texture.exr";
      if (std::filesystem::exists(nrmPath)) {
        std::filesystem::remove(nrmPath);
      }
      SaveEXR(pos_f32.data(), vatWidth, vatHeight, posPath.c_str());
      SaveEXR(nrm_f32.data(), vatWidth, vatHeight, nrmPath.c_str());
      return ZErr_OK;
    }
};

ZENDEFNODE_ABI(AlembicToDynamicRemeshVAT,
  Z_INPUTS(
    {"path", _gParamType_String, ZString(""), ReadPathEdit},
    {"outputPath", _gParamType_String, ZString(""), WritePathEdit},
    {"useXForm", _gParamType_Bool, ZInt(1)},
    {"flipFrontBack", _gParamType_Bool, ZInt(1)},
    {"frameStart", _gParamType_Int, ZInt(0)},
    {"frameEnd", _gParamType_Int, ZInt(1)}
  ),
  Z_OUTPUTS({"primitive", _gParamType_Geometry}),
  "alembic",
  "primitive",
  "",
  ""
);

}
#endif