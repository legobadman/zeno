#include "declares.h"

namespace zeno {

struct MakeFEMIntegrator : zeno::INode {
    virtual void apply() override {
        auto prim = get_input<zeno::PrimitiveObject>("prim");
        auto muscle = get_input<MuscleModelObject>("elasto");
        auto damp = get_input<DampingForceModel>("visco");
        auto dt = get_input2<float>("dt");
        auto inttype = get_param<std::string>(("integType"));

        auto res = std::make_shared<FEMIntegrator>();
        // if(inttype == "BE"){
        //     std::cerr << "CURRENTLY ONLY QUASI_STATIC SOLVERS ARE SUPPORTED" << std::endl;
        //     throw std::runtime_error("CURRENTLY ONLY QUASI_STATIC SOLVERS ARE SUPPORTED");
        //     res->_intPtr = std::make_shared<BackEulerIntegrator>();
        // }
        // else if(inttype == "QS")
        //     res->_intPtr = std::make_shared<QuasiStaticSolver>();


        res->_staticPtr = std::make_shared<QuasiStaticSolver>();
        res->_dynamicPtr = std::make_shared<BackEulerIntegrator>();
        res->_dynamicPtr->SetTimeStep(dt);

        res->muscle = muscle;
        res->damp = damp;

        res->_stepID = 0;

        res->PrecomputeFEMInfo(prim);

        set_output("FEMIntegrator",res);
    }
};

ZENDEFNODE(MakeFEMIntegrator,{
    {{gParamType_Primitive, "prim"},{gParamType_IObject, "elasto"},{gParamType_IObject, "visco"},{gParamType_Float,"dt","1"}},
    {{gParamType_Unknown, "FEMIntegrator"}},
    {{"enum BE QS EBQS SDQS", "integType", "QS"}},
    {"FEM"},
});


};
