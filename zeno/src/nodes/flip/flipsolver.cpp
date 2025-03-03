#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/geo/geometryutil.h>
#include "glm/gtc/matrix_transform.hpp"
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;

    struct ZDEFNODE() FlipSolver : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"init_fluid", ParamObject("Initialize Fluid")},
                {"static_collider", ParamObject("Static Collider")},
                {"emission_source", ParamObject("Emission Source")},

                {"accuracy", ParamPrimitive("Accuracy")},
                {"max_substep", ParamPrimitive("Max Substep")},
                {"timestep", ParamPrimitive("Timestep")},
                {"gravity", ParamPrimitive("Gravity")},
                {"emission", ParamPrimitive("Emission Velocity")},
                {"is_emission", ParamPrimitive("Is Emission")},
                {"dynamic_collide_strength", ParamPrimitive("Dynamic Collide Strength")},
                {"density", ParamPrimitive("Density")},
                {"surface_tension", ParamPrimitive("Surface Tension")},
                {"viscosity", ParamPrimitive("Viscosity")},
                {"wall_viscosity", ParamPrimitive("Wall Viscosity")},
                {"wall_viscosityRange", ParamPrimitive("Wall Viscosity Range")},
                {"curve_endframe", ParamPrimitive("Curve Endframe")},
                {"curve_range", ParamPrimitive("Curve Range")},
                {"preview_size", ParamPrimitive("Preview Size")},
                {"preview_minVelocity", ParamPrimitive("Preview Minimum Velocity")},
                {"preview_maxVelocity", ParamPrimitive("Preview Maximum Velocity")}
            }
        };

        void apply(
            std::shared_ptr<IObject> init_fluid,
            std::shared_ptr<IObject> static_collider,
            std::shared_ptr<IObject> emission_source,
            float accuracy = 0.08f,     /*精度*/
            float timestep = 0.04f,     /*时间步长*/
            float max_substep = 1.f,     /*最大子步数*/
            /*重力*/
            zeno::vec3f gravity = zeno::vec3f({ 0.f, -9.8f, 0.f }),
            /*发射源速度*/
            zeno::vec3f emission = zeno::vec3f({ 0.f, -9.8f, 0.f }),
            bool is_emission = true,                            /*是否发射*/
            float dynamic_collide_strength = 1.f,                 /*动态碰撞强度*/
            float density = 1000,                          /*密度*/
            float surface_tension = 0,       /*表面张力*/
            float viscosity = 0,            /*粘性*/
            float wall_viscosity = 0,        /*壁面粘性*/
            float wall_viscosityRange = 0,   /*壁面粘性作用范围*/
            /*曲线力: 暂时忽略*/
            int curve_endframe = 100,          /*曲线终止帧*/
            float curve_range = 1.1f,           /*曲线作用范围*/
            float preview_size = 0,          /*预览大小*/
            float preview_minVelocity = 0,   /*预览最小速度*/
            float preview_maxVelocity = 2.f   /*预览最大速度*/
            /*FSD忽略*/
        ) {

        }
    };
}