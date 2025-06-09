#include <zeno/core/INode.h>
#include <zeno/core/NodeImpl.h>
#include <zeno/utils/interfaceutil.h>


namespace zeno
{
    CustomUI INode::export_customui() const {
        return CustomUI();
    }

    NodeType INode::type() const {
        return Node_Normal;
    }

    zeno::String INode::uuid() const {
        return stdString2zs(m_pAdapter->get_uuid());
    }

    zany INode::get_input(const zeno::String& param) { 
        return m_pAdapter->get_input(zsString2Std(param));
    }

    zany INode::get_output(const zeno::String& param) {
        return m_pAdapter->get_output_obj(zsString2Std(param));
    }

    zeno::SharedPtr<PrimitiveObject> INode::get_input_PrimitiveObject(const zeno::String& param) { 
        return std::dynamic_pointer_cast<PrimitiveObject>(get_input(param));
    }

    zeno::SharedPtr<ListObject> INode::get_input_ListObject(const zeno::String& param) {
        return std::dynamic_pointer_cast<ListObject>(get_input(param));
    }

    zeno::SharedPtr<DictObject> INode::get_input_DictObject(const zeno::String& param) {
        return std::dynamic_pointer_cast<DictObject>(get_input(param));
    }

    int INode::get_input2_int(const zeno::String& param) {
        return any_cast<int>(m_pAdapter->get_param_result(zsString2Std(param)));
    }

    float INode::get_input2_float(const zeno::String& param) {
        return any_cast<float>(m_pAdapter->get_param_result(zsString2Std(param)));
    }

    String INode::get_input2_string(const zeno::String& param) {
        return stdString2zs(any_cast<std::string>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec2i INode::get_input2_vec2i(const zeno::String& param) {
        return toAbiVec2i(any_cast<vec2i>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec2f INode::get_input2_vec2f(const zeno::String& param) {
        return toAbiVec2f(any_cast<vec2f>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec3i INode::get_input2_vec3i(const zeno::String& param) {
        return toAbiVec3i(any_cast<vec3i>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec3f INode::get_input2_vec3f(const zeno::String& param) {
        return toAbiVec3f(any_cast<vec3f>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec4i INode::get_input2_vec4i(const zeno::String& param) {
        return toAbiVec4i(any_cast<vec4i>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    zeno::Vec4f INode::get_input2_vec4f(const zeno::String& param) {
        return toAbiVec4f(any_cast<vec4f>(m_pAdapter->get_param_result(zsString2Std(param))));
    }

    bool INode::get_input2_bool(const zeno::String& param) {
        return any_cast<bool>(m_pAdapter->get_param_result(zsString2Std(param)));
    }

    bool INode::has_input(const zeno::String& param) {
        return m_pAdapter->has_input(zsString2Std(param));
    }


    container_elem_update_info INode::get_input_container_info(const zeno::String& param) {
        return m_pAdapter->get_input_container_info(zsString2Std(param));
    }

    container_elem_update_info INode::get_output_container_info(const zeno::String& param) {
        return m_pAdapter->get_output_container_info(zsString2Std(param));
    }

    void INode::set_input_container_info(const zeno::String& param, const container_elem_update_info& info) {
        m_pAdapter->set_input_container_info(zsString2Std(param), info);
    }

    void INode::set_output_container_info(const zeno::String& param, const container_elem_update_info& info) {
        m_pAdapter->set_output_container_info(zsString2Std(param), info);
    }

    bool INode::set_output(const zeno::String& param, zany pObject) {
        return m_pAdapter->set_output(zsString2Std(param), pObject);
    }

    int INode::get_param_int(const zeno::String& param) {
        return get_input2_int(param);
    }

    float INode::get_param_float(const zeno::String& param) {
        return get_input2_float(param);
    }

    String INode::get_param_string(const zeno::String& param) {
        return get_input2_string(param);
    }

    bool INode::get_param_bool(const zeno::String& param) {
        return get_input2_bool(param);
    }

    void INode::set_output_int(const zeno::String& param, int val) {
        return m_pAdapter->set_output2(zsString2Std(param), val);
    }

    void INode::set_output_float(const zeno::String& param, float val) {
        return m_pAdapter->set_output2(zsString2Std(param), val);
    }

    void INode::set_output_string(const zeno::String& param, zeno::String val) {
        return m_pAdapter->set_output2(zsString2Std(param), zsString2Std(val));
    }

    void INode::set_output_vec2f(const zeno::String& param, Vec2f val) {
        return m_pAdapter->set_output2(zsString2Std(param), toVec2f(val));
    }

    void INode::set_output_vec2i(const zeno::String& param, Vec2i val) {
        return m_pAdapter->set_output2(zsString2Std(param), toVec2i(val));
    }

    void INode::set_output_vec3f(const zeno::String& param, Vec3f val) {
        return m_pAdapter->set_output2(zsString2Std(param), toVec3f(val));
    }

    void INode::set_output_vec3i(const zeno::String& param, Vec3i val) {
        return m_pAdapter->set_output2(zsString2Std(param), toVec3i(val));
    }

    int INode::GetFrameId() const {
        return m_pAdapter->getGlobalState()->getFrameId();
    }

    GlobalState* INode::getGlobalState() {
        return m_pAdapter->getGlobalState();
    }


}