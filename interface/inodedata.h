#ifndef __INF_INODE_DATA_H__
#define __INF_INODE_DATA_H__

#include "iobject2.h"

namespace zeno {
	struct INodeData {
		virtual IObject2* get_input_object(const char* param) = 0;
		virtual IObject2* clone_input_object(const char* param) = 0;
		virtual IPrimitiveObject* get_input_PrimitiveObject(const char* param) = 0;
		virtual IGeometryObject* get_input_Geometry(const char* param) = 0;
		virtual IGeometryObject* clone_input_Geometry(const char* param) = 0;
		virtual IListObject* get_input_ListObject(const char* param) = 0;
		virtual int get_input2_int(const char* param) = 0;
		virtual float get_input2_float(const char* param) = 0;
		virtual int get_input2_string(const char* param, char* ret, int cap) = 0;
		virtual bool get_input2_bool(const char* param) = 0;
		virtual bool has_input(const char* param) = 0;
		virtual bool has_link_input(const char* param) = 0;
		virtual Vec2i get_input2_vec2i(const char* param) = 0;
		virtual Vec2f get_input2_vec2f(const char* param) = 0;
		virtual Vec3i get_input2_vec3i(const char* param) = 0;
		virtual Vec3f get_input2_vec3f(const char* param) = 0;
		virtual Vec4i get_input2_vec4i(const char* param) = 0;
		virtual Vec4f get_input2_vec4f(const char* param) = 0;
		virtual int get_param_int(const char* param) = 0;
		virtual float get_param_float(const char* param) = 0;
		virtual int get_param_string(const char* param, char* buf, int bufsize) = 0;
		virtual bool get_param_bool(const char* param) = 0;
		virtual bool set_output_object(const char* param, IObject2* pObject) = 0;
		virtual bool set_output_int(const char* param, int val) = 0;
		virtual bool set_output_float(const char* param, float val) = 0;
		virtual bool set_output_bool(const char* param, bool val) = 0;
		virtual bool set_output_string(const char* param, const char* val) = 0;
		virtual bool set_output_vec2f(const char* param, Vec2f val) = 0;
		virtual bool set_output_vec2i(const char* param, Vec2i val) = 0;
		virtual bool set_output_vec3f(const char* param, Vec3f val) = 0;
		virtual bool set_output_vec3i(const char* param, Vec3i val) = 0;
		virtual bool set_output_vec4f(const char* param, Vec4f val) = 0;
		virtual bool set_output_vec4i(const char* param, Vec4i val) = 0;
		virtual int GetFrameId() const = 0;
	};
}



#endif