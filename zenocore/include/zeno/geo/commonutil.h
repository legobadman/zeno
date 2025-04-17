#pragma once

#include <zeno/core/coredata.h>
#include <zeno/types/PrimitiveObject.h>

//abi兼容（除了PrimitiveObject...)

namespace zeno
{
    ZENO_API void primTriangulate(PrimitiveObject* prim, bool with_uv = true, bool has_lines = true, bool with_attr = true);
    ZENO_API SharedPtr<zeno::PrimitiveObject> PrimMerge(Vector<zeno::PrimitiveObject*> const& primList, String const& tagAttr = "", bool tag_on_vert = true, bool tag_on_face = false);
    ZENO_API void primPolygonate(PrimitiveObject* prim, bool with_uv = true);
    ZENO_API void primFlipFaces(PrimitiveObject* prim, bool only_face = false);
    ZENO_API void primCalcNormal(zeno::PrimitiveObject* prim, float flip = 1.0f, String nrmAttr = "nrm");
    ZENO_API zeno::Vector<SharedPtr<zeno::PrimitiveObject>> get_prims_from_list(SharedPtr<zeno::ListObject> spList);
    ZENO_API zeno::Vector<std::shared_ptr<PrimitiveObject>> PrimUnmergeFaces(PrimitiveObject* prim, String tagAttr);
    ZENO_API void primKillDeadVerts(PrimitiveObject* prim);
    ZENO_API void prim_set_abcpath(PrimitiveObject* prim, zeno::String path_name);
    ZENO_API void prim_set_faceset(PrimitiveObject* prim, zeno::String faceset_name);

    ZENO_API SharedPtr<zeno::PrimitiveObject> primMergeWithFacesetMatid(Vector<zeno::PrimitiveObject*> const& primList, String const& tagAttr = "", bool tag_on_vert = true, bool tag_on_face = false);

}