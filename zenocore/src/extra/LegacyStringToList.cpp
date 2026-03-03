//
// LegacyStringToList - INode2 implementation
// Splits string by separator, outputs string arr (primitive type)
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <string>
#include <vector>
#include <cstring>

namespace zeno {

namespace {

static std::string& trim(std::string& s) {
    if (s.empty()) return s;
    s.erase(0, s.find_first_not_of(" \f\n\r\t\v"));
    s.erase(s.find_last_not_of(" \f\n\r\t\v") + 1);
    return s;
}

} // namespace

struct LegacyStringToList : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        char buf[4096];
        nd->get_input2_string("string", buf, sizeof(buf));
        std::string stringlist(buf);

        nd->get_input2_string("Separator", buf, sizeof(buf));
        std::string separator(buf);

        bool trimoption = nd->get_input2_bool("Trim");
        bool keepempty = nd->get_input2_bool("KeepEmpty");

        std::vector<std::string> strings;
        if (separator.empty()) {
            std::string word = stringlist;
            if (trimoption) trim(word);
            if (keepempty || !word.empty()) strings.push_back(word);
        } else {
            size_t pos = 0;
            size_t posbegin = 0;
            std::string word;
            while ((pos = stringlist.find(separator, pos)) != std::string::npos) {
                word = stringlist.substr(posbegin, pos - posbegin);
                if (trimoption) trim(word);
                if (keepempty || !word.empty()) strings.push_back(word);
                pos += separator.length();
                posbegin = pos;
            }
            if (posbegin < stringlist.length()) {
                word = stringlist.substr(posbegin);
                if (trimoption) trim(word);
                if (keepempty || !word.empty()) strings.push_back(word);
            } else if (keepempty && posbegin == stringlist.length()) {
                strings.emplace_back("");
            }
        }

        std::vector<const char*> arr;
        arr.reserve(strings.size());
        for (const auto& s : strings) arr.push_back(s.c_str());
        nd->set_output_string_list("list", arr.data(), arr.size());
        return ZErr_OK;
    }
};

ZENDEFNODE(LegacyStringToList, {
    {
        {gParamType_String, "string", ""},
        {gParamType_String, "Separator", ""},
        {gParamType_Bool, "Trim", "false"},
        {gParamType_Bool, "KeepEmpty", "false"},
    },
    {{gParamType_StringList, "list"}},
    {},
    {"string"},
});

} // namespace zeno
