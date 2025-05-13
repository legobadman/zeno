#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <functional>

#include <zeno/reflect/type>
#include <zeno/reflect/traits/type_traits>
#include <zeno/reflect/container/any>
#include <zeno/reflect/container/unique_ptr>
#include <zeno/reflect/metadata.hpp>
#include <zeno/reflect/reflection_traits.hpp>
#include <zeno/reflection/zenoreflecttypes.cpp.generated.hpp>


using namespace zeno::reflect;

#define _Bool bool

/// ==== Begin zeno::HeatmapData2 Register ====
namespace {

    /// === Begin Constructor Wrappers ===
    class ConstructorWrapperForstruct_zeno_HeatmapData2_0: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_0() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_1: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_1() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<const struct zeno::HeatmapData2 &>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<const struct zeno::HeatmapData2 &>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<const struct zeno::HeatmapData2 &>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    any_cast<const struct zeno::HeatmapData2 &>(any0)

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_2: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_2() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_3: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_3() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<struct zeno::HeatmapData2 &&>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<struct zeno::HeatmapData2 &&>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<struct zeno::HeatmapData2 &&>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    any_cast<struct zeno::HeatmapData2 &&>(any0)

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_4: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_4() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_5: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_5() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_6: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_6() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_7: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_7() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
                    zeno::reflect::type_info<TTDecay<class std::vector<struct zeno::_impl_vec::vec<3, float> >>>(),
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
                "colors",
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {

            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                Any& any0 = const_cast<Any&>(params[0]);
                return new  zeno::HeatmapData2
                {
                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                Any& any0 = const_cast<Any&>(params[0]);
                val.emplace< zeno::HeatmapData2>
                (

                    zeno::HeatmapData2 {

                    any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(any0)

                    }

                );
            }
            return val;
        }
    };
    class ConstructorWrapperForstruct_zeno_HeatmapData2_8: public ITypeConstructor {
    public:
        ConstructorWrapperForstruct_zeno_HeatmapData2_8() : ITypeConstructor(get_type<struct zeno::HeatmapData2>()) {}

        virtual const ArrayList<RTTITypeInfo>& get_params() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<RTTITypeInfo>& get_params_dacayed() const override {
            static ArrayList<RTTITypeInfo> STATIC_LIST {
            };
            return STATIC_LIST;
        }

        virtual const ArrayList<StringView>& get_params_name() const override {
            static ArrayList<StringView> STATIC_LIST {
            };
            return STATIC_LIST;
        }

        virtual Any get_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual Any init_param_default_value(size_t index) override {
            return Any::make_null();
        }

        virtual void* new_instance(const ArrayList<Any>& params) const override {
            if (is_suitable_to_invoke(params)) {
                return new  zeno::HeatmapData2
                {
                };
            }
            return nullptr;
        }

        virtual Any create_instance(const ArrayList<Any>& params) const override {
            Any val{};
            if (is_suitable_to_invoke(params)) {
                // Just praying for NRVO is working
                val.emplace< zeno::HeatmapData2>
                (


                );
            }
            return val;
        }
    };
    /// === End Constructor Wrappers ===

    /// === Begin Member Function Wrappers ===
    /// === End Member Function Wrappers ===

    /// === Begin Member Field Wrappers ===
    class Field_struct_zeno_HeatmapData2_0_colors: public IMemberField {
    public:
        Field_struct_zeno_HeatmapData2_0_colors() : IMemberField(get_type<struct zeno::HeatmapData2>()) {}

        virtual void* get_field_ptr(const Any& clazz_object) const override {
            if (clazz_object.type() == zeno::reflect::type_info<struct zeno::HeatmapData2>()) {
                Any& clazz_object_ = const_cast<Any&>(clazz_object);
                auto& clazz = any_cast<zeno::HeatmapData2&>(clazz_object_);
                return &clazz.colors;
            }
            return nullptr;
        }

        virtual void* get_field_ptr_directly(void* this_object) const override {
            if (!this_object) return nullptr;
            auto pThis = static_cast<zeno::HeatmapData2*>(this_object);
            return &pThis->colors;
        }
		
		virtual void set_field_value(void* this_object, Any value) const override {
            if (!this_object) return;
            auto pThis = static_cast<zeno::HeatmapData2*>(this_object);
            pThis->colors = any_cast<class std::vector<struct zeno::_impl_vec::vec<3, float> >>(value);
        }

        virtual Any get_field_value(void* this_object) const override {
            if (!this_object) return Any();
            auto pThis = static_cast<zeno::HeatmapData2*>(this_object);
            return pThis->colors;
        }

        virtual TypeHandle get_field_type() const override {
            return get_type<class std::vector<struct zeno::_impl_vec::vec<3, float> >>();
        }

        virtual StringView get_name() override {
            return "colors";
        }
};
    /// === End Member Field Wrappers ===

    /// === Begin Record Type Wrapper ===
    class Typestruct_zeno_HeatmapData2_Instance : public TypeBase {
    public:
        using Super = TypeBase;

        Typestruct_zeno_HeatmapData2_Instance(const ReflectedTypeInfo& type_info) : Super(type_info) {
        }

        virtual std::size_t type_hash() const override {
            return get_rtti_info().hash_code();
        }

        virtual const RTTITypeInfo& get_rtti_info() const override {
            return zeno::reflect::type_info<struct zeno::HeatmapData2>();
        }

        virtual const ArrayList<ITypeConstructor*>& get_constructors() const override {
            static ArrayList<ITypeConstructor*> ctors {
                new ConstructorWrapperForstruct_zeno_HeatmapData2_0(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_1(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_2(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_3(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_4(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_5(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_6(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_7(),
                new ConstructorWrapperForstruct_zeno_HeatmapData2_8(),
            };
            return ctors;
        }

        virtual const ArrayList<IMemberFunction*>& get_member_functions() const override {
            static ArrayList<IMemberFunction*> funcs {
            };
            return funcs;
        }

        virtual const ArrayList<IMemberField*>& get_member_fields() const override {
            static ArrayList<IMemberField*> fields {
                new Field_struct_zeno_HeatmapData2_0_colors(),
            };
            return fields;
        }

        virtual const ArrayList<TypeHandle>& get_base_classes() const override {
            static ArrayList<TypeHandle> bases {
            };

            return bases;
        }

        virtual const IRawMetadata* get_metadata() const override {
            // () can't be ignored below C++20
            static UniquePtr<IRawMetadata> metadata = [] () -> UniquePtr<IRawMetadata> {
                UniquePtr<IRawMetadata> data = IRawMetadata::create();

                {
                    
                }

                return data;
            }();

            return metadata.get();
        }

    };
    /// === End Record Type Wrapper ===

    /// === Begin Static Registor ===
    struct Sstruct_zeno_HeatmapData2Registrator {
        Sstruct_zeno_HeatmapData2Registrator() {
            ReflectedTypeInfo info {};
            info.prefix = "";
            info.qualified_name = "zeno::HeatmapData2";
            info.canonical_typename = "struct zeno::HeatmapData2";
            Typestruct_zeno_HeatmapData2_Instance* type_impl = new Typestruct_zeno_HeatmapData2_Instance(info);

            (ReflectionRegistry::get())->add(type_impl);



        }
    };
    static Sstruct_zeno_HeatmapData2Registrator global_Sstruct_zeno_HeatmapData2Registrator{};
    /// === End Static Registor ===
}
/// ==== End zeno::HeatmapData2 Register ====
