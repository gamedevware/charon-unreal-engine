// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include <type_traits>

template <typename T, typename = void>
struct has_unary_plus : std::false_type {};
template <typename T>
struct has_unary_plus<T, std::void_t<decltype(+std::declval<T>())>> : std::true_type {};

template <typename T, typename = void>
struct has_unary_minus : std::false_type {};
template <typename T>
struct has_unary_minus<T, std::void_t<decltype(-std::declval<T>())>> : std::true_type {};

template <typename T, typename = void>
struct has_logical_not : std::false_type {};
template <typename T>
struct has_logical_not<T, std::void_t<decltype(!std::declval<T>())>> : std::true_type {};

template <typename T, typename = void>
struct has_bitwise_not : std::false_type {};
template <typename T>
struct has_bitwise_not<T, std::void_t<decltype(~std::declval<T>())>> : std::true_type {};

#define DEFINE_BINARY_TRAIT(TraitName, Op) \
template <typename L, typename R, typename = void> \
struct TraitName : std::false_type {}; \
template <typename L, typename R> \
struct TraitName<L, R, std::void_t<decltype(std::declval<L>() Op std::declval<R>())>> : std::true_type {}; \
template <typename L, typename R> \
inline constexpr bool TraitName##_v = TraitName<L, R>::value;

// Define the traits using the macro for brevity
DEFINE_BINARY_TRAIT(has_bit_and, &)
DEFINE_BINARY_TRAIT(has_bit_or,  |)
DEFINE_BINARY_TRAIT(has_bit_xor, ^)
DEFINE_BINARY_TRAIT(has_mul,     *)
DEFINE_BINARY_TRAIT(has_div,     /)
DEFINE_BINARY_TRAIT(has_mod,     %)
DEFINE_BINARY_TRAIT(has_add,     +)
DEFINE_BINARY_TRAIT(has_sub,     -)
DEFINE_BINARY_TRAIT(has_shl,     <<)
DEFINE_BINARY_TRAIT(has_shr,     >>)
DEFINE_BINARY_TRAIT(has_gte,     >=)
DEFINE_BINARY_TRAIT(has_gt,      >)
DEFINE_BINARY_TRAIT(has_lt,      <)
DEFINE_BINARY_TRAIT(has_lte,     <=)
DEFINE_BINARY_TRAIT(has_eq,      ==)
DEFINE_BINARY_TRAIT(has_neq,     !=)