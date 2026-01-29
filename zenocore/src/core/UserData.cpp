#include <zeno/types/UserData.h>
#include <zeno/utils/helper.h>

namespace zeno
{
    size_t UserData::get_string(const char* key, const char* defl, char* ret_buf, size_t cap) const override {
        std::string skey(key);
        std::string sval = get2<std::string>(skey, zsString2Std(defl));
        return stdStr2charArr(sval, ret_buf, cap);
    }

}