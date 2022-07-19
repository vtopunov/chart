#pragma once

#include <core/warnings.h>

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_incorrect_logical_or)
D_WARNING_DISABLE_MSVC(W_redundant_code__left_and_right_subexpressions_are_identical)
D_WARNING_DISABLE_MSVC(W_arithmetic_overflow)
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized)
D_WARNING_DISABLE_MSVC(W_use_constexpr)
D_WARNING_DISABLE_MSVC(W_enum_is_unscoped__prefer_enum_class)

#include <fmt/xchar.h>
#include <fmt/format.h>

D_WARNING_POP
