#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>

namespace zeno {

template <class FuncT>
struct UnaryOperator {
    FuncT func;
    UnaryOperator(FuncT const &func) : func(func) {}

    template <class TOut, class TA>
    void operator()(std::vector<TOut> &arrOut, std::vector<TA> const &arrA) {
        size_t n = std::min(arrOut.size(), arrA.size());
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            auto val = func(arrA[i]);
            arrOut[i] = (decltype(arrOut[0]))val;
        }
    }
};

struct PrimitiveUnaryOp : INode {
  virtual void apply() override {
    auto primA = ZImpl(get_input<PrimitiveObject>("primA"));
    auto primOut = ZImpl(get_input<PrimitiveObject>("primOut"));
    auto attrA = ZImpl(get_param<std::string>("attrA"));
    auto attrOut = ZImpl(get_param<std::string>("attrOut"));
    auto op = ZImpl(get_param<std::string>("op"));
    primOut->attr_visit(attrOut, [&] (auto &arrOut) { primA->attr_visit(attrA, [&] (auto &arrA) {
        if constexpr (is_vec_castable_v<decltype(arrOut[0]), decltype(arrA[0])>) {
            if (0) {
#define _PER_OP(opname, expr) \
            } else if (op == opname) { \
                UnaryOperator([](auto const &a) { return expr; })(arrOut, arrA);
            _PER_OP("copy", a)
            _PER_OP("neg", -a)
            _PER_OP("sqrt", sqrt(a))
            _PER_OP("sin", sin(a))
            _PER_OP("cos", cos(a))
            _PER_OP("tan", tan(a))
            _PER_OP("asin", asin(a))
            _PER_OP("acos", acos(a))
            _PER_OP("atan", atan(a))
            _PER_OP("exp", exp(a))
            _PER_OP("log", log(a))
#undef _PER_OP
            } else {
                throw Exception("Bad operator type: " + op);
            }
        } else {
            throw Exception("Failed to promote variant type");
        }
    }); });

    ZImpl(set_output("primOut", ZImpl(clone_input("primOut"))));
  }
};

ZENDEFNODE(PrimitiveUnaryOp,
    { /* inputs: */ {
    {gParamType_Primitive, "primA", "", zeno::Socket_ReadOnly},
    {gParamType_Primitive, "primOut", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Primitive, "primOut"},
    }, /* params: */ {
    {gParamType_String, "attrA", "pos"},
    {gParamType_String, "attrOut", "pos"},
    {gParamType_String, "op", "copy"},
    }, /* category: */ {
    "deprecated",
    }});


template <class FuncT>
struct BinaryOperator {
    FuncT func;
    BinaryOperator(FuncT const &func) : func(func) {}

    template <class TOut, class TA, class TB>
    void operator()(std::vector<TOut> &arrOut,
        std::vector<TA> const &arrA, std::vector<TB> const &arrB) {
        size_t n = std::min(arrOut.size(), std::min(arrA.size(), arrB.size()));
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            auto val = func(arrA[i], arrB[i]);
            arrOut[i] = (decltype(arrOut[0]))val;
        }
    }
};

struct PrimitiveBinaryOp : INode {
  virtual void apply() override {
    auto primA = ZImpl(get_input<PrimitiveObject>("primA"));
    auto primB = ZImpl(get_input<PrimitiveObject>("primB"));
    auto primOut = ZImpl(get_input<PrimitiveObject>("primOut"));
    auto attrA = ZImpl(get_param<std::string>("attrA"));
    auto attrB = ZImpl(get_param<std::string>("attrB"));
    auto attrOut = ZImpl(get_param<std::string>("attrOut"));
    auto op = ZImpl(get_param<std::string>("op"));
    primOut->attr_visit(attrOut, [&](auto &arrOut) {
        using TarrOut = std::remove_cv_t<std::remove_reference_t<decltype(arrOut[0])>>;
        ;
        primA->attr_visit(attrA, [&](auto &arrA) {
            using TarrA = std::remove_cv_t<std::remove_reference_t<decltype(arrA[0])>>;
            primB->attr_visit(attrB, [&](auto &arrB) {
                using TarrB = std::remove_cv_t<std::remove_reference_t<decltype(arrB[0])>>;
                if constexpr (is_decay_same_v<TarrOut, is_vec_promotable_t<TarrA, TarrB>>) {
                    if constexpr (0) {
#define _PER_OP(opname, expr)                               \
    }                                                       \
    else if (op == opname) {                                \
        BinaryOperator([](auto const &a_, auto const &b_) { \
            using PromotedType = decltype(a_ + b_);         \
            auto a = PromotedType(a_);                      \
            auto b = PromotedType(b_);                      \
            return expr;                                    \
        })(arrOut, arrA, arrB);
                        _PER_OP("copyA", a)
                        _PER_OP("copyB", b)
                        _PER_OP("add", a + b)
                        _PER_OP("sub", a - b)
                        _PER_OP("rsub", b - a)
                        _PER_OP("mul", a * b)
                        _PER_OP("div", a / b)
                        _PER_OP("rdiv", b / a)
                        _PER_OP("pow", pow(a, b))
                        _PER_OP("rpow", pow(b, a))
                        _PER_OP("atan2", atan2(a, b))
                        _PER_OP("ratan2", atan2(b, a))
#undef _PER_OP
                    } else {
                        throw Exception("Bad operator type: " + op);
                    }
                } else {
                    throw Exception("Failed to promote variant type");
                }
            });
        });
    });

    ZImpl(set_output("primOut", ZImpl(clone_input("primOut"))));
  }
};

ZENDEFNODE(PrimitiveBinaryOp,
    { /* inputs: */ {
        {gParamType_Primitive, "primB", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primA", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primOut", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Primitive, "primOut"},
    }, /* params: */ {
    {gParamType_String, "attrA", "pos"},
    {gParamType_String, "attrB", "pos"},
    {gParamType_String, "attrOut", "pos"},
    {gParamType_String, "op", "copyA"},
    }, /* category: */ {
    "deprecated",
    }});


struct PrimitiveMix : INode {
  virtual void apply() override {
    auto primA = ZImpl(get_input<PrimitiveObject>("primA"));
    auto primB = ZImpl(get_input<PrimitiveObject>("primB"));

    std::shared_ptr<zeno::PrimitiveObject> primOut;
    if (ZImpl(has_input("primOut"))) {
            primOut = ZImpl(get_input<PrimitiveObject>("primOut"));
    } else {
            primOut = std::make_shared<zeno::PrimitiveObject>(*primA);
    }
    auto attrA = ZImpl(get_param<std::string>("attrA"));
    auto attrB = ZImpl(get_param<std::string>("attrB"));
    auto attrOut = ZImpl(get_param<std::string>("attrOut"));
    auto coef = ZImpl(get_input<NumericObject>("coef"))->get<float>();
    primOut->attr_visit(attrOut, [&](auto &arrOut) {
        using TarrOut = std::remove_cv_t<std::remove_reference_t<decltype(arrOut)>>;
        primA->attr_visit(attrA, [&](auto &arrA) {
            using TarrA = std::remove_cv_t<std::remove_reference_t<decltype(arrA)>>;
            if constexpr (std::is_same_v<TarrA, TarrOut>)
                primB->attr_visit(attrB, [&, &arrA = arrA](auto &arrB) {
                    using TarrB = std::remove_cv_t<std::remove_reference_t<decltype(arrB)>>;
                    if constexpr (std::is_same_v<TarrA, TarrB>) {
#pragma omp parallel for
                        for (int i = 0; i < arrOut.size(); i++) {
                            arrOut[i] = (1.0f - coef) * arrA[i] + coef * arrB[i];
                        }
                    }
                });
        });
    });
    ZImpl(set_output("primOut", primOut));
  }
};
ZENDEFNODE(PrimitiveMix,
    { /* inputs: */ {
        {gParamType_Float, "coef", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primB", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primA", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primOut", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Primitive, "primOut"},
    }, /* params: */ {
    {gParamType_String, "attrA", "pos"},
    {gParamType_String, "attrB", "pos"},
    {gParamType_String, "attrOut", "pos"},
    }, /* category: */ {
    "deprecated",
    }});


template <class FuncT>
struct HalfBinaryOperator {
    FuncT func;
    HalfBinaryOperator(FuncT const &func) : func(func) {}

    template <class TOut, class TA, class TB>
    void operator()(std::vector<TOut> &arrOut,
        std::vector<TA> const &arrA, TB const &valB) {
        size_t n = std::min(arrOut.size(), arrA.size());
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            auto val = func(arrA[i], valB);
            arrOut[i] = (decltype(arrOut[0]))val;
        }
    }
};

struct PrimitiveHalfBinaryOp : INode {
  virtual void apply() override {
    auto primA = ZImpl(get_input<PrimitiveObject>("primA"));
    auto primOut = ZImpl(get_input<PrimitiveObject>("primOut"));
    auto attrA = ZImpl(get_param<std::string>("attrA"));
    auto attrOut = ZImpl(get_param<std::string>("attrOut"));
    auto op = ZImpl(get_param<std::string>("op"));
    auto const &valB = ZImpl(get_input<NumericObject>("valueB"))->value;
    primOut->attr_visit(attrOut, [&](auto &arrOut) {
        using TarrOut = std::remove_cv_t<std::remove_reference_t<decltype(arrOut[0])>>;
        primA->attr_visit(attrA, [&](auto &arrA) {
            using TarrA = std::remove_cv_t<std::remove_reference_t<decltype(arrA[0])>>;
            std::visit(
                [&](auto &valB) {
                    using TvalB = std::remove_cv_t<std::remove_reference_t<decltype(valB)>>;
                    if constexpr (is_decay_same_v<TarrOut, is_vec_promotable_t<TarrA, TvalB>>) {
                        if constexpr (0) {
#define _PER_OP(opname, expr)                                   \
    }                                                           \
    else if (op == opname) {                                    \
        HalfBinaryOperator([](auto const &a_, auto const &b_) { \
            using PromotedType = decltype(a_ + b_);             \
            auto a = PromotedType(a_);                          \
            auto b = PromotedType(b_);                          \
            return expr;                                        \
        })(arrOut, arrA, valB);
                            _PER_OP("copyA", a)
                            _PER_OP("copyB", b)
                            _PER_OP("add", a + b)
                            _PER_OP("sub", a - b)
                            _PER_OP("rsub", b - a)
                            _PER_OP("mul", a * b)
                            _PER_OP("div", a / b)
                            _PER_OP("rdiv", b / a)
                            _PER_OP("pow", pow(a, b))
                            _PER_OP("rpow", pow(b, a))
                            _PER_OP("atan2", atan2(a, b))
                            _PER_OP("ratan2", atan2(b, a))
#undef _PER_OP
                        } else {
                            throw Exception("Bad operator type: " + op);
                        }
                    } else {
                        throw Exception("Failed to promote variant type");
                    }
                },
                valB);
        });
    });

    ZImpl(set_output("primOut", ZImpl(clone_input("primOut"))));
  }
};

ZENDEFNODE(PrimitiveHalfBinaryOp,
    { /* inputs: */ {
        {gParamType_Float, "valueB", ""},
        {gParamType_Primitive, "primA", "", zeno::Socket_ReadOnly},
        {gParamType_Primitive, "primOut", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Primitive, "primOut"},
    }, /* params: */ {
    {gParamType_String, "attrA", "pos"},
    {gParamType_String, "attrOut", "pos"},
    {gParamType_String, "op", "copyA"},
    }, /* category: */ {
    "deprecated",
    }});

}
