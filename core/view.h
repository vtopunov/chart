#pragma once

#include <core/member_detector.h>

template<class T>
using has_view_t = typename T::view_type;

template <class T>
using view_t = detected_or_t<T, has_view_t, T>;