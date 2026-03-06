// https://github.com/alembic/alembic/blob/master/lib/Alembic/AbcGeom/Tests/PolyMeshTest.cpp
// WHY THE FKING ALEMBIC OFFICIAL GIVES NO DOC BUT ONLY "TESTS" FOR ME TO LEARN THEIR FKING LIB
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include "ABCTree.h"
#include "ABCCommon.h"
#include "api_zs_alembic.h"
#include <inodeimpl.h>
#include <inodedata.h>
#include <zenum.h>
#include <zcommon.h>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <numeric>
#include <algorithm>
#include <Windows.h>
#include "format.h"

#ifdef ZENO_WITH_PYTHON
    #include <Python.h>
#endif

using namespace Alembic::AbcGeom;

namespace zeno {
void TimeAndSamplesMap::add(TimeSamplingPtr iTime, size_t iNumSamples)
{

    if (iNumSamples == 0)
    {
        iNumSamples = 1;
    }

    for (size_t i = 0; i < mTimeSampling.size(); ++i)
    {
        if (mTimeSampling[i]->getTimeSamplingType() ==
            iTime->getTimeSamplingType())
        {
            chrono_t curLastTime =
                    mTimeSampling[i]->getSampleTime(mExpectedSamples[i]);

            chrono_t lastTime = iTime->getSampleTime(iNumSamples);
            if (lastTime < curLastTime)
            {
                lastTime = curLastTime;
            }

            if (mTimeSampling[i]->getSampleTime(0) > iTime->getSampleTime(0))
            {
                mTimeSampling[i] = iTime;
            }

            mExpectedSamples[i] = mTimeSampling[i]->getNearIndex(lastTime,
                                                                 std::numeric_limits< index_t >::max()).first;

            return;
        }
    }

    mTimeSampling.push_back(iTime);
    mExpectedSamples.push_back(iNumSamples);
}

TimeSamplingPtr TimeAndSamplesMap::get(TimeSamplingPtr iTime,
                                       std::size_t & oNumSamples) const
{
    for (size_t i = 0; i < mTimeSampling.size(); ++i)
    {
        if (mTimeSampling[i]->getTimeSamplingType() ==
            iTime->getTimeSamplingType())
        {
            oNumSamples = mExpectedSamples[i];
            return mTimeSampling[i];
        }
    }

    oNumSamples = 0;
    return TimeSamplingPtr();
}
static int clamp(int i, int _min, int _max) {
    if (i < _min) {
        return _min;
    } else if (i > _max) {
        return _max;
    } else {
        return i;
    }
}

static void set_time_info(IUserData2* ud, TimeSamplingType tst, float start, int sample_count) {
    float time_per_cycle = tst.getTimePerCycle();
    if (tst.isUniform()) {
        ud->set_string("_abc_time_sampling_type", "Uniform");
    }
    else if (tst.isCyclic()) {
        ud->set_string("_abc_time_sampling_type", "Cyclic");
    }
    else if (tst.isAcyclic()) {
        ud->set_string("_abc_time_sampling_type", "Acyclic");
    }
    ud->set_float("_abc_start_time", float(start));
    ud->set_int("_abc_sample_count", sample_count);
    ud->set_float("_abc_time_per_cycle", time_per_cycle);
    if (time_per_cycle > 0) {
        ud->set_float("_abc_time_fps", 1.0f / time_per_cycle);
    }
    else {
        ud->set_float("_abc_time_fps", 0.0f);
    }
}
static void read_velocity(IGeometryObject* geom, V3fArraySamplePtr marr, bool read_done) {
    if (!geom || !marr || marr->size() == 0) return;
    std::vector<Vec3f> varr(marr->size());
    for (size_t i = 0; i < marr->size(); i++) {
        auto const& val = (*marr)[i];
        varr[i] = Vec3f{val[0], val[1], val[2]};
    }
    geom->create_attr_by_vec3(ATTR_POINT, "v", varr.data(), varr.size());
}

static void geom_set_abcpath(IGeometryObject* geom, const char* path) {
    if (geom && geom->userData()) geom->userData()->set_string("abcpath_0", path);
}

static void geom_copy_faceset_to_matid(IGeometryObject* geom) {
    if (!geom) return;
    IUserData2* ud = geom->userData();
    if (!ud) return;
    int faceset_count = ud->get_int("faceset_count", 0);
    ud->set_int("matNum", faceset_count);
    char buf[4096] = {};
    for (int i = 0; i < faceset_count; ++i) {
        char key[64];
        std::snprintf(key, sizeof(key), "faceset_%d", i);
        ud->get_string(key, "", buf, sizeof(buf));
        char mat_key[64];
        std::snprintf(mat_key, sizeof(mat_key), "Material_%d", i);
        ud->set_string(mat_key, buf);
    }
    int nfaces = geom->nfaces();
    if (nfaces > 0 && geom->has_attr(ATTR_FACE, "faceset", ATTR_INT)) {
        std::vector<int> faceset(nfaces);
        size_t got = geom->get_int_attr(ATTR_FACE, "faceset", faceset.data(), faceset.size());
        if (got == static_cast<size_t>(nfaces))
            geom->create_attr_by_int(ATTR_FACE, "matid", faceset.data(), faceset.size());
    }
}

static bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static void attr_from_data_igeom(IGeometryObject* geom, GeometryScope scope, std::string attr_name, std::vector<float>& data) {
    if (!geom) return;
    if (ends_with(attr_name, "_polys")) attr_name = attr_name.substr(0, attr_name.size() - 6);
    if (ends_with(attr_name, "_loops")) attr_name = attr_name.substr(0, attr_name.size() - 6);
    GeoAttrGroup grp = ATTR_POINT;
    int sz = geom->npoints();
    if (scope == GeometryScope::kUniformScope) { grp = ATTR_FACE; sz = geom->nfaces(); }
    else if (scope == GeometryScope::kFacevaryingScope) { grp = ATTR_VERTEX; sz = geom->nvertices(); }
    if (sz == (int)data.size()) geom->create_attr_by_float(grp, attr_name.c_str(), data.data(), data.size());
}

static void attr_from_data_igeom(IGeometryObject* geom, GeometryScope scope, std::string attr_name, std::vector<int>& data) {
    if (!geom) return;
    if (ends_with(attr_name, "_polys")) attr_name = attr_name.substr(0, attr_name.size() - 6);
    if (ends_with(attr_name, "_loops")) attr_name = attr_name.substr(0, attr_name.size() - 6);
    GeoAttrGroup grp = ATTR_POINT;
    int sz = geom->npoints();
    if (scope == GeometryScope::kUniformScope) { grp = ATTR_FACE; sz = geom->nfaces(); }
    else if (scope == GeometryScope::kFacevaryingScope) { grp = ATTR_VERTEX; sz = geom->nvertices(); }
    if (sz == (int)data.size()) geom->create_attr_by_int(grp, attr_name.c_str(), data.data(), data.size());
}

static void attr_from_data_vec_igeom(IGeometryObject* geom, GeometryScope scope, std::string attr_name, std::vector<Vec3f>& data) {
    if (!geom) return;
    GeoAttrGroup grp = ATTR_POINT;
    int sz = geom->npoints();
    if (scope == GeometryScope::kUniformScope) { grp = ATTR_FACE; sz = geom->nfaces(); }
    else if (scope == GeometryScope::kFacevaryingScope) { grp = ATTR_VERTEX; sz = geom->nvertices(); }
    if (sz == (int)data.size()) geom->create_attr_by_vec3(grp, attr_name.c_str(), data.data(), data.size());
}

#if 0
template<typename T>
void attr_from_data(PrimitiveObject* prim, GeometryScope scope, std::string attr_name, std::vector<T> &data) {
    if (scope == GeometryScope::kUniformScope) {
        if (zeno::ends_with(attr_name, "_polys")) {
            attr_name = attr_name.substr(0, attr_name.size() - 6);
        }
        if (prim->polys.size() == data.size()) {
            auto &attr = prim->polys.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->polys.size() * 2 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<2, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[2 * i], data[2 * i + 1]};
            }
        }
        else if (prim->polys.size() * 3 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<3, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            }
        }
        else if (prim->polys.size() * 4 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<4, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[4 * i], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]};
            }
        }
        else {
            log_warn("[alembic] can not load {} attr {}: {} in kUniformScope scope.", typeid(data[0]).name(), attr_name, data.size());
        }
    }
    else if (scope == GeometryScope::kFacevaryingScope) {
        if (zeno::ends_with(attr_name, "_loops")) {
            attr_name = attr_name.substr(0, attr_name.size() - 6);
        }
        if (prim->loops.size() == data.size()) {
            auto &attr = prim->loops.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->loops.size() * 2 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<2, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[2 * i], data[2 * i + 1]};
            }
        }
        else if (prim->loops.size() * 3 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<3, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            }
        }
        else if (prim->loops.size() * 4 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<4, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[4 * i], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]};
            }
        }
        else {
            log_warn("[alembic] can not load {} attr {}: {} in kFacevaryingScope scope.", typeid(data[0]).name(), attr_name, data.size());
        }
    }
    else {
        if (prim->verts.size() == data.size()) {
            auto &attr = prim->verts.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->verts.size() * 2 == data.size()) {
            auto &attr = prim->verts.add_attr<zeno::vec<2, T>>(attr_name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                attr[i] = {data[2 * i], data[2 * i + 1]};
            }
        }
        else if (prim->verts.size() * 3 == data.size()) {
            auto &attr = prim->verts.add_attr<zeno::vec<3, T>>(attr_name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                attr[i] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            }
        }
        else if (prim->verts.size() * 4 == data.size()) {
            auto &attr = prim->verts.add_attr<zeno::vec<4, T>>(attr_name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                attr[i] = {data[4 * i], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]};
            }
        }
        else if (prim->polys.size() == data.size()) {
            auto &attr = prim->polys.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->polys.size() * 2 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<2, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[2 * i], data[2 * i + 1]};
            }
        }
        else if (prim->polys.size() * 3 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<3, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            }
        }
        else if (prim->polys.size() * 4 == data.size()) {
            auto &attr = prim->polys.add_attr<zeno::vec<4, T>>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = {data[4 * i], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]};
            }
        }
        else if (prim->loops.size() == data.size()) {
            auto &attr = prim->loops.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->loops.size() * 2 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<2, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[2 * i], data[2 * i + 1]};
            }
        }
        else if (prim->loops.size() * 3 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<3, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            }
        }
        else if (prim->loops.size() * 4 == data.size()) {
            auto &attr = prim->loops.add_attr<zeno::vec<4, T>>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = {data[4 * i], data[4 * i + 1], data[4 * i + 2], data[4 * i + 3]};
            }
        }
        else {
            if (scope == GeometryScope::kVaryingScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kVaryingScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
            else if (scope == GeometryScope::kVertexScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kVertexScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
            else if (scope == GeometryScope::kUnknownScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kUnknownScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
        }
    }
}
template<typename T>
void attr_from_data_vec(PrimitiveObject* prim, GeometryScope scope, std::string attr_name, std::vector<T> &data) {
    if (scope == GeometryScope::kUniformScope) {
        if (prim->polys.size() == data.size()) {
            auto &attr = prim->polys.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = data[i];
            }
        }
        else {
            log_warn("[alembic] can not load {} attr {}: {} in kUniformScope scope.", typeid(data[0]).name(), attr_name, data.size());
        }
    }
    else if (scope == GeometryScope::kFacevaryingScope) {
        if (prim->loops.size() == data.size()) {
            auto &attr = prim->loops.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = data[i];
            }
        }
        else {
            log_warn("[alembic] can not load {} attr {}: {} in kFacevaryingScope scope.", typeid(data[0]).name(), attr_name, data.size());
        }
    }
    else {
        if (prim->verts.size() == data.size()) {
            auto &attr = prim->verts.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->polys.size() == data.size()) {
            auto &attr = prim->polys.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->polys.size(); i++) {
                attr[i] = data[i];
            }
        }
        else if (prim->loops.size() == data.size()) {
            auto &attr = prim->loops.add_attr<T>(attr_name);
            for (auto i = 0; i < prim->loops.size(); i++) {
                attr[i] = data[i];
            }
        }
        else {
            if (scope == GeometryScope::kVaryingScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kVaryingScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
            else if (scope == GeometryScope::kVertexScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kVertexScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
            else if (scope == GeometryScope::kUnknownScope) {
                log_warn("[alembic] can not load {} attr {}: {} in kUnknownScope scope.", typeid(data[0]).name(), attr_name, data.size());
            }
        }
    }
}
#endif

static void read_attributes2(IGeometryObject* geom, ICompoundProperty arbattrs, const ISampleSelector& iSS, bool read_done) {
    if (!arbattrs) {
        return;
    }
    size_t numProps = arbattrs.getNumProperties();
    for (auto i = 0; i < numProps; i++) {
        PropertyHeader p = arbattrs.getPropertyHeader(i);
        if (IFloatGeomParam::matches(p)) {
            IFloatGeomParam param(arbattrs, p.getName());

            IFloatGeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<float> data;
            data.resize(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                data[i] = samp.getVals()->get()[i];
            }
            if (!read_done) {
                //log_info("[alembic] float attr {}, len {}.", p.getName(), data.size());
            }
            attr_from_data_igeom(geom, samp.getScope(), p.getName(), data);
        }
        else if (IInt32GeomParam::matches(p)) {
            IInt32GeomParam param(arbattrs, p.getName());

            IInt32GeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<int> data;
            data.resize(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                data[i] = samp.getVals()->get()[i];
            }
            if (!read_done) {
                //log_info("[alembic] int attr {}, len {}.", p.getName(), data.size());
            }
            attr_from_data_igeom(geom, samp.getScope(), p.getName(), data);
        }
        else if (IV3fGeomParam::matches(p)) {
            IV3fGeomParam param(arbattrs, p.getName());

            IV3fGeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<Vec3f> data(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                auto v = samp.getVals()->get()[i];
                data[i] = Vec3f{v[0], v[1], v[2]};
            }
            if (!read_done) {
                //log_info("[alembic] V3f attr {}, len {}.", p.getName(), data.size());
            }
            attr_from_data_vec_igeom(geom, samp.getScope(), p.getName(), data);
        }
        else if (IN3fGeomParam::matches(p)) {
            IN3fGeomParam param(arbattrs, p.getName());

            IN3fGeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<Vec3f> data(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                auto v = samp.getVals()->get()[i];
                data[i] = Vec3f{v[0], v[1], v[2]};
            }
            if (!read_done) {
                //log_info("[alembic] N3f attr {}, len {}.", p.getName(), data.size());
            }
            attr_from_data_vec_igeom(geom, samp.getScope(), p.getName(), data);
        }
        else if (IC3fGeomParam::matches(p)) {
            IC3fGeomParam param(arbattrs, p.getName());

            IC3fGeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<Vec3f> data(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                auto v = samp.getVals()->get()[i];
                data[i] = Vec3f{v[0], v[1], v[2]};
            }
            if (!read_done) {
                //log_info("[alembic] C3f attr {}, len {}.", p.getName(), data.size());
            }
            attr_from_data_vec_igeom(geom, samp.getScope(), p.getName(), data);
        }
        else if (IC4fGeomParam::matches(p)) {
            IC4fGeomParam param(arbattrs, p.getName());

            IC4fGeomParam::Sample samp = param.getExpandedValue(iSS);
            std::vector<Vec3f> data_xyz(samp.getVals()->size());
            std::vector<float> data_w(samp.getVals()->size());
            for (auto i = 0; i < samp.getVals()->size(); i++) {
                auto v = samp.getVals()->get()[i];
                data_xyz[i] = Vec3f{v[0], v[1], v[2]};
                data_w[i] = v[3];
            }
            if (!read_done) {
                //log_info("[alembic] C4f attr {}, len {}.", p.getName(), data_xyz.size());
            }
            attr_from_data_vec_igeom(geom, samp.getScope(), p.getName() + "_rgb", data_xyz);
            attr_from_data_igeom(geom, samp.getScope(), p.getName() + "_a", data_w);
        }
        else {
            //log_info("[alembic] unknown attr {}.", p.getName());
            //zeno::log_info("getExtent {} ", p.getDataType().getExtent());
            //zeno::log_info("getNumBytes {} ", p.getDataType().getNumBytes());
            //zeno::log_info("getPod {} ", p.getDataType().getPod());
        }
    }
}

static void read_user_data(IGeometryObject* geom, ICompoundProperty arbattrs, const ISampleSelector& iSS, bool read_done) {
    if (!arbattrs) {
        return;
    }
    size_t numProps = arbattrs.getNumProperties();
    for (auto i = 0; i < numProps; i++) {
        PropertyHeader p = arbattrs.getPropertyHeader(i);
        auto propname = p.getName();
        if (IFloatProperty::matches(p)) {
            IFloatProperty param(arbattrs, p.getName());

            float v = param.getValue(iSS);
            geom->userData()->set_float(propname.c_str(), v);
        }
        else if (IInt32Property::matches(p)) {
            IInt32Property param(arbattrs, p.getName());

            int v = param.getValue(iSS);
            geom->userData()->set_int(propname.c_str(), v);
        }
        else if (IV2fProperty::matches(p)) {
            IV2fProperty param(arbattrs, p.getName());

            auto v = param.getValue(iSS);
            geom->userData()->set_vec2f(propname.c_str(), Vec2f{v[0], v[1]});
        }
        else if (IV3fProperty::matches(p)) {
            IV3fProperty param(arbattrs, p.getName());

            auto v = param.getValue(iSS);
            geom->userData()->set_vec3f(propname.c_str(), Vec3f{v[0], v[1], v[2]});
        }
        else if (IV2iProperty::matches(p)) {
            IV2iProperty param(arbattrs, p.getName());

            auto v = param.getValue(iSS);
            geom->userData()->set_vec2i(propname.c_str(), Vec2i{v[0], v[1]});
        }
        else if (IV3iProperty::matches(p)) {
            IV3iProperty param(arbattrs, p.getName());

            auto v = param.getValue(iSS);
            geom->userData()->set_vec3i(propname.c_str(), Vec3i{v[0], v[1], v[2]});
        }
        else if (IStringProperty::matches(p)) {
            IStringProperty param(arbattrs, p.getName());

            auto value = param.getValue(iSS);
            geom->userData()->set_string(propname.c_str(), value.c_str());
        }
        else if (IBoolProperty::matches(p)) {
            IBoolProperty param(arbattrs, p.getName());

            auto value = param.getValue(iSS);
            geom->userData()->set_int(propname.c_str(), int(value));
        }
        else if (IInt16Property::matches(p)) {
            IInt16Property param(arbattrs, p.getName());

            auto value = param.getValue(iSS);
            geom->userData()->set_int(propname.c_str(), int(value));
        }
        else {
            if (!read_done) {
                //log_warn("[alembic] can not load user data {}..", p.getName());
            }
        }
    }
}

static ObjectVisibility read_visible_attr(ICompoundProperty arbattrs, const ISampleSelector &iSS) {
    if (!arbattrs) {
        return ObjectVisibility::kVisibilityDeferred;
    }
    size_t numProps = arbattrs.getNumProperties();
    for (auto i = 0; i < numProps; i++) {
        PropertyHeader p = arbattrs.getPropertyHeader(i);
        if (p.getName() != "visible") {
            continue;
        }
        if (ICharProperty::matches(p)) {
            ICharProperty param(arbattrs, p.getName());

            auto value = param.getValue(iSS);
            if (value == 0) {
                return ObjectVisibility::kVisibilityHidden;
            }
            else if (value == 1) {
                return ObjectVisibility::kVisibilityVisible;
            }
            else {
                return ObjectVisibility::kVisibilityDeferred;
            }
        }
    }
    return ObjectVisibility::kVisibilityDeferred;
}

static ABCTreeUniqueGeom foundABCMesh(
        Alembic::AbcGeom::IPolyMeshSchema& mesh
        , int frameid
        , bool read_done
        , bool read_face_set
        , bool outOfRangeAsEmpty
        , std::string abc_name
) {
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;
    std::vector<int> face_indices;
    std::vector<int> face_counts;
    bool is_point = true;

    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = mesh.getTimeSampling();
    float time_per_cycle = time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle);

    int sample_index = clamp(frameid - start_frame, 0, (int)mesh.getNumSamples() - 1);
    if (outOfRangeAsEmpty && frameid - start_frame != sample_index) {
        IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
        if (geom) set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));
        return ABCTreeUniqueGeom(geom);
    }
    ISampleSelector iSS = Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index);
    Alembic::AbcGeom::IPolyMeshSchema::Sample mesamp = mesh.getValue(iSS);

    if (auto marr = mesamp.getPositions()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} positions", marr->size());
        }
        points.reserve(marr->size());
        for (size_t i = 0; i < marr->size(); i++) {
            auto const& val = (*marr)[i];
            points.emplace_back(val[0], val[1], val[2]);
        }
    }

    if (auto marr = mesamp.getFaceIndices()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} face indices", marr->size());
        }
        face_indices.reserve(marr->size());
        for (size_t i = 0; i < marr->size(); i++)
            face_indices.push_back((*marr)[i]);
    }

    if (auto marr = mesamp.getFaceCounts()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} faces", marr->size());
        }
        face_counts.reserve(marr->size());
        int base = 0;
        for (size_t i = 0; i < marr->size(); i++) {
            int cnt = (*marr)[i];
            face_counts.push_back(cnt);
            base += cnt;
            if (cnt != 1) is_point = false;
        }
    }

    if (is_point) {
        faces.clear();
    } else {
        faces.reserve(face_counts.size());
        size_t idx = 0;
        for (int cnt : face_counts) {
            std::vector<int> f;
            f.reserve(cnt);
            for (int j = 0; j < cnt && idx < face_indices.size(); j++, idx++)
                f.push_back(face_indices[idx]);
            faces.push_back(std::move(f));
        }
    }

    IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
    if (!geom) return ABCTreeUniqueGeom(nullptr);

    set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));

    if (auto nrm = mesh.getNormalsParam()) {
        auto nrmsamp = nrm.getIndexedValue(iSS);
        int value_size = (int)nrmsamp.getVals()->size();
        if (value_size == (int)points.size()) {
            std::vector<Vec3f> nrms(points.size());
            auto marr = nrmsamp.getVals();
            for (size_t i = 0; i < marr->size(); i++) {
                auto const& n = (*marr)[i];
                nrms[i] = Vec3f{n[0], n[1], n[2]};
            }
            geom->create_attr_by_vec3(ATTR_POINT, "nrm", nrms.data(), nrms.size());
        }
    }

    ICompoundProperty arbattrs = mesh.getArbGeomParams();
    read_attributes2(geom, arbattrs, iSS, read_done);
    read_user_data(geom, arbattrs, iSS, read_done);
    ICompoundProperty usrData = mesh.getUserProperties();
    read_user_data(geom, usrData, iSS, read_done);

    if (read_face_set && !is_point) {
        std::vector<int> faceset(geom->nfaces(), -1);
        IUserData2* ud = geom->userData();
        std::vector<std::string> faceSetNames;
        mesh.getFaceSetNames(faceSetNames);
        for (size_t i = 0; i < faceSetNames.size(); i++) {
            IFaceSet faceSet = mesh.getFaceSet(faceSetNames[i]);
            IFaceSetSchema::Sample faceSetSample = faceSet.getSchema().getValue();
            size_t s = faceSetSample.getFaces()->size();
            for (size_t j = 0; j < s; j++) {
                int f = faceSetSample.getFaces()->get()[j];
                if (f < (int)faceset.size()) faceset[f] = (int)i;
            }
        }
        bool found_unbind_faces = false;
        int next_faceset_index = (int)faceSetNames.size();
        for (size_t i = 0; i < faceset.size(); i++) {
            if (faceset[i] == -1) {
                found_unbind_faces = true;
                faceset[i] = next_faceset_index;
            }
        }
        if (found_unbind_faces) faceSetNames.push_back(abc_name);
        for (size_t i = 0; i < faceSetNames.size(); i++)
            ud->set_string(zeno::format("faceset_{}", i).c_str(), faceSetNames[i].c_str());
        ud->set_int("faceset_count", (int)faceSetNames.size());
        geom->create_attr_by_int(ATTR_FACE, "faceset", faceset.data(), faceset.size());
    }

    return ABCTreeUniqueGeom(geom);
}

static ABCTreeUniqueGeom foundABCSubd(Alembic::AbcGeom::ISubDSchema& subd, int frameid, bool read_done, bool read_face_set, bool outOfRangeAsEmpty) {
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;
    std::vector<int> face_indices;
    std::vector<int> face_counts;

    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = subd.getTimeSampling();
    float time_per_cycle = time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle);

    int sample_index = clamp(frameid - start_frame, 0, (int)subd.getNumSamples() - 1);
    if (outOfRangeAsEmpty && frameid - start_frame != sample_index) {
        IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
        if (geom) set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(subd.getNumSamples()));
        return ABCTreeUniqueGeom(geom);
    }
    ISampleSelector iSS = Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index);
    Alembic::AbcGeom::ISubDSchema::Sample mesamp = subd.getValue(iSS);

    if (auto marr = mesamp.getPositions()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} positions", marr->size());
        }
        points.reserve(marr->size());
        for (size_t i = 0; i < marr->size(); i++) {
            auto const& val = (*marr)[i];
            points.emplace_back(val[0], val[1], val[2]);
        }
    }

    if (auto marr = mesamp.getFaceIndices()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} face indices", marr->size());
        }
        for (size_t i = 0; i < marr->size(); i++)
            face_indices.push_back((*marr)[i]);
    }

    if (auto marr = mesamp.getFaceCounts()) {
        if (!read_done) {
            //log_debug("[alembic] totally {} faces", marr->size());
        }
        size_t base = 0;
        for (size_t i = 0; i < marr->size(); i++) {
            int cnt = (*marr)[i];
            std::vector<int> f;
            f.reserve(cnt);
            for (int j = 0; j < cnt && base + j < face_indices.size(); j++)
                f.push_back(face_indices[base + j]);
            base += cnt;
            faces.push_back(std::move(f));
        }
    }

    IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
    if (!geom) return ABCTreeUniqueGeom(nullptr);

    set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(subd.getNumSamples()));

    ICompoundProperty arbattrs = subd.getArbGeomParams();
    read_attributes2(geom, arbattrs, iSS, read_done);
    read_user_data(geom, arbattrs, iSS, read_done);
    ICompoundProperty usrData = subd.getUserProperties();
    read_user_data(geom, usrData, iSS, read_done);

    if (read_face_set) {
        std::vector<int> faceset(geom->nfaces(), -1);
        IUserData2* ud = geom->userData();
        std::vector<std::string> faceSetNames;
        subd.getFaceSetNames(faceSetNames);
        ud->set_int("faceset_count", (int)faceSetNames.size());
        for (size_t i = 0; i < faceSetNames.size(); i++) {
            ud->set_string(zeno::format("faceset_{}", i).c_str(), faceSetNames[i].c_str());
            IFaceSet faceSet = subd.getFaceSet(faceSetNames[i]);
            IFaceSetSchema::Sample faceSetSample = faceSet.getSchema().getValue();
            size_t s = faceSetSample.getFaces()->size();
            for (size_t j = 0; j < s; j++) {
                int f = faceSetSample.getFaces()->get()[j];
                if (f < (int)faceset.size()) faceset[f] = (int)i;
            }
        }
        geom->create_attr_by_int(ATTR_FACE, "faceset", faceset.data(), faceset.size());
    }

    return ABCTreeUniqueGeom(geom);
}

static std::unique_ptr<CameraInfo> foundABCCamera(Alembic::AbcGeom::ICameraSchema &cam, int frameid) {
    CameraInfo cam_info;
    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = cam.getTimeSampling();
    float time_per_cycle =  time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle );
    int sample_index = clamp(frameid - start_frame, 0, (int)cam.getNumSamples() - 1);

    auto samp = cam.getValue(Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index));
    cam_info.focal_length = samp.getFocalLength();
    cam_info._near = samp.getNearClippingPlane();
    cam_info._far = samp.getFarClippingPlane();
    cam_info.horizontalAperture = samp.getHorizontalAperture() * 10;
    cam_info.verticalAperture = samp.getVerticalAperture() * 10;
//    log_info(
//        "[alembic] Camera focal_length: {}, near: {}, far: {}",
//        cam_info.focal_length,
//        cam_info._near,
//        cam_info._far
//    );
    return std::make_unique<CameraInfo>(cam_info);
}

static Alembic::Abc::v12::M44d foundABCXform(Alembic::AbcGeom::IXformSchema &xfm, int frameid) {
    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = xfm.getTimeSampling();
    float time_per_cycle =  time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle );
    int sample_index = clamp(frameid - start_frame, 0, (int)xfm.getNumSamples() - 1);

    auto samp = xfm.getValue(Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index));
    return samp.getMatrix();
}

static ABCTreeUniqueGeom foundABCPoints(Alembic::AbcGeom::IPointsSchema& mesh, int frameid, bool read_done, bool outOfRangeAsEmpty) {
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;

    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = mesh.getTimeSampling();
    float time_per_cycle = time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle);

    int sample_index = clamp(frameid - start_frame, 0, (int)mesh.getNumSamples() - 1);
    if (outOfRangeAsEmpty && frameid - start_frame != sample_index) {
        IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
        if (geom) set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));
        return ABCTreeUniqueGeom(geom);
    }
    auto iSS = Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index);
    Alembic::AbcGeom::IPointsSchema::Sample mesamp = mesh.getValue(iSS);
    if (auto marr = mesamp.getPositions()) {
        points.reserve(marr->size());
        for (size_t i = 0; i < marr->size(); i++) {
            auto const& val = (*marr)[i];
            points.emplace_back(val[0], val[1], val[2]);
        }
    }

    IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
    if (!geom) return ABCTreeUniqueGeom(nullptr);

    set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));

    if (mesamp.getIds()) {
        auto count = mesamp.getIds()->size();
        if (count == (size_t)geom->npoints()) {
            std::vector<int> ids(count);
            for (size_t i = 0; i < count; i++)
                ids[i] = mesamp.getIds()->operator[](i);
            geom->create_attr_by_int(ATTR_POINT, "id", ids.data(), ids.size());
        }
    }
    ICompoundProperty arbattrs = mesh.getArbGeomParams();
    read_attributes2(geom, arbattrs, iSS, read_done);
    read_user_data(geom, arbattrs, iSS, read_done);
    ICompoundProperty usrData = mesh.getUserProperties();
    read_user_data(geom, usrData, iSS, read_done);
    return ABCTreeUniqueGeom(geom);
}

static ABCTreeUniqueGeom foundABCCurves(Alembic::AbcGeom::ICurvesSchema& mesh, int frameid, bool read_done, bool outOfRangeAsEmpty) {
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;

    std::shared_ptr<Alembic::AbcCoreAbstract::v12::TimeSampling> time = mesh.getTimeSampling();
    float time_per_cycle = time->getTimeSamplingType().getTimePerCycle();
    double start = time->getStoredTimes().front();
    int start_frame = std::lround(start / time_per_cycle);

    int sample_index = clamp(frameid - start_frame, 0, (int)mesh.getNumSamples() - 1);
    if (outOfRangeAsEmpty && frameid - start_frame != sample_index) {
        IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
        if (geom) set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));
        return ABCTreeUniqueGeom(geom);
    }
    auto iSS = Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index);
    Alembic::AbcGeom::ICurvesSchema::Sample mesamp = mesh.getValue(iSS);
    if (auto marr = mesamp.getPositions()) {
        points.reserve(marr->size());
        for (size_t i = 0; i < marr->size(); i++) {
            auto const& val = (*marr)[i];
            points.emplace_back(val[0], val[1], val[2]);
        }
    }

    if (auto numCurves = mesamp.getCurvesNumVertices()) {
        size_t offset = 0;
        for (size_t i = 0; i < numCurves->size(); i++) {
            int count = numCurves->operator[](i);
            for (int j = 0; j < count - 1; j++) {
                faces.push_back({(int)(offset + j), (int)(offset + j + 1)});
            }
            offset += count;
        }
    }

    IGeometryObject* geom = zeno::zs_alembic::createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
    if (!geom) return ABCTreeUniqueGeom(nullptr);

    set_time_info(geom->userData(), time->getTimeSamplingType(), start, int(mesh.getNumSamples()));

    if (auto width = mesh.getWidthsParam()) {
        auto widthsamp = width.getIndexedValue(iSS);
        int index_size = (int)widthsamp.getIndices()->size();
        if (geom->npoints() == index_size) {
            std::vector<float> width_attr(index_size);
            for (int i = 0; i < index_size; i++) {
                auto index = widthsamp.getIndices()->operator[](i);
                width_attr[i] = widthsamp.getVals()->operator[](index);
            }
            geom->create_attr_by_float(ATTR_POINT, "width", width_attr.data(), width_attr.size());
        }
    }
    ICompoundProperty arbattrs = mesh.getArbGeomParams();
    read_attributes2(geom, arbattrs, iSS, read_done);
    read_user_data(geom, arbattrs, iSS, read_done);
    ICompoundProperty usrData = mesh.getUserProperties();
    read_user_data(geom, usrData, iSS, read_done);
    return ABCTreeUniqueGeom(geom);
}

void traverseABC(
    Alembic::AbcGeom::IObject &obj,
    ABCTree &tree,
    int frameid,
    bool read_done,
    bool read_face_set,
    std::string path,
    const TimeAndSamplesMap & iTimeMap,
    ObjectVisibility parent_visible,
    bool skipInvisibleObject,
    bool outOfRangeAsEmpty,
    bool use_instance
) {
    if (use_instance) {
        tree.instanceSourcePath = obj.instanceSourcePath();
    }
    {
        auto const &md = obj.getMetaData();
        if (!read_done) {
            //log_debug("[alembic] meta data: [{}]", md.serialize());
        }
        tree.name = obj.getName();
        auto _path = zeno::format("{}/{}", path, tree.name);
        if (tree.instanceSourcePath.size()) {
            return;
        }
        path = zeno::format("{}/{}", path, tree.name);
        auto visible_prop = obj.getProperties().getPropertyHeader("visible");
        if (visible_prop) {
            size_t totalSamples = 0;
            TimeSamplingPtr timePtr =
                    iTimeMap.get(visible_prop->getTimeSampling(), totalSamples);
            float time_per_cycle = visible_prop->getTimeSampling()->getTimeSamplingType().getTimePerCycle();
            double start = visible_prop->getTimeSampling()->getStoredTimes().front();
            int start_frame = std::lround(start / time_per_cycle );

            int sample_index = clamp(frameid - start_frame, 0, (int)totalSamples - 1);
            ISampleSelector iSS = Alembic::Abc::v12::ISampleSelector((Alembic::AbcCoreAbstract::index_t)sample_index);
            auto visible = read_visible_attr(obj.getProperties(), iSS);
            if (visible != -1) {
                tree.visible = visible;
            }
            else {
                tree.visible = parent_visible;
            }
        }
        else {
            tree.visible = parent_visible;
        }
        if (!(tree.visible == ObjectVisibility::kVisibilityHidden && skipInvisibleObject)) {
        if (Alembic::AbcGeom::IPolyMesh::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found a mesh [{}]", obj.getName());
            }

            Alembic::AbcGeom::IPolyMesh meshy(obj);
            auto &mesh = meshy.getSchema();
            tree.prim = foundABCMesh(mesh, frameid, read_done, read_face_set, outOfRangeAsEmpty, obj.getName());
            if (tree.prim) {
                tree.prim->userData()->set_string("_abc_name", obj.getName().c_str());
                geom_set_abcpath(tree.prim.get(), _path.c_str());
            }
        } else if (Alembic::AbcGeom::IXformSchema::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found a Xform [{}]", obj.getName());
            }
            Alembic::AbcGeom::IXform xfm(obj);
            auto &cam_sch = xfm.getSchema();
            tree.xform = foundABCXform(cam_sch, frameid);
        } else if (Alembic::AbcGeom::ICameraSchema::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found a Camera [{}]", obj.getName());
            }
            Alembic::AbcGeom::ICamera cam(obj);
            auto &cam_sch = cam.getSchema();
            tree.camera_info = foundABCCamera(cam_sch, frameid);
        } else if(Alembic::AbcGeom::IPointsSchema::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found points [{}]", obj.getName());
            }
            Alembic::AbcGeom::IPoints points(obj);
            auto &points_sch = points.getSchema();
            tree.prim = foundABCPoints(points_sch, frameid, read_done, outOfRangeAsEmpty);
            if (tree.prim) {
                tree.prim->userData()->set_string("_abc_name", obj.getName().c_str());
                geom_set_abcpath(tree.prim.get(), _path.c_str());
                tree.prim->userData()->set_int("faceset_count", 0);
            }
        } else if(Alembic::AbcGeom::ICurvesSchema::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found curves [{}]", obj.getName());
            }
            Alembic::AbcGeom::ICurves curves(obj);
            auto &curves_sch = curves.getSchema();
            tree.prim = foundABCCurves(curves_sch, frameid, read_done, outOfRangeAsEmpty);
            if (tree.prim) {
                tree.prim->userData()->set_string("_abc_name", obj.getName().c_str());
                geom_set_abcpath(tree.prim.get(), _path.c_str());
                tree.prim->userData()->set_int("faceset_count", 0);
            }
        } else if (Alembic::AbcGeom::ISubDSchema::matches(md)) {
            if (!read_done) {
                //log_debug("[alembic] found SubD [{}]", obj.getName());
            }
            Alembic::AbcGeom::ISubD subd(obj);
            auto &subd_sch = subd.getSchema();
            tree.prim = foundABCSubd(subd_sch, frameid, read_done, read_face_set, outOfRangeAsEmpty);
            if (tree.prim) {
                tree.prim->userData()->set_string("_abc_name", obj.getName().c_str());
                geom_set_abcpath(tree.prim.get(), _path.c_str());
            }
        }
        if (tree.prim) {
            tree.prim->userData()->set_int("vis", tree.visible);
        }
    }
    }
    if (tree.prim) {
        return;
    }

    size_t nch = obj.getNumChildren();
    if (!read_done) {
        //log_debug("[alembic] found {} children", nch);
    }

    for (size_t i = 0; i < nch; i++) {
        auto const &name = obj.getChildHeader(i).getName();
        if (!read_done) {
            //log_debug("[alembic] at {} name: [{}]", i, name);
        }

        Alembic::AbcGeom::IObject child(obj, name);

        auto childTree = std::make_unique<ABCTree>();
        traverseABC(child, *childTree, frameid, read_done, read_face_set, path, iTimeMap, tree.visible, skipInvisibleObject, outOfRangeAsEmpty, use_instance);
        tree.children.push_back(std::move(childTree));
    }
}

Alembic::AbcGeom::IArchive readABC(std::string const &path) {
    std::string native_path = std::filesystem::u8path(path).string();
    std::string hdr;
    {
        char buf[5];
        std::memset(buf, 0, 5);
        auto fp = std::fopen(native_path.c_str(), "rb");
        if (!fp)
            throw Exception("[alembic] cannot open file for read: " + path);
        std::fread(buf, 4, 1, fp);
        std::fclose(fp);
        hdr = buf;
    }
    if (hdr == "\x89HDF") {
        //log_info("[alembic] opening as HDF5 format");
        return {Alembic::AbcCoreHDF5::ReadArchive(), native_path};
    } else if (hdr == "Ogaw") {
        //log_info("[alembic] opening as Ogawa format");
        return {Alembic::AbcCoreOgawa::ReadArchive(), native_path};
    } else {
        throw Exception("[alembic] unrecognized ABC header: [" + hdr + "]");
    }
}

static std::string read_alembic_get_input2_string(INodeData* nd, const char* name) {
    char buf[4096] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}
static std::vector<std::string> read_alembic_split_str(const std::string& s, std::initializer_list<char> delims) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        bool is_delim = false;
        for (char d : delims) if (c == d) { is_delim = true; break; }
        if (is_delim) {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else cur += c;
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

struct ReadAlembic : INode2 {
    Alembic::Abc::v12::IArchive archive;
    std::string usedPath;
    bool read_done = false;

    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        bool use_instance = nd->get_input2_bool("use_instance");
        int frameid = nd->has_link_input("frameid")
            ? static_cast<int>(std::lround(nd->get_input2_float("frameid")))
            : nd->GetFrameId();
        auto abctree = std::make_unique<ABCTree>();
        bool read_face_set = nd->get_input2_bool("read_face_set");
        std::string path = read_alembic_get_input2_string(nd, "path");
        if (usedPath != path) read_done = false;
        if (!read_done) {
            archive = readABC(path);
        }
        double start, _end;
        GetArchiveStartAndEndTime(archive, start, _end);
        auto obj = archive.getTop();
        bool outOfRangeAsEmpty = nd->get_input2_bool("outOfRangeAsEmpty");
        bool skipInvisibleObject = nd->get_input2_bool("skipInvisibleObject");
        Alembic::Util::uint32_t numSamplings = archive.getNumTimeSamplings();
        TimeAndSamplesMap timeMap;
        for (Alembic::Util::uint32_t s = 0; s < numSamplings; ++s)
            timeMap.add(archive.getTimeSampling(s), archive.getMaxNumSamplesForTimeSamplingIndex(s));
        traverseABC(obj, *abctree, frameid, read_done, read_face_set, "", timeMap, ObjectVisibility::kVisibilityDeferred,
                    skipInvisibleObject, outOfRangeAsEmpty, use_instance);
        read_done = true;
        usedPath = path;

        std::string namelist_str;
        char path_buf[4096] = {};
        abctree->visitPrims([&](IGeometryObject* p) {
            IUserData2* ud = p->userData();
            if (ud) {
                ud->get_string("abcpath_0", "", path_buf, sizeof(path_buf));
                namelist_str += path_buf;
                namelist_str += '\n';
            }
        });
        IUserData2* tree_ud = abctree->userData();
        if (tree_ud) {
            int n = 0;
            abctree->visitPrims([&](IGeometryObject*) { n++; });
            tree_ud->set_int("prim_count", n);
            n = 0;
            abctree->visitPrims([&](IGeometryObject* p) {
                IUserData2* ud = p->userData();
                if (ud) {
                    ud->get_string("abcpath_0", "", path_buf, sizeof(path_buf));
                    char key[32];
                    std::snprintf(key, sizeof(key), "path_%04d", n);
                    tree_ud->set_string(key, path_buf);
                    n++;
                }
            });
        }
        nd->set_output_string("namelist", namelist_str.c_str());

        if (nd->get_input2_bool("CopyFacesetToMatid") && read_face_set) {
            abctree->visitPrims([](IGeometryObject* p) {
                geom_copy_faceset_to_matid(p);
            });
        }
        nd->set_output_object("abctree", abctree.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ReadAlembic,
    Z_INPUTS(
        {"path", _gParamType_String, ZString("")},
        {"read_face_set", _gParamType_Bool, ZInt(1)},
        {"outOfRangeAsEmpty", _gParamType_Bool, ZInt(0)},
        {"skipInvisibleObject", _gParamType_Bool, ZInt(1)},
        {"CopyFacesetToMatid", _gParamType_Bool, ZInt(1)},
        {"use_instance", _gParamType_Bool, ZInt(1)},
        {"frameid", _gParamType_Float, ZFloat(0.f)}
    ),
    Z_OUTPUTS({"abctree", _gParamType_IObject}, {"namelist", _gParamType_String}),
    "alembic", "", "", "");

#if 0
std::unique_ptr<ListObject> abc_split_by_name(PrimitiveObject* prim, bool add_when_none) {
    auto list = create_ListObject();
    if (prim->verts.size() == 0) {
        return list;
    }
    int faceset_count = prim->userData()->get_int("faceset_count");
    if (add_when_none && faceset_count == 0) {
        auto name = prim->userData()->get_string("_abc_name");
        prim_set_faceset(prim, name);
        faceset_count = 1;
    }
    std::map<int, std::vector<int>> faceset_map;
    for (auto f = 0; f < faceset_count; f++) {
        faceset_map[f] = {};
    }
    if (prim->polys.size()) {
        auto &faceset = prim->polys.add_attr<int>("faceset");
        for (auto j = 0; j < faceset.size(); j++) {
            auto f = faceset[j];
            faceset_map[f].push_back(j);
        }
        for (auto f = 0; f < faceset_count; f++) {
            auto name = prim->userData()->get_string(stdString2zs(zeno::format("faceset_{}", f)));
            auto new_prim = safe_uniqueptr_cast<PrimitiveObject>(prim->clone());
            new_prim->polys.resize(faceset_map[f].size());
            for (auto i = 0; i < faceset_map[f].size(); i++) {
                new_prim->polys[i] = prim->polys[faceset_map[f][i]];
            }
            new_prim->polys.foreach_attr<AttrAcceptAll>([&](auto const &key, auto &arr) {
                using T = std::decay_t<decltype(arr[0])>;
                auto &attr = prim->polys.attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    arr[i] = attr[faceset_map[f][i]];
                }
            });
            for (auto j = 0; j < faceset_count; j++) {
                new_prim->userData()->del(stdString2zs(zeno::format("faceset_{}", j)));
            }
            prim_set_faceset(new_prim.get(), name);
            list->push_back(std::move(new_prim));
        }
    }
    else if (prim->tris.size()) {
        auto &faceset = prim->tris.add_attr<int>("faceset");
        for (auto j = 0; j < faceset.size(); j++) {
            auto f = faceset[j];
            faceset_map[f].push_back(j);
        }
        for (auto f = 0; f < faceset_count; f++) {
            auto name = prim->userData()->get_string(stdString2zs(zeno::format("faceset_{}", f)));
            auto new_prim = safe_uniqueptr_cast<PrimitiveObject>(prim->clone());
            new_prim->tris.resize(faceset_map[f].size());
            for (auto i = 0; i < faceset_map[f].size(); i++) {
                new_prim->tris[i] = prim->tris[faceset_map[f][i]];
            }
            new_prim->tris.foreach_attr<AttrAcceptAll>([&](auto const &key, auto &arr) {
                using T = std::decay_t<decltype(arr[0])>;
                auto &attr = prim->tris.attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    arr[i] = attr[faceset_map[f][i]];
                }
            });
            for (auto j = 0; j < faceset_count; j++) {
                new_prim->userData()->del(stdString2zs(zeno::format("faceset_{}", j)));
            }
            prim_set_faceset(new_prim.get(), name);
            list->push_back(std::move(new_prim));
        }
    }
    return list;
}
#endif
#if 0
struct AlembicSplitByName: INode {
    void apply() override {
        auto prim = get_input_PrimitiveObject("prim");
        int faceset_count = prim->userData()->get_int("faceset_count");
        {
            auto namelist = std::make_unique<zeno::ListObject>();
            for (auto f = 0; f < faceset_count; f++) {
                auto name = prim->userData()->get_string(stdString2zs(zeno::format("faceset_{}", f)));
                namelist->push_back(std::make_unique<StringObject>(zsString2Std(name)));
            }
            set_output("namelist", std::move(namelist));
        }

        auto dict = create_DictObject();
        auto list = abc_split_by_name(prim, false);
        auto prims = get_prims_from_list(list.get());

        for (auto& prim : prims) {
            auto name = zsString2Std(prim->userData()->get_string("faceset_0"));
            if (get_input2_bool("killDeadVerts")) {
                primKillDeadVerts(prim.get());
            }
            dict->lut[name] = std::move(prim);
        }
        set_output("dict", std::move(dict));
    }
};

ZENDEFNODE(AlembicSplitByName, {
    {
        {gParamType_Primitive, "prim"},
        {gParamType_Bool, "killDeadVerts", "1"},
    },
    {
        {gParamType_Dict,"dict"},
        {gParamType_List, "namelist"},
    },
    {},
    {"alembic"},
});
#endif
#if 0
struct CopyPosAndNrmByIndex: INode {
    void apply() override {
        auto prim = clone_input_PrimitiveObject("prim");
        auto lstobj = get_input_ListObject("list");
        auto prims = get_prims_from_list(lstobj);
        for (auto& p : prims) {
            size_t size = p->size();
            auto index = p->attr<int>("index");
            for (auto i = 0; i < size; i++) {
                prim->verts[index[i]] = p->verts[i];
            }
            if (prim->verts.attr_is<vec3f>("nrm")) {
                auto &nrm = prim->verts.attr<vec3f>("nrm");
                auto &nrm_sub = p->verts.attr<vec3f>("nrm");
                for (auto i = 0; i < size; i++) {
                    nrm[index[i]] = nrm_sub[i];
                }
            }
        }

        set_output("out", std::move(prim));
    }
};

ZENDEFNODE(CopyPosAndNrmByIndex, {
    {
        {gParamType_Primitive, "prim"},
        {gParamType_List, "list"},
    },
    {
        {gParamType_Primitive, "out"},
    },
    {},
    {"alembic"},
});

#endif

struct PrimsFilterInUserdata : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IListObject* list_in = nd->get_input_ListObject("list");
        if (!list_in) { nd->report_error("PrimsFilterInUserdata: need list"); return ZErr_ParamError; }
        std::string filter_str = read_alembic_get_input2_string(nd, "filters");
        std::vector<std::string> filters_ = read_alembic_split_str(filter_str, {' ', '\n'});
        filters_.erase(std::remove_if(filters_.begin(), filters_.end(), [](const std::string& s) { return s.empty(); }), filters_.end());

        char name_buf[256] = {};
        nd->get_input2_string("name", name_buf, sizeof(name_buf));
        const char* name = name_buf[0] ? name_buf : "abcpath_0";
        bool contain = nd->get_input2_bool("contain");
        bool fuzzy = nd->get_input2_bool("fuzzy");

        IListObject* out_list = zeno::zs_alembic::createList();
        if (!out_list) { nd->report_error("PrimsFilterInUserdata: createList failed"); return ZErr_ParamError; }
        char val_buf[4096] = {};
        for (size_t i = 0; i < list_in->size(); i++) {
            IObject2* obj = list_in->get(i);
            IUserData2* ud = obj ? obj->userData() : nullptr;
            bool this_contain = false;
            if (ud) {
                if (ud->has_string(name)) {
                    ud->get_string(name, "", val_buf, sizeof(val_buf));
                    std::string sname(val_buf);
                    if (fuzzy) {
                        for (const auto& filter : filters_) {
                            if (sname.find(filter) != std::string::npos) { this_contain = true; break; }
                        }
                    } else {
                        this_contain = std::count(filters_.begin(), filters_.end(), sname) > 0;
                    }
                } else if (ud->has_int(name)) {
                    std::string v = std::to_string(ud->get_int(name));
                    this_contain = std::count(filters_.begin(), filters_.end(), v) > 0;
                } else if (ud->has_float(name)) {
                    std::string v = std::to_string(ud->get_float(name));
                    this_contain = std::count(filters_.begin(), filters_.end(), v) > 0;
                }
            }
            bool insert = (contain && this_contain) || (!contain && !this_contain);
            if (insert && obj) {
                IObject2* clone_obj = obj->clone();
                if (clone_obj) out_list->push_back(clone_obj);
            }
        }
        nd->set_output_object("out", out_list);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimsFilterInUserdata,
    Z_INPUTS(
        {"list", _gParamType_List},
        {"name", _gParamType_String, ZString("abcpath_0")},
        {"filters", _gParamType_String, ZString("")},
        {"contain", _gParamType_Bool, ZInt(1)},
        {"fuzzy", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS({"out", _gParamType_List}),
    "alembic", "", "", "");

#if 0
#ifdef ZENO_WITH_PYTHON
static PyObject * pycheck(PyObject *pResult) {
    if (pResult == nullptr) {
        PyErr_Print();
        throw zeno::makeError("python err");
    }
    return pResult;
}

static void pycheck(int result) {
    if (result != 0) {
        PyErr_Print();
        throw zeno::makeError("python err");
    }
}
struct PrimsFilterInUserdataPython: INode {
    void apply() override {
        auto prims = get_input<ListObject>("list")->get<PrimitiveObject>();
        auto py_code = get_input2<std::string>("py_code");
        Py_Initialize();
        zeno::scope_exit init_defer([=]{ Py_Finalize(); });
        PyRun_SimpleString("import sys; sys.stderr = sys.stdout");

        auto out_list = create_ListObject();
        for (auto p: prims) {
            PyObject* userGlobals = PyDict_New();
            zeno::scope_exit userGlobals_defer([=]{ Py_DECREF(userGlobals); });

            PyObject* innerDict = PyDict_New();
            zeno::scope_exit innerDict_defer([=]{ Py_DECREF(innerDict); });

            auto &ud = p->userData();
            for (auto i = ud.begin(); i != ud.end(); i++) {
                auto key = i->first;
                if (ud.has<std::string>(key)) {
                    auto value = ud.get2<std::string>(key);
                    PyObject* pyInnerValue = PyUnicode_DecodeUTF8(key.c_str(), key.size(), "strict");
                    pycheck(PyDict_SetItemString(innerDict, key.c_str(), pyInnerValue));
                }
                else if (ud.has<float>(key)) {
                    auto value = ud.get2<float>(key);
                    PyObject* pyInnerValue = PyFloat_FromDouble(value);
                    pycheck(PyDict_SetItemString(innerDict, key.c_str(), pyInnerValue));
                }
                else if (ud.has<int>(key)) {
                    auto value = ud.get2<int>(key);
                    PyObject* pyInnerValue = PyLong_FromLong(value);
                    pycheck(PyDict_SetItemString(innerDict, key.c_str(), pyInnerValue));
                }
            }

            PyDict_SetItemString(userGlobals, "ud", innerDict);

            PyObject* pResult = pycheck(PyRun_String(py_code.c_str(), Py_file_input, userGlobals, nullptr));
            zeno::scope_exit pResult_defer([=]{ Py_DECREF(pResult); });
            PyObject* pValue = pycheck(PyRun_String("result", Py_eval_input, userGlobals, nullptr));
            zeno::scope_exit pValue_defer([=]{ Py_DECREF(pValue); });
            int need_insert = PyLong_AsLong(pValue);

            if (need_insert > 0) {
                out_list->push_back(p);
            }
        }
        set_output("out", out_list);
    }
};

ZENDEFNODE(PrimsFilterInUserdataPython, {
    {
        {gParamType_List, "list"},
        {gParamType_String, "py_code", "result = len(ud['label']) > 2", Socket_Primitve, CodeEditor},
    },
    {
        {gParamType_List, "out"},
    },
    {},
    {"alembic"},
});

#endif
struct SetFaceset: INode {
    void apply() override {
        auto prim = clone_input_PrimitiveObject("prim");
        auto faceset_name = get_input2_string("facesetName");
        prim_set_faceset(prim.get(), faceset_name);

        set_output("out", std::move(prim));
    }
};

ZENDEFNODE(SetFaceset, {
    {
        {gParamType_Primitive, "prim"},
        {gParamType_String, "facesetName", "defFS"},
    },
    {
        {gParamType_Primitive, "out"},
    },
    {},
    {"alembic"},
});

struct SetABCPath: INode {
    void apply() override {
        auto prim = clone_input_PrimitiveObject("prim");
        auto abcpathName = get_input2_string("abcpathName");
        prim_set_abcpath(prim.get(), abcpathName);
        set_output("out", std::move(prim));
    }
};

ZENDEFNODE(SetABCPath, {
    {
        {gParamType_Primitive, "prim"},
        {gParamType_String, "abcpathName", "/ABC/your_path"},
    },
    {
        {gParamType_Primitive, "out"},
    },
    {},
    {"alembic"},
});

struct PrimCopyFacesetToMatid: INode {
    void apply() override {
        auto prim = clone_input_PrimitiveObject("prim");
        prim_copy_faceset_to_matid(prim.get());

        set_output("out", std::move(prim));
    }
};

ZENDEFNODE(PrimCopyFacesetToMatid, {
    {
        {gParamType_Primitive, "prim"},
    },
    {
        {gParamType_Primitive, "out"},
    },
    {},
    {"alembic"},
});

#endif

} // namespace zeno

