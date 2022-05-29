#pragma once

#ifdef _MSC_VER

#define D_WARNING_PUSH              __pragma(warning(push))
#define D_WARNING_POP               __pragma(warning(pop))
#define D_WARNING_DISABLE_MSVC(id)  __pragma(warning(disable: id))

#else

#define D_WARNING_PUSH
#define D_WARNING_POP
#define D_WARNING_DISABLE_MSVC(id)

#endif

#define W_incorrect_logical_or                                         6285
#define W_redundant_code__left_and_right_subexpressions_are_identical  6287
#define W_potential_comparison_of_a_constant_with_another_constant     6326
#define W_avoid_malloc_and_free                                        26408
#define W_do_not_slice                                                 26437
#define W_use_not_null                                                 26429
#define W_unchecked_subscript_operator                                 26446 26482
#define W_do_not_use_const_cast                                        26465 26492
#define W_converting_from_floating_point_to_unsigned_integral          26467
#define W_do_not_use_static_cast                                       26472
#define W_do_not_use_pointer_arithmetic                                26481
#define W_do_not_use_reinterpret_cast                                  26490
#define W_variable_is_uninitialized                                    26495
#define W_enum_is_unscoped__prefer_enum_class                          26812
