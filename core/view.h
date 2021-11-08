#pragma once

#include <core/member_detector.h>

template<class T>
using decl_view_t = typename T::view_type;

template <class T>
using view_t = detected_or_t<T, decl_view_t, T>;