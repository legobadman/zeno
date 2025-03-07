#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/core/reflectdef.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/GeometryObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno
{
    struct ZDEFNODE() AttributeWrangle : zeno::INode
    {
        ReflectCustomUI m_uilayout = {
            //输入：
            _Group {
                {"spGeo", ParamObject("Input", Socket_Clone)},
                {"zfxCode", ParamPrimitive("Zfx Code", "", CodeEditor)},
            },
            //输出：
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(std::shared_ptr<zeno::GeometryObject> spGeo, std::string zfxCode) {
            if (!spGeo)
                return spGeo;

            ZfxContext ctx;
            ctx.spNode = shared_from_this();
            ctx.spObject = spGeo;
            ctx.code = zfxCode;
            ZfxExecute zfx(zfxCode, &ctx);
            zfx.execute();
            return spGeo;
        }
    };
}