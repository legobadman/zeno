#ifndef __ZENO_COMMON_H__
#define __ZENO_COMMON_H__

#include <variant>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <zeno/utils/vec.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


#define ENUM_FLAGS(enum_class) \
constexpr enum_class operator|(enum_class X, enum_class Y) {\
    return static_cast<enum_class>(\
        static_cast<unsigned int>(X) | static_cast<unsigned int>(Y));\
}\
\
constexpr enum_class operator&(enum_class X, enum_class Y) {\
    return static_cast<enum_class>(\
        static_cast<unsigned int>(X) & static_cast<unsigned int>(Y));\
}\
\
constexpr enum_class operator^(enum_class X, enum_class Y) {\
    return static_cast<enum_class>(\
        static_cast<unsigned int>(X) ^ static_cast<unsigned int>(Y));\
}\

namespace zeno {

    using ParamType = size_t;

    #define Param_Null 0
    #define gParamType_Unknown 233

    enum NodeStatus : unsigned int
    {
        None = 0,
        Mute = 1,
        View = 1 << 1,
    };
    ENUM_FLAGS(NodeStatus)

    enum NodeType
    {
        Node_Normal,
        SubInput,
        SubOutput,
        Node_Group,
        Node_Legacy,
        Node_SubgraphNode,
        Node_AssetInstance,     //the asset node which generated by main graph tree.
        Node_AssetReference,    //the asset node which created in another asset graph.
        NoVersionNode
    };

    enum SUBGRAPH_TYPE
    {
        SUBGRAPH_NOR = 0,
        SUBGRAPH_METERIAL,
        SUBGRAPH_PRESET
    };

    enum SubnetType
    {
        Subnet_Normal,
        Subnet_Material,
        Subnet_Main,    //only one main graph on whole zeno system.
    };

    enum SocketProperty : unsigned int
    {
        Socket_Normal,
        Socket_Editable,
        Socket_MultiInput,
        Socket_Legacy,
        Socket_Disable,     //内部参数的socket都是Socket_Disable
    };
    ENUM_FLAGS(SocketProperty)

    enum SocketType
    {
        NoSocket,
        //Socket_Primitve,
        //zeno::Socket_ReadOnly,
        Socket_Output,      //Output object

        //obj:
        Socket_ReadOnly,
        Socket_Clone,
        Socket_Owning,
        //primitive
        Socket_Primitve
    };

    enum NodeDataGroup
    {
        Role_InputObject,
        Role_InputPrimitive,
        Role_OutputObject,
        Role_OutputPrimitive
    };

    //ui issues:
    enum VParamType
    {
        Param_Root,
        Param_Tab,
        Param_Group,
        Param_Param
    };

    enum ParamControl
    {
        NullControl,
        Lineedit,
        Multiline,
        ReadPathEdit,
        WritePathEdit,
        DirectoryPathEdit,
        Combobox,
        Checkbox,
        Vec2edit,
        Vec3edit,
        Vec4edit,
        ColorVec,
        Heatmap,
        CurveEditor,
        SpinBox,
        Slider,
        DoubleSpinBox,
        SpinBoxSlider,
        PythonEditor,
        PushButton,
        Seperator,
        CodeEditor,
        NoMultiSockPanel,   //disable dist/list panel
    };

    enum LinkFunction
    {
        Link_Copy,
        Link_Ref
    };

    enum NodeRunStatus
    {
        Node_DirtyReadyToRun,       //已标脏，但还没执行计算。  （dirty)
        Node_Pending,               //已进入计算，但还须等待前序计算依赖。(dirty)
        Node_Running,               //进入apply   (dirty)
        Node_RunError,              //计算错误      (dirty)
        Node_RunSucceed             //计算成功并完成   (no dirty)
    };

    enum DirtyReason
    {
        Dirty_All,
        Dirty_FrameChanged,
        Dirty_ParamChanged,
    };

    //几何属性对应的分组
    enum GeoAttrGroup {
        ATTR_GEO,
        ATTR_FACE,
        ATTR_POINT,
        ATTR_VERTEX,
    };

    //几何属性类型
    enum GeoAttrType {
        ATTR_TYPE_UNKNOWN,
        ATTR_INT,
        ATTR_FLOAT,
        ATTR_STRING,
        ATTR_VEC2,
        ATTR_VEC3,
        ATTR_VEC4
    };

    enum UpdateReason {
        Update_View,            //只是view，计算已经完成了(dirty==false)
        Update_Reconstruct,     //经过了重新计算需要更新
        Update_OnlyUserData,    //只是更新了与拓扑无关的数据，无须在渲染端重新构造。
        Update_Remove,          //移除，可能是删除或者Unview
    };

    enum ZSG_VERSION
    {
        VER_2,          //old version io
        VER_2_5,        //new version io
        VER_3,          //the final io format, supporting tree layout.
        UNKNOWN_VER,
    };

    using zvariant = std::variant<
        int, zeno::vec2i, zeno::vec3i, zeno::vec4i,
        float, zeno::vec2f, zeno::vec3f, zeno::vec4f,
        zeno::vec2s, zeno::vec3s, zeno::vec4s, std::string>;

    using AttrValue = std::variant<float, int, vec3f, vec2f, vec4f, std::string>;

    using AttrVarVec = std::variant
        < std::vector<int>
        , std::vector<float>
        , std::vector<std::string>
        , std::vector<vec3f>
        , std::vector<vec2f>
        , std::vector<vec4f>
        >;

    using AttrVar = std::variant
        <float
        , int
        , std::string
        , vec2f
        , vec2i
        , vec3f
        , vec3i
        , vec4f
        , vec4i
        , glm::vec2
        , glm::vec3
        , glm::vec4
        , std::vector<vec3f>
        , std::vector<float>
        , std::vector<std::string>
        , std::vector<vec3i>
        , std::vector<int>
        , std::vector<vec2f>
        , std::vector<vec2i>
        , std::vector<vec4f>
        , std::vector<vec4i>
        , std::vector<glm::vec2>
        , std::vector<glm::vec3>
        , std::vector<glm::vec4>
        >;

    using ctrlpropvalue = std::variant<
        std::vector<std::string>,
        int,
        float,
        std::string>;

    using ObjPath = std::string;

    typedef std::function<void(ObjPath, bool, NodeRunStatus)> F_NodeStatus;

    enum render_reload_policy {
        Reload_Invalidate,      //无效的同步
        Reload_SwitchGraph,     //用户在编辑器上切换当前节点图的层次触发的Load
        Reload_ToggleView,      //用户在编辑器上当前节点图，切换节点间的view触发的load
        Reload_Calculation,     //由于标脏计算引发的load
    };

    struct render_update_info {
        UpdateReason reason;
        std::string uuidpath_node_objkey;   //节点的uuid路径，同时也是obj的key.
        std::vector<std::string> remove_objs;
    };
    typedef std::function<void(render_update_info)> F_CommitRender;

    struct render_reload_info {
        render_reload_policy policy;
        std::string current_ui_graph;   //当前用户在编辑器端的ui图层，以普通路径表达
        std::vector<render_update_info> objs;
    };
}


#endif