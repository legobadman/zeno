#include <zeno/core/NodeImpl.h>
#include <zenum.h>
#include <zeno/utils/helper.h>
#include <zeno/utils/interfaceutil.h>
#include "zeno_types/reflect/reflection.generated.hpp"

namespace zeno
{
	IObject2* NodeImpl::get_input_object(const char* param) {
		return get_input_obj(std::string(param));
	}

	IObject2* NodeImpl::clone_input_object(const char* param) {
		return clone_input(std::string(param)).release();
	}

	IPrimitiveObject* NodeImpl::get_input_PrimitiveObject(const char* param) {
		auto obj = get_input_object(param);
		if (ZObj_Geometry == obj->type()) {
			return static_cast<IPrimitiveObject*>(obj);
		}
		else {
			return nullptr;
		}
	}

	IGeometryObject* NodeImpl::get_input_Geometry(const char* param) {
		auto obj = get_input_object(param);
		if (ZObj_Geometry == obj->type()) {
			return static_cast<IGeometryObject*>(obj);
		}
		else {
			return nullptr;
		}
	}

	IGeometryObject* NodeImpl::clone_input_Geometry(const char* param) {
		auto obj = get_input_object(param);
		if (ZObj_Geometry == obj->type()) {
			return static_cast<IGeometryObject*>(obj->clone());
		}
		else {
			return nullptr;
		}
	}

	IListObject* NodeImpl::get_input_ListObject(const char* param) {
		auto obj = get_input_object(param);
		if (ZObj_List == obj->type()) {
			return static_cast<IListObject*>(obj);
		}
		else {
			return nullptr;
		}
	}

	int NodeImpl::get_input2_int(const char* param) {
		const auto& anyVal = get_param_result(std::string(param));
		int res = 0;
		if (anyVal.type().hash_code() == zeno::types::gParamType_Int) {
			res = zeno::reflect::any_cast<int>(anyVal);
		}
		else if (anyVal.type().hash_code() == zeno::types::gParamType_Float) {
			res = zeno::reflect::any_cast<float>(anyVal);
		}
		else if (anyVal.type().hash_code() == zeno::types::gParamType_Bool) {
			res = zeno::reflect::any_cast<bool>(anyVal);
		}
		return res;
	}

	float NodeImpl::get_input2_float(const char* param) {
		const auto& anyVal = get_param_result(std::string(param));
		float res = 0;
		if (anyVal.type().hash_code() == zeno::types::gParamType_Int) {
			res = zeno::reflect::any_cast<int>(anyVal);
		}
		else if (anyVal.type().hash_code() == zeno::types::gParamType_Float) {
			res = zeno::reflect::any_cast<float>(anyVal);
		}
		else if (anyVal.type().hash_code() == zeno::types::gParamType_Bool) {
			res = zeno::reflect::any_cast<bool>(anyVal);
		}
		return res;
	}

	int NodeImpl::get_input2_string(const char* param, char* ret, size_t cap) {
		const auto& str = any_cast<std::string>(get_param_result(std::string(param)));
		return stdStr2charArr(str, ret, cap);
	}

	std::string NodeImpl::get_input2_string(const std::string& param) {
		return any_cast<std::string>(get_param_result(std::string(param)));
	}

	bool NodeImpl::get_input2_bool(const char* param) {
		return any_cast<bool>(get_param_result(std::string(param)));
	}

	bool NodeImpl::has_input(const char* param) {
		return has_input(std::string(param));
	}

	bool NodeImpl::has_link_input(const char* param) {
		return has_link_input(std::string(param));
	}

	Vec2i NodeImpl::get_input2_vec2i(const char* param) {
		return toAbiVec2i(any_cast<vec2i>(get_param_result(zsString2Std(param))));
	}

	Vec2f NodeImpl::get_input2_vec2f(const char* param) {
		return toAbiVec2f(any_cast<vec2f>(get_param_result(zsString2Std(param))));
	}

	Vec3i NodeImpl::get_input2_vec3i(const char* param) {
		return toAbiVec3i(any_cast<vec3i>(get_param_result(zsString2Std(param))));
	}

	Vec3f NodeImpl::get_input2_vec3f(const char* param) {
		return toAbiVec3f(any_cast<vec3f>(get_param_result(zsString2Std(param))));
	}

	Vec4i NodeImpl::get_input2_vec4i(const char* param) {
		return toAbiVec4i(any_cast<vec4i>(get_param_result(zsString2Std(param))));
	}

	Vec4f NodeImpl::get_input2_vec4f(const char* param) {
		return toAbiVec4f(any_cast<vec4f>(get_param_result(zsString2Std(param))));
	}

	bool NodeImpl::set_output_object(const char* param, IObject2* detached_obj) {
		return set_output(std::string(param), zany2(detached_obj));
	}

	bool NodeImpl::set_output_int(const char* param, int val) {
		return set_primitive_output(zsString2Std(param), val);
	}

	bool NodeImpl::set_output_float(const char* param, float val) {
		return set_primitive_output(zsString2Std(param), val);
	}

	bool NodeImpl::set_output_bool(const char* param, bool val) {
		return set_primitive_output(zsString2Std(param), val);
	}

	bool NodeImpl::set_output_string(const char* param, const char* val) {
		return set_primitive_output(zsString2Std(param), zsString2Std(val));
	}

	bool NodeImpl::set_output_vec2f(const char* param, Vec2f val) {
		return set_primitive_output(zsString2Std(param), toVec2f(val));
	}

	bool NodeImpl::set_output_vec2i(const char* param, Vec2i val) {
		return set_primitive_output(zsString2Std(param), toVec2i(val));
	}

	bool NodeImpl::set_output_vec3f(const char* param, Vec3f val) {
		return set_primitive_output(zsString2Std(param), toVec3f(val));
	}

	bool NodeImpl::set_output_vec3i(const char* param, Vec3i val) {
		return set_primitive_output(zsString2Std(param), toVec3i(val));
	}

	bool NodeImpl::set_output_vec4f(const char* param, Vec4f val) {
		return set_primitive_output(zsString2Std(param), toVec4f(val));
	}

	bool NodeImpl::set_output_vec4i(const char* param, Vec4i val) {
		return set_primitive_output(zsString2Std(param), toVec4i(val));
	}

	int NodeImpl::GetFrameId() const {
		return getGlobalState()->getFrameId();
	}
}