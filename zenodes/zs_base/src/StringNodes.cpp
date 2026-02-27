#include "simple_geometry_common.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace zeno {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static std::string& trim(std::string& s) {
    if (s.empty()) return s;
    s.erase(0, s.find_first_not_of(" \f\n\r\t\v"));
    s.erase(s.find_last_not_of(" \f\n\r\t\v") + 1);
    return s;
}

struct GetStringFromList : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* list = nd->get_input_ListObject("list");
        if (!list) {
            throw std::runtime_error("GetStringFromList: input `list` is null");
        }

        const int index = nd->get_input2_int("index");
        if (index < 0) {
            throw std::runtime_error("GetStringFromList: index must be non-negative");
        }

        std::vector<char*> buf(list->size(), nullptr);
        const size_t n = list->get_string_arr(buf.data(), buf.size());
        if (static_cast<size_t>(index) >= n) {
            throw std::runtime_error("GetStringFromList: index out of range");
        }

        const char* sval = buf[static_cast<size_t>(index)] ? buf[static_cast<size_t>(index)] : "";
        nd->set_output_string("string", sval);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(GetStringFromList,
    Z_INPUTS(
        {"list", _gParamType_List},
        {"index", _gParamType_Int, ZInt(0)}
    ),
    Z_OUTPUTS(
        {"string", _gParamType_String}
    ),
    "string",
    "",
    "",
    ""
);

struct StringToList : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        const std::string stringlist = get_input2_string(nd, "string");
        const std::string separator = get_input2_string(nd, "Separator");
        const bool trimoption = nd->get_input2_bool("Trim");
        const bool keepempty = nd->get_input2_bool("KeepEmpty");

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
        nd->set_output_int("count", static_cast<int>(strings.size()));
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(StringToList,
    Z_INPUTS(
        {"string", _gParamType_String, ZString("")},
        {"Separator", _gParamType_String, ZString("")},
        {"Trim", _gParamType_Bool, ZInt(0)},
        {"KeepEmpty", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS(
        {"list", _gParamType_StringList},
        {"count", _gParamType_Int}
    ),
    "string",
    "",
    "",
    ""
);

} // namespace zeno

