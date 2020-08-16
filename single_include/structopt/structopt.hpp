
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

#include <type_traits>
#include <utility>

// Library version

#define VISIT_STRUCT_VERSION_MAJOR 1
#define VISIT_STRUCT_VERSION_MINOR 0
#define VISIT_STRUCT_VERSION_PATCH 0

#define VISIT_STRUCT_STRING_HELPER(X) #X
#define VISIT_STRUCT_STRING(X) VISIT_STRUCT_STRING_HELPER(X)

#define VISIT_STRUCT_VERSION_STRING                                                      \
  VISIT_STRUCT_STRING(VISIT_STRUCT_VERSION_MAJOR)                                        \
  "." VISIT_STRUCT_STRING(VISIT_STRUCT_VERSION_MINOR) "." VISIT_STRUCT_STRING(           \
      VISIT_STRUCT_VERSION_PATCH)

// For MSVC 2013 support, we put constexpr behind a define.

#ifndef VISIT_STRUCT_CONSTEXPR
#if (defined _MSC_VER) && (_MSC_VER <= 1800)
#define VISIT_STRUCT_CONSTEXPR
#else
#define VISIT_STRUCT_CONSTEXPR constexpr
#endif
#endif

// After C++14 the apply_visitor function can be constexpr.
// We target C++11, but such functions are tagged VISIT_STRUCT_CXX14_CONSTEXPR.

#ifndef VISIT_STRUCT_CXX14_CONSTEXPR
#if ((defined _MSC_VER) && (_MSC_VER <= 1900)) || (!defined __cplusplus) ||              \
    (__cplusplus == 201103L)
#define VISIT_STRUCT_CXX14_CONSTEXPR
#else
#define VISIT_STRUCT_CXX14_CONSTEXPR constexpr
#endif
#endif

namespace visit_struct {

namespace traits {

// Primary template which is specialized to register a type
template <typename T, typename ENABLE = void> struct visitable;

// Helper template which checks if a type is registered
template <typename T, typename ENABLE = void> struct is_visitable : std::false_type {};

template <typename T>
struct is_visitable<T, typename std::enable_if<traits::visitable<T>::value>::type>
    : std::true_type {};

// Helper template which removes cv and reference from a type (saves some typing)
template <typename T> struct clean {
  typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

template <typename T> using clean_t = typename clean<T>::type;

// Mini-version of std::common_type (we only require C++11)
template <typename T, typename U> struct common_type {
  typedef decltype(true ? std::declval<T>() : std::declval<U>()) type;
};

} // end namespace traits

// Tag for tag dispatch
template <typename T> struct type_c { using type = T; };

// Accessor type: function object encapsulating a pointer-to-member
template <typename MemPtr, MemPtr ptr> struct accessor {
  template <typename T>
  VISIT_STRUCT_CONSTEXPR auto operator()(T &&t) const
      -> decltype(std::forward<T>(t).*ptr) {
    return std::forward<T>(t).*ptr;
  }
};

//
// User-interface
//

// Return number of fields in a visitable struct
template <typename S> VISIT_STRUCT_CONSTEXPR std::size_t field_count() {
  return traits::visitable<traits::clean_t<S>>::field_count;
}

template <typename S> VISIT_STRUCT_CONSTEXPR std::size_t field_count(S &&) {
  return field_count<S>();
}

// apply_visitor (one struct instance)
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V &&v, S &&s) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// apply_visitor (two struct instances)
template <typename S1, typename S2, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V &&v, S1 &&s1, S2 &&s2) ->
    typename std::enable_if<traits::is_visitable<
        traits::clean_t<typename traits::common_type<S1, S2>::type>>::value>::type {
  using common_S = typename traits::common_type<S1, S2>::type;
  traits::visitable<traits::clean_t<common_S>>::apply(
      std::forward<V>(v), std::forward<S1>(s1), std::forward<S2>(s2));
}

// for_each (Alternate syntax for apply_visitor, reverses order of arguments)
template <typename V, typename S>
VISIT_STRUCT_CXX14_CONSTEXPR auto for_each(S &&s, V &&v) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// for_each with two structure instances
template <typename S1, typename S2, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto for_each(S1 &&s1, S2 &&s2, V &&v) ->
    typename std::enable_if<traits::is_visitable<
        traits::clean_t<typename traits::common_type<S1, S2>::type>>::value>::type {
  using common_S = typename traits::common_type<S1, S2>::type;
  traits::visitable<traits::clean_t<common_S>>::apply(
      std::forward<V>(v), std::forward<S1>(s1), std::forward<S2>(s2));
}

// Visit the types (visit_struct::type_c<...>) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_types(V &&v) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  traits::visitable<traits::clean_t<S>>::visit_types(std::forward<V>(v));
}

// Visit the member pointers (&S::a) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_pointers(V &&v) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  traits::visitable<traits::clean_t<S>>::visit_pointers(std::forward<V>(v));
}

// Visit the accessors (function objects) of the registered members
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto visit_accessors(V &&v) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  traits::visitable<traits::clean_t<S>>::visit_accessors(std::forward<V>(v));
}

// Apply visitor (with no instances)
// This calls visit_pointers, for backwards compat reasons
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V &&v) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
  visit_struct::visit_pointers<S>(std::forward<V>(v));
}

// Get value by index (like std::get for tuples)
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get(S &&s) ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value,
                            decltype(traits::visitable<traits::clean_t<S>>::get_value(
                                std::integral_constant<int, idx>{},
                                std::forward<S>(s)))>::type {
  return traits::visitable<traits::clean_t<S>>::get_value(
      std::integral_constant<int, idx>{}, std::forward<S>(s));
}

// Get name of field, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_name() ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value,
                            decltype(traits::visitable<traits::clean_t<S>>::get_name(
                                std::integral_constant<int, idx>{}))>::type {
  return traits::visitable<traits::clean_t<S>>::get_name(
      std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_name(S &&) -> decltype(get_name<idx, S>()) {
  return get_name<idx, S>();
}

// Get member pointer, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_pointer() ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value,
                            decltype(traits::visitable<traits::clean_t<S>>::get_pointer(
                                std::integral_constant<int, idx>{}))>::type {
  return traits::visitable<traits::clean_t<S>>::get_pointer(
      std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_pointer(S &&) -> decltype(get_pointer<idx, S>()) {
  return get_pointer<idx, S>();
}

// Get member accessor, by index
template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_accessor() ->
    typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value,
                            decltype(traits::visitable<traits::clean_t<S>>::get_accessor(
                                std::integral_constant<int, idx>{}))>::type {
  return traits::visitable<traits::clean_t<S>>::get_accessor(
      std::integral_constant<int, idx>{});
}

template <int idx, typename S>
VISIT_STRUCT_CONSTEXPR auto get_accessor(S &&) -> decltype(get_accessor<idx, S>()) {
  return get_accessor<idx, S>();
}

// Get type, by index
template <int idx, typename S> struct type_at_s {
  using type_c = decltype(
      traits::visitable<traits::clean_t<S>>::type_at(std::integral_constant<int, idx>{}));
  using type = typename type_c::type;
};

template <int idx, typename S> using type_at = typename type_at_s<idx, S>::type;

// Get name of structure
template <typename S>
VISIT_STRUCT_CONSTEXPR auto get_name() -> typename std::enable_if<
    traits::is_visitable<traits::clean_t<S>>::value,
    decltype(traits::visitable<traits::clean_t<S>>::get_name())>::type {
  return traits::visitable<traits::clean_t<S>>::get_name();
}

template <typename S>
VISIT_STRUCT_CONSTEXPR auto get_name(S &&) -> decltype(get_name<S>()) {
  return get_name<S>();
}

/***
 * To implement the VISITABLE_STRUCT macro, we need a map-macro, which can take
 * the name of a macro and some other arguments, and apply that macro to each other
 * argument.
 *
 * There are some techniques you can use within C preprocessor to accomplish this
 * succinctly, by settng up "recursive" macros.
 *
 * But this can also cause it to give worse error messages when something goes wrong.
 *
 * We are now doing it in a more "dumb", bulletproof way which has the advantage that it
 * is more portable and gives better error messages. For discussion see
 * IMPLEMENTATION_NOTES.md
 *
 * The code below is based on a patch from Jarod42, and is now generated by a python
 * script. The purpose of the generated code is to define VISIT_STRUCT_PP_MAP as
 * described.
 */

/*** Generated code ***/

static VISIT_STRUCT_CONSTEXPR const int max_visitable_members = 69;

#define VISIT_STRUCT_EXPAND(x) x
#define VISIT_STRUCT_PP_ARG_N(                                                           \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,     \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, \
    N, ...)                                                                              \
  N
#define VISIT_STRUCT_PP_NARG(...)                                                        \
  VISIT_STRUCT_EXPAND(VISIT_STRUCT_PP_ARG_N(                                             \
      __VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53,   \
      52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,    \
      32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,    \
      12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* need extra level to force extra eval */
#define VISIT_STRUCT_CONCAT_(a, b) a##b
#define VISIT_STRUCT_CONCAT(a, b) VISIT_STRUCT_CONCAT_(a, b)

#define VISIT_STRUCT_APPLYF0(f)
#define VISIT_STRUCT_APPLYF1(f, _1) f(_1)
#define VISIT_STRUCT_APPLYF2(f, _1, _2) f(_1) f(_2)
#define VISIT_STRUCT_APPLYF3(f, _1, _2, _3) f(_1) f(_2) f(_3)
#define VISIT_STRUCT_APPLYF4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4)
#define VISIT_STRUCT_APPLYF5(f, _1, _2, _3, _4, _5) f(_1) f(_2) f(_3) f(_4) f(_5)
#define VISIT_STRUCT_APPLYF6(f, _1, _2, _3, _4, _5, _6)                                  \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6)
#define VISIT_STRUCT_APPLYF7(f, _1, _2, _3, _4, _5, _6, _7)                              \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7)
#define VISIT_STRUCT_APPLYF8(f, _1, _2, _3, _4, _5, _6, _7, _8)                          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8)
#define VISIT_STRUCT_APPLYF9(f, _1, _2, _3, _4, _5, _6, _7, _8, _9)                      \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9)
#define VISIT_STRUCT_APPLYF10(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)                \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10)
#define VISIT_STRUCT_APPLYF11(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)           \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)
#define VISIT_STRUCT_APPLYF12(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)      \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12)
#define VISIT_STRUCT_APPLYF13(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)
#define VISIT_STRUCT_APPLYF14(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14)                                                       \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14)
#define VISIT_STRUCT_APPLYF15(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15)                                                  \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15)
#define VISIT_STRUCT_APPLYF16(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16)                                             \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16)
#define VISIT_STRUCT_APPLYF17(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17)                                        \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17)
#define VISIT_STRUCT_APPLYF18(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18)                                   \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18)
#define VISIT_STRUCT_APPLYF19(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19)                              \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19)
#define VISIT_STRUCT_APPLYF20(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20)                         \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20)
#define VISIT_STRUCT_APPLYF21(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21)                    \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21)
#define VISIT_STRUCT_APPLYF22(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22)               \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22)
#define VISIT_STRUCT_APPLYF23(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23)          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23)
#define VISIT_STRUCT_APPLYF24(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24)     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)
#define VISIT_STRUCT_APPLYF25(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25)                                                       \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25)
#define VISIT_STRUCT_APPLYF26(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26)                                                  \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26)
#define VISIT_STRUCT_APPLYF27(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27)                                             \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27)
#define VISIT_STRUCT_APPLYF28(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28)                                        \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28)
#define VISIT_STRUCT_APPLYF29(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29)                                   \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29)
#define VISIT_STRUCT_APPLYF30(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30)                              \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30)
#define VISIT_STRUCT_APPLYF31(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31)                         \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31)
#define VISIT_STRUCT_APPLYF32(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32)                    \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32)
#define VISIT_STRUCT_APPLYF33(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33)               \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33)
#define VISIT_STRUCT_APPLYF34(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34)
#define VISIT_STRUCT_APPLYF35(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)
#define VISIT_STRUCT_APPLYF36(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36)                                                       \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36)
#define VISIT_STRUCT_APPLYF37(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37)                                                  \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37)
#define VISIT_STRUCT_APPLYF38(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38)                                             \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38)
#define VISIT_STRUCT_APPLYF39(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39)                                        \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39)
#define VISIT_STRUCT_APPLYF40(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40)                                   \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40)
#define VISIT_STRUCT_APPLYF41(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41)                              \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41)
#define VISIT_STRUCT_APPLYF42(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42)                         \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42)
#define VISIT_STRUCT_APPLYF43(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43)                    \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43)
#define VISIT_STRUCT_APPLYF44(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44)               \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44)
#define VISIT_STRUCT_APPLYF45(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45)          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)
#define VISIT_STRUCT_APPLYF46(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46)     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46)
#define VISIT_STRUCT_APPLYF47(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)                          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47)
#define VISIT_STRUCT_APPLYF48(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)                     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48)
#define VISIT_STRUCT_APPLYF49(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)                \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49)
#define VISIT_STRUCT_APPLYF50(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)           \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50)
#define VISIT_STRUCT_APPLYF51(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51)      \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51)
#define VISIT_STRUCT_APPLYF52(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52)
#define VISIT_STRUCT_APPLYF53(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,     \
                              _47, _48, _49, _50, _51, _52, _53)                         \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53)
#define VISIT_STRUCT_APPLYF54(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,     \
                              _47, _48, _49, _50, _51, _52, _53, _54)                    \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54)
#define VISIT_STRUCT_APPLYF55(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,     \
                              _47, _48, _49, _50, _51, _52, _53, _54, _55)               \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)
#define VISIT_STRUCT_APPLYF56(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,     \
                              _47, _48, _49, _50, _51, _52, _53, _54, _55, _56)          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56)
#define VISIT_STRUCT_APPLYF57(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                              _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24,     \
                              _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,     \
                              _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,     \
                              _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57)     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57)
#define VISIT_STRUCT_APPLYF58(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58)                                                        \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58)
#define VISIT_STRUCT_APPLYF59(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59)                                                   \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59)
#define VISIT_STRUCT_APPLYF60(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60)                                              \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60)
#define VISIT_STRUCT_APPLYF61(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61)                                         \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61)
#define VISIT_STRUCT_APPLYF62(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62)                                    \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62)
#define VISIT_STRUCT_APPLYF63(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63)                               \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63)
#define VISIT_STRUCT_APPLYF64(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64)                          \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)
#define VISIT_STRUCT_APPLYF65(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65)                     \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)     \
                          f(_65)
#define VISIT_STRUCT_APPLYF66(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66)                \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)     \
                          f(_65) f(_66)
#define VISIT_STRUCT_APPLYF67(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67)           \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)     \
                          f(_65) f(_66) f(_67)
#define VISIT_STRUCT_APPLYF68(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68)      \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)     \
                          f(_65) f(_66) f(_67) f(_68)
#define VISIT_STRUCT_APPLYF69(                                                           \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,  \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, \
    _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, \
    _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69) \
  f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)      \
      f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)       \
          f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)   \
              f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)      \
                  f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)  \
                      f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)     \
                          f(_65) f(_66) f(_67) f(_68) f(_69)

#define VISIT_STRUCT_APPLY_F_(M, ...) VISIT_STRUCT_EXPAND(M(__VA_ARGS__))
#define VISIT_STRUCT_PP_MAP(f, ...)                                                      \
  VISIT_STRUCT_EXPAND(VISIT_STRUCT_APPLY_F_(                                             \
      VISIT_STRUCT_CONCAT(VISIT_STRUCT_APPLYF, VISIT_STRUCT_PP_NARG(__VA_ARGS__)), f,    \
      __VA_ARGS__))

/*** End generated code ***/

/***
 * These macros are used with VISIT_STRUCT_PP_MAP
 */

#define VISIT_STRUCT_FIELD_COUNT(MEMBER_NAME) +1

#define VISIT_STRUCT_MEMBER_HELPER(MEMBER_NAME)                                          \
  std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S>(struct_instance).MEMBER_NAME);

#define VISIT_STRUCT_MEMBER_HELPER_PTR(MEMBER_NAME)                                      \
  std::forward<V>(visitor)(#MEMBER_NAME, &this_type::MEMBER_NAME);

#define VISIT_STRUCT_MEMBER_HELPER_TYPE(MEMBER_NAME)                                     \
  std::forward<V>(visitor)(#MEMBER_NAME,                                                 \
                           visit_struct::type_c<decltype(this_type::MEMBER_NAME)>{});

#define VISIT_STRUCT_MEMBER_HELPER_ACC(MEMBER_NAME)                                      \
  std::forward<V>(visitor)(#MEMBER_NAME,                                                 \
                           visit_struct::accessor<decltype(&this_type::MEMBER_NAME),     \
                                                  &this_type::MEMBER_NAME>{});

#define VISIT_STRUCT_MEMBER_HELPER_PAIR(MEMBER_NAME)                                     \
  std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S1>(s1).MEMBER_NAME,               \
                           std::forward<S2>(s2).MEMBER_NAME);

#define VISIT_STRUCT_MAKE_GETTERS(MEMBER_NAME)                                           \
  template <typename S>                                                                  \
  static VISIT_STRUCT_CONSTEXPR auto get_value(                                          \
      std::integral_constant<int, fields_enum::MEMBER_NAME>, S &&s)                      \
      ->decltype((std::forward<S>(s).MEMBER_NAME)) {                                     \
    return std::forward<S>(s).MEMBER_NAME;                                               \
  }                                                                                      \
                                                                                         \
  static VISIT_STRUCT_CONSTEXPR auto get_name(                                           \
      std::integral_constant<int, fields_enum::MEMBER_NAME>)                             \
      ->decltype(#MEMBER_NAME) {                                                         \
    return #MEMBER_NAME;                                                                 \
  }                                                                                      \
                                                                                         \
  static VISIT_STRUCT_CONSTEXPR auto get_pointer(                                        \
      std::integral_constant<int, fields_enum::MEMBER_NAME>)                             \
      ->decltype(&this_type::MEMBER_NAME) {                                              \
    return &this_type::MEMBER_NAME;                                                      \
  }                                                                                      \
                                                                                         \
  static VISIT_STRUCT_CONSTEXPR auto get_accessor(                                       \
      std::integral_constant<int, fields_enum::MEMBER_NAME>)                             \
      ->visit_struct::accessor<decltype(&this_type::MEMBER_NAME),                        \
                               &this_type::MEMBER_NAME> {                                \
    return {};                                                                           \
  }                                                                                      \
                                                                                         \
  static auto type_at(std::integral_constant<int, fields_enum::MEMBER_NAME>)             \
      ->visit_struct::type_c<decltype(this_type::MEMBER_NAME)>;

// This macro specializes the trait, provides "apply" method which does the work.
// Below, template parameter S should always be the same as STRUCT_NAME modulo const and
// reference. The interface defined above ensures that STRUCT_NAME is clean_t<S>
// basically.
//
// Note: The code to make the indexed getters work is more convoluted than I'd like.
//       PP_MAP doesn't give you the index of each member. And rather than hack it so that
//       it will do that, what we do instead is: 1: Declare an enum `field_enum` in the
//       scope of visitable, which maps names to indices.
//          This gives an easy way for the macro to get the index from the name token.
//       2: Intuitively we'd like to use template partial specialization to make indices
//       map to
//          values, and have a new specialization for each member. But, specializations
//          can only be made at namespace scope. So to keep things tidy and contained
//          within this trait, we use tag dispatch with std::integral_constant<int>
//          instead.

#define VISITABLE_STRUCT(STRUCT_NAME, ...)                                               \
  namespace visit_struct {                                                               \
  namespace traits {                                                                     \
                                                                                         \
  template <> struct visitable<STRUCT_NAME, void> {                                      \
                                                                                         \
    using this_type = STRUCT_NAME;                                                       \
                                                                                         \
    static VISIT_STRUCT_CONSTEXPR auto get_name() -> decltype(#STRUCT_NAME) {            \
      return #STRUCT_NAME;                                                               \
    }                                                                                    \
                                                                                         \
    static VISIT_STRUCT_CONSTEXPR const std::size_t field_count =                        \
        0 VISIT_STRUCT_PP_MAP(VISIT_STRUCT_FIELD_COUNT, __VA_ARGS__);                    \
                                                                                         \
    template <typename V, typename S>                                                    \
    VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V &&visitor, S &&struct_instance) {   \
      VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER, __VA_ARGS__)                       \
    }                                                                                    \
                                                                                         \
    template <typename V, typename S1, typename S2>                                      \
    VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V &&visitor, S1 &&s1, S2 &&s2) {      \
      VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_PAIR, __VA_ARGS__)                  \
    }                                                                                    \
                                                                                         \
    template <typename V>                                                                \
    VISIT_STRUCT_CXX14_CONSTEXPR static void visit_pointers(V &&visitor) {               \
      VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_PTR, __VA_ARGS__)                   \
    }                                                                                    \
                                                                                         \
    template <typename V>                                                                \
    VISIT_STRUCT_CXX14_CONSTEXPR static void visit_types(V &&visitor) {                  \
      VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_TYPE, __VA_ARGS__)                  \
    }                                                                                    \
                                                                                         \
    template <typename V>                                                                \
    VISIT_STRUCT_CXX14_CONSTEXPR static void visit_accessors(V &&visitor) {              \
      VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_ACC, __VA_ARGS__)                   \
    }                                                                                    \
                                                                                         \
    struct fields_enum {                                                                 \
      enum index { __VA_ARGS__ };                                                        \
    };                                                                                   \
                                                                                         \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MAKE_GETTERS, __VA_ARGS__)                          \
                                                                                         \
    static VISIT_STRUCT_CONSTEXPR const bool value = true;                               \
  };                                                                                     \
  }                                                                                      \
  }                                                                                      \
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
//               |___/  version 0.6.6
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019 - 2020 Daniil Goncharov <neargye@gmail.com>.
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

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                                         \
    "-Wsign-conversion" // Implicit conversion changes signedness: 'int' to 'size_t'.
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored                                                           \
    "-Wsign-conversion" // Implicit conversion changes signedness: 'int' to 'size_t'.
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26495) // Variable 'static_string<N>::chars' is uninitialized.
#endif

// Checks magic_enum compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 ||  \
    defined(_MSC_VER) && _MSC_VER >= 1910
#undef MAGIC_ENUM_SUPPORTED
#define MAGIC_ENUM_SUPPORTED 1
#endif

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default
// MAGIC_ENUM_RANGE_MIN = -128. If need another min range for all enum types by default,
// redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
#define MAGIC_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default
// MAGIC_ENUM_RANGE_MAX = 128. If need another max range for all enum types by default,
// redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
#define MAGIC_ENUM_RANGE_MAX 128
#endif

namespace magic_enum {

// Enum value must be in range [MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX]. By default
// MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 128. If need another range for all
// enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN and
// MAGIC_ENUM_RANGE_MAX. If need another range for specific enum type, add specialization
// enum_range for necessary enum type.
template <typename E> struct enum_range {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_range requires enum type.");
  inline static constexpr int min = MAGIC_ENUM_RANGE_MIN;
  inline static constexpr int max = MAGIC_ENUM_RANGE_MAX;
  static_assert(max > min, "magic_enum::enum_range requires max > min.");
};

static_assert(MAGIC_ENUM_RANGE_MIN <= 0,
              "MAGIC_ENUM_RANGE_MIN must be less or equals than 0.");
static_assert(MAGIC_ENUM_RANGE_MIN > (std::numeric_limits<std::int16_t>::min)(),
              "MAGIC_ENUM_RANGE_MIN must be greater than INT16_MIN.");

static_assert(MAGIC_ENUM_RANGE_MAX > 0, "MAGIC_ENUM_RANGE_MAX must be greater than 0.");
static_assert(MAGIC_ENUM_RANGE_MAX < (std::numeric_limits<std::int16_t>::max)(),
              "MAGIC_ENUM_RANGE_MAX must be less than INT16_MAX.");

static_assert(MAGIC_ENUM_RANGE_MAX > MAGIC_ENUM_RANGE_MIN,
              "MAGIC_ENUM_RANGE_MAX must be greater than MAGIC_ENUM_RANGE_MIN.");

namespace detail {

template <typename T>
struct supported
#if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED ||                             \
    defined(MAGIC_ENUM_NO_CHECK_SUPPORT)
    : std::true_type {
};
#else
    : std::false_type {
};
#endif

template <typename T>
inline constexpr bool is_enum_v = std::is_enum_v<T> &&std::is_same_v<T, std::decay_t<T>>;

template <std::size_t N> struct static_string {
  constexpr explicit static_string(std::string_view str) noexcept
      : static_string{str, std::make_index_sequence<N>{}} {
    assert(str.size() == N);
  }

  constexpr const char *data() const noexcept { return chars.data(); }

  constexpr std::size_t size() const noexcept { return N; }

  constexpr operator std::string_view() const noexcept { return {data(), size()}; }

private:
  template <std::size_t... I>
  constexpr static_string(std::string_view str, std::index_sequence<I...>) noexcept
      : chars{{str[I]..., '\0'}} {}

  const std::array<char, N + 1> chars;
};

template <> struct static_string<0> {
  constexpr explicit static_string(std::string_view) noexcept {}

  constexpr const char *data() const noexcept { return nullptr; }

  constexpr std::size_t size() const noexcept { return 0; }

  constexpr operator std::string_view() const noexcept { return {}; }
};

constexpr std::string_view pretty_name(std::string_view name) noexcept {
  for (std::size_t i = name.size(); i > 0; --i) {
    if (!((name[i - 1] >= '0' && name[i - 1] <= '9') ||
          (name[i - 1] >= 'a' && name[i - 1] <= 'z') ||
          (name[i - 1] >= 'A' && name[i - 1] <= 'Z') || (name[i - 1] == '_'))) {
      name.remove_prefix(i);
      break;
    }
  }

  if (name.size() > 0 &&
      ((name.front() >= 'a' && name.front() <= 'z') ||
       (name.front() >= 'A' && name.front() <= 'Z') || (name.front() == '_'))) {
    return name;
  }

  return {}; // Invalid name.
}

template <typename BinaryPredicate>
constexpr bool
cmp_equal(std::string_view lhs, std::string_view rhs, BinaryPredicate &&p) noexcept(
    std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char, char>) {
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
}

template <typename L, typename R> constexpr bool cmp_less(L lhs, R rhs) noexcept {
  static_assert(std::is_integral_v<L> && std::is_integral_v<R>,
                "magic_enum::detail::cmp_less requires integral type.");

  if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
    // If same signedness (both signed or both unsigned).
    return lhs < rhs;
  } else if constexpr (std::is_signed_v<R>) {
    // If 'right' is negative, then result is 'false', otherwise cast & compare.
    return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
  } else {
    // If 'left' is negative, then result is 'true', otherwise cast & compare.
    return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
  }
}

template <typename E> constexpr auto n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");
#if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED
#if defined(__clang__)
  constexpr std::string_view name{__PRETTY_FUNCTION__ + 34,
                                  sizeof(__PRETTY_FUNCTION__) - 36};
#elif defined(__GNUC__)
  constexpr std::string_view name{__PRETTY_FUNCTION__ + 49,
                                  sizeof(__PRETTY_FUNCTION__) - 51};
#elif defined(_MSC_VER)
  constexpr std::string_view name{__FUNCSIG__ + 40, sizeof(__FUNCSIG__) - 57};
#endif
  return static_string<name.size()>{name};
#else
  return std::string_view{}; // Unsupported compiler.
#endif
}

template <typename E> inline constexpr auto type_name_v = n<E>();

template <typename E, E V> constexpr auto n() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::n requires enum type.");
#if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED
#if defined(__clang__) || defined(__GNUC__)
  constexpr auto name =
      pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
#elif defined(_MSC_VER)
  constexpr auto name = pretty_name({__FUNCSIG__, sizeof(__FUNCSIG__) - 17});
#endif
  return static_string<name.size()>{name};
#else
  return std::string_view{}; // Unsupported compiler.
#endif
}

template <typename E, E V> inline constexpr auto name_v = n<E, V>();

template <typename E, auto V> constexpr bool is_valid() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::is_valid requires enum type.");

  return n<E, static_cast<E>(V)>().size() != 0;
}

template <typename E, int Min, int Max> constexpr std::size_t range_size() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::range_size requires enum type.");
  constexpr auto size = Max - Min + 1;
  static_assert(size > 0, "magic_enum::enum_range requires valid size.");
  static_assert(size < (std::numeric_limits<std::uint16_t>::max)(),
                "magic_enum::enum_range requires valid size.");

  return static_cast<std::size_t>(size);
}

template <typename E> constexpr int reflected_min() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::reflected_min requires enum type.");
  constexpr auto lhs = enum_range<E>::min;
  static_assert(lhs > (std::numeric_limits<std::int16_t>::min)(),
                "magic_enum::enum_range requires min must be greater than INT16_MIN.");
  constexpr auto rhs = (std::numeric_limits<std::underlying_type_t<E>>::min)();

  return cmp_less(lhs, rhs) ? rhs : lhs;
}

template <typename E> constexpr int reflected_max() noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::reflected_max requires enum type.");
  constexpr auto lhs = enum_range<E>::max;
  static_assert(lhs < (std::numeric_limits<std::int16_t>::max)(),
                "magic_enum::enum_range requires max must be less than INT16_MAX.");
  constexpr auto rhs = (std::numeric_limits<std::underlying_type_t<E>>::max)();

  return cmp_less(lhs, rhs) ? lhs : rhs;
}

template <typename E> inline constexpr int reflected_min_v = reflected_min<E>();

template <typename E> inline constexpr int reflected_max_v = reflected_max<E>();

template <typename E, int... I>
constexpr auto values(std::integer_sequence<int, I...>) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::values requires enum type.");
  constexpr std::array<bool, sizeof...(I)> valid{
      {is_valid<E, I + reflected_min_v<E>>()...}};
  constexpr int count = ((valid[I] ? 1 : 0) + ...);

  std::array<E, count> values{};
  for (int i = 0, v = 0; v < count; ++i) {
    if (valid[i]) {
      values[v++] = static_cast<E>(i + reflected_min_v<E>);
    }
  }

  return values;
}

template <typename E>
inline constexpr auto
    values_v = values<E>(std::make_integer_sequence<
                         int, range_size<E, reflected_min_v<E>, reflected_max_v<E>>()>{});

template <typename E> inline constexpr std::size_t count_v = values_v<E>.size();

template <typename E> inline constexpr int min_v = static_cast<int>(values_v<E>.front());

template <typename E> inline constexpr int max_v = static_cast<int>(values_v<E>.back());

template <typename E>
inline constexpr std::size_t range_size_v = range_size<E, min_v<E>, max_v<E>>();

template <typename E>
using index_t = std::conditional_t < range_size_v<E>
                <(std::numeric_limits<std::uint8_t>::max)(), std::uint8_t, std::uint16_t>;

template <typename E>
inline constexpr auto invalid_index_v = (std::numeric_limits<index_t<E>>::max)();

template <typename E, int... I>
constexpr auto indexes(std::integer_sequence<int, I...>) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::indexes requires enum type.");
  [[maybe_unused]] index_t<E> i = 0;

  return std::array<index_t<E>, sizeof...(I)>{
      {(is_valid<E, I + min_v<E>>() ? i++ : invalid_index_v<E>)...}};
}

template <typename E>
inline constexpr auto
    indexes_v = indexes<E>(std::make_integer_sequence<int, range_size_v<E>>{});

template <typename E, std::size_t... I>
constexpr auto names(std::index_sequence<I...>) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::names requires enum type.");

  return std::array<std::string_view, sizeof...(I)>{{name_v<E, values_v<E>[I]>...}};
}

template <typename E>
inline constexpr auto names_v = names<E>(std::make_index_sequence<count_v<E>>{});

template <typename E, std::size_t... I>
constexpr auto entries(std::index_sequence<I...>) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::entries requires enum type.");

  return std::array<std::pair<E, std::string_view>, sizeof...(I)>{
      {{values_v<E>[I], name_v<E, values_v<E>[I]>}...}};
}

template <typename E>
inline constexpr auto entries_v = entries<E>(std::make_index_sequence<count_v<E>>{});

template <typename E> inline constexpr bool is_sparse_v = range_size_v<E> != count_v<E>;

template <typename E, typename U = std::underlying_type_t<E>>
constexpr int undex(U value) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::undex requires enum type.");

  if (const auto i = static_cast<int>(value) - min_v<E>;
      value >= static_cast<U>(min_v<E>) && value <= static_cast<U>(max_v<E>)) {
    if constexpr (is_sparse_v<E>) {
      if (const auto idx = indexes_v<E>[i]; idx != invalid_index_v<E>) {
        return idx;
      }
    } else {
      return i;
    }
  }

  return -1; // Value out of range.
}

template <typename E> constexpr int endex(E value) noexcept {
  static_assert(is_enum_v<E>, "magic_enum::detail::endex requires enum type.");

  return undex<E>(static_cast<std::underlying_type_t<E>>(value));
}

template <typename T, typename R>
using enable_if_enum_t = std::enable_if_t<std::is_enum_v<std::decay_t<T>>, R>;

template <typename T, typename Enable = std::enable_if_t<std::is_enum_v<std::decay_t<T>>>>
using enum_concept = T;

template <typename T, bool = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

template <typename T>
struct is_scoped_enum<T, true>
    : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<T>>
struct is_unscoped_enum : std::false_type {};

template <typename T>
struct is_unscoped_enum<T, true>
    : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T, bool = std::is_enum_v<std::decay_t<T>>> struct underlying_type {};

template <typename T>
struct underlying_type<T, true> : std::underlying_type<std::decay_t<T>> {};

} // namespace detail

// Checks is magic_enum supported compiler.
inline constexpr bool is_magic_enum_supported = detail::supported<void>::value;

template <typename T> using Enum = detail::enum_concept<T>;

// Checks whether T is an Unscoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Unscoped
// enumeration](https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumeration)
// type. Otherwise, value is equal to false.
template <typename T> struct is_unscoped_enum : detail::is_unscoped_enum<T> {};

template <typename T>
inline constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

// Checks whether T is an Scoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Scoped
// enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type.
// Otherwise, value is equal to false.
template <typename T> struct is_scoped_enum : detail::is_scoped_enum<T> {};

template <typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// If T is a complete enumeration type, provides a member typedef type that names the
// underlying type of T. Otherwise, if T is not an enumeration type, there is no member
// type. Otherwise (T is an incomplete enumeration type), the program is ill-formed.
template <typename T> struct underlying_type : detail::underlying_type<T> {};

template <typename T> using underlying_type_t = typename underlying_type<T>::type;

// Returns string name of enum type.
template <typename E>
[[nodiscard]] constexpr auto enum_type_name() noexcept
    -> detail::enable_if_enum_t<E, std::string_view> {
  using D = std::decay_t<E>;
  constexpr std::string_view name = detail::type_name_v<D>;
  static_assert(name.size() > 0, "Enum type does not have a name.");

  return name;
}

// Returns number of enum values.
template <typename E>
[[nodiscard]] constexpr auto enum_count() noexcept
    -> detail::enable_if_enum_t<E, std::size_t> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::count_v<D>;
}

// Returns enum value at specified index.
// No bounds checking is performed: the behavior is undefined if index >= number of enum
// values.
template <typename E>
[[nodiscard]] constexpr auto enum_value(std::size_t index) noexcept
    -> detail::enable_if_enum_t<E, std::decay_t<E>> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  if constexpr (detail::is_sparse_v<D>) {
    return assert(index < detail::count_v<D>), detail::values_v<D>[index];
  } else {
    return assert(index < detail::count_v<D>), static_cast<D>(index + detail::min_v<D>);
  }
}

// Obtains value enum sequence.
// Returns std::array with enum values, sorted by enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_values() noexcept
    -> detail::enable_if_enum_t<E, decltype(detail::values_v<std::decay_t<E>>) &> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::values_v<D>;
}

// Returns string enum name from static storage enum variable.
// This version is much lighter on the compile times and is not restricted to the
// enum_range limitation.
template <auto V>
[[nodiscard]] constexpr auto enum_name() noexcept
    -> detail::enable_if_enum_t<decltype(V), std::string_view> {
  using D = std::decay_t<decltype(V)>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  constexpr std::string_view name = detail::name_v<std::decay_t<D>, V>;
  static_assert(name.size() > 0, "Enum value does not have a name.");

  return name;
}

// Returns string enum name from enum value.
// If enum value does not have name or value out of range, returns empty string.
template <typename E>
[[nodiscard]] constexpr auto enum_name(E value) noexcept
    -> detail::enable_if_enum_t<E, std::string_view> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  if (const auto i = detail::endex<D>(value); i != -1) {
    return detail::names_v<D>[i];
  }

  return {}; // Value out of range.
}

// Obtains string enum name sequence.
// Returns std::array with string enum names, sorted by enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_names() noexcept
    -> detail::enable_if_enum_t<E, decltype(detail::names_v<std::decay_t<E>>) &> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::names_v<D>;
}

// Obtains pair (value enum, string enum name) sequence.
// Returns std::array with std::pair (value enum, string enum name), sorted by enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_entries() noexcept
    -> detail::enable_if_enum_t<E, decltype(detail::entries_v<std::decay_t<E>>) &> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::entries_v<D>;
}

// Obtains enum value from enum string name.
// Returns std::optional with enum value.
template <typename E, typename BinaryPredicate>
[[nodiscard]] constexpr auto
enum_cast(std::string_view value, BinaryPredicate p) noexcept(
    std::is_nothrow_invocable_r_v<bool, BinaryPredicate, char, char>)
    -> detail::enable_if_enum_t<E, std::optional<std::decay_t<E>>> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");
  static_assert(std::is_invocable_r_v<bool, BinaryPredicate, char, char>,
                "magic_enum::enum_cast requires bool(char, char) invocable predicate.");

  for (std::size_t i = 0; i < detail::count_v<D>; ++i) {
    if (detail::cmp_equal(value, detail::names_v<D>[i], p)) {
      return enum_value<D>(i);
    }
  }

  return std::nullopt; // Invalid value or out of range.
}

template <typename E>
[[nodiscard]] constexpr auto enum_cast(std::string_view value) noexcept
    -> detail::enable_if_enum_t<E, std::optional<std::decay_t<E>>> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  for (std::size_t i = 0; i < detail::count_v<D>; ++i) {
    if (value == detail::names_v<D>[i]) {
      return enum_value<D>(i);
    }
  }

  return std::nullopt; // Invalid value or out of range.
}

// Obtains enum value from integer value.
// Returns std::optional with enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_cast(underlying_type_t<E> value) noexcept
    -> detail::enable_if_enum_t<E, std::optional<std::decay_t<E>>> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  if (detail::undex<D>(value) != -1) {
    return static_cast<D>(value);
  }

  return std::nullopt; // Invalid value or out of range.
}

// Returns integer value from enum value.
template <typename E>
[[nodiscard]] constexpr auto enum_integer(E value) noexcept
    -> detail::enable_if_enum_t<E, underlying_type_t<E>> {
  return static_cast<underlying_type_t<E>>(value);
}

// Obtains index in enum value sequence from enum value.
// Returns std::optional with index.
template <typename E>
[[nodiscard]] constexpr auto enum_index(E value) noexcept
    -> detail::enable_if_enum_t<E, std::optional<std::size_t>> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  if (const auto i = detail::endex<D>(value); i != -1) {
    return i;
  }

  return std::nullopt; // Value out of range.
}

// Checks whether enum contains enumerator with such value.
template <typename E>
[[nodiscard]] constexpr auto enum_contains(E value) noexcept
    -> detail::enable_if_enum_t<E, bool> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::endex<D>(value) != -1;
}

// Checks whether enum contains enumerator with such integer value.
template <typename E>
[[nodiscard]] constexpr auto enum_contains(underlying_type_t<E> value) noexcept
    -> detail::enable_if_enum_t<E, bool> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return detail::undex<D>(value) != -1;
}

// Checks whether enum contains enumerator with such string enum name.
template <typename E>
[[nodiscard]] constexpr auto enum_contains(std::string_view value) noexcept
    -> detail::enable_if_enum_t<E, bool> {
  using D = std::decay_t<E>;
  static_assert(detail::supported<D>::value,
                "magic_enum unsupported compiler "
                "(https://github.com/Neargye/magic_enum#compiler-compatibility).");
  static_assert(detail::count_v<D>> 0,
                "magic_enum requires enum implementation and valid max and min.");

  return enum_cast<D>(value).has_value();
}

namespace ostream_operators {

template <typename Char, typename Traits, typename E>
auto operator<<(std::basic_ostream<Char, Traits> &os, E value)
    -> detail::enable_if_enum_t<E, std::basic_ostream<Char, Traits> &> {
  if (const auto name = enum_name(value); !name.empty()) {
    for (const auto c : name) {
      os.put(c);
    }
  } else {
    os << enum_integer(value);
  }

  return os;
}

template <typename Char, typename Traits, typename E>
auto operator<<(std::basic_ostream<Char, Traits> &os, std::optional<E> value)
    -> detail::enable_if_enum_t<E, std::basic_ostream<Char, Traits> &> {
  if (value.has_value()) {
    os << value.value();
  }

  return os;
}

} // namespace ostream_operators

namespace bitwise_operators {

template <typename E>
constexpr auto operator~(E rhs) noexcept -> detail::enable_if_enum_t<E, E> {
  return static_cast<E>(~static_cast<underlying_type_t<E>>(rhs));
}

template <typename E>
constexpr auto operator|(E lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E> {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) |
                        static_cast<underlying_type_t<E>>(rhs));
}

template <typename E>
constexpr auto operator&(E lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E> {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) &
                        static_cast<underlying_type_t<E>>(rhs));
}

template <typename E>
constexpr auto operator^(E lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E> {
  return static_cast<E>(static_cast<underlying_type_t<E>>(lhs) ^
                        static_cast<underlying_type_t<E>>(rhs));
}

template <typename E>
constexpr auto operator|=(E &lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E &> {
  return lhs = lhs | rhs;
}

template <typename E>
constexpr auto operator&=(E &lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E &> {
  return lhs = lhs & rhs;
}

template <typename E>
constexpr auto operator^=(E &lhs, E rhs) noexcept -> detail::enable_if_enum_t<E, E &> {
  return lhs = lhs ^ rhs;
}

} // namespace bitwise_operators

} // namespace magic_enum

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

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

} // namespace details

} // namespace structopt
#pragma once
#include <string>

namespace structopt {

namespace details {

static const bool is_binary_notation(std::string const &input) {
  return input.compare(0, 2, "0b") == 0 && input.size() > 2 &&
         input.find_first_not_of("01", 2) == std::string::npos;
}

static const bool is_hex_notation(std::string const &input) {
  return input.compare(0, 2, "0x") == 0 && input.size() > 2 &&
         input.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
}

static const bool is_octal_notation(std::string const &input) {
  return input.compare(0, 1, "0") == 0 && input.size() > 1 &&
         input.find_first_not_of("01234567", 1) == std::string::npos;
}

static inline bool is_valid_number(const std::string &input) {
  if (is_binary_notation(input) or is_hex_notation(input) or is_octal_notation(input)) {
    return true;
  }

  std::size_t i = 0, j = input.length() - 1;

  // Handling whitespaces
  while (i < input.length() && input[i] == ' ')
    i++;
  while (j >= 0 && input[j] == ' ')
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
#include <queue>
#include <string>
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
  std::vector<std::string> field_names;
  std::deque<std::string> positional_field_names; // mutated by parser
  std::deque<std::string> positional_field_names_for_help;
  std::deque<std::string> vector_like_positional_field_names;
  std::deque<std::string> flag_field_names;
  std::deque<std::string> optional_field_names;
  std::deque<std::string> nested_struct_field_names;

  visitor() = default;

  explicit visitor(const std::string &name, const std::string &version)
      : name(name), version(version) {}

  // Visitor function for std::optional - could be an option or a flag
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(const char *name, T &value) {
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
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    positional_field_names.push_back(name);
    positional_field_names_for_help.push_back(name);
    if constexpr (structopt::is_specialization<T, std::deque>::value or
                  structopt::is_specialization<T, std::list>::value or
                  structopt::is_specialization<T, std::vector>::value or
                  structopt::is_specialization<T, std::set>::value or
                  structopt::is_specialization<T, std::multiset>::value or
                  structopt::is_specialization<T, std::unordered_set>::value or
                  structopt::is_specialization<T, std::unordered_multiset>::value or
                  structopt::is_specialization<T, std::queue>::value or
                  structopt::is_specialization<T, std::stack>::value or
                  structopt::is_specialization<T, std::priority_queue>::value) {
      // keep track of vector-like fields as these (even though positional)
      // can be happy without any arguments
      vector_like_positional_field_names.push_back(name);
    }
  }

  // Visitor function for nested structs
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, void>::type
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    nested_struct_field_names.push_back(name);
  }

  bool is_field_name(const std::string &name) {
    return std::find(field_names.begin(), field_names.end(), name) != field_names.end();
  }

  void print_help(std::ostream &os) const {
    os << "\nUSAGE: " << name << " ";

    bool optional_arguments_available = false;

    if (flag_field_names.empty() == false) {
      optional_arguments_available = true;
      os << "[FLAGS] ";
    }

    if (optional_field_names.empty() == false) {
      optional_arguments_available = true;
      os << "[OPTIONS] ";
    }

    if (nested_struct_field_names.empty() == false) {
      os << "[SUBCOMMANDS] ";
    }

    for (auto &field : positional_field_names_for_help) {
      os << field << " ";
    }

    if (flag_field_names.empty() == false) {
      os << "\n\nFLAGS:\n";
      for (auto &flag : flag_field_names) {
        os << "    -" << flag[0] << ", --" << flag << "\n";
      }
    } else {
      os << "\n";
    }

    if (optional_field_names.empty() == false) {
      os << "\nOPTIONS:\n";
      for (auto &option : optional_field_names) {

        // Generate kebab case and present as option
        auto kebab_case = option;
        details::string_replace(kebab_case, "_", "-");
        std::string long_form = "";
        if (kebab_case != option) {
          long_form = kebab_case;
        } else {
          long_form = option;
        }

        os << "    -" << option[0] << ", --" << long_form << " <" << option << ">"
           << "\n";
      }
    }

    // if (!optional_arguments_available)
    //   os << "\n";

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
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
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

struct parser {
  structopt::details::visitor visitor;
  std::vector<std::string> arguments;
  std::size_t current_index{1};
  std::size_t next_index{1};
  bool double_dash_encountered{false}; // "--" option-argument delimiter
  bool sub_command_invoked{false};
  std::string already_invoked_subcommand_name{""};

  bool is_optional(const std::string &name) {
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

        // TODO: check if rest of name is NOT a decimal literal - this could be a negative
        // number if (name is a decimal literal) {
        //   result = false;
        // }

        if (name[1] == '-') {
          result = true;
        }
      }
    }
    return result;
  }

  bool is_optional_field(const std::string &next) {
    if (!is_optional(next)) {
      return false;
    }

    bool result = false;
    for (auto &field_name : visitor.field_names) {
      if (next == "-" + field_name or next == "--" + field_name or next == "-" + std::string(1, field_name[0])) {
        // okay `next` matches _a_ field name (which is an optional field)
        result = true;
      }
    }
    return result;
  }

  template <typename T> std::pair<T, bool> parse_argument(const char *name) {
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
    } else if constexpr (structopt::is_specialization<T, std::deque>::value or
                         structopt::is_specialization<T, std::list>::value or
                         structopt::is_specialization<T, std::vector>::value) {
      result = parse_vector_like_argument<T>(name);
    } else if constexpr (structopt::is_specialization<T, std::set>::value or
                         structopt::is_specialization<T, std::multiset>::value or
                         structopt::is_specialization<T, std::unordered_set>::value or
                         structopt::is_specialization<T,
                                                      std::unordered_multiset>::value) {
      result = parse_set_argument<T>(name);
    } else if constexpr (structopt::is_specialization<T, std::queue>::value or
                         structopt::is_specialization<T, std::stack>::value or
                         structopt::is_specialization<T, std::priority_queue>::value) {
      result = parse_container_adapter_argument<T>(name);
    } else {
      success = false;
    }
    return {result, success};
  }

  template <typename T> std::optional<T> parse_optional_argument(const char *name) {
    next_index += 1;
    std::optional<T> result;
    bool success;
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
  parse_single_argument(const char *name) {
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
        result = std::stoi(argument, nullptr, 2);
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
  parse_nested_struct(const char *name) {

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
      for (auto& field_name : parser.visitor.positional_field_names) {
        if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                      parser.visitor.vector_like_positional_field_names.end(),
                      field_name) == parser.visitor.vector_like_positional_field_names.end()) {
          // this positional argument is not a vector-like argument
          // it expects value(s)
          throw structopt::exception("Error: expected value for positional argument `" +
                                        field_name + "`.",
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
  std::pair<T1, T2> parse_pair_argument(const char *name) {
    std::pair<T1, T2> result;
    bool success;
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
  std::array<T, N> parse_array_argument(const char *name) {
    std::array<T, N> result;
    bool success;

    const auto arguments_left = arguments.size() - next_index;
    if (arguments_left == 0 or arguments_left < N) {
      throw structopt::exception("Error: expected " + std::to_string(N) +
                                     " values for std::array argument `" + name +
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
  void parse_tuple_element(const char *name, std::size_t index, std::size_t size,
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
  template <typename Tuple> Tuple parse_tuple_argument(const char *name) {
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
  template <typename T> T parse_vector_like_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next) or std::string{next} == "--") {
        if (std::string{next} == "--") {
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
  template <typename T> T parse_container_adapter_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next) or std::string{next} == "--") {
        if (std::string{next} == "--") {
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
  template <typename T> T parse_set_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next) or std::string{next} == "--") {
        if (std::string{next} == "--") {
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
  template <typename T> T parse_enum_argument(const char *name) {
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
  operator()(const char *name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

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
  operator()(const char *name, T &result) {
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
        // TODO: Report error, unexpected argument
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

      auto [value, success] = parse_argument<T>(field_name.c_str());
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
  operator()(const char *name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      if (next == "--" and double_dash_encountered == false) {
        double_dash_encountered = true;
        next_index += 1;
        return;
      }

      // Remove special characters from argument
      // e.g., --verbose => verbose
      // e.g., -v => v
      // e.g., --input-file => inputfile
      auto next_alpha = next;
      next_alpha.erase(std::remove_if(next_alpha.begin(), next_alpha.end(),
                                      [](char c) { return !std::isalpha(c); }),
                       next_alpha.end());

      // Remove special characters from field name
      // e.g., verbose => verbose
      // e.g., input_file => inputfile
      auto field_name_alpha = field_name;
      field_name_alpha.erase(std::remove_if(field_name_alpha.begin(),
                                            field_name_alpha.end(),
                                            [](char c) { return !std::isalpha(c); }),
                             field_name_alpha.end());

      // if `next` looks like an optional argument
      // i.e., starts with `-` or `--`
      // see if you can find an optional field in the struct with a matching name

      // check if the current argument looks like it could be this optional field
      if ((double_dash_encountered == false) and
          ((next == "-" + field_name or next == "--" + field_name or next == "-" + std::string(1, field_name[0])) or
           (next_alpha == field_name_alpha))) {

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
          if (next.size() > 1 and next[0] == '-') {
            const std::vector<char> option_delimiters = {'=', ':'};
            for (auto& c : option_delimiters) {
              std::string key, value;
              bool delimiter_found = false;
              for (size_t i = 0; i < next.size(); i++) {
                if (next[i] == c and !delimiter_found) {
                  delimiter_found = true;
                } else {
                  if (!delimiter_found) {
                    key += next[i];
                  } else {
                    value += next[i];
                  }
                }
              }

              // if an alternate option delimiter is used,
              // we have, at this point, split `--foo=bar` into `foo` and `bar`

              if (delimiter_found and is_optional_field(key)) {
                // found a delimiter in current argument

                // update next_index and return
                // the parser will take care of the rest
                for (auto& arg : {value, key}) {
                  const auto begin = arguments.begin();
                  arguments.insert(begin + next_index + 1, arg);
                }

                // get past the current argument, e.g., `--foo=bar`
                next_index += 1;
                return;
              }
            }
          }

          // A direct match of optional argument with field_name has not happened
          // This _could_ be a combined argument
          // e.g., -abc => -a, -b, and -c where each of these is a flag argument
          std::vector<std::string> potential_combined_argument;

          // if next is of the form `-abc` or `-de` and NOT of the form `--abc`
          // `--abc` is not a combined argument
          // `-abc` might be
          if (next[0] == '-' and (next.size() > 1 and next[1] != '-')) {
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
template <>
inline std::string parser::parse_single_argument<std::string>(const char *name) {
  return arguments[next_index];
}

// Specialization for bool
// yes, YES, on, 1, true, TRUE, etc. = true
// no, NO, off, 0, false, FALSE, etc. = false
// Converts argument to lower case before check
template <> inline bool parser::parse_single_argument<bool>(const char *name) {
  if (next_index > current_index) {
    current_index = next_index;
  }

  if (current_index < arguments.size()) {
    const std::vector<std::string> true_strings{"on", "yes", "1", "true"};
    const std::vector<std::string> false_strings{"off", "no", "0", "false"};
    std::string current_argument = arguments[current_index];

    // Convert argument to lower case
    std::transform(current_argument.begin(), current_argument.end(),
                   current_argument.begin(), ::tolower);

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
  explicit app(const std::string name, const std::string version = "")
      : visitor(name, version) {}

  template <typename T> T parse(const std::vector<std::string> &arguments) {
    T argument_struct;

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
      for (auto& field_name: parser.visitor.positional_field_names) {
        if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                      parser.visitor.vector_like_positional_field_names.end(),
                      field_name) == parser.visitor.vector_like_positional_field_names.end()) {
          // this positional argument is not a vector-like argument
          // it expects value(s)
          throw structopt::exception("Error: expected value for positional argument `" +
                                        field_name + "`.",
                                    parser.visitor);
        }
      }
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