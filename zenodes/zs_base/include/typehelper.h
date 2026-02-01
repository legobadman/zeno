#pragma once

#include <string>

namespace zeno {
	inline std::string get_string_param(INodeData* ptrNodeData, const std::string& param) {
		if (!ptrNodeData) return "";
		char buf[256];
		ptrNodeData->get_input2_string(param.c_str(), buf, sizeof(buf));
		return std::string(buf);
	}
}

