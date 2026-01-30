#include <zeno/geo/commonutil.h>
#include <zeno/para/parallel_for.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/fileio.h>
#include <zeno/para/parallel_scan.h>
#include <zeno/para/parallel_reduce.h>
#include <zeno/types/UserData.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/para/task_group.h>
#include <zeno/utils/arrayindex.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <zeno/utils/orthonormal.h>
#include <zeno/utils/overloaded.h>
#include <zeno/utils/ticktock.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/wangsrng.h>
#include <zeno/utils/tuple_hash.h>
#include <stdexcept>
#include <filesystem>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <tinygltf/stb_image.h>
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include "zeno/utils/string.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/stb_image_write.h>


namespace zeno
{
    template <class T>
    static void revamp_vector(std::vector<T>& arr, std::vector<int> const& revamp) {
        std::vector<T> newarr(arr.size());
        for (int i = 0; i < revamp.size(); i++) {
            newarr[i] = arr[revamp[i]];
        }
        std::swap(arr, newarr);
    }

    template <typename DstT, typename SrcT> constexpr auto reinterpret_bits(SrcT &&val) {
        using Src = std::remove_cv_t<std::remove_reference_t<SrcT>>;
        using Dst = std::remove_cv_t<std::remove_reference_t<DstT>>;
        static_assert(sizeof(Src) == sizeof(Dst),
                      "Source Type and Destination Type must be of the same size");
        static_assert(std::is_trivially_copyable_v<Src> && std::is_trivially_copyable_v<Dst>,
                      "Both types should be trivially copyable.");
        static_assert(std::alignment_of_v<Src> % std::alignment_of_v<Dst> == 0,
                      "The original type should at least have an alignment as strict.");
        Dst dst{};
        std::memcpy(&dst, const_cast<const Src *>(&val), sizeof(Dst));
        return dst;
    }

    static void set_special_attr_remap(PrimitiveObject* p, std::string attr_name, std::unordered_map<std::string, int>& facesetNameMap) {
        UserData* pUserData = dynamic_cast<UserData*>(p->userData());
        int faceset_count = pUserData->get2<int>(attr_name + "_count", 0);
        {
            std::unordered_map<int, int> cur_faceset_index_map;
            for (auto i = 0; i < faceset_count; i++) {
                auto path = pUserData->get2<std::string>(format("{}_{}", attr_name, i));
                if (facesetNameMap.count(path) == 0) {
                    int new_index = facesetNameMap.size();
                    facesetNameMap[path] = new_index;
                }
                cur_faceset_index_map[i] = facesetNameMap[path];
            }

            for (int i = 0; i < p->tris.size(); i++) {
                p->tris.attr<int>(attr_name)[i] = cur_faceset_index_map[p->tris.attr<int>(attr_name)[i]];
            }
            for (int i = 0; i < p->quads.size(); i++) {
                p->quads.attr<int>(attr_name)[i] = cur_faceset_index_map[p->quads.attr<int>(attr_name)[i]];
            }
            for (int i = 0; i < p->polys.size(); i++) {
                p->polys.attr<int>(attr_name)[i] = cur_faceset_index_map[p->polys.attr<int>(attr_name)[i]];
            }
        }
    }

    void primTriangulate(PrimitiveObject* prim, bool with_uv, bool has_lines, bool with_attr) {
        if (prim->polys.size() == 0) {
            return;
        }
        boolean_switch(has_lines, [&] (auto has_lines) {
            std::vector<std::conditional_t<has_lines.value, vec2i, int>> scansum(prim->polys.size());
            auto redsum = parallel_exclusive_scan_sum(prim->polys.begin(), prim->polys.end(),
                                           scansum.begin(), [&] (auto &ind) {
                                               if constexpr (has_lines.value) {
                                                   return vec2i(ind[1] >= 3 ? ind[1] - 2 : 0, ind[1] == 2 ? 1 : 0);
                                               } else {
                                                   return ind[1] >= 3 ? ind[1] - 2 : 0;
                                               }
                                           });
            std::vector<int> mapping;
            int tribase = prim->tris.size();
            int linebase = prim->lines.size();
            if constexpr (has_lines.value) {
                prim->tris.resize(tribase + redsum[0]);
                mapping.resize(tribase + redsum[0]);
                prim->lines.resize(linebase + redsum[1]);
            } else {
                prim->tris.resize(tribase + redsum);
                mapping.resize(tribase + redsum);
            }

            if (!(prim->loops.has_attr("uvs") && prim->uvs.size() > 0) || !with_uv) {
                parallel_for(prim->polys.size(), [&] (size_t i) {
                    auto [start, len] = prim->polys[i];

                    if (len >= 3) {
                        int scanbase;
                        if constexpr (has_lines.value) {
                            scanbase = scansum[i][0] + tribase;
                        } else {
                            scanbase = scansum[i] + tribase;
                        }
                        prim->tris[scanbase] = vec3i(
                                prim->loops[start],
                                prim->loops[start + 1],
                                prim->loops[start + 2]);
                        mapping[scanbase] = i;
                        scanbase++;
                        for (int j = 3; j < len; j++) {
                            prim->tris[scanbase] = vec3i(
                                    prim->loops[start],
                                    prim->loops[start + j - 1],
                                    prim->loops[start + j]);
                            mapping[scanbase] = i;
                            scanbase++;
                        }
                    }
                    if constexpr (has_lines.value) {
                        if (len == 2) {
                            int scanbase = scansum[i][1] + linebase;
                            prim->lines[scanbase] = vec2i(
                                prim->loops[start],
                                prim->loops[start + 1]);
                        }
                    }
                });

            } else {
                auto &loop_uv = prim->loops.attr<int>("uvs");
                auto &uvs = prim->uvs;
                auto &uv0 = prim->tris.add_attr<zeno::vec3f>("uv0");
                auto &uv1 = prim->tris.add_attr<zeno::vec3f>("uv1");
                auto &uv2 = prim->tris.add_attr<zeno::vec3f>("uv2");

                parallel_for(prim->polys.size(), [&] (size_t i) {
                    auto [start, len] = prim->polys[i];

                    if (len >= 3) {
                        int scanbase;
                        if constexpr (has_lines.value) {
                            scanbase = scansum[i][0] + tribase;
                        } else {
                            scanbase = scansum[i] + tribase;
                        }
                        uv0[scanbase] = {uvs[loop_uv[start]][0], uvs[loop_uv[start]][1], 0};
                        uv1[scanbase] = {uvs[loop_uv[start + 1]][0], uvs[loop_uv[start + 1]][1], 0};
                        uv2[scanbase] = {uvs[loop_uv[start + 2]][0], uvs[loop_uv[start + 2]][1], 0};
                        prim->tris[scanbase] = vec3i(
                                prim->loops[start],
                                prim->loops[start + 1],
                                prim->loops[start + 2]);
                        mapping[scanbase] = i;
                        scanbase++;
                        for (int j = 3; j < len; j++) {
                            uv0[scanbase] = {uvs[loop_uv[start]][0], uvs[loop_uv[start]][1], 0};
                            uv1[scanbase] = {uvs[loop_uv[start + j - 1]][0], uvs[loop_uv[start + j - 1]][1], 0};
                            uv2[scanbase] = {uvs[loop_uv[start + j]][0], uvs[loop_uv[start + j]][1], 0};
                            prim->tris[scanbase] = vec3i(
                                    prim->loops[start],
                                    prim->loops[start + j - 1],
                                    prim->loops[start + j]);
                            mapping[scanbase] = i;
                            scanbase++;
                        }
                    }
                    if constexpr (has_lines.value) {
                        if (len == 2) {
                            int scanbase = scansum[i][1] + linebase;
                            prim->lines[scanbase] = vec2i(
                                prim->loops[start],
                                prim->loops[start + 1]);
                        }
                    }
                });

            }
            if (with_attr) {
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const &key, auto &arr) {
                  using T = std::decay_t<decltype(arr[0])>;
                  auto &attr = prim->tris.add_attr<T>(key);
                  for (auto i = tribase; i < attr.size(); i++) {
                      attr[i] = arr[mapping[i]];
                  }
                });
            }
            prim->loops.clear_with_attr();
            prim->polys.clear_with_attr();
            prim->uvs.clear_with_attr();
        });
    }

    void primTriangulateQuads(PrimitiveObject* prim) {
        if (prim->quads.size() == 0) {
            return;
        }
        auto base = prim->tris.size();
        prim->tris.resize(base + prim->quads.size() * 2);
        bool hasmat = prim->quads.has_attr("matid");
        if (hasmat == false)
        {
            prim->quads.add_attr<int>("matid");
            prim->quads.attr<int>("matid").assign(prim->quads.size(), -1);
        }

        if (prim->tris.has_attr("matid")) {
            prim->tris.attr<int>("matid").resize(base + prim->quads.size() * 2);
        }
        else {
            prim->tris.add_attr<int>("matid");
        }


        for (size_t i = 0; i < prim->quads.size(); i++) {
            auto quad = prim->quads[i];
            prim->tris[base + i * 2 + 0] = vec3f(quad[0], quad[1], quad[2]);
            prim->tris[base + i * 2 + 1] = vec3f(quad[0], quad[2], quad[3]);
            if (hasmat) {
                prim->tris.attr<int>("matid")[base + i * 2 + 0] = prim->quads.attr<int>("matid")[i];
                prim->tris.attr<int>("matid")[base + i * 2 + 1] = prim->quads.attr<int>("matid")[i];
            }
            else
            {
                prim->tris.attr<int>("matid")[base + i * 2 + 0] = -1;
                prim->tris.attr<int>("matid")[base + i * 2 + 1] = -1;
            }
        }
        prim->quads.clear();
    }

    zeno::ZsVector<std::unique_ptr<zeno::PrimitiveObject>> get_prims_from_list(zeno::ListObject* spList) {
        zeno::ZsVector<std::unique_ptr<zeno::PrimitiveObject>> vec;
        for (auto obj : spList->get()) {
            auto prim = dynamic_cast<zeno::PrimitiveObject*>(obj);
            if (prim)
                vec.push_back(safe_uniqueptr_cast<PrimitiveObject>(prim->clone()));
        }
        return vec;
    }

    void primKillDeadUVs(PrimitiveObject* prim) {
        if (!(prim->loops.size() > 0 && prim->loops.attr_is<int>("uvs"))) {
            return;
        }
        auto& uvs = prim->loops.attr<int>("uvs");
        std::set<int> reached;
        for (auto const& [start, len] : prim->polys) {
            for (int i = start; i < start + len; i++) {
                reached.insert(uvs[i]);
            }
        }
        std::vector<int> revamp(reached.begin(), reached.end());
        auto old_prim_size = prim->uvs.size();
        prim->uvs.forall_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
            revamp_vector(arr, revamp);
            });
        prim->uvs.resize(revamp.size());

        std::vector<int> unrevamp(old_prim_size);
        for (int i = 0; i < revamp.size(); i++) {
            unrevamp[revamp[i]] = i;
        }

        auto mock = [&](int& ind) {
            ind = unrevamp[ind];
        };
        for (auto const& [start, len] : prim->polys) {
            for (int i = start; i < start + len; i++) {
                mock(uvs[i]);
            }
        }
    }

    void primKillDeadLoops(PrimitiveObject* prim) {
        if (prim->loops.size() == 0) {
            return;
        }
        std::set<int> reached;
        for (auto const& [start, len] : prim->polys) {
            for (int i = start; i < start + len; i++) {
                reached.insert(i);
            }
        }
        std::vector<int> revamp(reached.begin(), reached.end());
        auto old_prim_size = prim->loops.size();
        prim->loops.forall_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
            revamp_vector(arr, revamp);
            });
        prim->loops.resize(revamp.size());

        std::vector<int> unrevamp(old_prim_size);
        for (int i = 0; i < revamp.size(); i++) {
            unrevamp[revamp[i]] = i;
        }
        int count = 0;
        for (auto& [start, len] : prim->polys) {
            start = count;
            count += len;
        }
    }

    void remap_attr_on_faces(PrimitiveObject* prim, std::string tagAttr, std::map<int, int> mapping) {
        if (prim->tris.size()) {
            auto& attr = prim->tris.attr<int>(tagAttr);
            for (auto i = 0; i < prim->tris.size(); i++) {
                if (mapping.count(attr[i])) {
                    attr[i] = mapping[attr[i]];
                }
            }
        }
        if (prim->polys.size()) {
            auto& attr = prim->polys.attr<int>(tagAttr);
            for (auto i = 0; i < prim->polys.size(); i++) {
                if (mapping.count(attr[i])) {
                    attr[i] = mapping[attr[i]];
                }
            }
        }
    }

    std::set<int> get_attr_on_faces(PrimitiveObject* prim, std::string tagAttr, bool skip_negative_number) {
        std::set<int> set;
        if (prim->tris.size()) {
            auto& attr = prim->tris.attr<int>(tagAttr);
            for (auto i = 0; i < prim->tris.size(); i++) {
                if (skip_negative_number && attr[i] < 0) {
                    continue;
                }
                set.insert(attr[i]);
            }
        }
        if (prim->polys.size()) {
            auto& attr = prim->polys.attr<int>(tagAttr);
            for (auto i = 0; i < prim->polys.size(); i++) {
                if (skip_negative_number && attr[i] < 0) {
                    continue;
                }
                set.insert(attr[i]);
            }
        }
        return set;
    }

    void prim_set_faceset(PrimitiveObject* prim, String faceset_name) {
        IUserData* pUserData = prim->userData();
        int faceset_count = pUserData->get_int("faceset_count", 0);
        for (auto j = 0; j < faceset_count; j++) {
            pUserData->del(stdString2zs(zeno::format("faceset_{}", j)));
        }
        pUserData->set_int("faceset_count", 1);
        pUserData->set_string("faceset_0", faceset_name);

        if (prim->tris.size() > 0) {
            prim->tris.add_attr<int>("faceset").assign(prim->tris.size(), 0);
        }
        if (prim->quads.size() > 0) {
            prim->quads.add_attr<int>("faceset").assign(prim->quads.size(), 0);
        }
        if (prim->polys.size() > 0) {
            prim->polys.add_attr<int>("faceset").assign(prim->polys.size(), 0);
        }
    }

    void prim_set_abcpath(PrimitiveObject* prim, String path_name) {
        auto pUserData = prim->userData();
        int faceset_count = pUserData->get_int("abcpath_count", 0);
        for (auto j = 0; j < faceset_count; j++) {
            pUserData->del(stdString2zs(zeno::format("abcpath_{}", j)));
        }
        pUserData->set_int("abcpath_count", 1);
        pUserData->set_string("abcpath_0", path_name);

        if (prim->tris.size() > 0) {
            prim->tris.add_attr<int>("abcpath").assign(prim->tris.size(), 0);
        }
        if (prim->quads.size() > 0) {
            prim->quads.add_attr<int>("abcpath").assign(prim->quads.size(), 0);
        }
        if (prim->polys.size() > 0) {
            prim->polys.add_attr<int>("abcpath").assign(prim->polys.size(), 0);
        }
    }

    void prim_copy_faceset_to_matid(PrimitiveObject* prim) {
        auto ud = prim->userData();
        auto faceset_count = ud->get_int("faceset_count", 0);
        ud->set_int("matNum", faceset_count);
        for (auto i = 0; i < faceset_count; i++) {
            auto value = ud->get_string(stdString2zs(format("faceset_{}", i)));
            ud->set_string(stdString2zs(format("Material_{}", i)), value);
        }

        if (prim->tris.attr_is<int>("faceset")) {
            prim->tris.attr_visit<AttrAcceptAll>("faceset", [&](auto const& attarr) {
                using T = std::decay_t<decltype(attarr[0])>;
                auto& targetAttr = prim->tris.template add_attr<T>("matid");
                std::copy(attarr.begin(), attarr.end(), targetAttr.begin());
                });
        }
        if (prim->polys.attr_is<int>("faceset")) {
            prim->polys.attr_visit<AttrAcceptAll>("faceset", [&](auto const& attarr) {
                using T = std::decay_t<decltype(attarr[0])>;
                auto& targetAttr = prim->polys.template add_attr<T>("matid");
                std::copy(attarr.begin(), attarr.end(), targetAttr.begin());
                });
        }
    }

    ZENO_API std::unique_ptr<zeno::PrimitiveObject> primMergeWithFacesetMatid2(std::vector<zeno::PrimitiveObject*> const& primList, std::string const& tagAttr, bool tag_on_vert, bool tag_on_face) {
        std::vector<std::string> matNameList(0);
        std::unordered_map<std::string, int> facesetNameMap;
        std::unordered_map<std::string, int> abcpathNameMap;
        for (auto p : primList) {
            //if p has material
            int matNum = p->userData()->get_int("matNum", 0);
            if (matNum > 0) {
                //for p's tris, quads...
                //    tris("matid")[i] += matNameList.size();
                for (int i = 0; i < p->tris.size(); i++) {
                    if (p->tris.attr<int>("matid")[i] != -1) {
                        p->tris.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                for (int i = 0; i < p->quads.size(); i++) {
                    if (p->quads.attr<int>("matid")[i] != -1) {
                        p->quads.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                for (int i = 0; i < p->polys.size(); i++) {
                    if (p->polys.attr<int>("matid")[i] != -1) {
                        p->polys.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                //for p's materials
                //    add them to material list
                for (int i = 0; i < matNum; i++) {
                    auto matIdx = "Material_" + to_string(i);
                    auto matName = p->userData()->get_string(stdString2zs(matIdx), "Default");
                    matNameList.emplace_back(zsString2Std(matName));
                }
            }
            else {
                //for p's tris, quads...
                //    tris("matid")[] = -1;
                if (p->tris.size() > 0) {
                    p->tris.add_attr<int>("matid");
                    p->tris.attr<int>("matid").assign(p->tris.size(), -1);
                }
                if (p->quads.size() > 0) {
                    p->quads.add_attr<int>("matid");
                    p->quads.attr<int>("matid").assign(p->quads.size(), -1);
                }
                if (p->polys.size() > 0) {
                    p->polys.add_attr<int>("matid");
                    p->polys.attr<int>("matid").assign(p->polys.size(), -1);
                }
            }
            int faceset_count = p->userData()->get_int("faceset_count", 0);
            if (faceset_count == 0) {
                prim_set_faceset(p, "defFS");
                faceset_count = 1;
            }
            set_special_attr_remap(p, "faceset", facesetNameMap);
            int path_count = p->userData()->get_int("abcpath_count", 0);
            if (path_count == 0) {
                prim_set_abcpath(p, "/ABC/unassigned");
                path_count = 1;
            }
            set_special_attr_remap(p, "abcpath", abcpathNameMap);
        }

        auto outprim = PrimMerge(stdVec2zeVec(primList), stdString2zs(tagAttr), tag_on_vert, tag_on_face);
        auto pUserData = dynamic_cast<UserData*>(outprim->userData());
        for (auto& p : primList) {
            pUserData->merge(*dynamic_cast<UserData*>(p->userData()));
        }

        if (matNameList.size() > 0) {
            //add matNames to userData
            int i = 0;
            for (auto name : matNameList) {
                auto matIdx = "Material_" + to_string(i);
                pUserData->setLiterial(matIdx, name);
                i++;
            }
        }
        int oMatNum = matNameList.size();
        pUserData->set2("matNum", oMatNum);
        {
            for (auto const& [k, v] : facesetNameMap) {
                pUserData->set2(format("faceset_{}", v), k);
            }
            int faceset_count = facesetNameMap.size();
            pUserData->set2("faceset_count", faceset_count);
        }
        {
            for (auto const& [k, v] : abcpathNameMap) {
                pUserData->set2(format("abcpath_{}", v), k);
            }
            int abcpath_count = abcpathNameMap.size();
            pUserData->set2("abcpath_count", abcpath_count);
        }
        return outprim;
    }

    ZENO_API std::unique_ptr<zeno::PrimitiveObject> primMergeWithFacesetMatid(ZsVector<zeno::PrimitiveObject*> const& primList, String const& tagAttr, bool tag_on_vert, bool tag_on_face)
    {
        std::vector<std::string> matNameList(0);
        std::unordered_map<std::string, int> facesetNameMap;
        std::unordered_map<std::string, int> abcpathNameMap;
        for (auto p : primList) {
            //if p has material
            int matNum = p->userData()->get_int("matNum", 0);
            if (matNum > 0) {
                //for p's tris, quads...
                //    tris("matid")[i] += matNameList.size();
                for (int i = 0; i < p->tris.size(); i++) {
                    if (p->tris.attr<int>("matid")[i] != -1) {
                        p->tris.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                for (int i = 0; i < p->quads.size(); i++) {
                    if (p->quads.attr<int>("matid")[i] != -1) {
                        p->quads.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                for (int i = 0; i < p->polys.size(); i++) {
                    if (p->polys.attr<int>("matid")[i] != -1) {
                        p->polys.attr<int>("matid")[i] += matNameList.size();
                    }
                }
                //for p's materials
                //    add them to material list
                for (int i = 0; i < matNum; i++) {
                    auto matIdx = "Material_" + to_string(i);
                    auto matName = p->userData()->get_string(stdString2zs(matIdx), "Default");
                    matNameList.emplace_back(zsString2Std(matName));
                }
            }
            else {
                //for p's tris, quads...
                //    tris("matid")[] = -1;
                if (p->tris.size() > 0) {
                    p->tris.add_attr<int>("matid");
                    p->tris.attr<int>("matid").assign(p->tris.size(), -1);
                }
                if (p->quads.size() > 0) {
                    p->quads.add_attr<int>("matid");
                    p->quads.attr<int>("matid").assign(p->quads.size(), -1);
                }
                if (p->polys.size() > 0) {
                    p->polys.add_attr<int>("matid");
                    p->polys.attr<int>("matid").assign(p->polys.size(), -1);
                }
            }
            int faceset_count = p->userData()->get_int("faceset_count", 0);
            if (faceset_count == 0) {
                prim_set_faceset(p, "defFS");
                faceset_count = 1;
            }
            set_special_attr_remap(p, "faceset", facesetNameMap);
            int path_count = p->userData()->get_int("abcpath_count", 0);
            if (path_count == 0) {
                prim_set_abcpath(p, "/ABC/unassigned");
                path_count = 1;
            }
            set_special_attr_remap(p, "abcpath", abcpathNameMap);
        }
        auto outprim = PrimMerge(primList, tagAttr, tag_on_vert, tag_on_face);
        auto pUserData = dynamic_cast<UserData*>(outprim->userData());
        for (auto& p : primList) {
            pUserData->merge(*dynamic_cast<UserData*>(p->userData()));
        }

        if (matNameList.size() > 0) {
            //add matNames to userData
            int i = 0;
            for (auto name : matNameList) {
                auto matIdx = "Material_" + to_string(i);
                pUserData->setLiterial(matIdx, name);
                i++;
            }
        }
        int oMatNum = matNameList.size();
        pUserData->set2("matNum", oMatNum);
        {
            for (auto const& [k, v] : facesetNameMap) {
                pUserData->set2(format("faceset_{}", v), k);
            }
            int faceset_count = facesetNameMap.size();
            pUserData->set2("faceset_count", faceset_count);
        }
        {
            for (auto const& [k, v] : abcpathNameMap) {
                pUserData->set2(format("abcpath_{}", v), k);
            }
            int abcpath_count = abcpathNameMap.size();
            pUserData->set2("abcpath_count", abcpath_count);
        }
        return outprim;
    }

    void primKillDeadVerts(PrimitiveObject* prim) {
        std::set<int> reached(prim->points.begin(), prim->points.end());
        for (auto const& ind : prim->lines) {
            reached.insert(ind[0]);
            reached.insert(ind[1]);
        }
        for (auto const& ind : prim->tris) {
            reached.insert(ind[0]);
            reached.insert(ind[1]);
            reached.insert(ind[2]);
        }
        for (auto const& ind : prim->quads) {
            reached.insert(ind[0]);
            reached.insert(ind[1]);
            reached.insert(ind[2]);
            reached.insert(ind[3]);
        }
        for (auto const& [start, len] : prim->polys) {
            for (int i = start; i < start + len; i++) {
                reached.insert(prim->loops[i]);
            }
        }
        std::vector<int> revamp(reached.begin(), reached.end());
        auto old_prim_size = prim->verts.size();
        prim->verts.forall_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
            revamp_vector(arr, revamp);
            });
        prim->verts.resize(revamp.size());

        std::vector<int> unrevamp(old_prim_size);
        for (int i = 0; i < revamp.size(); i++) {
            unrevamp[revamp[i]] = i;
        }

        auto mock = [&](int& ind) {
            ind = unrevamp[ind];
        };
        for (auto& ind : prim->points) {
            mock(ind);
        }
        for (auto& ind : prim->lines) {
            mock(ind[0]);
            mock(ind[1]);
        }
        for (auto& ind : prim->tris) {
            mock(ind[0]);
            mock(ind[1]);
            mock(ind[2]);
        }
        for (auto& ind : prim->quads) {
            mock(ind[0]);
            mock(ind[1]);
            mock(ind[2]);
            mock(ind[3]);
        }
        for (auto const& [start, len] : prim->polys) {
            for (int i = start; i < start + len; i++) {
                mock(prim->loops[i]);
            }
        }
        primKillDeadUVs(prim);
        primKillDeadLoops(prim);
    }

    zeno::ZsVector<std::unique_ptr<PrimitiveObject>> PrimUnmergeFaces(PrimitiveObject* prim, String tagAttr) {
        if (!prim->verts.size()) return {};

        if (prim->tris.size() > 0 && prim->polys.size() > 0) {
            primPolygonate(prim, true);
        }

        std::string sTagAttr = zsString2Std(tagAttr);

        ZsVector<std::unique_ptr<PrimitiveObject>> list;

        std::map<int, std::vector<int>> mapping;
        if (prim->tris.size() > 0) {
            auto& attr = prim->tris.attr<int>(sTagAttr);
            for (auto i = 0; i < prim->tris.size(); i++) {
                if (mapping.count(attr[i]) == 0) {
                    mapping[attr[i]] = {};
                }
                mapping[attr[i]].push_back(i);
            }
            for (auto& [key, val] : mapping) {
                auto new_prim = safe_uniqueptr_cast<PrimitiveObject>(prim->clone());
                new_prim->tris.resize(val.size());
                for (auto i = 0; i < val.size(); i++) {
                    new_prim->tris[i] = prim->tris[val[i]];
                }
                new_prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    auto& attr = prim->tris.attr<T>(key);
                    for (auto i = 0; i < arr.size(); i++) {
                        arr[i] = attr[val[i]];
                    }
                    });
                list.push_back(std::move(new_prim));
            }
        }
        else if (prim->polys.size() > 0) {
            auto& attr = prim->polys.attr<int>(sTagAttr);
            for (auto i = 0; i < prim->polys.size(); i++) {
                if (mapping.count(attr[i]) == 0) {
                    mapping[attr[i]] = {};
                }
                mapping[attr[i]].push_back(i);
            }
            for (auto& [key, val] : mapping) {
                auto new_prim = safe_uniqueptr_cast<PrimitiveObject>(prim->clone());
                new_prim->polys.resize(val.size());
                for (auto i = 0; i < val.size(); i++) {
                    new_prim->polys[i] = prim->polys[val[i]];
                }
                new_prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    auto& attr = prim->polys.attr<T>(key);
                    for (auto i = 0; i < arr.size(); i++) {
                        arr[i] = attr[val[i]];
                    }
                    });
                list.push_back(std::move(new_prim));
            }
        }
        for (auto i = 0; i < list.size(); i++) {
            primKillDeadVerts(list[i].get());
            auto pUserData = dynamic_cast<UserData*>(list[i]->userData());
            // remove unused abcpath
            {
                auto abcpath_set = get_attr_on_faces(list[i].get(), "abcpath", true);
                std::map<int, int> mapping;
                std::vector<std::string> abcpaths;
                for (auto& k : abcpath_set) {
                    mapping[k] = abcpaths.size();
                    abcpaths.push_back(zsString2Std(pUserData->get_string(stdString2zs(format("abcpath_{}", k)))));
                }
                remap_attr_on_faces(list[i].get(), "abcpath", mapping);
                auto old_abcpath_count = pUserData->get_int("abcpath_count", 0);
                for (int j = 0; j < old_abcpath_count; j++) {
                    pUserData->del(stdString2zs(format("abcpath_{}", j)));
                }

                for (int j = 0; j < abcpaths.size(); j++) {
                    pUserData->set2(format("abcpath_{}", j), abcpaths[j]);
                }
                pUserData->set2("abcpath_count", int(abcpath_set.size()));
            }
            // remove unused faceset
            {
                auto abcpath_set = get_attr_on_faces(list[i].get(), "faceset", true);
                std::map<int, int> mapping;
                std::vector<std::string> abcpaths;
                for (auto& k : abcpath_set) {
                    mapping[k] = abcpaths.size();
                    abcpaths.push_back(zsString2Std(pUserData->get_string(stdString2zs(format("faceset_{}", k)))));
                }
                remap_attr_on_faces(list[i].get(), "faceset", mapping);
                int old_abcpath_count = pUserData->get_int("faceset_count", 0);
                for (int j = 0; j < old_abcpath_count; j++) {
                    pUserData->del(stdString2zs(format("faceset_{}", j)));
                }

                for (int j = 0; j < abcpaths.size(); j++) {
                    pUserData->set2(format("faceset_{}", j), abcpaths[j]);
                }
                pUserData->set2("faceset_count", int(abcpath_set.size()));
            }
        }
        return list;
    }

    void primFlipFaces(PrimitiveObject* prim, bool only_face) {
        if (!only_face && prim->lines.size())
            parallel_for_each(prim->lines.begin(), prim->lines.end(), [&](auto& line) {
            std::swap(line[1], line[0]);
                });
        if (prim->tris.size()) {
            parallel_for_each(prim->tris.begin(), prim->tris.end(), [&](auto& tri) {
                std::swap(tri[2], tri[0]);
                });
            if (prim->tris.attr_is<vec3f>("uv0")) {
                auto& uv0 = prim->tris.add_attr<zeno::vec3f>("uv0");
                auto& uv2 = prim->tris.add_attr<zeno::vec3f>("uv2");
                for (auto i = 0; i < prim->tris.size(); i++) {
                    std::swap(uv0[i], uv2[i]);
                }
            }
        }
        if (prim->quads.size())
            parallel_for_each(prim->quads.begin(), prim->quads.end(), [&](auto& quad) {
            std::swap(quad[3], quad[0]);
            std::swap(quad[2], quad[1]);
                });
        if (prim->polys.size()) {
            prim->loops.forall_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                parallel_for_each(prim->polys.begin(), prim->polys.end(), [&](auto const& poly) {
                    auto const& [start, len] = poly;
                    for (int i = 0; i < (len >> 1); i++) {
                        std::swap(arr[start + i], arr[start + len - 1 - i]);
                    }
                    });
                });
        }
    }

    void primCalcNormal(zeno::PrimitiveObject* prim, float flip, zeno::String nrmAttr)
    {
        const std::string sNrmAttr = zsString2Std(nrmAttr);
        auto& nrm = prim->add_attr<zeno::vec3f>(sNrmAttr);
        auto& pos = prim->verts.values;

#if defined(_OPENMP) && defined(__GNUG__)
#pragma omp parallel for
#endif
        for (size_t i = 0; i < nrm.size(); i++) {
            nrm[i] = zeno::vec3f(0);
        }

#if defined(_OPENMP) && defined(__GNUG__)
        auto atomicCas = [](int* dest, int expected, int desired) {
            __atomic_compare_exchange_n(const_cast<int volatile*>(dest), &expected,
                desired, false, __ATOMIC_ACQ_REL,
                __ATOMIC_RELAXED);
            return expected;
        };
        auto atomicFloatAdd = [&](float* dst, float val) {
            static_assert(sizeof(float) == sizeof(int), "sizeof float != sizeof int");
            int oldVal = reinterpret_bits<int>(*dst);
            int newVal = reinterpret_bits<int>(reinterpret_bits<float>(oldVal) + val), readVal{};
            while ((readVal = atomicCas((int*)dst, oldVal, newVal)) != oldVal) {
                oldVal = readVal;
                newVal = reinterpret_bits<int>(reinterpret_bits<float>(readVal) + val);
            }
            return reinterpret_bits<float>(oldVal);
        };
#endif

#if defined(_OPENMP) && defined(__GNUG__)
#pragma omp parallel for
#endif
        for (size_t i = 0; i < prim->tris.size(); i++) {
            auto ind = prim->tris[i];
            auto n = cross(pos[ind[1]] - pos[ind[0]], pos[ind[2]] - pos[ind[0]]);

#if defined(_OPENMP) && defined(__GNUG__)
            for (int j = 0; j != 3; ++j) {
                auto& n_i = nrm[ind[j]];
                for (int d = 0; d != 3; ++d)
                    atomicFloatAdd(&n_i[d], n[d]);
            }
#else
            nrm[ind[0]] += n;
            nrm[ind[1]] += n;
            nrm[ind[2]] += n;
#endif
        }

#if defined(_OPENMP) && defined(__GNUG__)
#pragma omp parallel for
#endif
        for (size_t i = 0; i < prim->quads.size(); i++) {
            auto ind = prim->quads[i];
            std::array<vec3f, 4> ns = {
                cross(pos[ind[1]] - pos[ind[0]], pos[ind[2]] - pos[ind[0]]),
                cross(pos[ind[2]] - pos[ind[1]], pos[ind[3]] - pos[ind[1]]),
                cross(pos[ind[3]] - pos[ind[2]], pos[ind[0]] - pos[ind[2]]),
                cross(pos[ind[0]] - pos[ind[3]], pos[ind[1]] - pos[ind[3]]),
            };

#if defined(_OPENMP) && defined(__GNUG__)
            for (int j = 0; j != 4; ++j) {
                auto& n_i = nrm[ind[j]];
                for (int d = 0; d != 3; ++d)
                    atomicFloatAdd(&n_i[d], ns[j][d]);
            }
#else
            for (int j = 0; j != 4; ++j) {
                nrm[ind[j]] += ns[j];
            }
#endif
        }

#if defined(_OPENMP) && defined(__GNUG__)
#pragma omp parallel for
#endif
        for (size_t i = 0; i < prim->polys.size(); i++) {
            auto [beg, len] = prim->polys[i];

            auto ind = [loops = prim->loops.data(), beg = beg, len = len](int t) -> int {
                if (t >= len) t -= len;
                return loops[beg + t];
            };
            for (int j = 0; j < len; ++j) {
                auto nsj = cross(pos[ind(j + 1)] - pos[ind(j)], pos[ind(j + 2)] - pos[ind(j)]);
#if defined(_OPENMP) && defined(__GNUG__)
                auto& n_i = nrm[ind(j)];
                for (int d = 0; d != 3; ++d)
                    atomicFloatAdd(&n_i[d], nsj[d]);
#else
                nrm[ind(j)] += nsj;
#endif
            }
        }

#if defined(_OPENMP) && defined(__GNUG__)
#pragma omp parallel for
#endif
        for (size_t i = 0; i < nrm.size(); i++) {
            nrm[i] = flip * normalizeSafe(nrm[i]);
        }
    }

    void primWireframe(PrimitiveObject* prim, bool removeFaces, bool toEdges) {
        struct segment_less {
            bool operator()(vec2i const& a, vec2i const& b) const {
                return std::make_pair(std::min(a[0], a[1]), std::max(a[0], a[1]))
                    < std::make_pair(std::min(b[0], b[1]), std::max(b[0], b[1]));
            }
        };
        std::set<vec2i, segment_less> segments;
        auto append = [&](int i, int j) {
            segments.emplace(i, j);
        };
        for (auto const& ind : prim->lines) {
            append(ind[0], ind[1]);
        }
        for (auto const& ind : prim->tris) {
            append(ind[0], ind[1]);
            append(ind[1], ind[2]);
            append(ind[2], ind[0]);
        }
        for (auto const& ind : prim->quads) {
            append(ind[0], ind[1]);
            append(ind[1], ind[2]);
            append(ind[2], ind[3]);
            append(ind[3], ind[0]);
        }
        for (auto const& [start, len] : prim->polys) {
            if (len < 2)
                continue;
            for (int i = start + 1; i < start + len; i++) {
                append(prim->loops[i - 1], prim->loops[i]);
            }
            append(prim->loops[start + len - 1], prim->loops[start]);
        }
        //if (isAccumulate) {
            //for (auto const &ind: prim->lines) {
                //segments.erase(vec2i(ind[0], ind[1]));
            //}
            //prim->lines.values.insert(prim->lines.values.end(), segments.begin(), segments.end());
            //prim->lines.update();
        //} else {
        if (toEdges) {
            prim->edges.attrs.clear();
            prim->edges.values.assign(segments.begin(), segments.end());
            prim->edges.update();
        }
        else {
            prim->lines.attrs.clear();
            prim->lines.values.assign(segments.begin(), segments.end());
            prim->lines.update();
        }
        //}
        if (removeFaces) {
            prim->tris.clear();
            prim->quads.clear();
            prim->loops.clear();
            prim->polys.clear();
        }
    }

    void primSampleTexture(
        PrimitiveObject* prim,
        const zeno::String& srcChannel,
        const zeno::String& srcSource,
        const zeno::String& dstChannel,
        PrimitiveObject* img,
        const zeno::String& wrap,
        zeno::Vec3f borderColor,
        float remapMin,
        float remapMax
    ) {

    }

    void primSampleHeatmap(
        PrimitiveObject* prim,
        const zeno::String& srcChannel,
        const zeno::String& dstChannel,
        HeatmapObject* heatmap,
        float remapMin,
        float remapMax
    ) {
        auto& clr = prim->add_attr<zeno::vec3f>(zsString2Std(dstChannel));
        auto& src = prim->attr<float>(zsString2Std(srcChannel));
#pragma omp parallel for //ideally this could be done in opengl
        for (int i = 0; i < src.size(); i++) {
            auto x = (src[i] - remapMin) / (remapMax - remapMin);
            clr[i] = heatmap->interp(x);
        }
    }

    void primPolygonate(PrimitiveObject* prim, bool with_uv) {
        prim->loops.reserve(prim->loops.size() + prim->tris.size() * 3 +
            prim->quads.size() * 4 + prim->lines.size() * 2 +
            prim->points.size());
        prim->polys.reserve(prim->polys.size() + prim->tris.size() +
            prim->quads.size() + prim->lines.size() +
            prim->points.size());

        int old_loop_base = prim->loops.size();
        int polynum = prim->polys.size();
        if (prim->tris.size()) {
            int base = prim->loops.size();
            for (int i = 0; i < prim->tris.size(); i++) {
                auto const& ind = prim->tris[i];
                prim->loops.push_back(ind[0]);
                prim->loops.push_back(ind[1]);
                prim->loops.push_back(ind[2]);
                prim->polys.push_back({ base + i * 3, 3 });
            }
            prim->polys.update();

            prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                if (key == "uv0" || key == "uv1" || key == "uv2") {
                    return;
                }
                using T = std::decay_t<decltype(arr[0])>;
                auto& newarr = prim->polys.add_attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    newarr[polynum + i] = arr[i];
                }
                });
        }

        polynum = prim->polys.size();
        if (prim->quads.size()) {
            int base = prim->loops.size();
            for (int i = 0; i < prim->quads.size(); i++) {
                auto const& ind = prim->quads[i];
                prim->loops.push_back(ind[0]);
                prim->loops.push_back(ind[1]);
                prim->loops.push_back(ind[2]);
                prim->loops.push_back(ind[3]);
                prim->polys.push_back({ base + i * 4, 4 });
            }
            prim->polys.update();

            prim->quads.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                if (key == "uv0" || key == "uv1" || key == "uv2" || key == "uv3") {
                    return;
                }
                using T = std::decay_t<decltype(arr[0])>;
                auto& newarr = prim->polys.add_attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    newarr[polynum + i] = arr[i];
                }
                });
        }

        polynum = prim->polys.size();
        if (prim->lines.size()) {
            int base = prim->loops.size();
            for (int i = 0; i < prim->lines.size(); i++) {
                auto const& ind = prim->lines[i];
                prim->loops.push_back(ind[0]);
                prim->loops.push_back(ind[1]);
                prim->polys.push_back({ base + i * 2, 2 });
            }
            prim->polys.update();

            prim->lines.foreach_attr([&](auto const& key, auto const& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                auto& newarr = prim->polys.add_attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    newarr[polynum + i] = arr[i];
                }
                });
        }

        polynum = prim->polys.size();
        if (prim->points.size()) {
            int base = prim->loops.size();
            for (int i = 0; i < prim->points.size(); i++) {
                auto ind = prim->points[i];
                prim->loops.push_back(ind);
                prim->polys.push_back({ base + i, 1 });
            }
            prim->polys.update();

            prim->points.foreach_attr([&](auto const& key, auto const& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                auto& newarr = prim->polys.add_attr<T>(key);
                for (auto i = 0; i < arr.size(); i++) {
                    newarr[polynum + i] = arr[i];
                }
                });
        }

        prim->loops.update();
        prim->polys.update();

        if (!(!prim->tris.has_attr("uv0") || !prim->tris.has_attr("uv1") ||
            !prim->tris.has_attr("uv2") || !with_uv)) {
            auto old_uvs_base = prim->uvs.size();
            prim->loops.add_attr<int>("uvs");
            auto& uv0 = prim->tris.attr<zeno::vec3f>("uv0");
            auto& uv1 = prim->tris.attr<zeno::vec3f>("uv1");
            auto& uv2 = prim->tris.attr<zeno::vec3f>("uv2");
            for (int i = 0; i < prim->tris.size(); i++) {
                prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 0] = old_uvs_base + i * 3 + 0;
                prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 1] = old_uvs_base + i * 3 + 1;
                prim->loops.attr<int>("uvs")[old_loop_base + i * 3 + 2] = old_uvs_base + i * 3 + 2;
                prim->uvs.emplace_back(uv0[i][0], uv0[i][1]);
                prim->uvs.emplace_back(uv1[i][0], uv1[i][1]);
                prim->uvs.emplace_back(uv2[i][0], uv2[i][1]);
            }
            // remove duplicate uv index
            {
                std::map<std::tuple<float, float>, int> mapping;
                auto& loopsuv = prim->loops.attr<int>("uvs");
                for (auto i = 0; i < prim->loops.size(); i++) {
                    vec2f uv = prim->uvs[loopsuv[i]];
                    if (mapping.count({ uv[0], uv[1] }) == false) {
                        auto index = mapping.size();
                        mapping[{uv[0], uv[1]}] = index;
                    }
                    loopsuv[i] = mapping[{uv[0], uv[1]}];
                }
                prim->uvs.resize(mapping.size());
                for (auto const& [uv, index] : mapping) {
                    prim->uvs[index] = { std::get<0>(uv), std::get<1>(uv) };
                }
            }
        }

        prim->tris.clear_with_attr();
        prim->quads.clear_with_attr();
        prim->lines.clear_with_attr();
        prim->points.clear_with_attr();
    }

    ZENO_API std::unique_ptr<zeno::PrimitiveObject> PrimMerge(ZsVector<zeno::PrimitiveObject*> const& primList, String const& tagAttr, bool tag_on_vert, bool tag_on_face) {
        //zeno::log_critical("asdfjhl {}", primList.size());
    //throw;
        std::string stagAttr = zsString2Std(tagAttr);

        int poly_flag = 0;
        for (auto& p : primList) {
            if (p->polys.size()) {
                poly_flag += 1;
            }
        }
        // check if mix polys and tris
        if (0 < poly_flag && poly_flag < primList.size()) {
            for (auto& p : primList) {
                if (p->polys.size() == 0) {
                    primPolygonate(p, true);
                }
            }
        }

        auto outprim = std::make_unique<PrimitiveObject>();

        if (primList.size()) {
            std::vector<size_t> bases(primList.size() + 1);
            std::vector<size_t> pointbases(primList.size() + 1);
            std::vector<size_t> linebases(primList.size() + 1);
            std::vector<size_t> tribases(primList.size() + 1);
            std::vector<size_t> quadbases(primList.size() + 1);
            std::vector<size_t> loopbases(primList.size() + 1);
            std::vector<size_t> uvbases(primList.size() + 1);
            std::vector<size_t> polybases(primList.size() + 1);
            size_t total = 0;
            size_t pointtotal = 0;
            size_t linetotal = 0;
            size_t tritotal = 0;
            size_t quadtotal = 0;
            size_t looptotal = 0;
            size_t uvtotal = 0;
            size_t polytotal = 0;
            for (size_t primIdx = 0; primIdx < primList.size(); primIdx++) {
                auto prim = primList[primIdx];
                /// @note promote pure vert prim to point-based prim
                if (!(prim->points.size() || prim->lines.size() || prim->tris.size() || prim->quads.size() ||
                    prim->polys.size())) {
                    auto nverts = prim->verts.size();
                    prim->points.resize(nverts);
                    parallel_for(nverts, [&points = prim->points.values](size_t i) { points[i] = i; });
                }
                ///
                total += prim->verts.size();
                pointtotal += prim->points.size();
                linetotal += prim->lines.size();
                tritotal += prim->tris.size();
                quadtotal += prim->quads.size();
                looptotal += prim->loops.size();
                uvtotal += prim->uvs.size();
                polytotal += prim->polys.size();
                bases[primIdx + 1] = total;
                pointbases[primIdx + 1] = pointtotal;
                linebases[primIdx + 1] = linetotal;
                tribases[primIdx + 1] = tritotal;
                quadbases[primIdx + 1] = quadtotal;
                loopbases[primIdx + 1] = looptotal;
                uvbases[primIdx + 1] = uvtotal;
                polybases[primIdx + 1] = polytotal;
            }
            outprim->verts.resize(total);
            outprim->points.resize(pointtotal);
            outprim->lines.resize(linetotal);
            outprim->tris.resize(tritotal);
            outprim->quads.resize(quadtotal);
            outprim->loops.resize(looptotal);
            outprim->uvs.resize(uvtotal);
            outprim->polys.resize(polytotal);

            if (stagAttr.size()) {
                if (tag_on_vert) {
                    outprim->verts.add_attr<int>(stagAttr);
                }
                if (tag_on_face) {
                    outprim->tris.add_attr<int>(stagAttr);
                    outprim->polys.add_attr<int>(stagAttr);
                }
            }
            for (size_t primIdx = 0; primIdx < primList.size(); primIdx++) {
                auto const& prim = primList[primIdx];
                prim->verts.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->verts.add_attr<T>(key);
                    });
                prim->points.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->points.add_attr<T>(key);
                    });
                prim->lines.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->lines.add_attr<T>(key);
                    });
                prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->tris.add_attr<T>(key);
                    });
                prim->quads.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->quads.add_attr<T>(key);
                    });
                prim->loops.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->loops.add_attr<T>(key);
                    });
                prim->uvs.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->uvs.add_attr<T>(key);
                    });
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->polys.add_attr<T>(key);
                    });
            }

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto base = bases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->verts.values;
                        }
                        else {
                            return outprim->verts.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->verts.size());
                    for (size_t i = 0; i < n; i++) {
                        outarr[base + i] = arr[i];
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->verts.values;
                        size_t n = std::min(arr.size(), prim->verts.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->verts.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->verts.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->verts.values);
                prim->verts.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_vert && stagAttr.size()) {
                    auto& outarr = outprim->verts.attr<int>(stagAttr);
                    for (size_t i = 0; i < prim->verts.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = pointbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->points.values;
                        }
                        else {
                            return outprim->points.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->points.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->points.values;
                        size_t n = std::min(arr.size(), prim->points.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->points.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->points.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->points.values);
                prim->points.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->points.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->points.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = linebases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->lines.values;
                        }
                        else {
                            return outprim->lines.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->lines.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->lines.values;
                        size_t n = std::min(arr.size(), prim->lines.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->lines.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->lines.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->lines.values);
                prim->lines.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->lines.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->lines.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = tribases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->tris.values;
                        }
                        else {
                            return outprim->tris.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->tris.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->tris.values;
                        size_t n = std::min(arr.size(), prim->tris.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->tris.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->tris.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->tris.values);
                prim->tris.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_face && stagAttr.size()) {
                    auto& outarr = outprim->tris.attr<int>(stagAttr);
                    for (size_t i = 0; i < prim->tris.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = quadbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->quads.values;
                        }
                        else {
                            return outprim->quads.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->quads.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->quads.values;
                        size_t n = std::min(arr.size(), prim->quads.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->quads.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->quads.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->quads.values);
                prim->quads.foreach_attr<AttrAcceptAll>(core);
                //            if (stagAttr.size()) {
                //                auto &outarr = outprim->quads.attr<int>(stagAttr);
                //                for (size_t i = 0; i < prim->quads.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = loopbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->loops.values;
                        }
                        else {
                            return outprim->loops.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->loops.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->loops.values;
                        size_t n = std::min(arr.size(), prim->loops.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        return;
                    }
                    else if constexpr (std::is_same_v<T, int>) {
                        if (key == "uvs") {
                            auto& outarr = outprim->loops.attr<int>("uvs");
                            size_t n = std::min(arr.size(), prim->loops.size());
                            size_t offset = uvbases[primIdx];
                            for (size_t i = 0; i < n; i++) {
                                outarr[base + i] = arr[i] + offset;
                            }
                            return;
                        }
                    }
                    if constexpr (!std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->loops.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->loops.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->loops.values);
                prim->loops.foreach_attr<AttrAcceptAll>(core);
                //            if (stagAttr.size()) {
                //                auto &outarr = outprim->loops.attr<int>(stagAttr);
                //                for (size_t i = 0; i < prim->loops.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto base = uvbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->uvs.values;
                        size_t n = std::min(arr.size(), prim->uvs.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->uvs.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->uvs.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                };
                core(std::true_type{}, prim->uvs.values);
                prim->uvs.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->uvs.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->uvs.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto lbase = loopbases[primIdx];
                auto base = polybases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->polys.values;
                        }
                        else {
                            return outprim->polys.attr<T>(key);
                        }
                    }();
                    size_t n = std::min(arr.size(), prim->polys.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = { arr[i].first + lbase, arr[i].second };
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->polys.values;
                        size_t n = std::min(arr.size(), prim->polys.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = { arr[i][0] + (int)lbase, arr[i][1] };
                        }
                    }
                    else {
                        auto& outarr = outprim->polys.add_attr<T>(key);
                        size_t n = std::min(arr.size(), prim->polys.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                };
                core(std::true_type{}, prim->polys.values);
                prim->polys.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_face && stagAttr.size()) {
                    auto& outarr = outprim->polys.attr<int>(stagAttr);
                    for (size_t i = 0; i < prim->polys.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
            });
        }

        return outprim;
    }

    std::unique_ptr<PrimitiveObject> readExrFile(zeno::String const& path) {
        int nx, ny, nc = 4;
        float* rgba;
        const char* err;
        std::string native_path = std::filesystem::u8path(zsString2Std(path)).string();
        int ret = LoadEXR(&rgba, &nx, &ny, native_path.c_str(), &err);
        if (ret != 0) {
            zeno::log_error("load exr: {}", err);
            throw std::runtime_error(zeno::format("load exr: {}", err));
        }
        nx = std::max(nx, 1);
        ny = std::max(ny, 1);
        for (auto i = 0; i < ny / 2; i++) {
            for (auto x = 0; x < nx * 4; x++) {
                auto index1 = i * (nx * 4) + x;
                auto index2 = (ny - 1 - i) * (nx * 4) + x;
                std::swap(rgba[index1], rgba[index2]);
            }
        }

        auto img = std::make_unique<PrimitiveObject>();
        img->verts.resize(nx * ny);

        auto& alpha = img->verts.add_attr<float>("alpha");
        for (int i = 0; i < nx * ny; i++) {
            img->verts[i] = { rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2] };
            alpha[i] = rgba[i * 4 + 3];
        }
        //
        img->userData()->set_int("isImage", 1);
        img->userData()->set_int("w", nx);
        img->userData()->set_int("h", ny);
        return img;
    }

    std::unique_ptr<PrimitiveObject> readImageFile(zeno::String const& path) {
        int w, h, n;
        stbi_set_flip_vertically_on_load(true);
        std::string native_path = std::filesystem::u8path(zsString2Std(path)).string();
        float* data = stbi_loadf(native_path.c_str(), &w, &h, &n, 0);
        if (!data) {
            throw zeno::Exception("cannot open image file at path: " + native_path);
        }
        scope_exit delData = [=] { stbi_image_free(data); };
        auto img = std::make_unique<PrimitiveObject>();
        img->verts.resize(w * h);
        if (n == 3) {
            std::memcpy(img->verts.data(), data, w * h * n * sizeof(float));
        }
        else if (n == 4) {
            auto& alpha = img->verts.add_attr<float>("alpha");
            for (int i = 0; i < w * h; i++) {
                img->verts[i] = { data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2] };
                alpha[i] = data[i * 4 + 3];
            }
        }
        else if (n == 2) {
            for (int i = 0; i < w * h; i++) {
                img->verts[i] = { data[i * 2 + 0], data[i * 2 + 1], 0 };
            }
        }
        else if (n == 1) {
            for (int i = 0; i < w * h; i++) {
                img->verts[i] = vec3f(data[i]);
            }
        }
        else {
            throw zeno::Exception("too much number of channels");
        }
        img->userData()->set_int("isImage", 1);
        img->userData()->set_int("w", w);
        img->userData()->set_int("h", h);
        return img;
    }

    std::unique_ptr<PrimitiveObject> readPFMFile(zeno::String const& path) {
        int nx = 0;
        int ny = 0;
        std::ifstream file(zsString2Std(path), std::ios::binary);
        std::string format;
        file >> format;
        file >> nx >> ny;
        float scale = 0;
        file >> scale;
        file.ignore(1);

        auto img = std::make_unique<PrimitiveObject>();
        int size = nx * ny;
        img->resize(size);
        file.read(reinterpret_cast<char*>(img->verts.data()), sizeof(vec3f) * nx * ny);

        img->userData()->set_int("isImage", 1);
        img->userData()->set_int("w", nx);
        img->userData()->set_int("h", ny);
        return img;
    }

    void write_pfm(const std::string& path, int w, int h, vec3f* rgb) {
        std::string header = zeno::format("PF\n{} {}\n-1.0\n", w, h);
        std::vector<char> data(header.size() + w * h * sizeof(vec3f));
        memcpy(data.data(), header.data(), header.size());
        memcpy(data.data() + header.size(), rgb, w * h * sizeof(vec3f));
        file_put_binary(data, path);
    }

    void write_pfm(const zeno::String& path, PrimitiveObject* image) {
        auto ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        write_pfm(zsString2Std(path), w, h, image->verts->data());
    }

    void write_jpg(const zeno::String& path, PrimitiveObject* image) {
        int w = image->userData()->get_int("w");
        int h = image->userData()->get_int("h");
        std::vector<uint8_t> colors;
        for (auto i = 0; i < w * h; i++) {
            auto rgb = zeno::pow(image->verts[i], 1.0f / 2.2f);
            int r = zeno::clamp(int(rgb[0] * 255.99), 0, 255);
            int g = zeno::clamp(int(rgb[1] * 255.99), 0, 255);
            int b = zeno::clamp(int(rgb[2] * 255.99), 0, 255);
            colors.push_back(r);
            colors.push_back(g);
            colors.push_back(b);
        }
        stbi_flip_vertically_on_write(1);
        stbi_write_jpg(path.c_str(), w, h, 3, colors.data(), 100);
    }

    static void primRevampVerts(PrimitiveObject* prim, std::vector<int> const& revamp) {
        prim->foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
            revamp_vector(arr, revamp);
            });
        auto old_prim_size = prim->size();
        prim->resize(revamp.size());

        if ((0
            || prim->tris.size()
            || prim->quads.size()
            || prim->lines.size()
            || prim->edges.size()
            || prim->polys.size()
            || prim->points.size()
            )) {

            std::vector<int> unrevamp(old_prim_size, -1);
            for (int i = 0; i < revamp.size(); i++) {
                unrevamp[revamp[i]] = i;
            }

            auto mock = [&](int& x) -> bool {
                int loc = unrevamp[x];
                if (loc == -1)
                    return false;
                x = loc;
                return true;
                };

            if (prim->tris.size()) {
                std::vector<int> trisrevamp;
                trisrevamp.reserve(prim->tris.size());
                for (int i = 0; i < prim->tris.size(); i++) {
                    auto& tri = prim->tris[i];
                    //ZENO_P(tri);
                    //ZENO_P(unrevamp[tri[0]]);
                    //ZENO_P(unrevamp[tri[1]]);
                    //ZENO_P(unrevamp[tri[2]]);
                    if (mock(tri[0]) && mock(tri[1]) && mock(tri[2]))
                        trisrevamp.emplace_back(i);
                }
                for (int i = 0; i < trisrevamp.size(); i++) {
                    prim->tris[i] = prim->tris[trisrevamp[i]];
                }
                prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, trisrevamp);
                    });
                prim->tris.resize(trisrevamp.size());
            }

            if (prim->quads.size()) {
                std::vector<int> quadsrevamp;
                quadsrevamp.reserve(prim->quads.size());
                for (int i = 0; i < prim->quads.size(); i++) {
                    auto& quad = prim->quads[i];
                    if (mock(quad[0]) && mock(quad[1]) && mock(quad[2]) && mock(quad[3]))
                        quadsrevamp.emplace_back(i);
                }
                for (int i = 0; i < quadsrevamp.size(); i++) {
                    prim->quads[i] = prim->quads[quadsrevamp[i]];
                }
                prim->quads.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, quadsrevamp);
                    });
                prim->quads.resize(quadsrevamp.size());
            }

            if (prim->lines.size()) {
                std::vector<int> linesrevamp;
                linesrevamp.reserve(prim->lines.size());
                for (int i = 0; i < prim->lines.size(); i++) {
                    auto& line = prim->lines[i];
                    if (mock(line[0]) && mock(line[1]))
                        linesrevamp.emplace_back(i);
                }
                for (int i = 0; i < linesrevamp.size(); i++) {
                    prim->lines[i] = prim->lines[linesrevamp[i]];
                }
                prim->lines.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, linesrevamp);
                    });
                prim->lines.resize(linesrevamp.size());
            }

            if (prim->edges.size()) {
                std::vector<int> edgesrevamp;
                edgesrevamp.reserve(prim->edges.size());
                for (int i = 0; i < prim->edges.size(); i++) {
                    auto& edge = prim->edges[i];
                    if (mock(edge[0]) && mock(edge[1]))
                        edgesrevamp.emplace_back(i);
                }
                for (int i = 0; i < edgesrevamp.size(); i++) {
                    prim->edges[i] = prim->edges[edgesrevamp[i]];
                }
                prim->edges.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, edgesrevamp);
                    });
                prim->edges.resize(edgesrevamp.size());
            }

            if (prim->polys.size()) {
                std::vector<int> polysrevamp;
                polysrevamp.reserve(prim->polys.size());
                for (int i = 0; i < prim->polys.size(); i++) {
                    auto& poly = prim->polys[i];
                    bool succ = [&] {
                        for (int p = poly[0]; p < poly[0] + poly[1]; p++)
                            if (!mock(prim->loops[p]))
                                return false;
                        return true;
                        }();
                    if (succ)
                        polysrevamp.emplace_back(i);
                }
                for (int i = 0; i < polysrevamp.size(); i++) {
                    prim->polys[i] = prim->polys[polysrevamp[i]];
                }
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, polysrevamp);
                    });
                prim->polys.resize(polysrevamp.size());
            }

            if (prim->points.size()) {
                std::vector<int> pointsrevamp;
                pointsrevamp.reserve(prim->points.size());
                for (int i = 0; i < prim->points.size(); i++) {
                    auto& point = prim->points[i];
                    if (mock(point))
                        pointsrevamp.emplace_back(i);
                }
                for (int i = 0; i < pointsrevamp.size(); i++) {
                    prim->points[i] = prim->points[pointsrevamp[i]];
                }
                prim->points.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, pointsrevamp);
                    });
                prim->points.resize(pointsrevamp.size());
            }

        }
    }
    static void primRevampFaces(PrimitiveObject* prim, std::vector<uint8_t> const& unrevamp) {
        if ((0
            || prim->tris.size()
            || prim->quads.size()
            || prim->lines.size()
            || prim->edges.size()
            || prim->polys.size()
            || prim->points.size()
            )) {

            auto mock = [&](int const& x) -> bool {
                return unrevamp[x];
                };

            if (prim->tris.size()) {
                std::vector<int> trisrevamp;
                trisrevamp.reserve(prim->tris.size());
                for (int i = 0; i < prim->tris.size(); i++) {
                    auto& tri = prim->tris[i];
                    //ZENO_P(tri);
                    //ZENO_P(unrevamp[tri[0]]);
                    //ZENO_P(unrevamp[tri[1]]);
                    //ZENO_P(unrevamp[tri[2]]);
                    if (mock(tri[0]) || mock(tri[1]) || mock(tri[2]))
                        trisrevamp.emplace_back(i);
                }
                for (int i = 0; i < trisrevamp.size(); i++) {
                    prim->tris[i] = prim->tris[trisrevamp[i]];
                }
                prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, trisrevamp);
                    });
                prim->tris.resize(trisrevamp.size());
            }

            if (prim->quads.size()) {
                std::vector<int> quadsrevamp;
                quadsrevamp.reserve(prim->quads.size());
                for (int i = 0; i < prim->quads.size(); i++) {
                    auto& quad = prim->quads[i];
                    if (mock(quad[0]) || mock(quad[1]) || mock(quad[2]) || mock(quad[3]))
                        quadsrevamp.emplace_back(i);
                }
                for (int i = 0; i < quadsrevamp.size(); i++) {
                    prim->quads[i] = prim->quads[quadsrevamp[i]];
                }
                prim->quads.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, quadsrevamp);
                    });
                prim->quads.resize(quadsrevamp.size());
            }

            if (prim->lines.size()) {
                std::vector<int> linesrevamp;
                linesrevamp.reserve(prim->lines.size());
                for (int i = 0; i < prim->lines.size(); i++) {
                    auto& line = prim->lines[i];
                    if (mock(line[0]) || mock(line[1]))
                        linesrevamp.emplace_back(i);
                }
                for (int i = 0; i < linesrevamp.size(); i++) {
                    prim->lines[i] = prim->lines[linesrevamp[i]];
                }
                prim->lines.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, linesrevamp);
                    });
                prim->lines.resize(linesrevamp.size());
            }

            if (prim->edges.size()) {
                std::vector<int> edgesrevamp;
                edgesrevamp.reserve(prim->edges.size());
                for (int i = 0; i < prim->edges.size(); i++) {
                    auto& edge = prim->edges[i];
                    if (mock(edge[0]) || mock(edge[1]))
                        edgesrevamp.emplace_back(i);
                }
                for (int i = 0; i < edgesrevamp.size(); i++) {
                    prim->edges[i] = prim->edges[edgesrevamp[i]];
                }
                prim->edges.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, edgesrevamp);
                    });
                prim->edges.resize(edgesrevamp.size());
            }

            if (prim->polys.size()) {
                std::vector<int> polysrevamp;
                polysrevamp.reserve(prim->polys.size());
                for (int i = 0; i < prim->polys.size(); i++) {
                    auto& poly = prim->polys[i];
                    bool succ = [&] {
                        for (int p = poly[0]; p < poly[0] + poly[1]; p++)
                            if (mock(prim->loops[p]))
                                return true;
                        return false;
                        }();
                    if (succ)
                        polysrevamp.emplace_back(i);
                }
                for (int i = 0; i < polysrevamp.size(); i++) {
                    prim->polys[i] = prim->polys[polysrevamp[i]];
                }
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, polysrevamp);
                    });
                prim->polys.resize(polysrevamp.size());
            }

            if (prim->points.size()) {
                std::vector<int> pointsrevamp;
                pointsrevamp.reserve(prim->points.size());
                for (int i = 0; i < prim->points.size(); i++) {
                    auto& point = prim->points[i];
                    if (mock(point))
                        pointsrevamp.emplace_back(i);
                }
                for (int i = 0; i < pointsrevamp.size(); i++) {
                    prim->points[i] = prim->points[pointsrevamp[i]];
                }
                prim->points.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    revamp_vector(arr, pointsrevamp);
                    });
                prim->points.resize(pointsrevamp.size());
            }

        }
    }

    ZENO_API void primFilterVerts(PrimitiveObject* prim,
        std::string tagAttr,
        int tagValue,
        bool isInversed,
        std::string revampAttrO,
        std::string method,
        int* aux, int aux_size,
        bool use_aux) {
        if (method == "faces") {
            std::vector<uint8_t> unrevamp(prim->size());
            auto const& tagArr = prim->verts.attr<int>(tagAttr);
            if (!isInversed) {
                for (int i = 0; i < prim->size(); i++) {
                    unrevamp[i] = tagArr[i] == tagValue;
                }
            } else {
                for (int i = 0; i < prim->size(); i++) {
                    unrevamp[i] = tagArr[i] != tagValue;
                }
            }
            primRevampFaces(prim, unrevamp);
            if (!revampAttrO.empty()) {
                auto& revamp = prim->add_attr<int>(revampAttrO);
                if (!isInversed) {
                    for (int i = 0; i < prim->size(); i++) {
                        revamp[i] = tagArr[i] == tagValue ? i : -1;
                    }
                } else {
                    for (int i = 0; i < prim->size(); i++) {
                        revamp[i] = tagArr[i] != tagValue ? i : -1;
                    }
                }
            }
        } else {
            std::vector<int> revamp;
            if (aux_size == 0 && use_aux == false) {
                revamp.reserve(prim->size());
                auto const& tagArr = prim->verts.attr<int>(tagAttr);
                if (!isInversed) {
                    for (int i = 0; i < prim->size(); i++) {
                        if (tagArr[i] == tagValue)
                            revamp.emplace_back(i);
                    }
                } else {
                    for (int i = 0; i < prim->size(); i++) {
                        if (tagArr[i] != tagValue)
                            revamp.emplace_back(i);
                    }
                }
            } else {
                revamp.resize(aux_size);
                for (int i = 0; i < aux_size; i++) {
                    revamp[i] = aux[i];
                }
            }
            primRevampVerts(prim, revamp);
            if (!revampAttrO.empty()) {
                prim->add_attr<int>(revampAttrO) = std::move(revamp);
            }
        }
    }

    ZENO_API std::vector<std::unique_ptr<PrimitiveObject>> primUnmergeVerts(PrimitiveObject* prim, std::string tagAttr) {
        if (!prim->verts.size()) return {};

        auto const& tagArr = prim->verts.attr<int>(tagAttr);
        int tagMax = parallel_reduce_max(tagArr.begin(), tagArr.end()) + 1;

        std::vector<std::unique_ptr<PrimitiveObject>> primList(tagMax);
        for (int tag = 0; tag < tagMax; tag++) {
            primList[tag] = std::make_unique<PrimitiveObject>();
        }

#if 1
        std::vector<std::vector<int>> aux_arrays;
        aux_arrays.resize(tagMax);
        for (int tag = 0; tag < tagMax; tag++) {
            aux_arrays[tag].resize(0);
        }
        //auto const &tagArr = prim->verts.attr<int>(tagAttr);
        for (int i = 0; i < prim->size(); i++) {
            aux_arrays[tagArr[i]].emplace_back(i);
        }
        for (int tag = 0; tag < tagMax; tag++) {
            primList[tag] = safe_uniqueptr_cast<PrimitiveObject>(prim->clone());
            primFilterVerts(primList[tag].get(), tagAttr, tag, false, {}, "verts", aux_arrays[tag].data(), aux_arrays[tag].size(), true);
        }

#else
        std::vector<std::vector<int>> vert_revamp(tagMax);
        std::vector<int> vert_unrevamp(prim->verts.size());

        for (size_t i = 0; i < prim->verts.size(); i++) {
            int tag = tagArr[i];
            vert_revamp[tag].push_back(i);
            vert_unrevamp[i] = tag;
        }

        for (int tag = 0; tag < tagMax; tag++) {
            auto& revamp = vert_revamp[tag];
            auto const& outprim = primList[tag];

            outprim->verts.resize(revamp.size());
            parallel_for((size_t)0, revamp.size(), [&](size_t i) {
                outprim->verts[i] = prim->verts[revamp[i]];
                });
            prim->verts.foreach_attr([&](auto const& key, auto const& inarr) {
                using T = std::decay_t<decltype(inarr[0])>;
                auto& outarr = outprim->verts.add_attr<T>(key);
                parallel_for((size_t)0, revamp.size(), [&](size_t i) {
                    outarr[i] = inarr[revamp[i]];
                    });
                });
        }

        std::vector<std::vector<int>> face_revamp;

        auto mock = [&](auto getter) {
            auto& prim_tris = getter(prim);
            if (prim_tris.size()) {
                face_revamp.clear();
                face_revamp.resize(tagMax);
                using T = std::decay_t<decltype(prim_tris[0])>;

                for (size_t i = 0; i < prim_tris.size(); i++) {
                    auto ind = reinterpret_cast<decay_vec_t<T> const*>(&prim_tris[i]);
                    int tag = vert_unrevamp[ind[0]];
                    bool bad = false;
                    for (int j = 1; j < is_vec_n<T>; j++) {
                        int new_tag = vert_unrevamp[ind[j]];
                        if (tag != new_tag) {
                            bad = true;
                            break;
                        }
                    }
                    if (!bad) face_revamp[tag].push_back(i);
                }

                for (int tag = 0; tag < tagMax; tag++) {
                    auto& revamp = face_revamp[tag];
                    auto& v_revamp = vert_revamp[tag];
                    auto* outprim = primList[tag].get();
                    auto& outprim_tris = getter(outprim);

                    outprim_tris.resize(revamp.size());
                    parallel_for((size_t)0, revamp.size(), [&](size_t i) {
                        auto ind = reinterpret_cast<decay_vec_t<T> const*>(&prim_tris[revamp[i]]);
                        auto outind = reinterpret_cast<decay_vec_t<T> *>(&outprim_tris[i]);
                        for (int j = 0; j < is_vec_n<T>; j++) {
                            outind[j] = v_revamp[ind[j]];
                        }
                        });

                    prim_tris.foreach_attr([&](auto const& key, auto const& inarr) {
                        using T = std::decay_t<decltype(inarr[0])>;
                        auto& outarr = outprim_tris.template add_attr<T>(key);
                        parallel_for((size_t)0, revamp.size(), [&](size_t i) {
                            outarr[i] = inarr[revamp[i]];
                            });
                        });
                }
            }
            };
        mock([](auto&& p) -> auto& { return p->points; });
        mock([](auto&& p) -> auto& { return p->lines; });
        mock([](auto&& p) -> auto& { return p->tris; });
        mock([](auto&& p) -> auto& { return p->quads; });

        if (prim->polys.size()) {
            face_revamp.clear();
            face_revamp.resize(tagMax);

            for (size_t i = 0; i < prim->polys.size(); i++) {
                auto& [base, len] = prim->polys[i];
                if (len <= 0) continue;
                int tag = vert_unrevamp[prim->loops[base]];
                bool bad = false;
                for (int j = base + 1; j < base + len; i++) {
                    int new_tag = vert_unrevamp[prim->loops[j]];
                    if (tag != new_tag) {
                        bad = true;
                        break;
                    }
                }
                if (!bad) face_revamp[tag].push_back(i);
            }

            for (int tag = 0; tag < tagMax; tag++) {
                auto& revamp = face_revamp[tag];
                auto& v_revamp = vert_revamp[tag];
                auto* outprim = primList[tag].get();

                outprim->polys.resize(revamp.size());
                for (size_t i = 0; i < revamp.size(); i++) {
                    auto const& [base, len] = prim->polys[revamp[i]];
                    int new_base = outprim->loops.size();
                    for (int j = base; j < base + len; j++) {
                        outprim->loops.push_back(prim->loops[j]);
                    }
                    outprim->polys[i] = { new_base, len };
                }

                prim->polys.foreach_attr([&](auto const& key, auto const& inarr) {
                    using T = std::decay_t<decltype(inarr[0])>;
                    auto& outarr = outprim->polys.add_attr<T>(key);
                    parallel_for((size_t)0, revamp.size(), [&](size_t i) {
                        outarr[i] = inarr[revamp[i]];
                        });
                    });
            }
        }
#endif

        return primList;
    }

    ZENO_API void primSimplifyTag(PrimitiveObject* prim, std::string tagAttr) {
        auto& tag = prim->verts.attr<int>(tagAttr);
        std::unordered_map<int, int> lut;
        int top = 0;
        for (int i = 0; i < tag.size(); i++) {
            auto k = tag[i];
            auto it = lut.find(k);
            if (it != lut.end()) {
                tag[i] = it->second;
            } else {
                int c = top++;
                lut.emplace(k, c);
                tag[i] = c;
            }
        }
    }

    ZENO_API void primTranslate(PrimitiveObject* prim, vec3f const& offset) {
        parallel_for((size_t)0, prim->verts.size(), [&](size_t i) {
            prim->verts[i] = prim->verts[i] + offset;
            });
    }

    ZENO_API std::pair<vec3f, vec3f> primBoundingBox(PrimitiveObject* prim) {
        if (!prim->verts.size())
            return { {0, 0, 0}, {0, 0, 0} };
        return parallel_reduce_minmax(prim->verts.begin(), prim->verts.end());
    }

    std::optional<std::pair<vec3f, vec3f>> primBoundingBox2(PrimitiveObject* prim) {
        if (!prim->verts.size())
            return std::nullopt;
        return parallel_reduce_minmax(prim->verts.begin(), prim->verts.end());
    }

    // 'smart loop_uvs' to 'qianqiang loop.attr(uv)'
    ZENO_API void primDecodeUVs(PrimitiveObject* prim) {
    }

    // 'smart loop_uvs' to 'veryqianqiang vert.attr(uv)'
    ZENO_API void primLoopUVsToVerts(PrimitiveObject* prim) {
        if (prim->loops.size() && prim->has_attr("uvs")) {
            auto& loop_uvs = prim->loops.attr<int>("uvs");
            auto& vert_uv = prim->verts.add_attr<zeno::vec3f>("uv"); // todo: support vec2f in attr...
            /*attr_uv.resize(prim->loop_uvs.size());*/
            for (size_t i = 0; i < loop_uvs.size(); i++) {
                auto uv = prim->uvs[loop_uvs[i]];
                int vertid = prim->loops[i];
                vert_uv[vertid] = { uv[0], uv[1], 0 };
                // uv may overlap and conflict at edges, but doesn't matter
                // this node is veryqianqiang after all, just to serve ZFX pw
            }
            prim->loops.erase_attr("uvs");
        }
    }

    ZENO_API std::unique_ptr<PrimitiveObject> primDuplicate(PrimitiveObject* parsPrim, PrimitiveObject* meshPrim, std::string dirAttr, std::string tanAttr, std::string radAttr, std::string onbType, float radius, bool copyParsAttr, bool copyMeshAttr) {
        auto prim = std::make_unique<PrimitiveObject>();
        auto hasDirAttr = boolean_variant(!dirAttr.empty());
        auto indOnbType = array_index({ "XYZ", "YXZ", "YZX", "ZYX", "ZXY", "XZY" }, onbType);
        auto hasOnbType = boolean_variant(indOnbType != 0);
        auto hasRadAttr = boolean_variant(!radAttr.empty());
        auto hasRadius = boolean_variant(radius != 1);

        immediate_task_group tg;

        prim->verts.resize(parsPrim->verts.size() * meshPrim->verts.size());
        prim->points.resize(parsPrim->verts.size() * meshPrim->points.size());
        prim->lines.resize(parsPrim->verts.size() * meshPrim->lines.size());
        prim->tris.resize(parsPrim->verts.size() * meshPrim->tris.size());
        prim->quads.resize(parsPrim->verts.size() * meshPrim->quads.size());
        prim->loops.resize(parsPrim->verts.size() * meshPrim->loops.size());
        prim->polys.resize(parsPrim->verts.size() * meshPrim->polys.size());

        std::visit([&](auto hasDirAttr, auto hasRadius, auto hasRadAttr, auto hasOnbType) {
            auto func = [&](auto const& accRad) {
                auto func = [&](auto const& accDir, auto hasTanAttr, auto const& accTan) {
                    tg.add([&] {
                        parallel_for((size_t)0, parsPrim->verts.size(), [&](size_t i) {
                            auto basePos = parsPrim->verts[i];
                            for (size_t j = 0; j < meshPrim->verts.size(); j++) {
                                auto pos = meshPrim->verts[j];
                                if constexpr (hasRadAttr.value) {
                                    pos *= accRad[i];
                                }
                                if constexpr (hasRadius.value) {
                                    pos *= radius;
                                }
                                if constexpr (hasOnbType.value) {
                                    const std::array<std::size_t, 6> a0{ 0, 1, 1, 2, 2, 0 };
                                    const std::array<std::size_t, 6> a1{ 1, 0, 2, 1, 0, 2 };
                                    const std::array<std::size_t, 6> a2{ 2, 2, 0, 0, 1, 1 };
                                    auto i0 = a0[indOnbType];
                                    auto i1 = a1[indOnbType];
                                    auto i2 = a2[indOnbType];
                                    pos = { pos[i0], pos[i1], pos[i2] };
                                }
                                if constexpr (hasDirAttr.value) {
                                    auto t0 = accDir[i];
                                    t0 = normalizeSafe(t0);
                                    vec3f t1, t2;
                                    if constexpr (hasTanAttr.value) {
                                        t1 = accTan[i];
                                        t1 = normalizeSafe(t1);
                                        t2 = cross(t0, t1);
                                        t2 = normalizeSafe(t2);
                                    }
                                    else {
                                        pixarONB(t0, t1, t2);
                                    }
                                    pos = pos[2] * t0 + pos[1] * t1 + pos[0] * t2;
                                }
                                prim->verts[i * meshPrim->verts.size() + j] = basePos + pos;
                            }
                            });
                        });
                    };
                if constexpr (hasDirAttr.value) {
                    auto const& accDir = parsPrim->attr<zeno::vec3f>(dirAttr);
                    if (!tanAttr.empty())
                        func(accDir, std::true_type{}, parsPrim->attr<zeno::vec3f>(tanAttr));
                    else
                        func(accDir, std::false_type{}, std::array<int, 0>{});
                }
                else {
                    func(std::array<int, 0>{}, std::false_type{}, std::array<int, 0>{});
                }
                };
            if constexpr (hasRadAttr)
                parsPrim->verts.attr_visit(radAttr, func);
            else
                func(std::array<int, 0>{});
            }, hasDirAttr, hasRadius, hasRadAttr, hasOnbType);

        auto copyattr = [&](auto& primAttrs, auto& meshAttrs, auto& parsAttrs) {
            if (copyMeshAttr) {
                meshAttrs.template foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arrMesh) {
                    using T = std::decay_t<decltype(arrMesh[0])>;
                    primAttrs.template add_attr<T>(key);
                    tg.add([&] {
                        auto& arrOut = primAttrs.template attr<T>(key);
                        parallel_for((size_t)0, parsAttrs.size(), [&](size_t i) {
                            for (size_t j = 0; j < meshAttrs.size(); j++) {
                                arrOut[i * meshAttrs.size() + j] = arrMesh[j];
                            }
                            });
                        });
                    });
            }
            if (copyParsAttr) {
                parsAttrs.template foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arrPars) {
                    if (meshAttrs.has_attr(key)) return;
                    using T = std::decay_t<decltype(arrPars[0])>;
                    primAttrs.template add_attr<T>(key);
                    tg.add([&] {
                        auto& arrOut = primAttrs.template attr<T>(key);
                        parallel_for((size_t)0, arrPars.size(), [&](size_t i) {
                            auto value = arrPars[i];
                            for (size_t j = 0; j < meshAttrs.size(); j++) {
                                arrOut[i * meshAttrs.size() + j] = value;
                            }
                            });
                        });
                    });
            }
            };
        copyattr(prim->verts, meshPrim->verts, parsPrim->verts);
        auto advanceinds = [&](auto& primAttrs, auto& meshAttrs, auto& parsAttrs, size_t parsVertsSize, size_t meshVertsSize) {
            copyattr(primAttrs, meshAttrs, parsAttrs);
            tg.add([&] {
                parallel_for((size_t)0, parsVertsSize, [&](size_t i) {
                    overloaded fixpairadd{
                        [](auto& x, size_t y) {
                            x += y;
                        },
                        [](std::pair<int, int>& x, size_t y) {
                            x.first += y;
                            x.second += y;
                        },
                    };
                    for (size_t j = 0; j < meshAttrs.size(); j++) {
                        auto index = meshAttrs[j];
                        fixpairadd(index, i * meshVertsSize);
                        primAttrs[i * meshAttrs.size() + j] = index;
                    }
                    });
                });
            };
        AttrVector<vec3f> dummyVec;
        advanceinds(prim->points, meshPrim->points, parsPrim->verts, parsPrim->verts.size(), meshPrim->verts.size());
        advanceinds(prim->lines, meshPrim->lines, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
        advanceinds(prim->tris, meshPrim->tris, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
        advanceinds(prim->quads, meshPrim->quads, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
        advanceinds(prim->polys, meshPrim->polys, dummyVec, parsPrim->verts.size(), meshPrim->loops.size());
        advanceinds(prim->loops, meshPrim->loops, dummyVec, parsPrim->verts.size(), meshPrim->verts.size());
        tg.add([&] {
            prim->uvs = meshPrim->uvs;
            });

        tg.run();

        return prim;
    }

    ZENO_API void primMarkIsland(PrimitiveObject* prim, std::string tagAttr) {
        // Oh, I mean, Tesla was a great DJ
        auto& tagVert = prim->add_attr<int>(tagAttr);
        auto m = tagVert.size();
        std::vector<int> found(m);
        for (int i = 0; i < m; i++) {
            found[i] = i;
        }
        auto find = [&](int i) {
            while (i != found[i])
                i = found[i];
            return i;
            };
        for (int i = 0; i < prim->lines.size(); i++) {
            auto ind = prim->lines[i];
            int e0 = find(ind[0]);
            int e1 = find(ind[1]);
            found[e1] = e0;
        }
        for (int i = 0; i < prim->tris.size(); i++) {
            auto ind = prim->tris[i];
            int e0 = find(ind[0]);
            int e1 = find(ind[1]);
            int e2 = find(ind[2]);
            found[e1] = e0;
            found[e2] = e0;
        }
        for (int i = 0; i < prim->quads.size(); i++) {
            auto ind = prim->quads[i];
            int e0 = find(ind[0]);
            int e1 = find(ind[1]);
            int e2 = find(ind[2]);
            int e3 = find(ind[3]);
            found[e1] = e0;
            found[e2] = e0;
            found[e3] = e0;
        }
        for (int i = 0; i < prim->polys.size(); i++) {
            auto [base, len] = prim->polys[i];
            if (len <= 1) continue;
            int e0 = find(prim->loops[base]);
            for (int j = base + 1; j < base + len; j++) {
                int ej = find(prim->loops[j]);
                found[ej] = e0;
            }
        }
        for (int i = 0; i < m; i++) {
            tagVert[i] = find(i);
        }
    }

    ZENO_API std::unique_ptr<zeno::PrimitiveObject> primMerge(std::vector<zeno::PrimitiveObject*> const& primList,
        std::string const& tagAttr, bool tag_on_vert, bool tag_on_face) {
        //zeno::log_critical("asdfjhl {}", primList.size());
        //throw;
        int poly_flag = 0;
        for (auto& p : primList) {
            if (p->polys.size()) {
                poly_flag += 1;
            }
        }
        // check if mix polys and tris
        if (0 < poly_flag && poly_flag < primList.size()) {
            for (auto& p : primList) {
                if (p->polys.size() == 0) {
                    primPolygonate(p, true);
                }
            }
        }

        auto outprim = std::make_unique<PrimitiveObject>();

        if (primList.size()) {
            std::vector<size_t> bases(primList.size() + 1);
            std::vector<size_t> pointbases(primList.size() + 1);
            std::vector<size_t> linebases(primList.size() + 1);
            std::vector<size_t> tribases(primList.size() + 1);
            std::vector<size_t> quadbases(primList.size() + 1);
            std::vector<size_t> loopbases(primList.size() + 1);
            std::vector<size_t> uvbases(primList.size() + 1);
            std::vector<size_t> polybases(primList.size() + 1);
            size_t total = 0;
            size_t pointtotal = 0;
            size_t linetotal = 0;
            size_t tritotal = 0;
            size_t quadtotal = 0;
            size_t looptotal = 0;
            size_t uvtotal = 0;
            size_t polytotal = 0;
            for (size_t primIdx = 0; primIdx < primList.size(); primIdx++) {
                auto prim = primList[primIdx];
                /// @note promote pure vert prim to point-based prim
                if (!(prim->points.size() || prim->lines.size() || prim->tris.size() || prim->quads.size() ||
                    prim->polys.size())) {
                    auto nverts = prim->verts.size();
                    prim->points.resize(nverts);
                    parallel_for(nverts, [&points = prim->points.values](size_t i) { points[i] = i; });
                }
                ///
                total += prim->verts.size();
                pointtotal += prim->points.size();
                linetotal += prim->lines.size();
                tritotal += prim->tris.size();
                quadtotal += prim->quads.size();
                looptotal += prim->loops.size();
                uvtotal += prim->uvs.size();
                polytotal += prim->polys.size();
                bases[primIdx + 1] = total;
                pointbases[primIdx + 1] = pointtotal;
                linebases[primIdx + 1] = linetotal;
                tribases[primIdx + 1] = tritotal;
                quadbases[primIdx + 1] = quadtotal;
                loopbases[primIdx + 1] = looptotal;
                uvbases[primIdx + 1] = uvtotal;
                polybases[primIdx + 1] = polytotal;
            }
            outprim->verts.resize(total);
            outprim->points.resize(pointtotal);
            outprim->lines.resize(linetotal);
            outprim->tris.resize(tritotal);
            outprim->quads.resize(quadtotal);
            outprim->loops.resize(looptotal);
            outprim->uvs.resize(uvtotal);
            outprim->polys.resize(polytotal);

            if (tagAttr.size()) {
                if (tag_on_vert) {
                    outprim->verts.add_attr<int>(tagAttr);
                }
                if (tag_on_face) {
                    outprim->tris.add_attr<int>(tagAttr);
                    outprim->polys.add_attr<int>(tagAttr);
                }
            }
            for (size_t primIdx = 0; primIdx < primList.size(); primIdx++) {
                auto const& prim = primList[primIdx];
                prim->verts.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->verts.add_attr<T>(key);
                    });
                prim->points.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->points.add_attr<T>(key);
                    });
                prim->lines.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->lines.add_attr<T>(key);
                    });
                prim->tris.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->tris.add_attr<T>(key);
                    });
                prim->quads.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->quads.add_attr<T>(key);
                    });
                prim->loops.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->loops.add_attr<T>(key);
                    });
                prim->uvs.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->uvs.add_attr<T>(key);
                    });
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    outprim->polys.add_attr<T>(key);
                    });
            }

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto base = bases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->verts.values;
                        }
                        else {
                            return outprim->verts.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->verts.size());
                    for (size_t i = 0; i < n; i++) {
                        outarr[base + i] = arr[i];
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->verts.values;
                        size_t n = std::min(arr.size(), prim->verts.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->verts.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->verts.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->verts.values);
                prim->verts.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_vert && tagAttr.size()) {
                    auto& outarr = outprim->verts.attr<int>(tagAttr);
                    for (size_t i = 0; i < prim->verts.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = pointbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->points.values;
                        }
                        else {
                            return outprim->points.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->points.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->points.values;
                        size_t n = std::min(arr.size(), prim->points.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->points.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->points.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->points.values);
                prim->points.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->points.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->points.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = linebases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->lines.values;
                        }
                        else {
                            return outprim->lines.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->lines.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->lines.values;
                        size_t n = std::min(arr.size(), prim->lines.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->lines.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->lines.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->lines.values);
                prim->lines.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->lines.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->lines.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = tribases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->tris.values;
                        }
                        else {
                            return outprim->tris.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->tris.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->tris.values;
                        size_t n = std::min(arr.size(), prim->tris.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->tris.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->tris.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->tris.values);
                prim->tris.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_face && tagAttr.size()) {
                    auto& outarr = outprim->tris.attr<int>(tagAttr);
                    for (size_t i = 0; i < prim->tris.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = quadbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->quads.values;
                        }
                        else {
                            return outprim->quads.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->quads.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->quads.values;
                        size_t n = std::min(arr.size(), prim->quads.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->quads.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->quads.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->quads.values);
                prim->quads.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->quads.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->quads.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto vbase = bases[primIdx];
                auto base = loopbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->loops.values;
                        }
                        else {
                            return outprim->loops.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->loops.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->loops.values;
                        size_t n = std::min(arr.size(), prim->loops.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = vbase + arr[i];
                        }
                        return;
                    }
                    else if constexpr (std::is_same_v<T, int>) {
                        if (key == "uvs") {
                            auto& outarr = outprim->loops.attr<int>("uvs");
                            size_t n = std::min(arr.size(), prim->loops.size());
                            size_t offset = uvbases[primIdx];
                            for (size_t i = 0; i < n; i++) {
                                outarr[base + i] = arr[i] + offset;
                            }
                            return;
                        }
                    }
                    if constexpr (!std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->loops.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->loops.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->loops.values);
                prim->loops.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->loops.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->loops.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto base = uvbases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->uvs.values;
                        size_t n = std::min(arr.size(), prim->uvs.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                    else {
                        auto& outarr = outprim->uvs.attr<T>(key);
                        size_t n = std::min(arr.size(), prim->uvs.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
                    };
                core(std::true_type{}, prim->uvs.values);
                prim->uvs.foreach_attr<AttrAcceptAll>(core);
                //            if (tagAttr.size()) {
                //                auto &outarr = outprim->uvs.attr<int>(tagAttr);
                //                for (size_t i = 0; i < prim->uvs.size(); i++) {
                //                    outarr[base + i] = primIdx;
                //                }
                //            }
                });

            parallel_for(primList.size(), [&](size_t primIdx) {
                auto prim = primList[primIdx];
                auto lbase = loopbases[primIdx];
                auto base = polybases[primIdx];
                auto core = [&](auto key, auto const& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
#if 0
                    auto& outarr = [&]() -> auto& {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            return outprim->polys.values;
                        }
                        else {
                            return outprim->polys.attr<T>(key);
                        }
                        }();
                    size_t n = std::min(arr.size(), prim->polys.size());
                    for (size_t i = 0; i < n; i++) {
                        if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                            outarr[base + i] = { arr[i].first + lbase, arr[i].second };
                        }
                        else {
                            outarr[base + i] = arr[i];
                        }
                    }
#else
                    if constexpr (std::is_same_v<decltype(key), std::true_type>) {
                        auto& outarr = outprim->polys.values;
                        size_t n = std::min(arr.size(), prim->polys.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = { arr[i][0] + (int)lbase, arr[i][1] };
                        }
                    }
                    else {
                        auto& outarr = outprim->polys.add_attr<T>(key);
                        size_t n = std::min(arr.size(), prim->polys.size());
                        for (size_t i = 0; i < n; i++) {
                            outarr[base + i] = arr[i];
                        }
                    }
#endif
                    };
                core(std::true_type{}, prim->polys.values);
                prim->polys.foreach_attr<AttrAcceptAll>(core);
                if (tag_on_face && tagAttr.size()) {
                    auto& outarr = outprim->polys.attr<int>(tagAttr);
                    for (size_t i = 0; i < prim->polys.size(); i++) {
                        outarr[base + i] = primIdx;
                    }
                }
                });
        }

        return outprim;
    }

    ZENO_API void primSepTriangles(PrimitiveObject* prim, bool smoothNormal, bool keepTriFaces) {
        if (!prim->tris.size() && !prim->quads.size() && !prim->polys.size()) {
            //if ((prim->points.size() || prim->lines.size()) && !prim->verts.has_attr("clr")) {
                //throw;
                //prim->verts.add_attr<zeno::vec3f>("clr").assign(prim->verts.size(), vec3f(1, 1, 0));
            //}
            return; // TODO: cihou pars and lines
        }
        // TODO: support index compress?
        //if (smoothNormal && !prim->verts.has_attr("nrm")) {
            //primSmoothNormal(prim);
        //}
        bool needCompNormal = !prim->verts.has_attr("nrm");
        bool needCompUVs = !prim->verts.has_attr("uv");

        std::vector<int> v;
        int loopcount = 0;
        for (size_t i = 0; i < prim->polys.size(); i++) {
            auto [base, len] = prim->polys[i];
            if (len < 3) continue;
            loopcount += len - 2;
        }
        v.resize(prim->tris.size() * 3 + prim->quads.size() * 6 + loopcount * 3);
        for (size_t i = 0; i < prim->tris.size(); i++) {
            auto ind = prim->tris[i];
            v[i * 3 + 0] = ind[0];
            v[i * 3 + 1] = ind[1];
            v[i * 3 + 2] = ind[2];
        }
        size_t b = prim->tris.size() * 3;
        for (size_t i = 0; i < prim->quads.size(); i++) {
            auto ind = prim->quads[i];
            v[b + i * 6 + 0] = ind[0];
            v[b + i * 6 + 1] = ind[1];
            v[b + i * 6 + 2] = ind[2];
            v[b + i * 6 + 3] = ind[0];
            v[b + i * 6 + 4] = ind[2];
            v[b + i * 6 + 5] = ind[3];
        }
        b += prim->quads.size() * 6;
        for (size_t i = 0; i < prim->polys.size(); i++) {
            auto [base, len] = prim->polys[i];
            if (len < 3) continue;
            v[b] = prim->loops[base];
            v[b + 1] = prim->loops[base + 1];
            v[b + 2] = prim->loops[base + 2];
            for (int j = 0; j < len - 3; j++) {
                v[b + 3 + 3 * j] = prim->loops[base];
                v[b + 4 + 3 * j] = prim->loops[base + j + 2];
                v[b + 5 + 3 * j] = prim->loops[base + j + 3];
            }
            b += (len - 2) * 3;
        }

        AttrVector<vec3f> new_verts;
        new_verts.resize(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            new_verts[i] = prim->verts[v[i]];
        }
        prim->verts.foreach_attr([&](auto const& key, auto const& arr) {
            using T = std::decay_t<decltype(arr[0])>;
            auto& new_arr = new_verts.add_attr<T>(key);
            for (size_t i = 0; i < v.size(); i++) {
                new_arr[i] = arr[v[i]];
            }
            });

        if (needCompUVs) {
            if (prim->tris.has_attr("uv0") &&
                prim->tris.has_attr("uv1") &&
                prim->tris.has_attr("uv2")) {
                auto& uv0 = prim->tris.attr<zeno::vec3f>("uv0");
                auto& uv1 = prim->tris.attr<zeno::vec3f>("uv1");
                auto& uv2 = prim->tris.attr<zeno::vec3f>("uv2");
                auto& new_uv = new_verts.add_attr<zeno::vec3f>("uv");
                for (int i = 0; i < prim->tris.size(); i++) {
                    auto uv = uv0[i];
                    new_uv[i * 3 + 0] = { uv[0], uv[1], 0 };
                    uv = uv1[i];
                    new_uv[i * 3 + 1] = { uv[0], uv[1], 0 };
                    uv = uv2[i];
                    new_uv[i * 3 + 2] = { uv[0], uv[1], 0 };
                }
            }
            if (prim->quads.has_attr("uv0") &&
                prim->quads.has_attr("uv1") &&
                prim->quads.has_attr("uv2") &&
                prim->quads.has_attr("uv3")) {
                auto& uv0 = prim->quads.attr<zeno::vec3f>("uv0");
                auto& uv1 = prim->quads.attr<zeno::vec3f>("uv1");
                auto& uv2 = prim->quads.attr<zeno::vec3f>("uv2");
                auto& uv3 = prim->quads.attr<zeno::vec3f>("uv3");
                auto& new_uv = new_verts.add_attr<zeno::vec3f>("uv");
                size_t b = prim->tris.size() * 3;
                for (int i = 0; i < prim->quads.size(); i++) {
                    new_uv[b + i * 6 + 0] = uv0[i];
                    new_uv[b + i * 6 + 1] = uv1[i];
                    new_uv[b + i * 6 + 2] = uv2[i];
                    new_uv[b + i * 6 + 3] = uv0[i];
                    new_uv[b + i * 6 + 4] = uv2[i];
                    new_uv[b + i * 6 + 5] = uv3[i];
                }
            }
            if (prim->loops.size() && prim->loops.has_attr("uvs")) {
                auto& loop_uvs = prim->loops.attr<int>("uvs");
                size_t b = 0;
                std::vector<int> v(loopcount * 3);
                for (size_t i = 0; i < prim->polys.size(); i++) {
                    auto [base, len] = prim->polys[i];
                    if (len < 3) continue;
                    v[b] = loop_uvs[base];
                    v[b + 1] = loop_uvs[base + 1];
                    v[b + 2] = loop_uvs[base + 2];
                    for (int j = 0; j < len - 3; j++) {
                        v[b + 3 + 3 * j] = loop_uvs[base];
                        v[b + 4 + 3 * j] = loop_uvs[base + j + 2];
                        v[b + 5 + 3 * j] = loop_uvs[base + j + 3];
                    }
                    b += (len - 2) * 3;
                }
                b = prim->tris.size() * 3 + prim->quads.size() * 6;
                auto& new_uv = new_verts.add_attr<zeno::vec3f>("uv");
                for (int i = 0; i < v.size(); i++) {
                    auto uv = prim->uvs[v[i]];
                    new_uv[b + i] = { uv[0], uv[1], 0 };
                }
            }
        }

        prim->tris.clear();
        prim->quads.clear();
        prim->polys.clear();
        prim->loops.clear();
        prim->uvs.clear();

        if (smoothNormal && needCompNormal) {
            std::vector<vec3f> shn(prim->verts.size());
            for (size_t i = 0; i < v.size() / 3; i++) {
                auto a = new_verts[i * 3 + 0];
                auto b = new_verts[i * 3 + 1];
                auto c = new_verts[i * 3 + 2];
                auto n = cross(b - a, c - a);
                n = normalizeSafe(n);
                shn[v[i * 3 + 0]] += n;
                shn[v[i * 3 + 1]] += n;
                shn[v[i * 3 + 2]] += n;
            }
            auto& new_nrm = new_verts.add_attr<zeno::vec3f>("nrm");
            for (size_t i = 0; i < v.size(); i++) {
                auto n = shn[v[i]];
                n = normalizeSafe(n);
                new_nrm[i] = n;
            }
        }

        std::swap(new_verts, prim->verts);

        if (!smoothNormal && needCompNormal) {
            auto& nrm = prim->verts.add_attr<zeno::vec3f>("nrm");
            for (size_t i = 0; i < v.size() / 3; i++) {
                auto a = prim->verts[i * 3 + 0];
                auto b = prim->verts[i * 3 + 1];
                auto c = prim->verts[i * 3 + 2];
                auto n = cross(b - a, c - a);
                n = normalizeSafe(n);
                nrm[i * 3 + 0] = n;
                nrm[i * 3 + 1] = n;
                nrm[i * 3 + 2] = n;
            }
        }

        if (keepTriFaces) {
            auto& uv0 = prim->tris.add_attr<zeno::vec3f>("uv0");
            auto& uv1 = prim->tris.add_attr<zeno::vec3f>("uv1");
            auto& uv2 = prim->tris.add_attr<zeno::vec3f>("uv2");
            auto& uv = prim->attr<zeno::vec3f>("uv");
            prim->tris.resize(v.size() / 3);
            for (int i = 0; i < prim->tris.size(); i++) {
                prim->tris[i] = { i * 3, i * 3 + 1, i * 3 + 2 };
                uv0[i] = uv[i * 3];
                uv1[i] = uv[i * 3 + 1];
                uv2[i] = uv[i * 3 + 2];
            }
        }
    }

    static void primPossionFilter(PrimitiveObject* prim, float minRadius) {
        if (minRadius <= 0) return;

        TICK(possion);
        float invRadius = 1.f / minRadius;
        std::unordered_map<vec3i, std::vector<int>, tuple_hash, tuple_equal> lut;
        for (int i = 0; i < prim->verts.size(); i++) {
            vec3i ipos(prim->verts[i] * invRadius);
            lut[ipos].push_back(i);
        }

        std::vector<uint8_t> erased(prim->verts.size());
        for (int i = 0; i < prim->verts.size(); i++) {
            if (erased[i])
                continue;
            vec3i ipos(prim->verts[i] * invRadius);
            erased[i] = [&] {
                for (int dz = -1; dz <= 1; dz++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            auto it = lut.find(ipos + vec3i(dx, dy, dz));
                            if (it != lut.end()) {
                                for (int j : it->second) {
                                    if (j == i || erased[j])
                                        continue;
                                    auto dis = prim->verts[i] - prim->verts[j];
                                    if (length(dis) < minRadius)
                                        return true;
                                }
                            }
                        }
                    }
                }
                return false;
                }();
        }

        std::vector<int> revamp(prim->verts.size());
        int nrevamp = 0;
        for (int i = 0; i < erased.size(); i++) {
            if (!erased[i])
                revamp[nrevamp++] = i;
        }
        revamp.resize(nrevamp);

        prim->verts.forall_attr([&](auto const& key, auto& arr) {
            revamp_vector(arr, revamp);
            });
        prim->verts.resize(nrevamp);
        TOCK(possion);
    }

#define ZENO_NOTICKTOCK

    ZENO_API std::unique_ptr<PrimitiveObject> primScatter(
        PrimitiveObject* prim, std::string type, std::string denAttr, float density, float minRadius, bool interpAttrs, int seed) {
        auto retprim = std::make_unique<PrimitiveObject>();

        if (seed == -1) seed = std::random_device{}();
        bool hasDenAttr = !denAttr.empty();
        TICK(scatter);

        std::vector<float> cdf;
        if (type == "tris") {
            if (!prim->tris.size()) return retprim;
            cdf.resize(prim->tris.size());
            parallel_inclusive_scan_sum(prim->tris.begin(), prim->tris.end(), cdf.begin(), [&](auto const& ind) {
                auto a = prim->verts[ind[0]];
                auto b = prim->verts[ind[1]];
                auto c = prim->verts[ind[2]];
                auto area = length(cross(c - a, c - b));
                if (hasDenAttr) {
                    auto& den = prim->verts.attr<float>(denAttr);
                    auto da = den[ind[0]];
                    auto db = den[ind[1]];
                    auto dc = den[ind[2]];
                    area *= std::abs(da + db + dc) / 3;
                }
                return area;
                });
        }
        else if (type == "lines") {
            if (!prim->lines.size()) return retprim;
            cdf.resize(prim->lines.size());
            parallel_inclusive_scan_sum(prim->lines.begin(), prim->lines.end(), cdf.begin(), [&](auto const& ind) {
                auto a = prim->lines[ind[0]];
                auto b = prim->lines[ind[1]];
                auto area = length(a - b);
                if (hasDenAttr) {
                    auto& den = prim->verts.attr<float>(denAttr);
                    auto da = den[ind[0]];
                    auto db = den[ind[1]];
                    area *= std::abs(da + db) / 2;
                }
                return area;
                });
        }
        if (cdf.empty()) return retprim;

        auto npoints = (int)std::rint(cdf.back() * density);
        auto inv_total = 1 / cdf.back();
        parallel_for((size_t)0, cdf.size(), [&](size_t i) {
            cdf[i] *= inv_total;
            });
        zeno::log_info("PrimScatter total npoints {}", npoints);

        retprim->verts.resize(npoints);

        if (!prim->verts.num_attrs()) {
            interpAttrs = false;
        }
        if (interpAttrs) {
            prim->verts.foreach_attr([&](auto const& key, auto const& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                retprim->add_attr<T>(key);
                });
        }

        if (type == "tris") {
            parallel_for((size_t)0, (size_t)npoints, [&](size_t i) {
                wangsrng rng(seed, i);
                auto val = rng.next_float();
                auto it = std::lower_bound(cdf.begin(), cdf.end(), val);
                size_t index = it - cdf.begin();
                index = std::min(index, prim->tris.size() - 1);
                auto const& ind = prim->tris[index];
                auto a = prim->verts[ind[0]];
                auto b = prim->verts[ind[1]];
                auto c = prim->verts[ind[2]];
                auto r1 = std::sqrt(rng.next_float());
                auto r2 = rng.next_float();
                auto w1 = 1 - r1;
                auto w2 = r1 * (1 - r2);
                auto w3 = r1 * r2;
                auto p = w1 * a + w2 * b + w3 * c;
                retprim->verts[i] = p;
                if (interpAttrs) {
                    prim->verts.foreach_attr([&](auto const& key, auto const& arr) {
                        using T = std::decay_t<decltype(arr[0])>;
                        auto& retarr = retprim->attr<T>(key);
                        auto a = arr[ind[0]];
                        auto b = arr[ind[1]];
                        auto c = arr[ind[2]];
                        auto p = w1 * a + w2 * b + w3 * c;
                        retarr[i] = p;
                        });
                }
                });
        }
        else if (type == "lines") {
            parallel_for((size_t)0, (size_t)npoints, [&](size_t i) {
                wangsrng rng(seed, i);
                auto val = rng.next_float();
                auto it = std::lower_bound(cdf.begin(), cdf.end(), val);
                size_t index = it - cdf.begin();
                index = std::min(index, prim->lines.size() - 1);
                auto const& ind = prim->lines[index];
                auto a = prim->verts[ind[0]];
                auto b = prim->verts[ind[1]];
                auto r1 = rng.next_float();
                auto p = a * (1 - r1) + b * r1;
                retprim->verts[i] = p;
                if (interpAttrs) {
                    prim->verts.foreach_attr([&](auto const& key, auto const& arr) {
                        using T = std::decay_t<decltype(arr[0])>;
                        auto& retarr = retprim->attr<T>(key);
                        auto a = arr[ind[0]];
                        auto b = arr[ind[1]];
                        auto p = a * (1 - r1) + b * r1;
                        retarr[i] = p;
                        });
                }
                });
        }

        TOCK(scatter);
        primPossionFilter(retprim.get(), minRadius);

        return retprim;
    }

    // to do where to put this func??
    template <class T>
    T baryCentricInterpolation(T& v1, T& v2, T& v3, zeno::vec3f& p,
        zeno::vec3f& vert1, zeno::vec3f& vert2,
        zeno::vec3f& vert3) {
        float a1 = area(p, vert2, vert3);
        float a2 = area(p, vert1, vert3);
        float a = area(vert1, vert2, vert3);
        float w1 = a1 / a;
        float w2 = a2 / a;
        float w3 = 1 - w1 - w2;
        return w1 * v1 + w2 * v2 + w3 * v3;
    }

    // todo where to put this func???
    static float area(zeno::vec3f& p1, zeno::vec3f& p2, zeno::vec3f& p3) {
        zeno::vec3f e1 = p3 - p1;
        zeno::vec3f e2 = p2 - p1;
        zeno::vec3f areavec = zeno::cross(e1, e2);
        return 0.5 * sqrt(zeno::dot(areavec, areavec));
    }

    // todo where to put this func????
    static bool ptInTriangle(zeno::vec3f p, zeno::vec3f p0, zeno::vec3f p1,
        zeno::vec3f p2) {
        // float A = 0.5 * (-p1[1] * p2[0] + p0[1] * (-p1[0] + p2[0]) +
        //                  p0[0] * (p1[1] - p2[1]) + p1[0] * p2[1]);
        // float sign = A < 0 ? -1.0f : 1.0f;
        // float s = (p0[1] * p2[0] - p0[0] * p2[1] + (p2[1] - p0[1]) * p[0] +
        //            (p0[0] - p2[0]) * p[1]) *
        //           sign;
        // float t = (p0[0] * p1[1] - p0[1] * p1[0] + (p0[1] - p1[1]) * p[0] +
        //            (p1[0] - p0[0]) * p[1]) *
        //           sign;

        // return s > 0 && t > 0 && ((s + t) < 2 * A * sign);
        p0 -= p;
        p1 -= p;
        p2 -= p;
        auto u = zeno::cross(p1, p2);
        auto v = zeno::cross(p2, p0);
        auto w = zeno::cross(p0, p1);
        if (zeno::dot(u, v) < 0)
        {
            return false;
        }
        if (zeno::dot(u, w) < 0)
        {
            return false;
        }
        return true;
    }

    static bool pointInTriangle(const zeno::vec3f& query_point,
        const zeno::vec3f& triangle_vertex_0,
        const zeno::vec3f& triangle_vertex_1,
        const zeno::vec3f& triangle_vertex_2,
        zeno::vec3f& weights)
    {
        // u=P2−P1
        zeno::vec3f u = triangle_vertex_1 - triangle_vertex_0;
        // v=P3−P1
        zeno::vec3f v = triangle_vertex_2 - triangle_vertex_0;
        // n=u×v
        zeno::vec3f n = zeno::cross(u,v);
        // w=P−P1
        zeno::vec3f w = query_point - triangle_vertex_0;
        // Barycentric coordinates of the projection P′of P onto T:
        // γ=[(u×w)⋅n]/n²
        float gamma = zeno::dot(n, zeno::cross(u,w)) / zeno::dot(n,n);
        // β=[(w×v)⋅n]/n²
        float beta = zeno::dot(zeno::cross(w, v),n) / zeno::dot(n,n);
        float alpha = 1 - gamma - beta;
        // The point P′ lies inside T if:
        weights = vec3f(gamma, beta, alpha);
        return ((0 <= alpha) && (alpha <= 1) &&
                (0 <= beta)  && (beta  <= 1) &&
                (0 <= gamma) && (gamma <= 1));
    }

    // to do where to put this func??
    static void baryCentricInterpolation(zeno::vec3f& p,
        zeno::vec3f& vert1, zeno::vec3f& vert2,
        zeno::vec3f& vert3, zeno::vec3f& weights) {
        float a1 = area(p, vert2, vert3);
        float a2 = area(p, vert1, vert3);
        float a = area(vert1, vert2, vert3);
        float w1 = a1 / (a + 1e-7);
        float w2 = a2 / (a + 1e-7);
        float w3 = 1 - w1 - w2;
        weights = zeno::vec3f(w1, w2, w3);
    }

    ZENO_API void BarycentricInterpPrimitive(PrimitiveObject* _dst, const PrimitiveObject* _src, int i, int v0, int v1, int v2,
        zeno::vec3f& pdst, zeno::vec3f& pos0, zeno::vec3f& pos1, zeno::vec3f& pos2)
    {
        for (auto key : _src->attr_keys())
        {
            if (key != "TriIndex" && key != "pos" && key != "InitWeight")
                std::visit([i, v0, v1, v2, &pdst, &pos0, &pos1, &pos2](auto&& dst, auto&& src) {
                using DstT = std::remove_cv_t<std::remove_reference_t<decltype(dst)>>;
                using SrcT = std::remove_cv_t<std::remove_reference_t<decltype(src)>>;
                if constexpr (std::is_same_v<DstT, SrcT>) {
                    auto val1 = src[v0];
                    auto val2 = src[v1];
                    auto val3 = src[v2];
                    zeno::vec3f w;
                    baryCentricInterpolation(pdst, pos0, pos1, pos2, w);
                    auto val = w[0] * val1 + w[1] * val2 + w[2] * val3;
                    dst[i] = val;
                }
                else {
                    throw std::runtime_error("the same attr of both primitives are of different types.");
                }
                    }, _dst->attr(key), _src->attr(key));
        }
    }

    ZENO_API void primLineSort(PrimitiveObject* prim, bool reversed) {
        std::vector<int> visited;
        {
            std::unordered_multimap<int, int> v2l;
            for (int i = 0; i < prim->lines.size(); i++) {
                auto line = prim->lines[i];
                //log_debug("line {} {}", line[0], line[1]);
                v2l.emplace(line[1], i);
            }

            int nsorted = 0;
            visited.resize(prim->verts.size(), -1);

            auto visit = [&](auto& visit, int vert) -> void {
                if (visited[vert] != -1)
                    return;
                visited[vert] = -2;
                auto [it0, it1] = v2l.equal_range(vert);
                for (auto it = it0; it != it1; ++it) {
                    auto line = prim->lines[it->second];
                    //assert(line[1] == vert);
                    auto next = line[0];
                    visit(visit, next);
                }
                visited[vert] = nsorted++;
                };

            for (int i = 0; i < prim->lines.size(); i++) {
                auto line = prim->lines[i];
                visit(visit, line[1]);
            }

            log_debug("sorted {} of {}", nsorted, visited.size());
            if (nsorted != visited.size()) {
                for (int i = 0; i < visited.size(); i++) {
                    if (visited[i] == -1) {
                        visited[i] = nsorted++;
                    }
                }
                assert(nsorted == visited.size());
            }

            //for (int i = 0; i < visited.size(); i++) {
                //log_debug("{} -> {} = {} {} {}", i, visited[i], prim->verts[i][0], prim->verts[i][1], prim->verts[i][2]);
            //}
        }

        {
            auto revamp = [&](int& x) { x = visited[x]; };
            for (auto& point : prim->points) {
                revamp(point);
            }
            for (auto& line : prim->lines) {
                revamp(line[0]);
                revamp(line[1]);
            }
            for (auto& tri : prim->tris) {
                revamp(tri[0]);
                revamp(tri[1]);
                revamp(tri[2]);
            }
            for (auto& quad : prim->quads) {
                revamp(quad[0]);
                revamp(quad[1]);
                revamp(quad[2]);
            }
            for (auto& loop : prim->loops) {
                revamp(loop);
            }
        }

        {
            auto revampvec = [&](auto& arr) {
                std::vector<std::decay_t<decltype(arr[0])>> newArr(arr.size());
                boolean_switch(reversed, [&](auto reversed) {
                    for (int i = 0; i < arr.size(); i++) {
                        int j = visited[i];
                        if constexpr (reversed.value)
                            j = arr.size() - 1 - j;
                        newArr[j] = arr[i];
                    }
                    });
                /*if constexpr (std::is_same_v<vec3f, std::decay_t<decltype(arr[0])>>) {
                    for (int i = 0; i < newArr.size(); i++) {
                        log_info("{} = {} {} {}", i, newArr[i][0], newArr[i][1], newArr[i][2]);
                    }
                }*/
                std::swap(arr, newArr);
                };
            revampvec(prim->verts.values);
            prim->verts.foreach_attr([&](auto const& key, auto& attr) {
                revampvec(attr);
                });

            //for (int i = 0; i < prim->verts.size(); i++) {
                //log_info("{} = {} {} {}", i, prim->verts[i][0], prim->verts[i][1], prim->verts[i][2]);
            //}
        }
    }

    ZENO_API std::unique_ptr<PrimitiveObject> primitive_merge(zeno::ListObject* list, std::string tagAttr) {
        auto outprim = std::make_unique<PrimitiveObject>();

        size_t len = 0;
        size_t poly_len = 0;

        //
#if defined(_OPENMP)
        size_t nTotalVerts{ 0 }, nTotalPts{ 0 }, nTotalLines{ 0 }, nTotalTris{ 0 }, nTotalQuads{ 0 }, nTotalLoops{ 0 }, nTotalPolys{ 0 };
        size_t nCurPts{ 0 }, nCurLines{ 0 }, nCurTris{ 0 }, nCurQuads{ 0 }, nCurLoops{ 0 }, nCurPolys{ 0 };
#endif
        //

        for (auto const& prim : list->get<PrimitiveObject>()) {
#if defined(_OPENMP)
            nTotalVerts += prim->verts.size();
            nTotalPts += prim->points.size();
            nTotalLines += prim->lines.size();
            nTotalTris += prim->tris.size();
            nTotalQuads += prim->quads.size();
            nTotalLoops += prim->loops.size();
            nTotalPolys += prim->polys.size();
#endif
            prim->foreach_attr([&](auto const& key, auto const& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                outprim->add_attr<T>(key);
                });
        }
#if defined(_OPENMP)
        // leave verts out, since these are 'std::insert'ed
        outprim->verts.resize(nTotalVerts);
        outprim->points.resize(nTotalPts);
        outprim->lines.resize(nTotalLines);
        outprim->tris.resize(nTotalTris);
        outprim->quads.resize(nTotalQuads);
        outprim->loops.resize(nTotalLoops);
        outprim->polys.resize(nTotalPolys);
#endif

        int tagcounter = 0;
        if (!tagAttr.empty()) {
            outprim->add_attr<int>(tagAttr);
        }

        for (auto const& prim : list->get<PrimitiveObject>()) {
            //const auto base = outprim->size();
            prim->foreach_attr([&](auto const& key, auto const& arr) {
                using T = std::decay_t<decltype(arr[0])>;
                //fix pyb
                auto& outarr = outprim->attr<T>(key);

#if defined(_OPENMP)
                static_assert(std::is_trivially_copyable_v<T>, "if T is not trivially copyable, then perf is damaged.");
                // since attr is stored in std::vector, its iterator must be LegacyContiguousIterator.
                std::memcpy(outarr.data() + len, arr.data(), sizeof(T) * arr.size());
                // std::copy(std::begin(arr), std::end(arr), std::begin(outarr) + len);
#else
                outarr.insert(outarr.end(), std::begin(arr), std::end(arr));
#endif
                //for (auto const &val: arr) outarr.push_back(val);
                //end fix pyb
                });
#if defined(_MSC_VER) && defined(_OPENMP)
#define omp_size_t intptr_t
#else
#define omp_size_t size_t
#endif
            if (!tagAttr.empty()) {
                auto& tagArr = outprim->attr<int>(tagAttr);
#if defined(_OPENMP)
                for (std::size_t i = 0; i < prim->size(); i++) {
                    tagArr[len + i] = tagcounter;
                }
#else
                for (std::size_t i = 0; i < prim->size(); i++) {
                    tagArr.push_back(tagcounter);
                }
#endif
            }
#if defined(_OPENMP)
            auto concat = [&](auto& dst, const auto& src, size_t& offset) {
#pragma omp parallel for
                for (omp_size_t i = 0; i < src.size(); ++i) {
                    dst[offset + i] = src[i] + len;
                }
                offset += src.size();
                };
            // insertion
            concat(outprim->points, prim->points, nCurPts);
            concat(outprim->lines, prim->lines, nCurLines);
            concat(outprim->tris, prim->tris, nCurTris);
            concat(outprim->quads, prim->quads, nCurQuads);
            // exception: poly
#pragma omp parallel for
            for (omp_size_t i = 0; i < prim->polys.size(); ++i) {
                const auto& poly = prim->polys[i];
                outprim->polys[nCurPolys + i] = { poly[0] + (int)nCurLoops, poly[1] };
            }
            nCurPolys += prim->polys.size();
            // update nCurLoops after poly update!
            concat(outprim->loops, prim->loops, nCurLoops);
#else
            for (auto const& idx : prim->points) {
                outprim->points.push_back(idx + len);
            }
            for (auto const& idx : prim->lines) {
                outprim->lines.push_back(idx + len);
            }
            for (auto const& idx : prim->tris) {
                outprim->tris.push_back(idx + len);
            }
            for (auto const& idx : prim->quads) {
                outprim->quads.push_back(idx + len);
            }
            for (auto const& idx : prim->loops) {
                outprim->loops.push_back(idx + len);
            }
            size_t sub_poly_len = 0;
            for (auto const& poly : prim->polys) {
                sub_poly_len = std::max(sub_poly_len, (size_t)(poly[0] + poly[1]));
                outprim->polys.emplace_back(poly[0] + poly_len, poly[1]);
            }
            poly_len += sub_poly_len;
#endif
            len += prim->size();
            //fix pyb
#if defined(_OPENMP)
#else
            outprim->resize(len);
#endif
        }

        return outprim;
    }

    ZENO_API void primTriangulateIntoPolys(PrimitiveObject* prim) {
        if (prim->tris.size()) {
            primPolygonate(prim, true);
        }
        else if (prim->polys.size()) {
            bool all_is_tri = true;
            for (auto const& [_start, len] : prim->polys) {
                if (len != 3) {
                    all_is_tri = false;
                    break;
                }
            }
            if (all_is_tri) {
                return;
            }
            int new_poly_count = 0;
            int new_loops_count = 0;
            for (auto [_s, c] : prim->polys) {
                new_poly_count += c > 3 ? c - 2 : 1;
                new_loops_count += c > 3 ? 3 * (c - 2) : c;
            }

            {
                AttrVector<int> loops;
                loops.values.reserve(new_loops_count);
                std::vector<int> loops_mapping_old;
                loops_mapping_old.reserve(new_loops_count);
                for (auto j = 0; j < prim->polys.size(); j++) {
                    auto [s, c] = prim->polys[j];
                    if (c > 3) {
                        for (auto i = 0; i < c - 2; i++) {
                            loops.emplace_back(prim->loops[s]);
                            loops.emplace_back(prim->loops[s + 1 + i]);
                            loops.emplace_back(prim->loops[s + 2 + i]);
                            loops_mapping_old.emplace_back(s);
                            loops_mapping_old.emplace_back(s + 1 + i);
                            loops_mapping_old.emplace_back(s + 2 + i);
                        }
                    }
                    else {
                        for (auto i = s; i < s + c; i++) {
                            loops.emplace_back(prim->loops[i]);
                            loops_mapping_old.emplace_back(i);
                        }
                    }
                }
                prim->loops.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    auto& attr = loops.add_attr<T>(key);
                    for (auto i = 0; i < attr.size(); i++) {
                        attr[i] = arr[loops_mapping_old[i]];
                    }
                    });
                prim->loops = loops;
            }
            {
                AttrVector<vec2i> polys;
                polys.values.reserve(new_poly_count);
                std::vector<int> polys_mapping_old;
                polys_mapping_old.reserve(new_poly_count);
                for (auto j = 0; j < prim->polys.size(); j++) {
                    auto [_s, c] = prim->polys[j];
                    if (c > 3) {
                        for (auto i = 0; i < c - 2; i++) {
                            polys.values.emplace_back(0, 3);
                            polys_mapping_old.emplace_back(j);
                        }
                    }
                    else {
                        polys.values.emplace_back(0, c);
                        polys_mapping_old.emplace_back(j);
                    }
                }
                int start = 0;
                for (auto i = 0; i < polys.size(); i++) {
                    auto [_s, c] = polys[i];
                    polys[i] = { start, c };
                    start += c;
                }
                prim->polys.foreach_attr<AttrAcceptAll>([&](auto const& key, auto& arr) {
                    using T = std::decay_t<decltype(arr[0])>;
                    auto& attr = polys.add_attr<T>(key);
                    for (auto i = 0; i < attr.size(); i++) {
                        attr[i] = arr[polys_mapping_old[i]];
                    }
                    });
                prim->polys = polys;
            }
        }
    }

}