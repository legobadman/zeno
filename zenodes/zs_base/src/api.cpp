#include "api.h"
#include <Windows.h>
#include "iobject2.h"
#include "vecutil.h"
#include "utils.h"

namespace zeno {

	IGeometryObject* createGeometry(GeomTopoType type, bool bTriangle, int nPoints, int nFaces, bool bInitFaces) {
#ifdef _WIN32
		HMODULE hDll = ::LoadLibrary("zenocore.dll");
		if (hDll == INVALID_HANDLE_VALUE || hDll == 0) {
			return nullptr;
		}
		using fnCreateGeo = IGeometryObject * (__cdecl*)(GeomTopoType, bool, int, int, bool);
		auto fCall = (fnCreateGeo)GetProcAddress(hDll, "createGeometry");
		if (fCall) {
			return fCall(type, bTriangle, nPoints, nFaces, bInitFaces);
		}
#else
		//TODO:
#endif
		return nullptr;
	}

	IGeometryObject* create_GeometryObject(
		GeomTopoType type,
		bool bTriangle,
		const std::vector<vec3f>& points,
		const std::vector<std::vector<int>>& faces)
	{
		const Vec3f* abiPoints = nullptr;
		size_t pointCount = points.size();

		if (!points.empty()) {
			Vec3f* p = new Vec3f[pointCount];
			for (size_t i = 0; i < pointCount; ++i) {
				p[i] = toAbiVec3f(points[i]);
			}
			abiPoints = p;
		}

		// ---- faces ----
		ZIntArray* abiFaces = nullptr;
		size_t faceCount = faces.size();

		if (!faces.empty()) {
			abiFaces = new ZIntArray[faceCount];

			for (size_t i = 0; i < faceCount; ++i) {
				const auto& f = faces[i];

				abiFaces[i].size = (int)f.size();

				if (!f.empty()) {
					abiFaces[i].arr = new int[f.size()];
					memcpy(abiFaces[i].arr, f.data(), f.size() * sizeof(int));
				}
				else {
					abiFaces[i].arr = nullptr;
				}
			}
		}


		// ---- scope cleanup ----
		auto cleanup = zeno::scope_exit([&]() {
			delete[] abiPoints;

			if (abiFaces) {
				for (size_t i = 0; i < faceCount; ++i) {
					delete[] abiFaces[i].arr;
				}
				delete[] abiFaces;
			}
			});

#ifdef _WIN32
		HMODULE hDll = ::LoadLibrary("zenocore.dll");
		if (hDll == INVALID_HANDLE_VALUE || hDll == 0) {
			return nullptr;
		}
		using fnCreateGeo = IGeometryObject * (__cdecl*)(GeomTopoType, bool, const zeno::Vec3f*, size_t, const ZIntArray*, size_t);
		auto fCall = (fnCreateGeo)GetProcAddress(hDll, "createGeometryByPointFace");
		if (fCall) {
			return fCall(type, bTriangle, abiPoints, pointCount, abiFaces, faceCount);
		}
#else
		//TODO:
#endif
	}

	IGeometryObject* mergeObjects(
		IListObject* spList,
		const char* tagAtt,
		bool tag_on_vert,
		bool tag_on_face)
	{
#ifdef _WIN32
		HMODULE hDll = ::LoadLibrary("zenocore.dll");
		if (hDll == INVALID_HANDLE_VALUE || hDll == 0) {
			return nullptr;
		}
		using fnMergeGeo = IGeometryObject* (__cdecl*)(IListObject*, const char*, bool, bool);
		auto fCall = (fnMergeGeo)GetProcAddress(hDll, "mergeObjects");
		if (fCall) {
			return fCall(spList, tagAtt, tag_on_vert, tag_on_face);
		}
#else
		//TODO:
#endif
		return nullptr;
	}

}