#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/common.h>
#include <vector>
#include <string>
#include <memory>

namespace zeno {

ZENO_API zany2 decodeObject(const char *buf, size_t len);
ZENO_API bool encodeObject(IObject2 *object, std::vector<char> &buf);

}
