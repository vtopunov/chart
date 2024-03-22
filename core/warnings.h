#pragma once


#if defined(_MSC_VER)

#define D_WARNING_PUSH              __pragma(warning(push))
#define D_WARNING_POP               __pragma(warning(pop))
#define D_WARNING_DISABLE_MSVC(id)  __pragma(warning(disable: id))

#elif defined(__clang__)

#define D_TEXT_PRAGMA(text)            _Pragma(#text)       
#define D_WARNING_PUSH                 _Pragma("clang diagnostic push")
#define D_WARNING_POP                  _Pragma("clang diagnostic pop")
#define D_WARNING_DISABLE_CLANG(text)  D_TEXT_PRAGMA(clang diagnostic ignored text)

#else 

#define D_WARNING_PUSH
#define D_WARNING_POP
#define D_WARNING_DISABLE_MSVC(id)

#endif


#ifndef D_WARNING_DISABLE_MSVC
#define D_WARNING_DISABLE_MSVC(id)
#endif

#ifndef D_WARNING_DISABLE_CLANG
#define D_WARNING_DISABLE_CLANG(text)
#endif

#define W_signed_unsigned_mismatch                                     4018
#define W_arithmetic_overflow                                          4056 26450 26451
#define W_truncation_of_value                                          4305 4309
#define W_variable_is_uninitialized                                    4701 26495
#define W_unreachable_code                                             4702
#define W_incorrect_logical_or                                         6285
#define W_redundant_code__left_and_right_subexpressions_are_identical  6287
#define W_potential_comparison_of_a_constant_with_another_constant     6326
#define W_avoid_malloc_and_free                                        26408
#define W_use_not_null                                                 26429                                                     
#define W_unchecked_subscript_operator                                 26446 26482
#define W_do_not_use_const_cast                                        26465 26492
#define W_converting_from_floating_point_to_unsigned_integral          26467
#define W_do_not_use_static_cast                                       26472
#define W_do_not_use_pointer_arithmetic                                26481
#define W_do_not_use_reinterpret_cast                                  26490
#define W_use_constexpr                                                26498
#define W_enum_is_unscoped__prefer_enum_class                          26812
#define W_use_bitwise_and_to_check_enum_flags                          26813
#define W_inconsistent_annotation                                      28251
