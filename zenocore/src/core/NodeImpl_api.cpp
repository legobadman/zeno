#include <zeno/core/NodeImpl.h>
#include <zenum.h>

namespace zeno
{
	IObject2* NodeImpl::get_input_object(const char* param) {
		return nullptr;
	}

	IObject2* NodeImpl::clone_input_object(const char* param) {
		return nullptr;
	}

	IPrimitiveObject* NodeImpl::get_input_PrimitiveObject(const char* param) {
		return nullptr;
	}

	IGeometryObject* NodeImpl::get_input_Geometry(const char* param) {
		return nullptr;
	}

	IGeometryObject* NodeImpl::clone_input_Geometry(const char* param) {
		return nullptr;
	}

	IListObject* NodeImpl::get_input_ListObject(const char* param) {
		return nullptr;
	}

	int NodeImpl::get_input2_int(const char* param) {
		return -1;
	}

	float NodeImpl::get_input2_float(const char* param) {
		return -1;
	}

	int NodeImpl::get_input2_string(const char* param, char* ret, int cap) {
		return -1;
	}

	bool NodeImpl::get_input2_bool(const char* param) {
		return false;
	}

	bool NodeImpl::has_input(const char* param) {
		return false;
	}

	bool NodeImpl::has_link_input(const char* param) {
		return false;
	}

	Vec2i NodeImpl::get_input2_vec2i(const char* param) {
		return Vec2i();
	}

	Vec2f NodeImpl::get_input2_vec2f(const char* param) {
		return Vec2f();
	}

	Vec3i NodeImpl::get_input2_vec3i(const char* param) {
		return Vec3i();
	}

	Vec3f NodeImpl::get_input2_vec3f(const char* param) {
		return Vec3f();
	}

	Vec4i NodeImpl::get_input2_vec4i(const char* param) {
		return Vec4i();
	}

	Vec4f NodeImpl::get_input2_vec4f(const char* param) {
		return Vec4f();
	}

	int NodeImpl::get_param_int(const char* param) {
		return -1;
	}

	float NodeImpl::get_param_float(const char* param) {
		return 0;
	}

	int NodeImpl::get_param_string(const char* param, char* buf, int bufsize) {
		return -1;
	}

	bool NodeImpl::get_param_bool(const char* param) {
		return false;
	}

	bool NodeImpl::set_output_object(const char* param, IObject2* pObject) {
		return false;
	}

	bool NodeImpl::set_output_int(const char* param, int val) {
		return false;
	}

	bool NodeImpl::set_output_float(const char* param, float val) {
		return false;
	}

	bool NodeImpl::set_output_bool(const char* param, bool val) {
		return false;
	}

	bool NodeImpl::set_output_string(const char* param, const char* val) {
		return false;
	}

	bool NodeImpl::set_output_vec2f(const char* param, Vec2f val) {
		return false;
	}

	bool NodeImpl::set_output_vec2i(const char* param, Vec2i val) {
		return false;
	}

	bool NodeImpl::set_output_vec3f(const char* param, Vec3f val) {
		return false;
	}

	bool NodeImpl::set_output_vec3i(const char* param, Vec3i val) {
		return false;
	}

	bool NodeImpl::set_output_vec4f(const char* param, Vec4f val) {
		return false;
	}

	bool NodeImpl::set_output_vec4i(const char* param, Vec4i val) {
		return false;
	}

	int NodeImpl::GetFrameId() const {
		return -1;
	}
}