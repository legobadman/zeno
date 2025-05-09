#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/format.h>
#include <zeno/utils/fileio.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/utils/string.h>
#include <zeno/utils/logger.h>
#include <string_view>
#include <algorithm>
#include <regex>
#include <charconv>

namespace zeno {
namespace {

struct MakeWritePath : zeno::INode {
    virtual void apply() override {
        ZImpl(set_primitive_output("path", ZImpl(get_param<std::string>("path"))));
    }
};

ZENDEFNODE(MakeWritePath, {
    {{gParamType_String, "path", "", NoSocket, WritePathEdit}},
    {{gParamType_String, "path"}},
    {},
    {"string"},
});

struct MakeReadPath : zeno::INode {
    virtual void apply() override {
        ZImpl(set_primitive_output("path", ZImpl(get_param<std::string>("path"))));
    }
};

ZENDEFNODE(MakeReadPath, {
    {{gParamType_String, "path", "", NoSocket, ReadPathEdit}},
    {{gParamType_String, "path"}},
    {},
    {"string"},
});

struct MakeString : zeno::INode {
    virtual void apply() override {
        ZImpl(set_primitive_output("value", ZImpl(get_param<std::string>("value"))));
    }
};

ZENDEFNODE(MakeString, {
    {},
    {{gParamType_String, "value"}},
    {{gParamType_String, "value", ""}},
    {"string"},
});

struct MakeMultilineString : MakeString {
};

ZENDEFNODE(MakeMultilineString, {
    {},
    {{gParamType_String, "value"}},
    {{gParamType_String, "value", "", "", zeno::Multiline}},
    {"string"},
});

struct StringEqual : zeno::INode {
    virtual void apply() override {
        auto lhs = ZImpl(get_input2<std::string>("lhs"));
        auto rhs = ZImpl(get_input2<std::string>("rhs"));
        ZImpl(set_output2("isEqual", lhs == rhs));
    }
};

ZENDEFNODE(StringEqual, {
    {{gParamType_String, "lhs"}, {gParamType_String, "rhs"}},
    {{gParamType_Bool, "isEqual"}},
    {},
    {"string"},
});

struct PrintString : zeno::INode {
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("str"));
        printf("%s\n", str.c_str());
    }
};

ZENDEFNODE(PrintString, {
    {{gParamType_String, "str"}},
    {},
    {},
    {"string"},
});

struct FileWriteString
    : zeno::INode
{
    virtual void apply() override
    {
        auto path = ZImpl(get_input<zeno::StringObject>("path"))->get();
        path = create_directories_when_write_file(path);
        auto str = ZImpl(get_input<zeno::StringObject>("str"))->get();
        zeno::file_put_content(path, str);
    }
};

ZENDEFNODE(
    FileWriteString,
    {
        {
            {gParamType_String, "str", ""},
            {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::WritePathEdit},
        },
        {},
        {},
        {"string"},
    });

struct FileReadString
    : zeno::INode
{
    virtual void apply() override
    {
        auto path = ZImpl(get_input<zeno::StringObject>("path"))->get();
        auto str = zeno::file_get_content(path);
        ZImpl(set_output2("str", std::move(str)));
    }
};

ZENDEFNODE(
    FileReadString,
    {
        {
            {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::ReadPathEdit},
        },
        {
            {gParamType_String, "str"},
        },
        {},
        {"string"},
    });

struct StringFormat : zeno::INode {
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("str"));
        for (int i = 0; i < str.size() - 1; i++) {
            if (str[i] == '$' && str[i + 1] == 'F') {
                str.replace(i, 2, std::to_string(ZImpl(getGlobalState())->getFrameId()));
                break;
            }
        }
        ZImpl(set_output2("str", str));
    }
};

ZENDEFNODE(StringFormat, {
    {{gParamType_String, "str"}},
    {{gParamType_String, "str"}},
    {},
    {"deprecated"},
});

struct StringFormatNumber : zeno::INode {
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("str"));
        auto num = ZImpl(get_input<zeno::NumericObject>("number"));

        std::string output;
        std::visit([&](const auto &v) {
            //using T = std::decay_t<decltype(v)>;
            //if constexpr (std::is_same_v<T, int>) {
                //output = zeno::format(str, T(v));
                output = zeno::format(str, v);
            //}
            //else if constexpr (std::is_same_v<T, float>) {
                //output = zeno::format(str, T(v));
            //}
            //else {
                //output = str;
            //}
        }, num->value);
        ZImpl(set_output2("str", output));
    }
};

ZENDEFNODE(StringFormatNumber, {
    {
        {gParamType_String, "str", "{}"},
        {gParamType_Float, "number"},
    },
    {{gParamType_String, "str"}},
    {},
    {"deprecated"},
});

struct StringFormatNumStr : zeno::INode {
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("str"));
        auto num_str = ZImpl(get_input<zeno::IObject>("num_str"));
        std::string output;

        std::shared_ptr<zeno::NumericObject> num = std::dynamic_pointer_cast<zeno::NumericObject>(num_str);
        if (num) {
            std::visit([&](const auto &v) {
                output = zeno::format(str, v);
            }, num->value);
        }
        std::shared_ptr<zeno::StringObject> pStr = std::dynamic_pointer_cast<zeno::StringObject>(num_str);
        if (pStr) {
            output = zeno::format(str, pStr->get());
        }
        ZImpl(set_output2("str", output));
    }
};

ZENDEFNODE(StringFormatNumStr, {
    {
        {gParamType_String, "str", "{}"},
        {"object", "num_str"},
    },
    {{gParamType_String, "str"}},
    {},
    {"string"},
});

struct StringRegexMatch : zeno::INode {
    virtual void apply() override {
        using namespace std::regex_constants;

        auto str = ZImpl(get_input2<std::string>("str"));
        auto regex_str = ZImpl(get_input2<std::string>("regex"));

        auto case_sensitive = ZImpl(get_input2<bool>("case_sensitive"));

        auto default_flags = ECMAScript;
        if(!case_sensitive)
            default_flags |= icase;

        std::regex self_regex(regex_str,default_flags);
        int output = std::regex_match(str, self_regex);

        ZImpl(set_output2("output", output));
    }
};

ZENDEFNODE(StringRegexMatch, {
    {
        {gParamType_String, "str", ""},
        {gParamType_String, "regex", ""},
        {gParamType_Bool,"case_sensitive","1"}
    },
    {
        {gParamType_Int, "output"}
    },
    {},
    {"string"},
});

struct StringRegexSearch : zeno::INode {
    virtual void apply() override {
        using namespace std::regex_constants;

        auto str = ZImpl(get_input2<std::string>("str"));
        
        auto regex_str = ZImpl(get_input2<std::string>("regex"));
        auto case_sensitive = ZImpl(get_input2<bool>("case_sensitive"));

        std::smatch res{};

        auto flags = ECMAScript;
        if(!case_sensitive)
            flags |= icase;

        std::regex self_regex(regex_str,flags);

        auto matched_substr_list = std::make_shared<zeno::ListObject>();

        // int search_success = std::regex_search(str,res,self_regex);
        int search_success = 0;
        

        while(std::regex_search(str,res,self_regex)) {
            search_success = 1;
            auto is_first_matched = true;
            for(auto w : res) {
                if(is_first_matched) {
                    is_first_matched = false;
                    continue;
                }
                auto zstr = std::make_shared<zeno::StringObject>();
                zstr->set(w.str());
                matched_substr_list->m_impl->push_back(std::move(zstr));
            }

            str = res.suffix().str();
        }

        ZImpl(set_output2("search_success",search_success));
        ZImpl(set_output("res",std::move(matched_substr_list)));
    }
};

ZENDEFNODE(StringRegexSearch, {
    {
        {gParamType_String, "str", ""},
        {gParamType_String, "regex", ""},
        {gParamType_Bool,"case_sensitive","1"}
    },
    {
        {gParamType_Int, "search_success"},
        {gParamType_List, "res"}
    },
    {},
    {"string"},
});


struct StringSplitAndMerge: zeno::INode{
    
    std::vector<std::string> split(const std::string& s, std::string seperator)
    {
        std::vector<std::string> output;

        std::string::size_type prev_pos = 0, pos = 0;

        while((pos = s.find(seperator, pos)) != std::string::npos)
        {
            std::string substring( s.substr(prev_pos, pos-prev_pos) );

            output.push_back(substring);

            prev_pos = ++pos;
        }

        output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

        return output;
    }
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("str"));
        auto schar = ZImpl(get_input2<std::string>("schar"));
        auto merge = ZImpl(get_input2<std::string>("merge"));
        
        const auto &strings = split(str, schar);
        const auto &merges = split(merge, ",");
        std::string outputstr = "";
        for(auto idx:merges)
        {
            outputstr += strings[std::atoi(idx.c_str())];
        }

        ZImpl(set_output2("output", outputstr));
    }
};

ZENDEFNODE(StringSplitAndMerge, {
    {
        {gParamType_String, "str", ""},
        {gParamType_String, "schar", "_"},
        {gParamType_String, "merge", "0"},
    },
    {
        {gParamType_String, "output"}
    },
    {},
    {"deprecated"},
});

std::vector<std::string_view> stringsplit(std::string_view str, std::string_view delims = " ")//do not keep empty
{
	std::vector<std::string_view> output;
        size_t pos = 0;
        size_t posbegin = 0;
        std::string_view word;
        while ((pos = str.find(delims, pos)) != std::string_view::npos) {
            word = str.substr(posbegin, pos-posbegin);
            output.push_back(word);
            pos += delims.length();
            posbegin = pos;
        }
        if (posbegin < str.length()) { 
            word = str.substr(posbegin);
            output.push_back(word);
        }
	return output;
}

struct StringSplitAndMerge2: zeno::INode{
    virtual void apply() override {
        auto str = ZImpl(get_input2<std::string>("String"));
        auto separator = ZImpl(get_input2<std::string>("Separator"));
        auto mergeMethod = ZImpl(get_input2<std::string>("Merge Method"));
        auto mergeIndex = ZImpl(get_input2<std::string>("Merge index"));
        auto clipCountFromStart = ZImpl(get_input2<int>("Clip Count From Start"));
        auto clipCountFromEnd = ZImpl(get_input2<int>("Clip Count From End"));
        auto remainSeparator = ZImpl(get_input2<bool>("Remain Separator"));
        auto splitStr = stringsplit(str, separator);
        std::string output;
        output.reserve(str.size());
        if (mergeMethod == "Custom_Index_Merge") {
            std::vector<std::string_view> mergeIndexList = stringsplit(mergeIndex, ",");
            for (size_t j = 0; j < mergeIndexList.size(); ++j) {
                auto idx = mergeIndexList[j];
                if (idx.empty()) continue;
                int i;
                auto result = std::from_chars(idx.data(), idx.data() + idx.size(), i);
                if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range || result.ptr != idx.data() + idx.size()) {
                    throw std::runtime_error("[StringSplitAndMerge2] Merge index is not a valid number.");//invalid_argument, result_out_of_range, or not all characters are parsed(eg. 123a)
                }
                if (i < 0) i = splitStr.size() + i;
                if (i < 0 || i >= splitStr.size()) {
                    throw std::runtime_error("[StringSplitAndMerge2] Merge index is out of range.");
                }
                output += splitStr[i];
                if (remainSeparator && j != mergeIndexList.size() - 1) {
                    output += separator;
                }
            }
        }
        else if (mergeMethod == "Clip_And_Merge") {
            int start = std::max(0, clipCountFromStart);
            int end = std::max(start, static_cast<int>(splitStr.size()) - clipCountFromEnd);
            for (int i = start; i < end; ++i) {
                output += splitStr[i];
                if (remainSeparator && i != end - 1) {
                    output += separator;
                }
            }
        }
        else {
            throw std::runtime_error("[StringSplitAndMerge2] Unknown merge method.");
        }
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringSplitAndMerge2, {
    {
        {gParamType_String, "String", "", Socket_Primitve, Multiline},
        {gParamType_String, "Separator", "_"},
        {"enum Custom_Index_Merge Clip_And_Merge", "Merge Method", "Custom_Index_Merge"},
        {gParamType_String, "Merge index", "0,1"},
        {gParamType_Int, "Clip Count From Start", "0"},
        {gParamType_Int, "Clip Count From End", "0"},
        {gParamType_Bool, "Remain Separator", "false"},
    },
    {
        {gParamType_String, "string"}
    },
    {},
    {"string"},
});

struct FormatString : zeno::INode {
    virtual void apply() override {
        auto formatStr = ZImpl(get_input2<std::string>("str"));

        auto list = ZImpl(get_input<zeno::ListObject>("args"));
        std::string output = formatStr;
        for (auto obj : list->m_impl->get())
        {
            std::shared_ptr<zeno::NumericObject> num = std::dynamic_pointer_cast<zeno::NumericObject>(obj);
            if (num) {
                std::visit([&](const auto& v) {
                    output = zeno::format(output, v);
                    }, num->value);
            }
            std::shared_ptr<zeno::StringObject> pStr = std::dynamic_pointer_cast<zeno::StringObject>(obj);
            if (pStr) {
                output = zeno::format(output, pStr->get());
            }
        }

        ZImpl(set_output2("str", output));
    }
};

ZENDEFNODE(FormatString, {
    {
        {gParamType_String, "str", "{}"},
        {gParamType_List, "args"},
    },
    {{gParamType_String, "str"}},
    {},
    {"string"},
});


/*static int objid = 0;

struct ExportPath : zeno::INode {  // deprecated
    virtual void apply() override {
        char buf[100];
        auto ext = ZImpl(get_param<std::string>("ext"));
        sprintf(buf, "%06d", getGlobalState()->getFrameId());
        auto path = fs::path(getGlobalState()->iopath) / buf;
        if (!fs::is_directory(path)) {
            fs::create_directory(path);
        }
        sprintf(buf, "%06d.%s", objid++, ext.c_str());
        path /= buf;
        auto ret = std::make_unique<zeno::StringObject>();
        //printf("EXPORTPATH: %s\n", path.c_str());
        ret->set(path.string());
        ZImpl(set_output("path", std::move(ret)));
    }
};

ZENDEFNODE(ExportPath, {
    {},
    {"path"},
    {{gParamType_String, "ext", "zpm"}},
    {"fileio"},
});

struct EndFrame : zeno::INode {  // deprecated
    virtual void apply() override {
        char buf[100];
        sprintf(buf, "%06d", getGlobalState()->getFrameId());
        auto path = fs::path(getGlobalState()->iopath) / buf;
        if (!fs::is_directory(path)) {
            fs::create_directory(path);
        }
        path /= "done.lock";
        std::ofstream ofs(path.string());
        ofs.write("DONE", 4);
        objid = 0;
    }
};

ZENDEFNODE(EndFrame, {
    {"chain"},
    {},
    {},
    {"fileio"},
});*/

struct StringToNumber : zeno::INode {
    virtual void apply() override {
        auto in_str = ZImpl(get_input2<std::string>("str"));
        auto type = ZImpl(get_input2<std::string>("type"));
        if (type == "float") {
            float v = std::stof(in_str);
            ZImpl(set_primitive_output("num_str", v));
        }
        else if (type == "int") {
            int v = std::stoi(in_str);
            ZImpl(set_primitive_output("num_str", v));
        }
        else {
            throw zeno::makeError("Unknown type");
        }
    }
};

ZENDEFNODE(StringToNumber, {{
                                /* inputs: */
                                {"enum float int", "type", "float"},
                                {gParamType_String, "str", "0"},
                            },

                            {
                                /* outputs: */
                                {"NumericObject","num_str"},
                            },

                            {
                                /* params: */

                            },

                            {
                                /* category: */
                                "string",
                            }});

std::string& trim(std::string &s) 
{
    if (s.empty()) 
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" \f\n\r\t\v"));
    s.erase(s.find_last_not_of(" \f\n\r\t\v") + 1);
    return s;
}

struct StringToList : zeno::INode {
    virtual void apply() override {
        auto stringlist = ZImpl(get_input2<std::string>("string"));
        auto list = create_ListObject();
        auto separator = ZImpl(get_input2<std::string>("Separator"));
        auto trimoption = ZImpl(get_input2<bool>("Trim"));
        auto keepempty = ZImpl(get_input2<bool>("KeepEmpty"));
        std::vector<std::string> strings;
        size_t pos = 0;
        size_t posbegin = 0;
        std::string word;
        while ((pos = stringlist.find(separator, pos)) != std::string::npos) {
            word = stringlist.substr(posbegin, pos-posbegin);
            if(trimoption) trim(word);
            if(keepempty || !word.empty()) strings.push_back(word);
            pos += separator.length();
            posbegin = pos;
        }
        if (posbegin < stringlist.length()) { //push last word
            word = stringlist.substr(posbegin);
            if(trimoption) trim(word);
            if(keepempty || !word.empty()) strings.push_back(word);
        }
        for(const auto &string : strings) {
            auto obj = std::make_unique<StringObject>();
            obj->set(string);
            list->m_impl->push_back(std::move(obj));
        }
        ZImpl(set_output("list", std::move(list)));
    }
};

ZENDEFNODE(StringToList, {
    {
        {gParamType_String, "string", ""},
        {gParamType_String, "Separator", ""},
        {gParamType_Bool, "Trim", "false"},
        {gParamType_Bool, "KeepEmpty", "false"},
    },
    {{gParamType_List, "list"},
    },
    {},
    {"string"},
});

struct StringJoin : zeno::INode {//zeno string only support list for now
    virtual void apply() override {
        auto list = ZImpl(get_input<zeno::ListObject>("list"));
        auto stringvec = list->m_impl->get2<std::string>();
        auto separator = ZImpl(get_input2<std::string>("Separator"));
        auto output = join_str(stringvec, separator);
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringJoin, {
    {
        {gParamType_List, "list"},
        {gParamType_String, "Separator", ""},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct NumbertoString : zeno::INode {
    virtual void apply() override {
        auto num = ZImpl(get_input<zeno::NumericObject>("number"));
        std::visit([&](const auto &v) {
            ZImpl(set_primitive_output("string", zeno::to_string(v)));
        }, num->value);
    }
};

ZENDEFNODE(NumbertoString, {
    {
        {gParamType_Float, "number"},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

std::string strreplace(std::string textToSearch, std::string_view toReplace, std::string_view replacement)
{
    size_t pos = 0;
    for (;;)
    {
        pos = textToSearch.find(toReplace, pos);
        if (pos == std::string::npos)
            return textToSearch;
        textToSearch.replace(pos, toReplace.length(), replacement);
        pos += replacement.length();
    }
}

struct StringReplace : zeno::INode {
    virtual void apply() override {
        std::string string = ZImpl(get_input2<std::string>("string"));
        std::string oldstr = ZImpl(get_input2<std::string>("old"));
        std::string newstr = ZImpl(get_input2<std::string>("new"));
        if (oldstr.empty()) {
            zeno::log_error("[StringReplace] old string is empty.");
            return;
        }
        auto output = strreplace(string, oldstr, newstr);
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringReplace, {
    {
        {gParamType_String, "string", "", Socket_Primitve, Multiline},
        {gParamType_String, "old", ""},
        {gParamType_String, "new", ""},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringFind : zeno::INode {//return -1 if not found
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        auto substring = ZImpl(get_input2<std::string>("substring"));
        auto start = ZImpl(get_input2<int>("start"));
        std::string::size_type n = string.find(substring, start);
        int output = (n == std::string::npos) ? -1 : static_cast<int>(n);
        ZImpl(set_output2("Position", output));
    }
};

ZENDEFNODE(StringFind, {
    {
        {gParamType_String, "string", "", Socket_Primitve, Multiline},
        {gParamType_String, "substring", ""},
        {gParamType_Int, "start", "0"},
    },
    {{gParamType_Int, "Position"},
    },
    {},
    {"string"},
});

struct SubString : zeno::INode {//slice...
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        auto start = ZImpl(get_input2<int>("start"));
        auto length = ZImpl(get_input2<int>("length"));
        if (start < 0) {
            start = string.size() + start;
        }
        if (start < 0 || start >= string.size()) {
            throw std::runtime_error("[SubString] start is out of range.");
        }
        auto output = string.substr(start, length);
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(SubString, {
    {
        {gParamType_String, "string", "", Socket_Primitve, Multiline},
        {gParamType_Int, "start", "0"},
        {gParamType_Int, "length", "1"},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringtoLower : zeno::INode {
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        std::string output = string;
        std::transform(output.begin(), output.end(), output.begin(), [] (auto c) { 
            return static_cast<char> (std::tolower (static_cast<unsigned char> (c))); });
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringtoLower, {
    {
        {gParamType_String, "string", ""},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringtoUpper : zeno::INode {
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        std::string output = string;
        std::transform(output.begin(), output.end(), output.begin(), [] (auto c) { 
            return static_cast<char> (std::toupper (static_cast<unsigned char> (c))); });
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringtoUpper, {
    {
        {gParamType_String, "string", ""},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringLength : zeno::INode {
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        int output = string.length();
        ZImpl(set_output2("length", output));
    }
};

ZENDEFNODE(StringLength, {
    {
        {gParamType_String, "string", ""},
    },
    {{gParamType_Int, "length"},
    },
    {},
    {"string"},
});

struct StringSplitPath : zeno::INode {
    virtual void apply() override {
        auto stringpath = ZImpl(get_input2<std::string>("string"));
        bool SplitExtension = ZImpl(get_input2<bool>("SplitExtension"));
        std::string directory, filename, extension;
        std::string::size_type last_slash_pos = stringpath.find_last_of("/\\");
        std::string::size_type last_dot_pos = stringpath.find_last_of('.');
        if (last_slash_pos == std::string::npos) {
            directory = "";
            filename = (last_dot_pos == std::string::npos) ? stringpath : stringpath.substr(0, last_dot_pos);
            extension = (last_dot_pos == std::string::npos) ? "" : stringpath.substr(last_dot_pos + 1);
        }
        else {
            directory = stringpath.substr(0, last_slash_pos);
            filename = stringpath.substr(last_slash_pos + 1, (last_dot_pos == std::string::npos ? stringpath.length() - last_slash_pos - 1 : last_dot_pos - last_slash_pos - 1));
            extension = (last_dot_pos == std::string::npos) ? "" : stringpath.substr(last_dot_pos + 1);
        }
        if(!SplitExtension) filename += extension;//extension output is empty if SplitExtension is false
        ZImpl(set_output2("directory", directory));
        ZImpl(set_output2("filename", filename));
        ZImpl(set_output2("extension", extension));
    }
};

ZENDEFNODE(StringSplitPath, {
    {
        {gParamType_String, "string", "", zeno::Socket_Primitve, zeno::ReadPathEdit},
        {gParamType_Bool, "SplitExtension", "true"},
    },
    {{gParamType_String, "directory"},
    {gParamType_String, "filename"},
    {gParamType_String, "extension"},
    },
    {},
    {"string"},
});

struct StringInsert : zeno::INode {//if start is less than 0, reverse counting from the end
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        auto substring = ZImpl(get_input2<std::string>("substring"));
        auto start = ZImpl(get_input2<int>("start"));
        auto output = string;
        if (start < 0) {
            start = output.size() + start + 1;
        } 
        if (start < 0 || start > string.size()) {
            throw std::runtime_error("[StringInsert] start is out of range.");
        }
        output.insert(start, substring);
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringInsert, {
    {
        {gParamType_String, "string", "", Socket_Primitve, Multiline},
        {gParamType_String, "substring", ""},
        {gParamType_Int, "start", "0"},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringTrim : zeno::INode {
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("string"));
        auto trimleft = ZImpl(get_input2<bool>("trimleft"));
        auto trimright = ZImpl(get_input2<bool>("trimright"));
        std::string output = string;
        if (!output.empty()) {
            if (trimleft) {
                output.erase(output.begin(), std::find_if(output.begin(), output.end(), [](int ch) {
                    return !std::isspace(ch);
                }));
            }
            if (trimright) {
                output.erase(std::find_if(output.rbegin(), output.rend(), [](int ch) {
                    return !std::isspace(ch);
                }).base(), output.end());
            }
        }
        ZImpl(set_output2("string", output));
        
    }
};

ZENDEFNODE(StringTrim, {
    {
        {gParamType_String, "string", ""},
        {gParamType_Bool, "trimleft", "true"},
        {gParamType_Bool, "trimright", "true"},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringDeleteOrReplace : zeno::INode {
    virtual void apply() override {
        std::string multiline_string = ZImpl(get_input2<std::string>("String"));
        std::string oldString = ZImpl(get_input2<std::string>("oldString"));
        std::string RefString = ZImpl(get_input2<std::string>("RefString"));
        auto N = ZImpl(get_input2<int>("N"));
        std::string newString = ZImpl(get_input2<std::string>("newString"));
        bool UseLastRefString = ZImpl(get_input2<bool>("UseLastRefString"));
        std::string output = multiline_string;
        if(oldString == "AllRefString") {
            output = strreplace(multiline_string, RefString, newString);
        }
        else if(oldString == "First_N_characters") {
            if(N >= 0 && N <= multiline_string.size()) {
                output.replace(0, N, newString);
            }
            else {
                //zeno::log_error("[StringDeleteOrReplace] N is out of range.");
                throw std::runtime_error("[StringDeleteOrReplace] N is out of range.");
            }
        }
        else if(oldString == "Last_N_characters") {
            if(N >= 0 && N <= multiline_string.size()) {
                output.replace(multiline_string.size() - N, N, newString);
            }
            else {
                throw std::runtime_error("[StringDeleteOrReplace] N is out of range.");
            }
        }
        else if(oldString == "All_characters_before_RefString") {
            auto pos = UseLastRefString ? multiline_string.rfind(RefString) : multiline_string.find(RefString);
            if(pos != std::string::npos) {
                output.replace(0, pos, newString);
            }
            else {
                throw std::runtime_error("[StringDeleteOrReplace] RefString not found.");
            }
        }
        else if(oldString == "N_characters_before_RefString") {
            auto pos = UseLastRefString ? multiline_string.rfind(RefString) : multiline_string.find(RefString);
            if(pos != std::string::npos && pos >= N) {
                output.replace(pos - N, N, newString);
            }
            else {
                throw std::runtime_error("[StringDeleteOrReplace] RefString not found or N is too large.");
            }
        }
        else if(oldString == "All_characters_after_RefString") {
            auto pos = UseLastRefString ? multiline_string.rfind(RefString) : multiline_string.find(RefString);
            if(pos != std::string::npos) {
                output.replace(pos + RefString.size(), multiline_string.size() - pos - RefString.size(), newString);
            }
            else {
                throw std::runtime_error("[StringDeleteOrReplace] RefString not found.");
            }
        }
        else if(oldString == "N_characters_after_RefString") {
            auto pos = UseLastRefString ? multiline_string.rfind(RefString) : multiline_string.find(RefString);
            if(pos != std::string::npos && pos + RefString.size() + N <= multiline_string.size()) {
                output.replace(pos + RefString.size(), N, newString);
            }
            else {
                throw std::runtime_error("[StringDeleteOrReplace] RefString not found or N is too large.");
            }
        }
        ZImpl(set_output2("string", output));
    }
};

ZENDEFNODE(StringDeleteOrReplace, {
    {
        {gParamType_String, "String", "", Socket_Primitve, Multiline},
        {"enum AllRefString First_N_characters  Last_N_characters All_characters_before_RefString  N_characters_before_RefString All_characters_after_RefString N_characters_after_RefString", "oldString", "AllRefString"},
        {gParamType_String, "RefString", ""},
        {gParamType_Bool, "UseLastRefString", "false"},
        {gParamType_Int, "N", "1"},
        {gParamType_String, "newString", ""},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

struct StringEditNumber : zeno::INode {
    virtual void apply() override {
        auto string = ZImpl(get_input2<std::string>("String"));
        auto method = ZImpl(get_input2<std::string>("Method"));
        if (method == "Remove_all_numbers") {
            string.erase(std::remove_if(string.begin(), string.end(), [](char c) { return std::isdigit(c); }), string.end());
        }
        else if (method == "Remove_all_non_numbers") {
            string.erase(std::remove_if(string.begin(), string.end(), [](unsigned char c) { return !std::isdigit(c); }), string.end());
        }
        else if (method == "Remove_last_number") {
            auto it = std::find_if(string.rbegin(), string.rend(), [](unsigned char c) { return std::isdigit(c); });
            if (it != string.rend()) {
            string.erase((it+1).base());
            }
        }
        else if (method == "Return_last_number") {
            std::string num = "";
            bool number_found = false;
            for (auto it = string.rbegin(); it != string.rend(); ++it) {
                if (std::isdigit(*it)) {
                    num = *it + num;
                    number_found = true;
                } else if (number_found) {
                    break;
                }
            }
            string = num;
        }
        ZImpl(set_output2("string", string));
        
    }
};

ZENDEFNODE(StringEditNumber, {
    {
        {gParamType_String, "String", "", Socket_Primitve, Multiline},
        {"enum Remove_all_numbers Remove_all_non_numbers Remove_last_number Return_last_number_Sequence", "Method", "Remove_all_numbers"},
    },
    {{gParamType_String, "string"},
    },
    {},
    {"string"},
});

}
}
