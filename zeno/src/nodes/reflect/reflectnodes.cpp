﻿#include <zeno/core/NodeImpl.h>
#include <zeno/core/IObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/core/reflectdef.h>
#include "zeno_types/reflect/reflection.generated.hpp"

namespace zeno
{
    struct ZDEFNODE() TestReflectNode : zeno::INode
    {
        TestReflectNode() = default;

        ZMETHOD(Name = "abc")
        int apply(std::string wtf, zeno::vec3f c) {
            param_b = wtf;
            return 233;
        }

        ZPROPERTY(Role = zeno::Role_InputObject, DisplayName = "Input Object", Socket = zeno::Socket_Owning)
        std::shared_ptr<zeno::IObject> m_prim;

        ZPROPERTY(Role = zeno::Role_OutputObject, DisplayName = "Output Object")
        std::shared_ptr<zeno::IObject> m_result;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Param A")
        int param_a = 3;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName="Param B", Control = zeno::Multiline)
        std::string param_b = "default";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Param Options", Control = zeno::Combobox, ComboBoxItems = ("option A", "option B"))
        std::string param_options = "Option A";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Dt", Control = zeno::Slider, range = (1, 100, 1))
        int dt = 0;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Position")
        zeno::vec3f pos;

        ZPROPERTY(Role = zeno::Role_OutputPrimitive)
        zeno::vec3f outvec;
    };

    struct ZDEFNODE() SimpleReflect : zeno::INode
    {
        SimpleReflect() = default;

        std::shared_ptr<zeno::PrimitiveObject> apply(std::shared_ptr<zeno::PrimitiveObject> input_obj, std::string wtf = "abc", zeno::vec3f c = zeno::vec3f({ 0,1,0 })/*, float& ret1, std::shared_ptr<zeno::IObject>&output_obj*/) {
            //ret1 = 8;
            zeno::reflect::Any vec = zeno::reflect::make_any<zeno::vec3f>(zeno::vec3f({ 0.,1.0,2. }));
            zeno::reflect::any_cast<zeno::vec3f>(vec);
            vec.type();
            return input_obj;
        }
    };

    struct ZDEFNODE() ReadOnlyNode : zeno::INode
    {
        ReflectCustomUI m_uilayout = {
            //输入：
            _Group {
                    {"input_obj", ParamObject("Input Object", Socket_Owning) },
                    {"name1",     ParamPrimitive("Name 1")},
                    {"name2",     ParamPrimitive("Name 2")},
                    {"a", ParamPrimitive("A")},
                    {"b", ParamPrimitive("B")},
            },
            //输出：
            _Group {
                    {"", ParamObject("Output Object", Socket_Owning)},
            },
            //数值参数布局：
            CustomUIParams {
                ParamTab {
                    "CustomTab1",
                    {
                        ParamGroup {
                            "Group1",
                            {
                                ParamPrimitive("c"),
                                ParamPrimitive("d"),
                                ParamPrimitive("e"),
                                ParamPrimitive("f"),
                                ParamPrimitive("g"),
                                ParamPrimitive("h"),
                                ParamPrimitive("i"),
                                ParamPrimitive("j"),
                                ParamPrimitive("k"),
                            }
                        },
                        ParamGroup {
                            "Group2",
                            {
                                ParamPrimitive("A"),
                                ParamPrimitive("B"),
                                ParamPrimitive("l"),
                                ParamPrimitive("m"),
                                ParamPrimitive("n"),
                                ParamPrimitive("o"),
                                ParamPrimitive("p"),
                                ParamPrimitive("q"),
                                ParamPrimitive("zzz"),
                                ParamPrimitive("r4r"),
                                ParamPrimitive("fwefew"),
                                ParamPrimitive("af")
                            }
                        },
                    }
                },
                ParamTab {
                    "Tab2",
                },
            }
        };

        std::shared_ptr<zeno::PrimitiveObject> apply(
            std::shared_ptr<const zeno::PrimitiveObject> input_obj,
            const std::string& name1 = "a16",
            const std::string& name2 = "a24",
            int a = 234,
            float b = 456.234,
            int c = 6,
            int d = 7,
            int e = 8,
            int f = 8,
            int g = 9,
            int h = 10,
            int i = 11,
            int j = 12,
            int k = 13,
            int l = 14,
            int m = 15,
            int n = 16,
            int o = 4543,
            int p = 3424,
            int q =563,
            int zzz = 45235,
            int r4r = 3543,
            std::string fwefew="fgrgreg",
            float af = 3.4f)
        {
            std::shared_ptr<zeno::PrimitiveObject> res = std::const_pointer_cast<zeno::PrimitiveObject>(input_obj);
            return res;
        }
    };
}

REFLECT_REGISTER_RTTI_TYPE_WITH_NAME(INode, INode)