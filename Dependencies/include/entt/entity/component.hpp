#ifndef ENTT_ENTITY_COMPONENT_HPP
#define ENTT_ENTITY_COMPONENT_HPP

#include <cstddef>
#include <type_traits>
#include "../config/config.h"
#include "fwd.hpp"

namespace entt {

/**
 * @cond TURN_OFF_DOXYGEN
 * Internal details not to be documented.
 */

namespace internal {

template<typename Type, typename = void>
//struct in_place_delete : std::bool_constant<!(std::is_move_constructible_v<Type> &&std::is_move_assignable_v<Type>)> {};
struct in_place_delete: std::true_type {}; // Modified so in_place_delete defaults to true

template<>
struct in_place_delete<void>: std::false_type {};

template<typename Type>
struct in_place_delete<Type, std::enable_if_t<Type::in_place_delete>>
    : std::true_type {};

template<typename Type, typename = void>
struct page_size: std::integral_constant<std::size_t, !std::is_empty_v<ENTT_ETO_TYPE(Type)> * ENTT_PACKED_PAGE> {};

template<>
struct page_size<void>: std::integral_constant<std::size_t, 0u> {};

template<typename Type>
struct page_size<Type, std::void_t<decltype(Type::page_size)>>
    : std::integral_constant<std::size_t, Type::page_size> {};

} // namespace internal

/**
 * Internal details not to be documented.
 * @endcond
 */

/**
 * @brief Common way to access various properties of components.
 * @tparam Type Type of component.
 */
template<typename Type, typename = void>
struct component_traits {
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Unsupported type");

    /*! @brief Component type. */
    using type = Type;

    /*! @brief Pointer stability, default is `false`. */
    static constexpr bool in_place_delete = internal::in_place_delete<Type>::value;
    /*! @brief Page size, default is `ENTT_PACKED_PAGE` for non-empty types. */
    static constexpr std::size_t page_size = internal::page_size<Type>::value;
};

} // namespace entt

#endif
