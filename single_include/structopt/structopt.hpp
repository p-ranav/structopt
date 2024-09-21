//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef VISIT_STRUCT_HPP_INCLUDED
#define VISIT_STRUCT_HPP_INCLUDED

/***
 * Provides a facility to declare a structure as "visitable" and apply a visitor
 * to it. The list of members is a compile-time data structure, and there is no
 * run-time overhead.
 */

#include <utility>
#include <type_traits>

// Library version

#define VISIT_STRUCT_VERSION_MAJOR 1
#define VISIT_STRUCT_VERSION_MINOR 1
#define VISIT_STRUCT_VERSION_PATCH 0

#define VISIT_STRUCT_STRING_HELPER(X) #X
#define VISIT_STRUCT_STRING(X) VISIT_STRUCT_STRING_HELPER(X)

#define VISIT_STRUCT_VERSION_STRING VISIT_STRUCT_STRING(VISIT_STRUCT_VERSION_MAJOR) "." VISIT_STRUCT_STRING(VISIT_STRUCT_VERSION_MINOR) "." VISIT_STRUCT_STRING(VISIT_STRUCT_VERSION_PATCH)

// For MSVC 2013 support, we put constexpr behind a define.

# ifndef VISIT_STRUCT_CONSTEXPR
#   if (defined _MSC_VER) && (_MSC_VER <= 1800)
#     define VISIT_STRUCT_CONSTEXPR
#   else
#     define VISIT_STRUCT_CONSTEXPR constexpr
#   endif
# endif

// After C++14 the apply_visitor function can be constexpr.
// We target C++11, but such functions are tagged VISIT_STRUCT_CXX14_CONSTEXPR.

# ifndef VISIT_STRUCT_CXX14_CONSTEXPR
#   if ((defined _MSC_VER) && (_MSC_VER <= 1900)) || (!defined __cplusplus) || (__cplusplus == 201103L)
#     define VISIT_STRUCT_CXX14_CONSTEXPR
#   else
#     define VISIT_STRUCT_CXX14_CONSTEXPR constexpr
#   endif
# endif

namespace visit_struct {

namespace traits {

// Primary template which is specialized to register a type
template <typename T, typename ENABLE = void>
struct visitable;

// Helper template which checks if a type is registered
template <typename T, typename ENABLE = void>
struct is_visitable : std::false_type {};

template <typename T>
struct is_visitable<T,
                    typename std::enable_if<traits::visitable<T>::value>::type>
 : std::true_type {};

// Helper template which removes cv and reference from a type (saves some typing)
template <typename T>
struct clean {
  typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

template <typename T>
using clean_t = typename clean<T>::type;

// Mini-version of std::common_type (we only require C++11)
template <typename T, typename U>
struct common_type {
  typedef decltype(true ? std::declval<T>() : std::declval<U>()) type;
};

} // end namespace traits

// Tag for tag dispatch
template <typename T>
struct type_c { using type = T; };

// Accessor type: function object encapsulating a pointer-to-member
template <typename MemPtr, MemPtr ptr>
struct accessor {
  template <typename T>
  VISIT_STRUCT_CONSTEXPR auto operator()(T && t) const -> decltype(std::forward<T>(t).*ptr) {
    return std::forward<T>(t).*ptr;
  }
};

//
// User-interface
//

// Return number of fields in a visitable struct
template <typename S>
VISIT_STRUCT_CONSTEXPR std::size_t field_count()
{
  return traits::visitable<traits::clean_t<S>>::field_count;
}

template <typename S>
VISIT_STRUCT_CONSTEXPR std::size_t field_count(S &&) { return field_count<S>(); }


// apply_visitor (one struct instance)
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V && v, S && s) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// apply_visitor (two struct instances)
template <typename S1, typename S2, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V && v, S1 && s1, S2 && s2) ->
  typename std::enable_if<
             traits::is_visitable<
               traits::clean_t<typename traits::common_type<S1, S2>::type>
             >::value
           >::type
{
  using common_S = typename traits::common_type<S1, S2>::type;
  traits::visitable<traits::clean_t<common_S>>::apply(std::forward<V>(v),
                                                      std::forward<S1>(s1),
                                                      std::forward<S2>(s2));
}

// for_each (Alternate syntax for apply_visitor, reverses order of arguments)
template <typename V, typename S>
VISIT_STRUCT_CXX14_CONSTEXPR auto for_each(S && s, V && v) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// for_each with two structure instances
template <typename S1, typename S2, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto for_each(S1 && s1, S2 && s2, V && v) ->
  typename std::enable_if<
             traits::is_visitable<
               traits::clean_t<typename traits::common_type<S1, S2>::type>
             >::value
           >::type
{
  using common_S = typename traits::common_type<S1, S2>::type;
  traits::visitable<traits::clean_t<common_S>>::apply(std::forward<V>(v),
                                                      std::forward<S1>(s1),
                                                      std::forward<S2>(s2));
}

// Visit the types (visit_struct::type_c<...>) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_types(V && v) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  traits::visitable<traits::clean_t<S>>::visit_types(std::forward<V>(v));
}

// Visit the member pointers (&S::a) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_pointers(V && v) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  traits::visitable<traits::clean_t<S>>::visit_pointers(std::forward<V>(v));
}

// Visit the accessors (function objects) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_accessors(V && v) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  traits::visitable<traits::clean_t<S>>::visit_accessors(std::forward<V>(v));
}


// Apply visitor (with no instances)
// This calls visit_pointers, for backwards compat reasons
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V && v) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value
           >::type
{
  visit_struct::visit_pointers<S>(std::forward<V>(v));
}


// Get value by index (like std::get for tuples)
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get(S && s) ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value,
             decltype(traits::visitable<traits::clean_t<S>>::get_value(std::integral_constant<int, idx>{}, std::forward<S>(s)))
           >::type
{
  return traits::visitable<traits::clean_t<S>>::get_value(std::integral_constant<int, idx>{}, std::forward<S>(s));
}

// Get name of field, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_name() ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value,
             decltype(traits::visitable<traits::clean_t<S>>::get_name(std::integral_constant<int, idx>{}))
           >::type
{
  return traits::visitable<traits::clean_t<S>>::get_name(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_name(S &&) -> decltype(get_name<idx, S>()) {
  return get_name<idx, S>();
}

// Get member pointer, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_pointer() ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value,
             decltype(traits::visitable<traits::clean_t<S>>::get_pointer(std::integral_constant<int, idx>{}))
           >::type
{
  return traits::visitable<traits::clean_t<S>>::get_pointer(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_pointer(S &&) -> decltype(get_pointer<idx, S>()) {
  return get_pointer<idx, S>();
}

// Get member accessor, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_accessor() ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value,
             decltype(traits::visitable<traits::clean_t<S>>::get_accessor(std::integral_constant<int, idx>{}))
           >::type
{
  return traits::visitable<traits::clean_t<S>>::get_accessor(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_accessor(S &&) -> decltype(get_accessor<idx, S>()) {
  return get_accessor<idx, S>();
}

// Get type, by index
template <int idx, typename S>
struct type_at_s {
  using type_c = decltype(traits::visitable<traits::clean_t<S>>::type_at(std::integral_constant<int, idx>{}));
  using type = typename type_c::type;
};

template <int idx, typename S>
using type_at = typename type_at_s<idx, S>::type;

// Get name of structure
template <typename S>
VISIT_STRUCT_CONSTEXPR auto get_name() ->
  typename std::enable_if<
             traits::is_visitable<traits::clean_t<S>>::value,
             decltype(traits::visitable<traits::clean_t<S>>::get_name())
           >::type
{
  return traits::visitable<traits::clean_t<S>>::get_name();
}

template <typename S>
VISIT_STRUCT_CONSTEXPR auto get_name(S &&) -> decltype(get_name<S>()) {
  return get_name<S>();
}

/***
 * To implement the VISITABLE_STRUCT macro, we need a map-macro, which can take
 * the name of a macro and some other arguments, and apply that macro to each other argument.
 *
 * There are some techniques you can use within C preprocessor to accomplish this succinctly,
 * by settng up "recursive" macros.
 *
 * But this can also cause it to give worse error messages when something goes wrong.
 *
 * We are now doing it in a more "dumb", bulletproof way which has the advantage that it is
 * more portable and gives better error messages.
 * For discussion see IMPLEMENTATION_NOTES.md
 *
 * The code below is based on a patch from Jarod42, and is now generated by a python script.
 * The purpose of the generated code is to define VISIT_STRUCT_PP_MAP as described.
 */

/*** Generated code ***/

static VISIT_STRUCT_CONSTEXPR const int max_visitable_members = 69;

#define VISIT_STRUCT_EXPAND(x) x
#define VISIT_STRUCT_PP_ARG_N( \
        _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,\
        _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,\
        _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,\
        _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,\
        _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,\
        _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,\
        _61, _62, _63, _64, _65, _66, _67, _68, _69, N, ...) N
#define VISIT_STRUCT_PP_NARG(...) VISIT_STRUCT_EXPAND(VISIT_STRUCT_PP_ARG_N(__VA_ARGS__,  \
        69, 68, 67, 66, 65, 64, 63, 62, 61, 60,  \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50,  \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40,  \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30,  \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20,  \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  \
        9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* need extra level to force extra eval */
#define VISIT_STRUCT_CONCAT_(a,b) a ## b
#define VISIT_STRUCT_CONCAT(a,b) VISIT_STRUCT_CONCAT_(a,b)

#define VISIT_STRUCT_APPLYF0(f)
#define VISIT_STRUCT_APPLYF1(f,_1) f(_1)
#define VISIT_STRUCT_APPLYF2(f,_1,_2) f(_1) f(_2)
#define VISIT_STRUCT_APPLYF3(f,_1,_2,_3) f(_1) f(_2) f(_3)
#define VISIT_STRUCT_APPLYF4(f,_1,_2,_3,_4) f(_1) f(_2) f(_3) f(_4)
#define VISIT_STRUCT_APPLYF5(f,_1,_2,_3,_4,_5) f(_1) f(_2) f(_3) f(_4) f(_5)
#define VISIT_STRUCT_APPLYF6(f,_1,_2,_3,_4,_5,_6) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6)
#define VISIT_STRUCT_APPLYF7(f,_1,_2,_3,_4,_5,_6,_7) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7)
#define VISIT_STRUCT_APPLYF8(f,_1,_2,_3,_4,_5,_6,_7,_8) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8)
#define VISIT_STRUCT_APPLYF9(f,_1,_2,_3,_4,_5,_6,_7,_8,_9) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9)
#define VISIT_STRUCT_APPLYF10(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10)
#define VISIT_STRUCT_APPLYF11(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)
#define VISIT_STRUCT_APPLYF12(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12)
#define VISIT_STRUCT_APPLYF13(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)
#define VISIT_STRUCT_APPLYF14(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14)
#define VISIT_STRUCT_APPLYF15(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15)
#define VISIT_STRUCT_APPLYF16(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16)
#define VISIT_STRUCT_APPLYF17(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17)
#define VISIT_STRUCT_APPLYF18(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18)
#define VISIT_STRUCT_APPLYF19(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19)
#define VISIT_STRUCT_APPLYF20(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20)
#define VISIT_STRUCT_APPLYF21(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21)
#define VISIT_STRUCT_APPLYF22(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22)
#define VISIT_STRUCT_APPLYF23(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23)
#define VISIT_STRUCT_APPLYF24(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)
#define VISIT_STRUCT_APPLYF25(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25)
#define VISIT_STRUCT_APPLYF26(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26)
#define VISIT_STRUCT_APPLYF27(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27)
#define VISIT_STRUCT_APPLYF28(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28)
#define VISIT_STRUCT_APPLYF29(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)
#define VISIT_STRUCT_APPLYF30(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30)
#define VISIT_STRUCT_APPLYF31(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31)
#define VISIT_STRUCT_APPLYF32(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32)
#define VISIT_STRUCT_APPLYF33(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33)
#define VISIT_STRUCT_APPLYF34(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34)
#define VISIT_STRUCT_APPLYF35(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)
#define VISIT_STRUCT_APPLYF36(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36)
#define VISIT_STRUCT_APPLYF37(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37)
#define VISIT_STRUCT_APPLYF38(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38)
#define VISIT_STRUCT_APPLYF39(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39)
#define VISIT_STRUCT_APPLYF40(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40)
#define VISIT_STRUCT_APPLYF41(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41)
#define VISIT_STRUCT_APPLYF42(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42)
#define VISIT_STRUCT_APPLYF43(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43)
#define VISIT_STRUCT_APPLYF44(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44)
#define VISIT_STRUCT_APPLYF45(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)
#define VISIT_STRUCT_APPLYF46(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46)
#define VISIT_STRUCT_APPLYF47(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47)
#define VISIT_STRUCT_APPLYF48(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48)
#define VISIT_STRUCT_APPLYF49(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49)
#define VISIT_STRUCT_APPLYF50(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50)
#define VISIT_STRUCT_APPLYF51(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51)
#define VISIT_STRUCT_APPLYF52(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52)
#define VISIT_STRUCT_APPLYF53(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53)
#define VISIT_STRUCT_APPLYF54(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54)
#define VISIT_STRUCT_APPLYF55(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)
#define VISIT_STRUCT_APPLYF56(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56)
#define VISIT_STRUCT_APPLYF57(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57)
#define VISIT_STRUCT_APPLYF58(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58)
#define VISIT_STRUCT_APPLYF59(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59)
#define VISIT_STRUCT_APPLYF60(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60)
#define VISIT_STRUCT_APPLYF61(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61)
#define VISIT_STRUCT_APPLYF62(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62)
#define VISIT_STRUCT_APPLYF63(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63)
#define VISIT_STRUCT_APPLYF64(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)
#define VISIT_STRUCT_APPLYF65(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65)
#define VISIT_STRUCT_APPLYF66(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66)
#define VISIT_STRUCT_APPLYF67(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67)
#define VISIT_STRUCT_APPLYF68(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68)
#define VISIT_STRUCT_APPLYF69(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68) f(_69)

#define VISIT_STRUCT_APPLY_F_(M, ...) VISIT_STRUCT_EXPAND(M(__VA_ARGS__))
#define VISIT_STRUCT_PP_MAP(f, ...) VISIT_STRUCT_EXPAND(VISIT_STRUCT_APPLY_F_(VISIT_STRUCT_CONCAT(VISIT_STRUCT_APPLYF, VISIT_STRUCT_PP_NARG(__VA_ARGS__)), f, __VA_ARGS__))

/*** End generated code ***/

/***
 * These macros are used with VISIT_STRUCT_PP_MAP
 */

#define VISIT_STRUCT_FIELD_COUNT(MEMBER_NAME)                                                      \
  + 1

#define VISIT_STRUCT_MEMBER_HELPER(MEMBER_NAME)                                                    \
  std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S>(struct_instance).MEMBER_NAME);

#define VISIT_STRUCT_MEMBER_HELPER_PTR(MEMBER_NAME)                                                \
  std::forward<V>(visitor)(#MEMBER_NAME, &this_type::MEMBER_NAME);

#define VISIT_STRUCT_MEMBER_HELPER_TYPE(MEMBER_NAME)                                               \
  std::forward<V>(visitor)(#MEMBER_NAME, visit_struct::type_c<decltype(this_type::MEMBER_NAME)>{});

#define VISIT_STRUCT_MEMBER_HELPER_ACC(MEMBER_NAME)                                                \
  std::forward<V>(visitor)(#MEMBER_NAME, visit_struct::accessor<decltype(&this_type::MEMBER_NAME), &this_type::MEMBER_NAME>{});


#define VISIT_STRUCT_MEMBER_HELPER_PAIR(MEMBER_NAME)                                               \
  std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S1>(s1).MEMBER_NAME, std::forward<S2>(s2).MEMBER_NAME);

#define VISIT_STRUCT_MAKE_GETTERS(MEMBER_NAME)                                                     \
  template <typename S>                                                                            \
  static VISIT_STRUCT_CONSTEXPR auto                                                               \
    get_value(std::integral_constant<int, fields_enum::MEMBER_NAME>, S && s) ->                    \
    decltype((std::forward<S>(s).MEMBER_NAME)) {                                                   \
    return std::forward<S>(s).MEMBER_NAME;                                                         \
  }                                                                                                \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR auto                                                               \
    get_name(std::integral_constant<int, fields_enum::MEMBER_NAME>) ->                             \
      decltype(#MEMBER_NAME) {                                                                     \
    return #MEMBER_NAME;                                                                           \
  }                                                                                                \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR auto                                                               \
    get_pointer(std::integral_constant<int, fields_enum::MEMBER_NAME>) ->                          \
      decltype(&this_type::MEMBER_NAME) {                                                          \
    return &this_type::MEMBER_NAME;                                                                \
  }                                                                                                \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR auto                                                               \
    get_accessor(std::integral_constant<int, fields_enum::MEMBER_NAME>) ->                         \
      visit_struct::accessor<decltype(&this_type::MEMBER_NAME), &this_type::MEMBER_NAME > {        \
    return {};                                                                                     \
  }                                                                                                \
                                                                                                   \
  static auto                                                                                      \
    type_at(std::integral_constant<int, fields_enum::MEMBER_NAME>) ->                              \
      visit_struct::type_c<decltype(this_type::MEMBER_NAME)>;


// This macro specializes the trait, provides "apply" method which does the work.
// Below, template parameter S should always be the same as STRUCT_NAME modulo const and reference.
// The interface defined above ensures that STRUCT_NAME is clean_t<S> basically.
//
// Note: The code to make the indexed getters work is more convoluted than I'd like.
//       PP_MAP doesn't give you the index of each member. And rather than hack it so that it will
//       do that, what we do instead is:
//       1: Declare an enum `field_enum` in the scope of visitable, which maps names to indices.
//          This gives an easy way for the macro to get the index from the name token.
//       2: Intuitively we'd like to use template partial specialization to make indices map to
//          values, and have a new specialization for each member. But, specializations can only
//          be made at namespace scope. So to keep things tidy and contained within this trait,
//          we use tag dispatch with std::integral_constant<int> instead.

#define VISITABLE_STRUCT(STRUCT_NAME, ...)                                                         \
namespace visit_struct {                                                                           \
namespace traits {                                                                                 \
                                                                                                   \
template <>                                                                                        \
struct visitable<STRUCT_NAME, void> {                                                              \
                                                                                                   \
  using this_type = STRUCT_NAME;                                                                   \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR auto get_name()                                                    \
    -> decltype(#STRUCT_NAME) {                                                                    \
    return #STRUCT_NAME;                                                                           \
  }                                                                                                \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR const std::size_t field_count = 0                                  \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_FIELD_COUNT, __VA_ARGS__);                                    \
                                                                                                   \
  template <typename V, typename S>                                                                \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V && visitor, S && struct_instance)               \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER, __VA_ARGS__)                                   \
  }                                                                                                \
                                                                                                   \
  template <typename V, typename S1, typename S2>                                                  \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V && visitor, S1 && s1, S2 && s2)                 \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_PAIR, __VA_ARGS__)                              \
  }                                                                                                \
                                                                                                   \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void visit_pointers(V && visitor)                            \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_PTR, __VA_ARGS__)                               \
  }                                                                                                \
                                                                                                   \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void visit_types(V && visitor)                               \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_TYPE, __VA_ARGS__)                              \
  }                                                                                                \
                                                                                                   \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void visit_accessors(V && visitor)                           \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_ACC, __VA_ARGS__)                               \
  }                                                                                                \
                                                                                                   \
  struct fields_enum {                                                                             \
    enum index { __VA_ARGS__ };                                                                    \
  };                                                                                               \
                                                                                                   \
  VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MAKE_GETTERS, __VA_ARGS__)                                      \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR const bool value = true;                                           \
};                                                                                                 \
                                                                                                   \
}                                                                                                  \
}                                                                                                  \
static_assert(true, "")

} // end namespace visit_struct

#endif // VISIT_STRUCT_HPP_INCLUDED
//  __  __             _        ______                          _____
// |  \/  |           (_)      |  ____|                        / ____|_     _
// | \  / | __ _  __ _ _  ___  | |__   _ __  _   _ _ __ ___   | |   _| |_ _| |_
// | |\/| |/ _` |/ _` | |/ __| |  __| | '_ \| | | | '_ ` _ \  | |  |_   _|_   _|
// | |  | | (_| | (_| | | (__  | |____| | | | |_| | | | | | | | |____|_|   |_|
// |_|  |_|\__,_|\__, |_|\___| |______|_| |_|\__,_|_| |_| |_|  \_____|
//                __/ | https://github.com/Neargye/magic_enum
//               |___/  version 0.9.6
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019 - 2024 Daniil Goncharov <neargye@gmail.com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef NEARGYE_MAGIC_ENUM_HPP
#define NEARGYE_MAGIC_ENUM_HPP

#define MAGIC_ENUM_VERSION_MAJOR 0
#define MAGIC_ENUM_VERSION_MINOR 9
#define MAGIC_ENUM_VERSION_PATCH 6

#ifndef MAGIC_ENUM_USE_STD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#endif

#if defined(MAGIC_ENUM_CONFIG_FILE)
#  include MAGIC_ENUM_CONFIG_FILE
#endif

#ifndef MAGIC_ENUM_USE_STD_MODULE
#if !defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
#  include <optional>
#endif
#if !defined(MAGIC_ENUM_USING_ALIAS_STRING)
#  include <string>
#endif
#if !defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
#  include <string_view>
#endif
#endif

#if defined(MAGIC_ENUM_NO_ASSERT)
#  define MAGIC_ENUM_ASSERT(...) static_cast<void>(0)
#elif !defined(MAGIC_ENUM_ASSERT)
#  include <cassert>
#  define MAGIC_ENUM_ASSERT(...) assert((__VA_ARGS__))
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#  pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#  pragma clang diagnostic ignored "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // May be used uninitialized 'return {};'.
#  pragma GCC diagnostic ignored "-Wuseless-cast" // suppresses 'static_cast<char_type>('\0')' for char_type = char (common on Linux).
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 26495) // Variable 'static_str<N>::chars_' is uninitialized.
#  pragma warning(disable : 28020) // Arithmetic overflow: Using operator '-' on a 4 byte value and then casting the result to a 8 byte value.
#  pragma warning(disable : 26451) // The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
#  pragma warning(disable : 4514) // Unreferenced inline function has been removed.
#endif

// Checks magic_enum compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1910 || defined(__RESHARPER__)
#  undef  MAGIC_ENUM_SUPPORTED
#  define MAGIC_ENUM_SUPPORTED 1
#endif

// Checks magic_enum compiler aliases compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1920
#  undef  MAGIC_ENUM_SUPPORTED_ALIASES
#  define MAGIC_ENUM_SUPPORTED_ALIASES 1
#endif

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default MAGIC_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
#  define MAGIC_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default MAGIC_ENUM_RANGE_MAX = 127.
// If need another max range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
#  define MAGIC_ENUM_RANGE_MAX 127
#endif

// Improve ReSharper C++ intellisense performance with builtins, avoiding unnecessary template instantiations.
#if defined(__RESHARPER__)
#  undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#  undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#  if __RESHARPER__ >= 20230100
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) __rscpp_enumerator_name(V)
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) __rscpp_type_name<T>()
#  else
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) nullptr
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) nullptr
#  endif
#endif

namespace magic_enum {

// If need another optional type, define the macro MAGIC_ENUM_USING_ALIAS_OPTIONAL.
#if defined(MAGIC_ENUM_USING_ALIAS_OPTIONAL)
MAGIC_ENUM_USING_ALIAS_OPTIONAL
#else
using std::optional;
#endif

// If need another string_view type, define the macro MAGIC_ENUM_USING_ALIAS_STRING_VIEW.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING_VIEW)
MAGIC_ENUM_USING_ALIAS_STRING_VIEW
#else
using std::string_view;
#endif

// If need another string type, define the macro MAGIC_ENUM_USING_ALIAS_STRING.
#if defined(MAGIC_ENUM_USING_ALIAS_STRING)
MAGIC_ENUM_USING_ALIAS_STRING
#else
using std::string;
#endif

using char_type = string_view::value_type;
static_assert(std::is_same_v<string_view::value_type, string::value_type>, "magic_enum::customize requires same string_view::value_type and string::value_type");
static_assert([] {
  if constexpr (std::is_same_v<char_type, wchar_t>) {
    constexpr const char     c[] =  "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
    constexpr const wchar_t wc[] = L"abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789|";
    static_assert(std::size(c) == std::size(wc), "magic_enum::customize identifier characters are multichars in wchar_t.");

    for (std::size_t i = 0; i < std::size(c); ++i) {
      if (c[i] != wc[i]) {
        return false;
      }
    }
  }
  return true;
} (), "magic_enum::customize wchar_t is not compatible with ASCII.");

namespace customize {

// Enum value must be in range [MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX]. By default MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 127.
// If need another range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN and MAGIC_ENUM_RANGE_MAX.
// If need another range for specific enum type, add specialization enum_range for necessary enum type.
template <typename E>
struct enum_range {
  static constexpr int min = MAGIC_ENUM_RANGE_MIN;
  static constexpr int max = MAGIC_ENUM_RANGE_MAX;
};

static_assert(MAGIC_ENUM_RANGE_MAX > MAGIC_ENUM_RANGE_MIN, "MAGIC_ENUM_RANGE_MAX must be greater than MAGIC_ENUM_RANGE_MIN.");

namespace detail {

enum class customize_tag {
  default_tag,
  invalid_tag,
  custom_tag
};

} // namespace magic_enum::customize::detail

class customize_t : public std::pair<detail::customize_tag, string_view> {
 public:
  constexpr customize_t(string_view srt) : std::pair<detail::customize_tag, string_view>{detail::customize_tag::custom_tag, srt} {}
  constexpr customize_t(const char_type* srt) : customize_t{string_view{srt}} {}
  constexpr customize_t(detail::customize_tag tag) : std::pair<detail::customize_tag, string_view>{tag, string_view{}} {
    MAGIC_ENUM_ASSERT(tag != detail::customize_tag::custom_tag);
  }
};

// Default customize.
inline constexpr auto default_tag = customize_t{detail::customize_tag::default_tag};
// Invalid customize.
inline constexpr auto invalid_tag = customize_t{detail::customize_tag::invalid_tag};

// If need custom names for enum, add specialization enum_name for necessary enum type.
template <typename E>
constexpr customize_t enum_name(E) noexcept {
  return default_tag;
}

// If need custom type name for enum, add specialization enum_type_name for necessary enum type.
template <typename E>
constexpr customize_t enum_type_name() noexcept {
  return default_tag;
}

} // namespace magic_enum::customize

namespace detail {

template <typename T>
struct supported
#if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED || defined(MAGIC_ENUM_NO_CHECK_SUPPORT)
  : std::true_type {};
#else
  : std::false_type {};
#endif

template <auto V, typename E = std::decay_t<decltype(V)>, std::enable_if_t<std::is_enum_v<E>, int> = 0>
using enum_constant = std::integral_constant<E, V>;

template <typename... T>
inline constexpr bool always_false_v = false;

template <typename T, typename = void>
struct has_is_flags : std::false_type {};

template <typename T>
struct has_is_flags<T, std::void_t<decltype(customize::enum_range<T>::is_flags)>> : std::bool_constant<std::is_same_v<bool, std::decay_t<decltype(customize::enum_range<T>::is_flags)>>> {};

template <typename T, typename = void>
struct range_min : std::integral_constant<int, MAGIC_ENUM_RANGE_MIN> {};

template <typename T>
struct range_min<T, std::void_t<decltype(customize::enum_range<T>::min)>> : std::integral_constant<decltype(customize::enum_range<T>::min), customize::enum_range<T>::min> {};

template <typename T, typename = void>
struct range_max : std::integral_constant<int, MAGIC_ENUM_RANGE_MAX> {};

template <typename T>
struct range_max<T, std::void_t<decltype(customize::enum_range<T>::max)>> : std::integral_constant<decltype(customize::enum_range<T>::max), customize::enum_range<T>::max> {};

struct str_view {
  const char* str_ = nullptr;
  std::size_t size_ = 0;
};

template <std::uint16_t N>
class static_str {
 public:
  constexpr explicit static_str(str_view str) noexcept : static_str{str.str_, std::make_integer_sequence<std::uint16_t, N>{}} {
    MAGIC_ENUM_ASSERT(str.size_ == N);
  }

  constexpr explicit static_str(string_view str) noexcept : static_str{str.data(), std::make_integer_sequence<std::uint16_t, N>{}} {
    MAGIC_ENUM_ASSERT(str.size() == N);
  }

  constexpr const char_type* data() const noexcept { return chars_; }

  constexpr std::uint16_t size() const noexcept { return N; }

  constexpr operator string_view() const noexcept { return {data(), size()}; }

 private:
  template <std::uint16_t... I>
  constexpr static_str(const char* str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_{static_cast<char_type>(str[I])..., static_cast<char_type>('\0')} {}

  template <std::uint16_t... I>
  constexpr static_str(string_view str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_{str[I]..., static_cast<char_type>('\0')} {}

  char_type chars_[static_cast<std::size_t>(N) + 1];
};

template <>
class static_str<0> {
 public:
  constexpr explicit static_str() = default;

  constexpr explicit static_str(str_view) noexcept {}

  constexpr explicit static_str(string_view) noexcept {}

  constexpr const char_type* data() const noexcept { return nullptr; }

  constexpr std::uint16_t size() const noexcept { return 0; }

  constexpr operator string_view() const noexcept { return {}; }
};

template <typename Op = std::equal_to<>>
class case_insensitive {
  static constexpr char_type to_lower(char_type c) noexcept {
    return (c >= static_cast<char_type>('A') && c <= static_cast<char_type>('Z')) ? static_cast<char_type>(c + (static_cast<char_type>('a') - static_cast<char_type>('A'))) : c;
  }

 public:
  template <typename L, typename R>
  constexpr auto operator()(L lhs, R rhs) const noexcept -> std::enable_if_t<std::is_same_v<std::decay_t<L>, char_type> && std::is_same_v<std::decay_t<R>, char_type>, bool> {
    return Op{}(to_lower(lhs), to_lower(rhs));
  }
};

constexpr std::size_t find(string_view str, char_type c) noexcept {
#if defined(__clang__) && __clang_major__ < 9 && defined(__GLIBCXX__) || defined(_MSC_VER) && _MSC_VER < 1920 && !defined(__clang__)
// https://stackoverflow.com/questions/56484834/constexpr-stdstring-viewfind-last-of-doesnt-work-on-clang-8-with-libstdc
// https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
  constexpr bool workaround = true;
#else
  constexpr bool workaround = false;
#endif

  if constexpr (workaround) {
    for (std::size_t i = 0; i < str.size(); ++i) {
      if (str[i] == c) {
        return i;
      }
    }

    return string_view::npos;
  } else {
    return str.find(c);
  }
}

template <typename BinaryPredicate>
constexpr bool is_default_predicate() noexcept {
  return std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<string_view::value_type>> ||
         std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<>>;
}

template <typename BinaryPredicate>
constexpr bool is_nothrow_invocable() {
  return is_default_predicate<BinaryPredicate>() ||
         std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char_type, char_type>;
}

template <typename BinaryPredicate>
constexpr bool cmp_equal(string_view lhs, string_view rhs, [[maybe_unused]] BinaryPredicate&& p) noexcept(is_nothrow_invocable<BinaryPredicate>()) {
#if defined(_MSC_VER) && _MSC_VER < 1920 && !defined(__clang__)
  // https://developercommunity.visualstudio.com/content/problem/360432/vs20178-regression-c-failed-in-test.html
  // https://developercommunity.visualstudio.com/content/problem/232218/c-constexpr-string-view.html
  constexpr bool workaround = true;
#else
  constexpr bool workaround = false;
#endif

  if constexpr (!is_default_predicate<BinaryPredicate>() || workaround) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    const auto size = lhs.size();
    for (std::size_t i = 0; i < size; ++i) {
      if (!p(lhs[i], rhs[i])) {
        return false;
      }
    }

    return true;
  } else {
    return lhs == rhs;
  }
}

template <typename L, typename R>
constexpr bool cmp_less(L lhs, R rhs) noexcept {
  static_assert(std::is_integral_v<L> && std::is_integral_v<R>, "magic_enum::detail::cmp_less requires integral type.");

  if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
    // If same signedness (both signed or both unsigned).
    return lhs < rhs;
  } else if constexpr (std::is_same_v<L, bool>) { // bool special case
      return static_cast<R>(lhs) < rhs;
  } else if constexpr (std::is_same_v<R, bool>) { // bool special case
      return lhs < static_cast<L>(rhs);
  } else if constexpr (std::is_signed_v<R>) {
    // If 'right' is negative, then result is 'false', otherwise cast & compare.
    return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
  } else {
    // If 'left' is negative, then result is 'true', otherwise cast & compare.
    return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
  }
}

template <typename I>
constexpr I log2(I value) noexcept {
  static_assert(std::is_integral_v<I>, "magic_enum::detail::log2 requires integral type.");

  if constexpr (std::is_same_v<I, bool>) { // bool special case
    return MAGIC_ENUM_ASSERT(false), value;
  } else {
    auto ret = I{0};
    for (; value > I{1}; value >>= I{1}, ++ret) {}

    return ret;
  }
}

#if defined(__cpp_lib_array_constexpr) && __cpp_lib_array_constexpr >= 201603L
#  define MAGIC_ENUM_ARRAY_CONSTEXPR 1
#else
template <typename T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N], std::index_sequence<I...>) noexcept {
  return {{a[I]...}};
}
#endif

template <typename T>
inline constexpr bool is_enum_v = std::is_enum_v<T> && std::is_same_v<T, std::decay_t<T>>;

template <typename E>
constexpr auto n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

  if constexpr (supported<E>::value) {
#if defined(MAGIC_ENUM_GET_TYPE_NAME_BUILTIN)
    constexpr auto name_ptr = MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(E);
    constexpr auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#elif defined(__clang__)
    str_view name;
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else {
      name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
      name.str_ = __PRETTY_FUNCTION__ + 34;
    }
#elif defined(__GNUC__)
    auto name = str_view{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<E>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else if (name.str_[name.size_ - 1] == ']') {
      name.size_ -= 50;
      name.str_ += 49;
    } else {
      name.size_ -= 40;
      name.str_ += 37;
    }
#elif defined(_MSC_VER)
    // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
    str_view name;
    name.str_ = __FUNCSIG__;
    name.str_ += 40;
    name.size_ += sizeof(__FUNCSIG__) - 57;
#else
    auto name = str_view{};
#endif
    std::size_t p = 0;
    for (std::size_t i = name.size_; i > 0; --i) {
      if (name.str_[i] == ':') {
        p = i + 1;
        break;
      }
    }
    if (p > 0) {
      name.size_ -= p;
      name.str_ += p;
    }
    return name;
  } else {
    return str_view{}; // Unsupported compiler or Invalid customize.
  }
}

template <typename E>
constexpr auto type_name() noexcept {
  [[maybe_unused]] constexpr auto custom = customize::enum_type_name<E>();
  static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>, "magic_enum::customize requires customize_t type.");
  if constexpr (custom.first == customize::detail::customize_tag::custom_tag) {
    constexpr auto name = custom.second;
    static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
    return static_str<name.size()>{name};
  } else if constexpr (custom.first == customize::detail::customize_tag::invalid_tag) {
    return static_str<0>{};
  } else if constexpr (custom.first == customize::detail::customize_tag::default_tag) {
    constexpr auto name = n<E>();
    return static_str<name.size_>{name};
  } else {
    static_assert(always_false_v<E>, "magic_enum::customize invalid.");
  }
}

template <typename E>
inline constexpr auto type_name_v = type_name<E>();

template <auto V>
constexpr auto n() noexcept {
  static_assert(is_enum_v<decltype(V)>, "magic_enum::detail::n requires enum type.");

  if constexpr (supported<decltype(V)>::value) {
#if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
    constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
    auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#elif defined(__clang__)
    str_view name;
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else {
      name.size_ = sizeof(__PRETTY_FUNCTION__) - 36;
      name.str_ = __PRETTY_FUNCTION__ + 34;
    }
    if (name.size_ > 22 && name.str_[0] == '(' && name.str_[1] == 'a' && name.str_[10] == ' ' && name.str_[22] == ':') {
      name.size_ -= 23;
      name.str_ += 23;
    }
    if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9')) {
      name = str_view{};
    }
#elif defined(__GNUC__)
    auto name = str_view{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
    if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__)) {
      static_assert(always_false_v<decltype(V)>, "magic_enum::detail::n requires __PRETTY_FUNCTION__.");
      return str_view{};
    } else if (name.str_[name.size_ - 1] == ']') {
      name.size_ -= 55;
      name.str_ += 54;
    } else {
      name.size_ -= 40;
      name.str_ += 37;
    }
    if (name.str_[0] == '(') {
      name = str_view{};
    }
#elif defined(_MSC_VER)
    str_view name;
    if ((__FUNCSIG__[5] == '_' && __FUNCSIG__[35] != '(') || (__FUNCSIG__[5] == 'c' && __FUNCSIG__[41] != '(')) {
      // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
      name.str_ = __FUNCSIG__;
      name.str_ += 35;
      name.size_ = sizeof(__FUNCSIG__) - 52;
    }
#else
    auto name = str_view{};
#endif
    std::size_t p = 0;
    for (std::size_t i = name.size_; i > 0; --i) {
      if (name.str_[i] == ':') {
        p = i + 1;
        break;
      }
    }
    if (p > 0) {
      name.size_ -= p;
      name.str_ += p;
    }
    return name;
  } else {
    return str_view{}; // Unsupported compiler or Invalid customize.
  }
}

#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER < 1920
#  define MAGIC_ENUM_VS_2017_WORKAROUND 1
#endif

#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
template <typename E, E V>
constexpr auto n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");

#  if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
  constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
  auto name = name_ptr ? str_view{name_ptr, std::char_traits<char>::length(name_ptr)} : str_view{};
#  else
  // CLI/C++ workaround (see https://github.com/Neargye/magic_enum/issues/284).
  str_view name;
  name.str_ = __FUNCSIG__;
  name.size_ = sizeof(__FUNCSIG__) - 17;
  std::size_t p = 0;
  for (std::size_t i = name.size_; i > 0; --i) {
    if (name.str_[i] == ',' || name.str_[i] == ':') {
      p = i + 1;
      break;
    }
  }
  if (p > 0) {
    name.size_ -= p;
    name.str_ += p;
  }
  if (name.str_[0] == '(' || name.str_[0] == '-' || (name.str_[0] >= '0' && name.str_[0] <= '9')) {
    name = str_view{};
  }
  return name;
#  endif
}
#endif

template <typename E, E V>
constexpr auto enum_name() noexcept {
  [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(V);
  static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>, "magic_enum::customize requires customize_t type.");
  if constexpr (custom.first == customize::detail::customize_tag::custom_tag) {
    constexpr auto name = custom.second;
    static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
    return static_str<name.size()>{name};
  } else if constexpr (custom.first == customize::detail::customize_tag::invalid_tag) {
    return static_str<0>{};
  } else if constexpr (custom.first == customize::detail::customize_tag::default_tag) {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
    constexpr auto name = n<E, V>();
#else
    constexpr auto name = n<V>();
#endif
    return static_str<name.size_>{name};
  } else {
    static_assert(always_false_v<E>, "magic_enum::customize invalid.");
  }
}

template <typename E, E V>
inline constexpr auto enum_name_v = enum_name<E, V>();

template <typename E, auto V>
constexpr bool is_valid() noexcept {
#if defined(__clang__) && __clang_major__ >= 16
  // https://reviews.llvm.org/D130058, https://reviews.llvm.org/D131307
  constexpr E v = __builtin_bit_cast(E, V);
#else
  constexpr E v = static_cast<E>(V);
#endif
  [[maybe_unused]] constexpr auto custom = customize::enum_name<E>(v);
  static_assert(std::is_same_v<std::decay_t<decltype(custom)>, customize::customize_t>, "magic_enum::customize requires customize_t type.");
  if constexpr (custom.first == customize::detail::customize_tag::custom_tag) {
    constexpr auto name = custom.second;
    static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
    return name.size() != 0;
  } else if constexpr (custom.first == customize::detail::customize_tag::default_tag) {
#if defined(MAGIC_ENUM_VS_2017_WORKAROUND)
    return n<E, v>().size_ != 0;
#else
    return n<v>().size_ != 0;
#endif
  } else {
    return false;
  }
}

enum class enum_subtype {
  common,
  flags
};

template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr U ualue(std::size_t i) noexcept {
  if constexpr (std::is_same_v<U, bool>) { // bool special case
    static_assert(O == 0, "magic_enum::detail::ualue requires valid offset.");

    return static_cast<U>(i);
  } else if constexpr (S == enum_subtype::flags) {
    return static_cast<U>(U{1} << static_cast<U>(static_cast<int>(i) + O));
  } else {
    return static_cast<U>(static_cast<int>(i) + O);
  }
}

template <typename E, int O, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr E value(std::size_t i) noexcept {
  return static_cast<E>(ualue<E, O, S>(i));
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr int reflected_min() noexcept {
  if constexpr (S == enum_subtype::flags) {
    return 0;
  } else {
    constexpr auto lhs = range_min<E>::value;
    constexpr auto rhs = (std::numeric_limits<U>::min)();

    if constexpr (cmp_less(rhs, lhs)) {
      return lhs;
    } else {
      return rhs;
    }
  }
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr int reflected_max() noexcept {
  if constexpr (S == enum_subtype::flags) {
    return std::numeric_limits<U>::digits - 1;
  } else {
    constexpr auto lhs = range_max<E>::value;
    constexpr auto rhs = (std::numeric_limits<U>::max)();

    if constexpr (cmp_less(lhs, rhs)) {
      return lhs;
    } else {
      return rhs;
    }
  }
}

#define MAGIC_ENUM_FOR_EACH_256(T)                                                                                                                                                                 \
  T(  0)T(  1)T(  2)T(  3)T(  4)T(  5)T(  6)T(  7)T(  8)T(  9)T( 10)T( 11)T( 12)T( 13)T( 14)T( 15)T( 16)T( 17)T( 18)T( 19)T( 20)T( 21)T( 22)T( 23)T( 24)T( 25)T( 26)T( 27)T( 28)T( 29)T( 30)T( 31) \
  T( 32)T( 33)T( 34)T( 35)T( 36)T( 37)T( 38)T( 39)T( 40)T( 41)T( 42)T( 43)T( 44)T( 45)T( 46)T( 47)T( 48)T( 49)T( 50)T( 51)T( 52)T( 53)T( 54)T( 55)T( 56)T( 57)T( 58)T( 59)T( 60)T( 61)T( 62)T( 63) \
  T( 64)T( 65)T( 66)T( 67)T( 68)T( 69)T( 70)T( 71)T( 72)T( 73)T( 74)T( 75)T( 76)T( 77)T( 78)T( 79)T( 80)T( 81)T( 82)T( 83)T( 84)T( 85)T( 86)T( 87)T( 88)T( 89)T( 90)T( 91)T( 92)T( 93)T( 94)T( 95) \
  T( 96)T( 97)T( 98)T( 99)T(100)T(101)T(102)T(103)T(104)T(105)T(106)T(107)T(108)T(109)T(110)T(111)T(112)T(113)T(114)T(115)T(116)T(117)T(118)T(119)T(120)T(121)T(122)T(123)T(124)T(125)T(126)T(127) \
  T(128)T(129)T(130)T(131)T(132)T(133)T(134)T(135)T(136)T(137)T(138)T(139)T(140)T(141)T(142)T(143)T(144)T(145)T(146)T(147)T(148)T(149)T(150)T(151)T(152)T(153)T(154)T(155)T(156)T(157)T(158)T(159) \
  T(160)T(161)T(162)T(163)T(164)T(165)T(166)T(167)T(168)T(169)T(170)T(171)T(172)T(173)T(174)T(175)T(176)T(177)T(178)T(179)T(180)T(181)T(182)T(183)T(184)T(185)T(186)T(187)T(188)T(189)T(190)T(191) \
  T(192)T(193)T(194)T(195)T(196)T(197)T(198)T(199)T(200)T(201)T(202)T(203)T(204)T(205)T(206)T(207)T(208)T(209)T(210)T(211)T(212)T(213)T(214)T(215)T(216)T(217)T(218)T(219)T(220)T(221)T(222)T(223) \
  T(224)T(225)T(226)T(227)T(228)T(229)T(230)T(231)T(232)T(233)T(234)T(235)T(236)T(237)T(238)T(239)T(240)T(241)T(242)T(243)T(244)T(245)T(246)T(247)T(248)T(249)T(250)T(251)T(252)T(253)T(254)T(255)

template <typename E, enum_subtype S, std::size_t Size, int Min, std::size_t I>
constexpr void valid_count(bool* valid, std::size_t& count) noexcept {
#define MAGIC_ENUM_V(O)                                     \
  if constexpr ((I + O) < Size) {                           \
    if constexpr (is_valid<E, ualue<E, Min, S>(I + O)>()) { \
      valid[I + O] = true;                                  \
      ++count;                                              \
    }                                                       \
  }

  MAGIC_ENUM_FOR_EACH_256(MAGIC_ENUM_V)

  if constexpr ((I + 256) < Size) {
    valid_count<E, S, Size, Min, I + 256>(valid, count);
  }
#undef MAGIC_ENUM_V
}

template <std::size_t N>
struct valid_count_t {
  std::size_t count = 0;
  bool valid[N] = {};
};

template <typename E, enum_subtype S, std::size_t Size, int Min>
constexpr auto valid_count() noexcept {
  valid_count_t<Size> vc;
  valid_count<E, S, Size, Min, 0>(vc.valid, vc.count);
  return vc;
}

template <typename E, enum_subtype S, std::size_t Size, int Min>
constexpr auto values() noexcept {
  constexpr auto vc = valid_count<E, S, Size, Min>();

  if constexpr (vc.count > 0) {
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
    std::array<E, vc.count> values = {};
#else
    E values[vc.count] = {};
#endif
    for (std::size_t i = 0, v = 0; v < vc.count; ++i) {
      if (vc.valid[i]) {
        values[v++] = value<E, Min, S>(i);
      }
    }
#if defined(MAGIC_ENUM_ARRAY_CONSTEXPR)
    return values;
#else
    return to_array(values, std::make_index_sequence<vc.count>{});
#endif
  } else {
    return std::array<E, 0>{};
  }
}

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr auto values() noexcept {
  constexpr auto min = reflected_min<E, S>();
  constexpr auto max = reflected_max<E, S>();
  constexpr auto range_size = max - min + 1;
  static_assert(range_size > 0, "magic_enum::enum_range requires valid size.");

  return values<E, S, range_size, min>();
}

template <typename E, typename U = std::underlying_type_t<E>>
constexpr enum_subtype subtype(std::true_type) noexcept {
  if constexpr (std::is_same_v<U, bool>) { // bool special case
    return enum_subtype::common;
  } else if constexpr (has_is_flags<E>::value) {
    return customize::enum_range<E>::is_flags ? enum_subtype::flags : enum_subtype::common;
  } else {
#if defined(MAGIC_ENUM_AUTO_IS_FLAGS)
    constexpr auto flags_values = values<E, enum_subtype::flags>();
    constexpr auto default_values = values<E, enum_subtype::common>();
    if (flags_values.size() == 0 || default_values.size() > flags_values.size()) {
      return enum_subtype::common;
    }
    for (std::size_t i = 0; i < default_values.size(); ++i) {
      const auto v = static_cast<U>(default_values[i]);
      if (v != 0 && (v & (v - 1)) != 0) {
        return enum_subtype::common;
      }
    }
    return enum_subtype::flags;
#else
    return enum_subtype::common;
#endif
  }
}

template <typename T>
constexpr enum_subtype subtype(std::false_type) noexcept {
  // For non-enum type return default common subtype.
  return enum_subtype::common;
}

template <typename E, typename D = std::decay_t<E>>
inline constexpr auto subtype_v = subtype<D>(std::is_enum<D>{});

template <typename E, enum_subtype S>
inline constexpr auto values_v = values<E, S>();

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using values_t = decltype((values_v<D, S>));

template <typename E, enum_subtype S>
inline constexpr auto count_v = values_v<E, S>.size();

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
inline constexpr auto min_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.front()) : U{0};

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
inline constexpr auto max_v = (count_v<E, S> > 0) ? static_cast<U>(values_v<E, S>.back()) : U{0};

template <typename E, enum_subtype S, std::size_t... I>
constexpr auto names(std::index_sequence<I...>) noexcept {
  constexpr auto names = std::array<string_view, sizeof...(I)>{{enum_name_v<E, values_v<E, S>[I]>...}};
  return names;
}

template <typename E, enum_subtype S>
inline constexpr auto names_v = names<E, S>(std::make_index_sequence<count_v<E, S>>{});

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using names_t = decltype((names_v<D, S>));

template <typename E, enum_subtype S, std::size_t... I>
constexpr auto entries(std::index_sequence<I...>) noexcept {
  constexpr auto entries = std::array<std::pair<E, string_view>, sizeof...(I)>{{{values_v<E, S>[I], enum_name_v<E, values_v<E, S>[I]>}...}};
  return entries;
}

template <typename E, enum_subtype S>
inline constexpr auto entries_v = entries<E, S>(std::make_index_sequence<count_v<E, S>>{});

template <typename E, enum_subtype S, typename D = std::decay_t<E>>
using entries_t = decltype((entries_v<D, S>));

template <typename E, enum_subtype S, typename U = std::underlying_type_t<E>>
constexpr bool is_sparse() noexcept {
  if constexpr (count_v<E, S> == 0) {
    return false;
  } else if constexpr (std::is_same_v<U, bool>) { // bool special case
    return false;
  } else {
    constexpr auto max = (S == enum_subtype::flags) ? log2(max_v<E, S>) : max_v<E, S>;
    constexpr auto min = (S == enum_subtype::flags) ? log2(min_v<E, S>) : min_v<E, S>;
    constexpr auto range_size = max - min + 1;

    return range_size != count_v<E, S>;
  }
}

template <typename E, enum_subtype S = subtype_v<E>>
inline constexpr bool is_sparse_v = is_sparse<E, S>();

template <typename E, enum_subtype S>
struct is_reflected
#if defined(MAGIC_ENUM_NO_CHECK_REFLECTED_ENUM)
  : std::true_type {};
#else
  : std::bool_constant<std::is_enum_v<E> && (count_v<E, S> != 0)> {};
#endif

template <typename E, enum_subtype S>
inline constexpr bool is_reflected_v = is_reflected<std::decay_t<E>, S>{};

template <bool, typename R>
struct enable_if_enum {};

template <typename R>
struct enable_if_enum<true, R> {
  using type = R;
  static_assert(supported<R>::value, "magic_enum unsupported compiler (https://github.com/Neargye/magic_enum#compiler-compatibility).");
};

template <typename T, typename R, typename BinaryPredicate = std::equal_to<>, typename D = std::decay_t<T>>
using enable_if_t = typename enable_if_enum<std::is_enum_v<D> && std::is_invocable_r_v<bool, BinaryPredicate, char_type, char_type>, R>::type;

template <typename T, std::enable_if_t<std::is_enum_v<std::decay_t<T>>, int> = 0>
using enum_concept = T;

template <typename T, bool = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

template <typename T>
struct is_scoped_enum<T, true> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<T>>
struct is_unscoped_enum : std::false_type {};

template <typename T>
struct is_unscoped_enum<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<std::decay_t<T>>>
struct underlying_type {};

template <typename T>
struct underlying_type<T, true> : std::underlying_type<std::decay_t<T>> {};

#if defined(MAGIC_ENUM_ENABLE_HASH) || defined(MAGIC_ENUM_ENABLE_HASH_SWITCH)

template <typename Value, typename = void>
struct constexpr_hash_t;

template <typename Value>
struct constexpr_hash_t<Value, std::enable_if_t<is_enum_v<Value>>> {
  constexpr auto operator()(Value value) const noexcept {
    using U = typename underlying_type<Value>::type;
    if constexpr (std::is_same_v<U, bool>) { // bool special case
      return static_cast<std::size_t>(value);
    } else {
      return static_cast<U>(value);
    }
  }
  using secondary_hash = constexpr_hash_t;
};

template <typename Value>
struct constexpr_hash_t<Value, std::enable_if_t<std::is_same_v<Value, string_view>>> {
  static constexpr std::uint32_t crc_table[256] {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
  };
  constexpr std::uint32_t operator()(string_view value) const noexcept {
    auto crc = static_cast<std::uint32_t>(0xffffffffL);
    for (const auto c : value) {
      crc = (crc >> 8) ^ crc_table[(crc ^ static_cast<std::uint32_t>(c)) & 0xff];
    }
    return crc ^ 0xffffffffL;
  }

  struct secondary_hash {
    constexpr std::uint32_t operator()(string_view value) const noexcept {
      auto acc = static_cast<std::uint64_t>(2166136261ULL);
      for (const auto c : value) {
        acc = ((acc ^ static_cast<std::uint64_t>(c)) * static_cast<std::uint64_t>(16777619ULL)) & (std::numeric_limits<std::uint32_t>::max)();
      }
      return static_cast<std::uint32_t>(acc);
    }
  };
};

template <typename Hash>
inline constexpr Hash hash_v{};

template <auto* GlobValues, typename Hash>
constexpr auto calculate_cases(std::size_t Page) noexcept {
  constexpr std::array values = *GlobValues;
  constexpr std::size_t size = values.size();

  using switch_t = std::invoke_result_t<Hash, typename decltype(values)::value_type>;
  static_assert(std::is_integral_v<switch_t> && !std::is_same_v<switch_t, bool>);
  const std::size_t values_to = (std::min)(static_cast<std::size_t>(256), size - Page);

  std::array<switch_t, 256> result{};
  auto fill = result.begin();
  {
    auto first = values.begin() + static_cast<std::ptrdiff_t>(Page);
    auto last = values.begin() + static_cast<std::ptrdiff_t>(Page + values_to);
    while (first != last) {
      *fill++ = hash_v<Hash>(*first++);
    }
  }

  // dead cases, try to avoid case collisions
  for (switch_t last_value = result[values_to - 1]; fill != result.end() && last_value != (std::numeric_limits<switch_t>::max)(); *fill++ = ++last_value) {
  }

  {
    auto it = result.begin();
    auto last_value = (std::numeric_limits<switch_t>::min)();
    for (; fill != result.end(); *fill++ = last_value++) {
      while (last_value == *it) {
        ++last_value, ++it;
      }
    }
  }

  return result;
}

template <typename R, typename F, typename... Args>
constexpr R invoke_r(F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>) {
  if constexpr (std::is_void_v<R>) {
    std::forward<F>(f)(std::forward<Args>(args)...);
  } else {
    return static_cast<R>(std::forward<F>(f)(std::forward<Args>(args)...));
  }
}

enum class case_call_t {
  index,
  value
};

template <typename T = void>
inline constexpr auto default_result_type_lambda = []() noexcept(std::is_nothrow_default_constructible_v<T>) { return T{}; };

template <>
inline constexpr auto default_result_type_lambda<void> = []() noexcept {};

template <auto* Arr, typename Hash>
constexpr bool has_duplicate() noexcept {
  using value_t = std::decay_t<decltype((*Arr)[0])>;
  using hash_value_t = std::invoke_result_t<Hash, value_t>;
  std::array<hash_value_t, Arr->size()> hashes{};
  std::size_t size = 0;
  for (auto elem : *Arr) {
    hashes[size] = hash_v<Hash>(elem);
    for (auto i = size++; i > 0; --i) {
      if (hashes[i] < hashes[i - 1]) {
        auto tmp = hashes[i];
        hashes[i] = hashes[i - 1];
        hashes[i - 1] = tmp;
      } else if (hashes[i] == hashes[i - 1]) {
        return false;
      } else {
        break;
      }
    }
  }
  return true;
}

#define MAGIC_ENUM_CASE(val)                                                                                                  \
  case cases[val]:                                                                                                            \
    if constexpr ((val) + Page < size) {                                                                                      \
      if (!pred(values[val + Page], searched)) {                                                                              \
        break;                                                                                                                \
      }                                                                                                                       \
      if constexpr (CallValue == case_call_t::index) {                                                                        \
        if constexpr (std::is_invocable_r_v<result_t, Lambda, std::integral_constant<std::size_t, val + Page>>) {             \
          return detail::invoke_r<result_t>(std::forward<Lambda>(lambda), std::integral_constant<std::size_t, val + Page>{}); \
        } else if constexpr (std::is_invocable_v<Lambda, std::integral_constant<std::size_t, val + Page>>) {                  \
          MAGIC_ENUM_ASSERT(false && "magic_enum::detail::constexpr_switch wrong result type.");                                         \
        }                                                                                                                     \
      } else if constexpr (CallValue == case_call_t::value) {                                                                 \
        if constexpr (std::is_invocable_r_v<result_t, Lambda, enum_constant<values[val + Page]>>) {                           \
          return detail::invoke_r<result_t>(std::forward<Lambda>(lambda), enum_constant<values[val + Page]>{});               \
        } else if constexpr (std::is_invocable_r_v<result_t, Lambda, enum_constant<values[val + Page]>>) {                    \
          MAGIC_ENUM_ASSERT(false && "magic_enum::detail::constexpr_switch wrong result type.");                                         \
        }                                                                                                                     \
      }                                                                                                                       \
      break;                                                                                                                  \
    } else [[fallthrough]];

template <auto* GlobValues,
          case_call_t CallValue,
          std::size_t Page = 0,
          typename Hash = constexpr_hash_t<typename std::decay_t<decltype(*GlobValues)>::value_type>,
          typename BinaryPredicate = std::equal_to<>,
          typename Lambda,
          typename ResultGetterType>
constexpr decltype(auto) constexpr_switch(
    Lambda&& lambda,
    typename std::decay_t<decltype(*GlobValues)>::value_type searched,
    ResultGetterType&& def,
    BinaryPredicate&& pred = {}) {
  using result_t = std::invoke_result_t<ResultGetterType>;
  using hash_t = std::conditional_t<has_duplicate<GlobValues, Hash>(), Hash, typename Hash::secondary_hash>;
  static_assert(has_duplicate<GlobValues, hash_t>(), "magic_enum::detail::constexpr_switch duplicated hash found, please report it: https://github.com/Neargye/magic_enum/issues.");
  constexpr std::array values = *GlobValues;
  constexpr std::size_t size = values.size();
  constexpr std::array cases = calculate_cases<GlobValues, hash_t>(Page);

  switch (hash_v<hash_t>(searched)) {
    MAGIC_ENUM_FOR_EACH_256(MAGIC_ENUM_CASE)
    default:
      if constexpr (size > 256 + Page) {
        return constexpr_switch<GlobValues, CallValue, Page + 256, Hash>(std::forward<Lambda>(lambda), searched, std::forward<ResultGetterType>(def));
      }
      break;
  }
  return def();
}

#undef MAGIC_ENUM_CASE

#endif

} // namespace magic_enum::detail

// Checks is magic_enum supported compiler.
inline constexpr bool is_magic_enum_supported = detail::supported<void>::value;

template <typename T>
using Enum = detail::enum_concept<T>;

// Checks whether T is an Unscoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Unscoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumeration) type. Otherwise, value is equal to false.
template <typename T>
struct is_unscoped_enum : detail::is_unscoped_enum<T> {};

template <typename T>
inline constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

// Checks whether T is an Scoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Scoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type. Otherwise, value is equal to false.
template <typename T>
struct is_scoped_enum : detail::is_scoped_enum<T> {};

template <typename T>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// If T is a complete enumeration type, provides a member typedef type that names the underlying type of T.
// Otherwise, if T is not an enumeration type, there is no member type. Otherwise (T is an incomplete enumeration type), the program is ill-formed.
template <typename T>
struct underlying_type : detail::underlying_type<T> {};

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

template <auto V>
using enum_constant = detail::enum_constant<V>;

// Returns type name of enum.
template <typename E>
[[nodiscard]] constexpr auto enum_type_name() noexcept -> detail::enable_if_t<E, string_view> {
  constexpr string_view name = detail::type_name_v<std::decay_t<E>>;
  static_assert(!name.empty(), "magic_enum::enum_type_name enum type does not have a name.");

  return name;
}

// Returns number of enum values.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_count() noexcept -> detail::enable_if_t<E, std::size_t> {
  return detail::count_v<std::decay_t<E>, S>;
}

// Returns enum value at specified index.
// No bounds checking is performed: the behavior is undefined if index >= number of enum values.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_value(std::size_t index) noexcept -> detail::enable_if_t<E, std::decay_t<E>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D, S>) {
    return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::values_v<D, S>[index];
  } else {
    constexpr auto min = (S == detail::enum_subtype::flags) ? detail::log2(detail::min_v<D, S>) : detail::min_v<D, S>;

    return MAGIC_ENUM_ASSERT(index < detail::count_v<D, S>), detail::value<D, min, S>(index);
  }
}

// Returns enum value at specified index.
template <typename E, std::size_t I, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_value() noexcept -> detail::enable_if_t<E, std::decay_t<E>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");
  static_assert(I < detail::count_v<D, S>, "magic_enum::enum_value out of range.");

  return enum_value<D, S>(I);
}

// Returns std::array with enum values, sorted by enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_values() noexcept -> detail::enable_if_t<E, detail::values_t<E, S>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return detail::values_v<D, S>;
}

// Returns integer value from enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_integer(E value) noexcept -> detail::enable_if_t<E, underlying_type_t<E>> {
  return static_cast<underlying_type_t<E>>(value);
}

// Returns underlying value from enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_underlying(E value) noexcept -> detail::enable_if_t<E, underlying_type_t<E>> {
  return static_cast<underlying_type_t<E>>(value);
}

// Obtains index in enum values from enum value.
// Returns optional with index.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>> {
  using D = std::decay_t<E>;
  using U = underlying_type_t<D>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D, S> || (S == detail::enum_subtype::flags)) {
#if defined(MAGIC_ENUM_ENABLE_HASH)
    return detail::constexpr_switch<&detail::values_v<D, S>, detail::case_call_t::index>(
        [](std::size_t i) { return optional<std::size_t>{i}; },
        value,
        detail::default_result_type_lambda<optional<std::size_t>>);
#else
    for (std::size_t i = 0; i < detail::count_v<D, S>; ++i) {
      if (enum_value<D, S>(i) == value) {
        return i;
      }
    }
    return {}; // Invalid value or out of range.
#endif
  } else {
    const auto v = static_cast<U>(value);
    if (v >= detail::min_v<D, S> && v <= detail::max_v<D, S>) {
      return static_cast<std::size_t>(v - detail::min_v<D, S>);
    }
    return {}; // Invalid value or out of range.
  }
}

// Obtains index in enum values from enum value.
// Returns optional with index.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_index(E value) noexcept -> detail::enable_if_t<E, optional<std::size_t>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return enum_index<D, S>(value);
}

// Obtains index in enum values from static storage enum variable.
template <auto V, detail::enum_subtype S = detail::subtype_v<std::decay_t<decltype(V)>>>
[[nodiscard]] constexpr auto enum_index() noexcept -> detail::enable_if_t<decltype(V), std::size_t> {\
  using D = std::decay_t<decltype(V)>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");
  constexpr auto index = enum_index<D, S>(V);
  static_assert(index, "magic_enum::enum_index enum value does not have a index.");

  return *index;
}

// Returns name from static storage enum variable.
// This version is much lighter on the compile times and is not restricted to the enum_range limitation.
template <auto V>
[[nodiscard]] constexpr auto enum_name() noexcept -> detail::enable_if_t<decltype(V), string_view> {
  constexpr string_view name = detail::enum_name_v<std::decay_t<decltype(V)>, V>;
  static_assert(!name.empty(), "magic_enum::enum_name enum value does not have a name.");

  return name;
}

// Returns name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_name(E value) noexcept -> detail::enable_if_t<E, string_view> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if (const auto i = enum_index<D, S>(value)) {
    return detail::names_v<D, S>[*i];
  }
  return {};
}

// Returns name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_name(E value) -> detail::enable_if_t<E, string_view> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return enum_name<D, S>(value);
}

// Returns std::array with names, sorted by enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_names() noexcept -> detail::enable_if_t<E, detail::names_t<E, S>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return detail::names_v<D, S>;
}

// Returns std::array with pairs (value, name), sorted by enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_entries() noexcept -> detail::enable_if_t<E, detail::entries_t<E, S>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  return detail::entries_v<D, S>;
}

// Allows you to write magic_enum::enum_cast<foo>("bar", magic_enum::case_insensitive);
inline constexpr auto case_insensitive = detail::case_insensitive<>{};

// Obtains enum value from integer value.
// Returns optional with enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_cast(underlying_type_t<E> value) noexcept -> detail::enable_if_t<E, optional<std::decay_t<E>>> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D, S> || (S == detail::enum_subtype::flags)) {
#if defined(MAGIC_ENUM_ENABLE_HASH)
    return detail::constexpr_switch<&detail::values_v<D, S>, detail::case_call_t::value>(
        [](D v) { return optional<D>{v}; },
        static_cast<D>(value),
        detail::default_result_type_lambda<optional<D>>);
#else
    for (std::size_t i = 0; i < detail::count_v<D, S>; ++i) {
      if (value == static_cast<underlying_type_t<D>>(enum_value<D, S>(i))) {
        return static_cast<D>(value);
      }
    }
    return {}; // Invalid value or out of range.
#endif
  } else {
    if (value >= detail::min_v<D, S> && value <= detail::max_v<D, S>) {
      return static_cast<D>(value);
    }
    return {}; // Invalid value or out of range.
  }
}

// Obtains enum value from name.
// Returns optional with enum value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>, typename BinaryPredicate = std::equal_to<>>
[[nodiscard]] constexpr auto enum_cast(string_view value, [[maybe_unused]] BinaryPredicate p = {}) noexcept(detail::is_nothrow_invocable<BinaryPredicate>()) -> detail::enable_if_t<E, optional<std::decay_t<E>>, BinaryPredicate> {
  using D = std::decay_t<E>;
  static_assert(detail::is_reflected_v<D, S>, "magic_enum requires enum implementation and valid max and min.");

#if defined(MAGIC_ENUM_ENABLE_HASH)
  if constexpr (detail::is_default_predicate<BinaryPredicate>()) {
    return detail::constexpr_switch<&detail::names_v<D, S>, detail::case_call_t::index>(
        [](std::size_t i) { return optional<D>{detail::values_v<D, S>[i]}; },
        value,
        detail::default_result_type_lambda<optional<D>>,
        [&p](string_view lhs, string_view rhs) { return detail::cmp_equal(lhs, rhs, p); });
  }
#endif
  for (std::size_t i = 0; i < detail::count_v<D, S>; ++i) {
    if (detail::cmp_equal(value, detail::names_v<D, S>[i], p)) {
      return enum_value<D, S>(i);
    }
  }
  return {}; // Invalid value or out of range.
}

// Checks whether enum contains value with such value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_contains(E value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;
  using U = underlying_type_t<D>;

  return static_cast<bool>(enum_cast<D, S>(static_cast<U>(value)));
}

// Checks whether enum contains value with such value.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_contains(E value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;
  using U = underlying_type_t<D>;

  return static_cast<bool>(enum_cast<D, S>(static_cast<U>(value)));
}

// Checks whether enum contains value with such integer value.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_contains(underlying_type_t<E> value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;

  return static_cast<bool>(enum_cast<D, S>(value));
}

// Checks whether enum contains enumerator with such name.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>, typename BinaryPredicate = std::equal_to<>>
[[nodiscard]] constexpr auto enum_contains(string_view value, BinaryPredicate p = {}) noexcept(detail::is_nothrow_invocable<BinaryPredicate>()) -> detail::enable_if_t<E, bool, BinaryPredicate> {
  using D = std::decay_t<E>;

  return static_cast<bool>(enum_cast<D, S>(value, std::move(p)));
}

// Returns true if the enum integer value is in the range of values that can be reflected.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_reflected(underlying_type_t<E> value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;

  if constexpr (detail::is_reflected_v<D, S>) {
    constexpr auto min = detail::reflected_min<E, S>();
    constexpr auto max = detail::reflected_max<E, S>();
    return value >= min && value <= max;
  } else {
    return false;
  }
}

// Returns true if the enum value is in the range of values that can be reflected.
template <typename E, detail::enum_subtype S = detail::subtype_v<E>>
[[nodiscard]] constexpr auto enum_reflected(E value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;

  return enum_reflected<D, S>(static_cast<underlying_type_t<D>>(value));
}

// Returns true if the enum value is in the range of values that can be reflected.
template <detail::enum_subtype S, typename E>
[[nodiscard]] constexpr auto enum_reflected(E value) noexcept -> detail::enable_if_t<E, bool> {
  using D = std::decay_t<E>;

  return enum_reflected<D, S>(value);
}

template <bool AsFlags = true>
inline constexpr auto as_flags = AsFlags ? detail::enum_subtype::flags : detail::enum_subtype::common;

template <bool AsFlags = true>
inline constexpr auto as_common = AsFlags ? detail::enum_subtype::common : detail::enum_subtype::flags;

namespace bitwise_operators {

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator~(E rhs) noexcept {
  return static_cast<E>(~static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator|(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) | static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator&(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) & static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E operator^(E lhs, E rhs) noexcept {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) ^ static_cast<underlying_type_t<E>>(rhs));
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator|=(E& lhs, E rhs) noexcept {
  return lhs = (lhs | rhs);
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator&=(E& lhs, E rhs) noexcept {
  return lhs = (lhs & rhs);
}

template <typename E, detail::enable_if_t<E, int> = 0>
constexpr E& operator^=(E& lhs, E rhs) noexcept {
  return lhs = (lhs ^ rhs);
}

} // namespace magic_enum::bitwise_operators

} // namespace magic_enum

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#undef MAGIC_ENUM_VS_2017_WORKAROUND
#undef MAGIC_ENUM_ARRAY_CONSTEXPR
#undef MAGIC_ENUM_FOR_EACH_256

#endif // NEARGYE_MAGIC_ENUM_HPP

#pragma once
#include <array>
using std::array;

namespace structopt {

template <typename> struct array_size;
template <typename T, size_t N> struct array_size<array<T, N>> {
  static size_t const size = N;
};

} // namespace structopt
#pragma once

namespace structopt {

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

} // namespace structopt
#pragma once
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace structopt {

// specialize a type for all of the STL containers.
namespace is_stl_container_impl {
template <typename T> struct is_stl_container : std::false_type {};
template <typename T, std::size_t N>
struct is_stl_container<std::array<T, N>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::vector<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::deque<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::list<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::forward_list<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::set<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::multiset<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::map<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::multimap<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::unordered_set<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::unordered_multiset<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::unordered_map<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::unordered_multimap<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::stack<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::queue<Args...>> : std::true_type {};
template <typename... Args>
struct is_stl_container<std::priority_queue<Args...>> : std::true_type {};
} // namespace is_stl_container_impl

template <class T> struct is_array : std::is_array<T> {};
template <class T, std::size_t N> struct is_array<std::array<T, N>> : std::true_type {};
// optional:
template <class T> struct is_array<T const> : is_array<T> {};
template <class T> struct is_array<T volatile> : is_array<T> {};
template <class T> struct is_array<T volatile const> : is_array<T> {};

// type trait to utilize the implementation type traits as well as decay the type
template <typename T> struct is_stl_container {
  static constexpr bool const value =
      is_stl_container_impl::is_stl_container<std::decay_t<T>>::value;
};

} // namespace structopt

#pragma once
#include <string>
#include <string_view>

namespace structopt {

namespace details {

static inline bool string_replace(std::string &str, const std::string &from,
                                  const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

inline std::string string_to_kebab(std::string_view str) {
  // Generate kebab case and present as option
  std::string str2(str.begin(), str.end());
  details::string_replace(str2, "_", "-");
  return str2;
}

} // namespace details

} // namespace structopt
#pragma once
#include <string_view>

namespace structopt {

namespace details {

static inline bool is_binary_notation(std::string_view input) {
  return input.compare(0, 2, "0b") == 0 && input.size() > 2 &&
         input.find_first_not_of("01", 2) == std::string_view::npos;
}

static inline bool is_hex_notation(std::string_view input) {
  return input.compare(0, 2, "0x") == 0 && input.size() > 2 &&
         input.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string_view::npos;
}

static inline bool is_octal_notation(std::string_view input) {
  return input.compare(0, 1, "0") == 0 && input.size() > 1 &&
         input.find_first_not_of("01234567", 1) == std::string_view::npos;
}

static inline bool is_valid_number(std::string_view input) {
  if (is_binary_notation(input) || is_hex_notation(input) || is_octal_notation(input)) {
    return true;
  }

  if (input.empty()) {
    return false;
  }

  std::size_t i = 0, j = input.length() - 1;

  // Handling whitespaces
  while (i < input.length() && input[i] == ' ')
    i++;
  while (input[j] == ' ')
    j--;

  if (i > j)
    return false;

  // if string is of length 1 and the only
  // character is not a digit
  if (i == j && !(input[i] >= '0' && input[i] <= '9'))
    return false;

  // If the 1st char is not '+', '-', '.' or digit
  if (input[i] != '.' && input[i] != '+' && input[i] != '-' &&
      !(input[i] >= '0' && input[i] <= '9'))
    return false;

  // To check if a '.' or 'e' is found in given
  // string. We use this flag to make sure that
  // either of them appear only once.
  bool dot_or_exp = false;

  for (; i <= j; i++) {
    // If any of the char does not belong to
    // {digit, +, -, ., e}
    if (input[i] != 'e' && input[i] != '.' && input[i] != '+' && input[i] != '-' &&
        !(input[i] >= '0' && input[i] <= '9'))
      return false;

    if (input[i] == '.') {
      // checks if the char 'e' has already
      // occurred before '.' If yes, return false;.
      if (dot_or_exp == true)
        return false;

      // If '.' is the last character.
      if (i + 1 > input.length())
        return false;

      // if '.' is not followed by a digit.
      if (!(input[i + 1] >= '0' && input[i + 1] <= '9'))
        return false;
    }

    else if (input[i] == 'e') {
      // set dot_or_exp = 1 when e is encountered.
      dot_or_exp = true;

      // if there is no digit before 'e'.
      if (!(input[i - 1] >= '0' && input[i - 1] <= '9'))
        return false;

      // If 'e' is the last Character
      if (i + 1 > input.length())
        return false;

      // if e is not followed either by
      // '+', '-' or a digit
      if (input[i + 1] != '+' && input[i + 1] != '-' &&
          (input[i + 1] >= '0' && input[i] <= '9'))
        return false;
    }
  }

  /* If the string skips all above cases, then
  it is numeric*/
  return true;
}

} // namespace details

} // namespace structopt

#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
// #include <structopt/is_specialization.hpp>
// #include <structopt/string.hpp>
// #include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

namespace structopt {

class app;

namespace details {

struct visitor {
  std::string name;
  std::string version;
  std::optional<std::string> help;
  std::vector<std::string_view> field_names;
  std::deque<std::string_view> positional_field_names; // mutated by parser
  std::deque<std::string_view> positional_field_names_for_help;
  std::deque<std::string_view> vector_like_positional_field_names;
  std::deque<std::string_view> flag_field_names;
  std::deque<std::string_view> optional_field_names;
  std::deque<std::string_view> nested_struct_field_names;

  visitor() = default;

  explicit visitor(std::string name, std::string version)
      : name(std::move(name)), version(std::move(version)) {}

  explicit visitor(std::string name, std::string version, std::string help)
      : name(std::move(name)), version(std::move(version)),
        help(std::move(help)) {}

  // Visitor function for std::optional - could be an option or a flag
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(std::string_view name, T &) {
    field_names.push_back(name);
    if constexpr (std::is_same<typename T::value_type, bool>::value) {
      flag_field_names.push_back(name);
    } else {
      optional_field_names.push_back(name);
    }
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value &&
                                     !visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(std::string_view name, T &) {
    field_names.push_back(name);
    positional_field_names.push_back(name);
    positional_field_names_for_help.push_back(name);
    if constexpr (structopt::is_specialization<T, std::deque>::value ||
                  structopt::is_specialization<T, std::list>::value ||
                  structopt::is_specialization<T, std::vector>::value ||
                  structopt::is_specialization<T, std::set>::value ||
                  structopt::is_specialization<T, std::multiset>::value ||
                  structopt::is_specialization<T, std::unordered_set>::value ||
                  structopt::is_specialization<T, std::unordered_multiset>::value ||
                  structopt::is_specialization<T, std::queue>::value ||
                  structopt::is_specialization<T, std::stack>::value ||
                  structopt::is_specialization<T, std::priority_queue>::value) {
      // keep track of vector-like fields as these (even though positional)
      // can be happy without any arguments
      vector_like_positional_field_names.push_back(name);
    }
  }

  // Visitor function for nested structs
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, void>::type
  operator()(std::string_view name, T &) {
    field_names.push_back(name);
    nested_struct_field_names.push_back(name);
  }

  bool is_field_name(std::string_view field_name) {
    return std::find(field_names.begin(), field_names.end(), field_name) !=
           field_names.end();
  }

  void print_help(std::ostream &os) const {
    if (help.has_value() && help.value().size() > 0) {
      os << help.value();
    } else {
      os << "\nUSAGE: " << name << " ";

      if (flag_field_names.empty() == false) {
        os << "[FLAGS] ";
      }

      if (optional_field_names.empty() == false) {
        os << "[OPTIONS] ";
      }

      if (nested_struct_field_names.empty() == false) {
        os << "[SUBCOMMANDS] ";
      }

      for (auto &field : positional_field_names_for_help) {
        os << field << " ";
      }

      bool has_h = false;
      bool has_v = false;
      if (flag_field_names.empty() == false) {
        os << "\n\nFLAGS:\n";
        for (auto &flag : flag_field_names) {

          // Generate kebab case and present as flag
          auto kebab_case = details::string_to_kebab(flag);
          std::string long_form = "";
          if (kebab_case != flag) {
            long_form = kebab_case;
          } else {
            long_form = flag;
          }

          os << "    -" << flag[0] << ", --" << flag << "\n";

          switch (flag[0]) {
          case 'h':
            has_h = true;
            break;
          case 'v':
            has_v = true;
            break;
          }
        }
      } else {
        os << "\n";
      }

      if (optional_field_names.empty() == false) {
        os << "\nOPTIONS:\n";
        for (auto &option : optional_field_names) {

          // Generate kebab case and present as option
          auto kebab_case = details::string_to_kebab(option);
          std::string long_form = "";
          if (kebab_case != option) {
            long_form = kebab_case;
          } else {
            long_form = option;
          }

          if ((has_v && option == "version") || (has_h && option == "help")) {
            os << "    --" << long_form << " <" << option << ">\n";
          } else {
            os << "    -" << option[0] << ", --" << long_form << " <" << option << ">"
               << "\n";
          }

          switch (option[0]) {
          case 'h':
            has_h = true;
            break;
          case 'v':
            has_v = true;
            break;
          }
        }
      }

      if (nested_struct_field_names.empty() == false) {
        os << "\nSUBCOMMANDS:\n";
        for (auto &sc : nested_struct_field_names) {
          os << "    " << sc << "\n";
        }
      }

      if (positional_field_names_for_help.empty() == false) {
        os << "\nARGS:\n";
        for (auto &arg : positional_field_names_for_help) {
          os << "    " << arg << "\n";
        }
      }
    }
  }
};

} // namespace details

} // namespace structopt

#pragma once
#include <exception>
#include <sstream>
#include <string>
// #include <structopt/visitor.hpp>

namespace structopt {

class exception : public std::exception {
  std::string what_{""};
  std::string help_{""};
  details::visitor visitor_;

public:
  exception(const std::string &what, const details::visitor &visitor)
      : what_(what), help_(""), visitor_(visitor) {
    std::stringstream os;
    visitor_.print_help(os);
    help_ = os.str();
  }

  const char *what() const throw() { return what_.c_str(); }

  const char *help() const throw() { return help_.c_str(); }
};

} // namespace structopt
#pragma once
#include <optional>
// #include <structopt/visitor.hpp>

namespace structopt {

namespace details {
struct parser;
}

class sub_command {
  std::optional<bool> invoked_;
  details::visitor visitor_;

  friend struct structopt::details::parser;

public:
  bool has_value() const { return invoked_.has_value(); }
};

} // namespace structopt
#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
// #include <structopt/array_size.hpp>
// #include <structopt/exception.hpp>
// #include <structopt/is_number.hpp>
// #include <structopt/is_specialization.hpp>
// #include <structopt/sub_command.hpp>
// #include <structopt/third_party/magic_enum/magic_enum.hpp>
// #include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace structopt {

namespace details {

// Effectively s1.replace('-', '_') == s2
inline bool equal_strings_replace_hyphens(std::string_view s1, std::string_view s2) {
  return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char c1, char c2) {
    return c1 == c2 || (c1 == '-' && c2 == '_');
  });
}

struct parser {
  structopt::details::visitor visitor;
  std::vector<std::string> arguments;
  std::size_t current_index{1};
  std::size_t next_index{1};
  bool double_dash_encountered{false}; // "--" option-argument delimiter
  bool sub_command_invoked{false};
  std::string already_invoked_subcommand_name{""};

  bool is_optional(std::string_view name) {
    if (double_dash_encountered) {
      return false;
    } else if (name == "--") {
      double_dash_encountered = true;
      return false;
    } else if (is_valid_number(name)) {
      return false;
    }

    bool result = false;
    if (name.size() >= 2) {
      // e.g., -b, -v
      if (name[0] == '-') {
        result = true;
        // e.g., --verbose
        if (name[1] == '-') {
          result = true;
        }
      }
    }
    return result;
  }

  bool is_kebab_case(std::string_view next, std::string_view field_name) {
    auto maybe_kebab_case = next;
    if (maybe_kebab_case.size() > 1 && maybe_kebab_case[0] == '-') {
      // remove first dash
      maybe_kebab_case.remove_prefix(1);
      if (maybe_kebab_case[0] == '-') {
        // there is a second leading dash
        // remove it
        maybe_kebab_case.remove_prefix(1);
      }
      if (equal_strings_replace_hyphens(maybe_kebab_case, field_name)) {
        return true;
      }
    }
    return false;
  }

  bool is_optional_field(std::string_view next, std::string_view field_name) {
    if (next.rfind("-", 0) == 0 && next.substr(1) == field_name) {
      return true;
    }
    if (next.rfind("--", 0) == 0 && next.substr(2) == field_name) {
      return true;
    }
    if (next == std::data({'-', field_name[0], '\0'})) {
      return true;
    }
    return is_kebab_case(next, field_name);
  }

  bool is_optional_field(std::string_view next) {
    if (!is_optional(next)) {
      return false;
    }

    bool result = false;
    for (auto &field_name : visitor.field_names) {
      result = is_optional_field(next, field_name);
      if (result) {
        break;
      }
    }
    return result;
  }

  // checks if the next argument is a delimited optional field
  // e.g., -std=c++17, where std matches a field name
  // and it is delimited by one of the two allowed delimiters: `=` and `:`
  //
  // if true, the return value includes the delimiter that was used
  std::pair<bool, char> is_delimited_optional_argument(std::string_view next) {
    bool success = false;
    char delimiter = '\0';

    auto equal_pos = next.find('=');
    auto colon_pos = next.find(':');

    if (equal_pos == std::string_view::npos && colon_pos == std::string_view::npos) {
      // not delimited
      return {success, delimiter};
    } else {
      // assume `=` comes first
      char c = '=';

      if (colon_pos < equal_pos) {
        // confirmed: `:` comes first
        c = ':';
      }

      // split `next` into key and value
      // check if key is an optional field
      std::string key;
      bool delimiter_found = false;
      for (size_t i = 0; i < next.size(); i++) {
        if (next[i] == c && !delimiter_found) {
          delimiter = c;
          delimiter_found = true;
        } else {
          if (!delimiter_found) {
            key += next[i];
          }
        }
      }

      // check if `key` is a valid optional field
      if (delimiter_found && is_optional_field(key)) {
        success = true;
      }
    }
    return {success, delimiter};
  }

  std::pair<std::string, std::string> split_delimited_argument(char delimiter,
                                                               std::string_view next) {
    std::string key, value;
    bool delimiter_found = false;
    for (size_t i = 0; i < next.size(); i++) {
      if (next[i] == delimiter && !delimiter_found) {
        delimiter_found = true;
      } else {
        if (!delimiter_found) {
          key += next[i];
        } else {
          value += next[i];
        }
      }
    }
    return {key, value};
  }

  // Get the optional field name if any from 
  // e.g., `-v` => `verbose`
  // e.g., `-log-level` => `log_level`
  std::optional<std::string_view> get_full_optional_field_name(std::string_view next) {
    std::optional<std::string_view> result;

    if (next.size() == 2 && next[0] == '-') {
      // short form of optional argument
      for (auto& oarg : visitor.optional_field_names) {
        if (oarg[0] == next[1]) {
          // second character of next matches first character of some optional field_name
          result = oarg;
          break;
        }
      }
    } else {
      // long form of optional argument

      // strip dashes on the left
      const auto potential_field_name = next.substr(next.find_first_not_of('-'));

      // check if `potential_field_name` is in the optional field names list
      for (auto& oarg : visitor.optional_field_names) {
        if (equal_strings_replace_hyphens(potential_field_name, oarg)) {
          result = oarg;
          break;
        }
      }

    }

    return result;
  }

  template <typename T> std::pair<T, bool> parse_argument(std::string_view name) {
    if (next_index >= arguments.size()) {
      return {T(), false};
    }
    T result;
    bool success = true;
    if constexpr (visit_struct::traits::is_visitable<T>::value) {
      result = parse_nested_struct<T>(name);
    } else if constexpr (std::is_enum<T>::value) {
      result = parse_enum_argument<T>(name);
      next_index += 1;
    } else if constexpr (structopt::is_specialization<T, std::pair>::value) {
      result = parse_pair_argument<typename T::first_type, typename T::second_type>(name);
    } else if constexpr (structopt::is_specialization<T, std::tuple>::value) {
      result = parse_tuple_argument<T>(name);
    } else if constexpr (!is_stl_container<T>::value) {
      result = parse_single_argument<T>(name);
      next_index += 1;
    } else if constexpr (structopt::is_array<T>::value) {
      constexpr std::size_t N = structopt::array_size<T>::size;
      result = parse_array_argument<typename T::value_type, N>(name);
    } else if constexpr (structopt::is_specialization<T, std::deque>::value ||
                         structopt::is_specialization<T, std::list>::value ||
                         structopt::is_specialization<T, std::vector>::value) {
      result = parse_vector_like_argument<T>(name);
    } else if constexpr (structopt::is_specialization<T, std::set>::value ||
                         structopt::is_specialization<T, std::multiset>::value ||
                         structopt::is_specialization<T, std::unordered_set>::value ||
                         structopt::is_specialization<T,
                                                      std::unordered_multiset>::value) {
      result = parse_set_argument<T>(name);
    } else if constexpr (structopt::is_specialization<T, std::queue>::value ||
                         structopt::is_specialization<T, std::stack>::value ||
                         structopt::is_specialization<T, std::priority_queue>::value) {
      result = parse_container_adapter_argument<T>(name);
    } else {
      success = false;
    }
    return {result, success};
  }

  template <typename T> std::optional<T> parse_optional_argument(std::string_view name) {
    next_index += 1;
    std::optional<T> result;
    if (next_index < arguments.size()) {
      auto [value, success] = parse_argument<T>(name);
      if (success) {
        result = value;
      } else {
        throw structopt::exception(
            "Error: failed to correctly parse optional argument `" + std::string{name} +
                "`.",
            visitor);
      }
    } else {
      throw structopt::exception("Error: expected value for optional argument `" +
                                     std::string{name} + "`.",
                                 visitor);
    }
    return result;
  }

  // Any field that can be constructed using std::stringstream
  // Not container type
  // Not a visitable type, i.e., a nested struct
  template <typename T>
  inline typename std::enable_if<!visit_struct::traits::is_visitable<T>::value, T>::type
  parse_single_argument(std::string_view) {
    std::string argument = arguments[next_index];
    std::istringstream ss(argument);
    T result;

    if constexpr (std::is_integral<T>::value) {
      if (is_hex_notation(argument)) {
        ss >> std::hex >> result;
      } else if (is_octal_notation(argument)) {
        ss >> std::oct >> result;
      } else if (is_binary_notation(argument)) {
        argument.erase(0, 2); // remove "0b"
        result = static_cast<T>(std::stoi(argument, nullptr, 2));
      } else {
        ss >> std::dec >> result;
      }
    } else {
      ss >> result;
    }

    return result;
  }

  // Nested visitable struct
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, T>::type
  parse_nested_struct(std::string_view name) {

    T argument_struct;

    if constexpr (std::is_base_of<structopt::sub_command, T>::value) {
      argument_struct.invoked_ = true;
    }

    // Save struct field names
    argument_struct.visitor_.name = name; // sub-command name; not the program
    argument_struct.visitor_.version = visitor.version;
    visit_struct::for_each(argument_struct, argument_struct.visitor_);

    // add `help` and `version` optional arguments
    argument_struct.visitor_.optional_field_names.push_back("help");
    argument_struct.visitor_.optional_field_names.push_back("version");

    if (!sub_command_invoked) {
      sub_command_invoked = true;
      already_invoked_subcommand_name = name;
    } else {
      // a sub-command has already been invoked
      throw structopt::exception(
          "Error: failed to invoke sub-command `" + std::string{name} +
              "` because a different sub-command, `" + already_invoked_subcommand_name +
              "`, has already been invoked.",
          argument_struct.visitor_);
    }

    structopt::details::parser parser;
    parser.next_index = 0;
    parser.current_index = 0;
    parser.double_dash_encountered = double_dash_encountered;
    parser.visitor = argument_struct.visitor_;

    std::copy(arguments.begin() + next_index, arguments.end(),
              std::back_inserter(parser.arguments));

    for (std::size_t i = 0; i < parser.arguments.size(); i++) {
      parser.current_index = i;
      visit_struct::for_each(argument_struct, parser);
    }

    // directly call the parser to check for `help` and `version` flags
    std::optional<bool> help = false, version = false;
    for (std::size_t i = 0; i < parser.arguments.size(); i++) {
      parser.operator()("help", help);
      parser.operator()("version", version);

      if (help == true) {
        // if help is requested, print help and exit
        argument_struct.visitor_.print_help(std::cout);
        exit(EXIT_SUCCESS);
      } else if (version == true) {
        // if version is requested, print version and exit
        std::cout << argument_struct.visitor_.version << "\n";
        exit(EXIT_SUCCESS);
      }
    }

    // if all positional arguments were provided
    // this list would be empty
    if (!parser.visitor.positional_field_names.empty()) {
      for (auto &field_name : parser.visitor.positional_field_names) {
        if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                      parser.visitor.vector_like_positional_field_names.end(),
                      field_name) ==
            parser.visitor.vector_like_positional_field_names.end()) {
          // this positional argument is not a vector-like argument
          // it expects value(s)
          throw structopt::exception("Error: expected value for positional argument `" +
                                         std::string(field_name) + "`.",
                                     argument_struct.visitor_);
        }
      }
    }

    // update current and next
    current_index += parser.next_index;
    next_index += parser.next_index;

    return argument_struct;
  }

  // Pair argument
  template <typename T1, typename T2>
  std::pair<T1, T2> parse_pair_argument(std::string_view name) {
    std::pair<T1, T2> result;
    {
      // Pair first
      auto [value, success] = parse_argument<T1>(name);
      if (success) {
        result.first = value;
      } else {
        if (next_index == arguments.size()) {
          // end of arguments list
          // first argument not provided
          throw structopt::exception("Error: failed to correctly parse the pair `" +
                                         std::string{name} +
                                         "`. Expected 2 arguments, 0 provided.",
                                     visitor);
        } else {
          throw structopt::exception(
              "Error: failed to correctly parse first element of pair `" +
                  std::string{name} + "`",
              visitor);
        }
      }
    }
    {
      // Pair second
      auto [value, success] = parse_argument<T2>(name);
      if (success) {
        result.second = value;
      } else {
        if (next_index == arguments.size()) {
          // end of arguments list
          // second argument not provided
          throw structopt::exception("Error: failed to correctly parse the pair `" +
                                         std::string{name} +
                                         "`. Expected 2 arguments, only 1 provided.",
                                     visitor);
        } else {
          throw structopt::exception(
              "Error: failed to correctly parse second element of pair `" +
                  std::string{name} + "`",
              visitor);
        }
      }
    }
    return result;
  }

  // Array argument
  template <typename T, std::size_t N>
  std::array<T, N> parse_array_argument(std::string_view name) {
    std::array<T, N> result{};

    const auto arguments_left = arguments.size() - next_index;
    if (arguments_left == 0 || arguments_left < N) {
      throw structopt::exception("Error: expected " + std::to_string(N) +
                                     " values for std::array argument `" + std::string(name) +
                                     "` - instead got only " +
                                     std::to_string(arguments_left) + " arguments.",
                                 visitor);
    }

    for (std::size_t i = 0; i < N; i++) {
      auto [value, success] = parse_argument<T>(name);
      if (success) {
        result[i] = value;
      }
    }
    return result;
  }

  template <class Tuple, class F, std::size_t... I>
  constexpr F for_each_impl(Tuple &&t, F &&f, std::index_sequence<I...>) {
    return (void)std::initializer_list<int>{
               (std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))), 0)...},
           f;
  }

  template <class Tuple, class F> constexpr F for_each(Tuple &&t, F &&f) {
    return for_each_impl(std::forward<Tuple>(t), std::forward<F>(f),
                         std::make_index_sequence<
                             std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
  }

  // Parse single tuple element
  template <typename T>
  void parse_tuple_element(std::string_view name, std::size_t index, std::size_t size,
                           T &&result) {
    auto [value, success] = parse_argument<typename std::remove_reference<T>::type>(name);
    if (success) {
      result = value;
    } else {
      if (next_index == arguments.size()) {
        // end of arguments list
        // failed to parse tuple <>. expected `size` arguments, `index` provided
        throw structopt::exception("Error: failed to correctly parse tuple `" +
                                       std::string{name} + "`. Expected " +
                                       std::to_string(size) + " arguments, " +
                                       std::to_string(index) + " provided.",
                                   visitor);
      } else {
        throw structopt::exception("Error: failed to correctly parse tuple `" +
                                       std::string{name} +
                                       "` {size = " + std::to_string(size) +
                                       "} at index " + std::to_string(index) + ".",
                                   visitor);
      }
    }
  }

  // Tuple argument
  template <typename Tuple> Tuple parse_tuple_argument(std::string_view name) {
    Tuple result;
    std::size_t i = 0;
    constexpr auto tuple_size = std::tuple_size<Tuple>::value;
    for_each(result, [&](auto &&arg) {
      parse_tuple_element(name, i, tuple_size, arg);
      i += 1;
    });
    return result;
  }

  // Vector, deque, list
  template <typename T> T parse_vector_like_argument(std::string_view name) {
    T result;

    // Parse from current till end
    while (next_index < arguments.size()) {
      const std::string_view next = arguments[next_index];
      if (is_optional_field(next) || next == "--" ||
          is_delimited_optional_argument(next).first) {
        if (next == "--") {
          double_dash_encountered = true;
          next_index += 1;
        }
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.push_back(value);
      }
    }
    return result;
  }

  // stack, queue, priority_queue
  template <typename T> T parse_container_adapter_argument(std::string_view name) {
    T result;
    // Parse from current till end
    while (next_index < arguments.size()) {
      const std::string_view next = arguments[next_index];
      if (is_optional_field(next) || next == "--" ||
          is_delimited_optional_argument(next).first) {
        if (next == "--") {
          double_dash_encountered = true;
          next_index += 1;
        }
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.push(value);
      }
    }
    return result;
  }

  // Set, multiset, unordered_set, unordered_multiset
  template <typename T> T parse_set_argument(std::string_view name) {
    T result;
    // Parse from current till end
    while (next_index < arguments.size()) {
      const std::string_view next = arguments[next_index];
      if (is_optional_field(next) || next == "--" ||
          is_delimited_optional_argument(next).first) {
        if (next == "--") {
          double_dash_encountered = true;
          next_index += 1;
        }
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.insert(value);
      }
    }
    return result;
  }

  // Enum class
  template <typename T> T parse_enum_argument(std::string_view name) {
    T result;
    auto maybe_enum_value = magic_enum::enum_cast<T>(arguments[next_index]);
    if (maybe_enum_value.has_value()) {
      result = maybe_enum_value.value();
    } else {
      constexpr auto allowed_names = magic_enum::enum_names<T>();

      std::string allowed_names_string = "";
      if (allowed_names.size()) {
        for (size_t i = 0; i < allowed_names.size() - 1; i++) {
          allowed_names_string += std::string{allowed_names[i]} + ", ";
        }
        allowed_names_string += allowed_names[allowed_names.size() - 1];
      }

      throw structopt::exception(
          "Error: unexpected input `" + std::string{arguments[next_index]} +
              "` provided for enum argument `" + std::string{name} +
              "`. Allowed values are {" + allowed_names_string + "}",
          visitor);
      // TODO: Throw error invalid enum option
    }
    return result;
  }

  // Visitor function for nested struct
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, void>::type
  operator()(std::string_view name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string_view{name};

      // Check if `next` is the start of a subcommand
      if (visitor.is_field_name(next) && next == field_name) {
        next_index += 1;
        value = parse_nested_struct<T>(name);
      }
    }
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value &&
                                     !visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(std::string_view name, T &result) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];

      if (is_optional(next)) {
        return;
      }

      if (visitor.positional_field_names.empty()) {
        // We're not looking to save any more positional fields
        // all of them already have a value
        throw structopt::exception("Error: unexpected argument '" + next + "'", visitor);
        return;
      }

      const auto field_name = visitor.positional_field_names.front();

      // // This will be parsed as a subcommand (nested struct)
      // if (visitor.is_field_name(next) && next == field_name) {
      //   return;
      // }

      if (field_name != std::string{name}) {
        // current field is not the one we want to parse
        return;
      }

      // Remove from the positional field list as it is about to be parsed
      visitor.positional_field_names.pop_front();

      auto [value, success] = parse_argument<T>(field_name);
      if (success) {
        result = value;
      } else {
        // positional field does not yet have a value
        visitor.positional_field_names.push_front(field_name);
      }
    }
  }

  // Visitor function for std::optional field
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(std::string_view name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      if (next == "--" && double_dash_encountered == false) {
        double_dash_encountered = true;
        next_index += 1;
        return;
      }

      // if `next` looks like an optional argument
      // i.e., starts with `-` or `--`
      // see if you can find an optional field in the struct with a matching name

      // check if the current argument looks like it could be this optional field
      if (double_dash_encountered == false && is_optional_field(next, field_name)) {

        // this is an optional argument matching the current struct field
        if constexpr (std::is_same<typename T::value_type, bool>::value) {
          // It is a boolean optional argument
          // Does it have a default value?
          // If yes, this is a FLAG argument, e.g,, "--verbose" will set it to true if the
          // default value is false No need to write "--verbose true"
          if (value.has_value()) {
            // The field already has a default value!
            value = !value.value(); // simply toggle it
            next_index += 1;
          } else {
            // boolean optional argument doesn't have a default value
            // expect one
            value = parse_optional_argument<typename T::value_type>(name);
          }
        } else {
          // Not std::optional<bool>
          // Parse the argument type <T>
          value = parse_optional_argument<typename T::value_type>(name);
        }
      } else {
        if (double_dash_encountered == false) {

          // maybe this is an optional argument that is delimited with '=' or ':'
          // e.g., --foo=bar or --foo:BAR
          if (next.size() > 1 && next[0] == '-') {
            const auto [success, delimiter] = is_delimited_optional_argument(next);
            if (success) {
              const auto [lhs, rhs] = split_delimited_argument(delimiter, next);
              // update next_index and return
              // the parser will take care of the rest

              // if `lhs` is an optional argument (i.e., maps to an optional field in the original struct), then insert into arguments list
              auto potential_field_name = get_full_optional_field_name(lhs);
              if (potential_field_name.has_value()) {
                for (auto &arg : {rhs, lhs}) {
                  const auto begin = arguments.begin();
                  arguments.insert(begin + next_index + 1, arg);
                }
              }
              // get past the current argument, e.g., `--foo=bar`
              next_index += 1;
              return;
            }
          }

          // A direct match of optional argument with field_name has not happened
          // This _could_ be a combined argument
          // e.g., -abc => -a, -b, and -c where each of these is a flag argument

          std::vector<std::string> potential_combined_argument;

          if (is_optional_field(next) == false && next[0] == '-' &&
              (next.size() > 1 && next[1] != '-')) {
            for (std::size_t i = 1; i < next.size(); i++) {
              potential_combined_argument.push_back("-" + std::string(1, next[i]));
            }
          }

          if (!potential_combined_argument.empty()) {
            bool is_combined_argument = true;
            for (auto &arg : potential_combined_argument) {
              if (!is_optional_field(arg)) {
                is_combined_argument = false;
                // TODO: report error unrecognized option in combined argument
              }
            }

            if (is_combined_argument) {

              // check and make sure the current field_name is
              // in `potential_combined_argument`
              //
              // Let's say the argument `next` is `-abc`
              // the current field name is `b`
              // 1. Split `-abc` into `-a`, `-b`, and `-c`
              // 2. Check if `-b` is in the above list
              //    1. If yes, consider this as a combined argument
              //       push the list of arguments (split up) into `arguments`
              //    2. If no, nothing to do here
              bool field_name_matched = false;
              for (auto &arg : potential_combined_argument) {
                if (arg == "-" + std::string(1, field_name[0])) {
                  field_name_matched = true;
                }
              }

              if (field_name_matched) {
                // confirmed: this is a combined argument

                // insert the individual options that make up the combined argument
                // right after the combined argument
                // e.g., ""./main -abc" becomes "./main -abc -a -b -c"
                // Once this is done, increment `next_index` so that the parser loop will
                // service
                // `-a`, `-b` and `-c` like any other optional arguments (flags and
                // otherwise)
                for (std::vector<std::string>::reverse_iterator it =
                         potential_combined_argument.rbegin();
                     it != potential_combined_argument.rend(); ++it) {
                  auto &arg = *it;
                  if (next_index < arguments.size()) {
                    auto begin = arguments.begin();
                    arguments.insert(begin + next_index + 1, arg);
                  } else {
                    arguments.push_back(arg);
                  }
                }

                // get past the current combined argument
                next_index += 1;
              }
            }
          }
        }
      }
    }
  }
};

// Specialization for std::string
template <> inline std::string parser::parse_single_argument<std::string>(std::string_view) {
  return arguments[next_index];
}

// Specialization for bool
// yes, YES, on, 1, true, TRUE, etc. = true
// no, NO, off, 0, false, FALSE, etc. = false
// Converts argument to lower case before check
template <> inline bool parser::parse_single_argument<bool>(std::string_view name) {
  if (next_index > current_index) {
    current_index = next_index;
  }

  if (current_index < arguments.size()) {
    const std::vector<std::string> true_strings{"on", "yes", "1", "true"};
    const std::vector<std::string> false_strings{"off", "no", "0", "false"};
    std::string current_argument = arguments[current_index];

    // Convert argument to lower case
    std::transform(current_argument.begin(), current_argument.end(),
                   current_argument.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // Detect if argument is true or false
    if (std::find(true_strings.begin(), true_strings.end(), current_argument) !=
        true_strings.end()) {
      return true;
    } else if (std::find(false_strings.begin(), false_strings.end(), current_argument) !=
               false_strings.end()) {
      return false;
    } else {
      throw structopt::exception("Error: failed to parse boolean argument `" +
                                     std::string{name} + "`." + " `" + current_argument +
                                     "`" + " is invalid.",
                                 visitor);
      return false;
    }
  } else {
    return false;
  }
}

} // namespace details

} // namespace structopt

#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
// #include <structopt/is_stl_container.hpp>
// #include <structopt/parser.hpp>
// #include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

#define STRUCTOPT VISITABLE_STRUCT

namespace structopt {

class app {
  details::visitor visitor;

public:
  explicit app(std::string name, std::string version = "", std::string help = "")
      : visitor(std::move(name), std::move(version), std::move(help)) {}

  template <typename T> T parse(const std::vector<std::string> &arguments) {
    T argument_struct = T();

    // Visit the struct and save flag, optional and positional field names
    visit_struct::for_each(argument_struct, visitor);

    // add `help` and `version` optional arguments
    visitor.optional_field_names.push_back("help");
    visitor.optional_field_names.push_back("version");

    // Construct the argument parser
    structopt::details::parser parser;
    parser.visitor = visitor;
    parser.arguments = arguments;

    for (std::size_t i = 1; i < parser.arguments.size(); i++) {
      parser.current_index = i;
      visit_struct::for_each(argument_struct, parser);
    }

    // directly call the parser to check for `help` and `version` flags
    std::optional<bool> help = false, version = false;
    for (std::size_t i = 1; i < parser.arguments.size(); i++) {
      parser.operator()("help", help);
      parser.operator()("version", version);

      if (help == true) {
        // if help is requested, print help and exit
        visitor.print_help(std::cout);
        exit(EXIT_SUCCESS);
      } else if (version == true) {
        // if version is requested, print version and exit
        std::cout << visitor.version << "\n";
        exit(EXIT_SUCCESS);
      }
    }

    // if all positional arguments were provided
    // this list would be empty
    if (!parser.visitor.positional_field_names.empty()) {
      for (auto &field_name : parser.visitor.positional_field_names) {
        if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                      parser.visitor.vector_like_positional_field_names.end(),
                      field_name) ==
            parser.visitor.vector_like_positional_field_names.end()) {
          // this positional argument is not a vector-like argument
          // it expects value(s)
          throw structopt::exception("Error: expected value for positional argument `" +
                                         std::string(field_name) + "`.",
                                     parser.visitor);
        }
      }
    }

    if (parser.current_index < parser.arguments.size()) {
      throw structopt::exception("Error: unrecognized argument '" + parser.arguments[parser.current_index] + "'", parser.visitor);
    }

    return argument_struct;
  }

  template <typename T> T parse(int argc, char *argv[]) {
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    return parse<T>(arguments);
  }

  std::string help() const {
    std::stringstream os;
    visitor.print_help(os);
    return os.str();
  }
};

} // namespace structopt
