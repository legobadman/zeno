#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/IObject.h>
#include <zeno/core/INode.h>
#include <zeno/core/common.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <variant>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <optional>
#include <zeno/types/CurveObject.h>
#include <zeno/extra/GlobalState.h>

namespace zeno {

struct CoreLink {
    ObjectParam* fromparam = nullptr;  //IParam stored as unique ptr in the INode, so we need no smart pointer.
    ObjectParam* toparam = nullptr;
    std::string fromkey;    //for dict/list ����list��˵��keyName���񲻺��ʣ�����ILink�����ʹ�����links���棬�Ѿ����б������ˡ�
    std::string tokey;
    //LinkFunction lnkProp = Link_Copy;
};


struct ParamLink {
    PrimitiveParam* fromparam = nullptr;
    PrimitiveParam* toparam = nullptr;
};

struct CoreParam {
    std::string name;
    std::weak_ptr<INode> m_wpNode;
    
    ParamType type = Param_Null;
    SocketType socketType = NoSocket;

    bool m_idModify = false;    //��output param�����obj���´�����(false)���ǻ������е��޸�(true)
};

struct ObjectParam : CoreParam {
    std::list<std::shared_ptr<CoreLink>> links;
    zany spObject;
};

struct PrimitiveParam : CoreParam {
    zvariant defl;
    zvariant result;
    std::list<std::shared_ptr<ParamLink>> links;
    ParamControl control = NullControl;
    std::optional<ControlProperty> optCtrlprops;
};

}