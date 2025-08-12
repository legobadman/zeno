#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/reflection/zenoreflecttypes.cpp.generated.hpp>


namespace zeno {

    using namespace zeno::reflect;

    struct PrimitiveToGeometry : INode {
        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_prim", ParamObject("Input", Socket_Clone)},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(std::shared_ptr<PrimitiveObject> input_prim) {
            std::shared_ptr<GeometryObject> out_geo = std::make_shared<GeometryObject>(input_prim);
            return out_geo;
        }
    };

    struct GeometryToPrimitive : INode {
        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_geometry", ParamObject("Input", Socket_Clone)},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<PrimitiveObject> apply(std::shared_ptr<GeometryObject> input_geometry) {
            return input_geometry->toPrimitive();
        }
    };
}

