#pragma once

#include <variant>
#include <type_traits>

namespace zeno {

static std::variant<std::false_type, std::true_type> boolean_variant(bool b) {
    if (b) return std::true_type{};
    else return std::false_type{};
}

using boolean_variant_t = std::variant<std::false_type, std::true_type>;

template <class Func>
decltype(auto) boolean_switch(bool b, Func &&func) {
    return std::visit(std::forward<Func>(func), boolean_variant(b));
}

namespace index_variant_details {
    template <class Ret, bool HasMono, size_t I, size_t N>
    Ret helper_impl(size_t i) {
        if constexpr (I >= N) {
            if constexpr (HasMono) {
                return std::monostate{};
            } else {
                throw std::bad_variant_access{};
            }
        } else {
            if (i == I) {
                return std::integral_constant<size_t, I>{};
            } else {
                return helper_impl<Ret, HasMono, I + 1, N>(i);
            }
        }
    }

    template <size_t N, bool HasMono, size_t ...Is>
    auto helper_call(size_t i, std::index_sequence<Is...>) {
        using Ret = std::conditional_t<HasMono
            , std::variant<std::monostate, std::integral_constant<size_t, Is>...>
            , std::variant<std::integral_constant<size_t, Is>...>
            >;
        return helper_impl<Ret, HasMono, 0, N>(i);
    }
}

template <size_t N, bool HasMono = false>
using index_variant_t = decltype(index_variant_details::helper_call<N, HasMono>(0, std::make_index_sequence<N>{}));

template <size_t N, bool HasMono = false>
static auto index_variant(size_t i) {
    return index_variant_details::helper_call<N, HasMono>(i, std::make_index_sequence<N>{});
}

template <size_t N, bool HasMono = false, class Func>
decltype(auto) index_switch(size_t i, Func &&func) {
    return std::visit(std::forward<Func>(func), index_variant<N, HasMono>(i));
}

/*template <class Variant>
std::type_info const &typeid_of_variant(size_t index) {
    return index_switch<std::variant_size_v<Variant>>(
        index, [&] (auto index) -> std::type_info const & {
        return typeid(std::variant_alternative_t<index.value, Variant>);
    });
}*/

template <class Variant>
auto const &typeid_of_variant(Variant const &var) {
    return std::visit([&] (auto const &val) -> auto const & {
        return typeid(std::decay_t<decltype(val)>);
    }, var);
}

}