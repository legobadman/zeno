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
        return m_pAdapter->get_input2<int>(zsString2Std(param));
    }

    float INode::get_input2_float(const zeno::String& param) {
        return m_pAdapter->get_input2<float>(zsString2Std(param));
    }

    String INode::get_input2_string(const zeno::String& param) {
        std::string val = m_pAdapter->get_input2<std::string>(zsString2Std(param));
        return stdString2zs(val);
    }

    zeno::Vec2i INode::get_input2_vec2i(const zeno::String& param) {
        return toAbiVec2i(m_pAdapter->get_input2<zeno::vec2i>(zsString2Std(param)));
    }

    zeno::Vec2f INode::get_input2_vec2f(const zeno::String& param) {
        return toAbiVec2f(m_pAdapter->get_input2<zeno::vec2f>(zsString2Std(param)));
    }

    zeno::Vec3i INode::get_input2_vec3i(const zeno::String& param) {
        return toAbiVec3i(m_pAdapter->get_input2<zeno::vec3i>(zsString2Std(param)));
    }

    zeno::Vec3f INode::get_input2_vec3f(const zeno::String& param) {
        return toAbiVec3f(m_pAdapter->get_input2<zeno::vec3f>(zsString2Std(param)));
    }

    zeno::Vec4i INode::get_input2_vec4i(const zeno::String& param) {
        return toAbiVec4i(m_pAdapter->get_input2<zeno::vec4i>(zsString2Std(param)));
    }

    zeno::Vec4f INode::get_input2_vec4f(const zeno::String& param) {
        return toAbiVec4f(m_pAdapter->get_input2<zeno::vec4f>(zsString2Std(param)));
    }

    bool INode::get_input2_bool(const zeno::String& param) {
        return m_pAdapter->get_input2<bool>(zsString2Std(param));
    }

    bool INode::has_input(const zeno::String& param) {
        return m_pAdapter->has_input(zsString2Std(param));
    }

    bool INode::set_output(const zeno::String& param, zany pObject) {
        return m_pAdapter->set_output(zsString2Std(param), pObject);
    }

    int INode::get_param_int(const zeno::String& param) {
        return m_pAdapter->get_param<int>(zsString2Std(param));
    }

    float INode::get_param_float(const zeno::String& param) {
        return m_pAdapter->get_param<float>(zsString2Std(param));
    }

    String INode::get_param_string(const zeno::String& param) {
        return stdString2zs(m_pAdapter->get_param<std::string>(zsString2Std(param)));
    }

    bool INode::get_param_bool(const zeno::String& param) {
        return m_pAdapter->get_param<bool>(zsString2Std(param));
    }


    bool INode::is_continue_to_run() { return false; }
    void INode::increment() {}
    void INode::reset_forloop_settings() {}
    zany INode::get_iterate_object() { return 0; }

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