#pragma once

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif

#pragma warning(push, 0)
#include <windows.h>
#include <windowsx.h>
#pragma warning(pop)


#define LPCTSTR LPCWSTR
#define LPTSTR LPWSTR

[[nodiscard]]
constexpr LPCWSTR MAKEINTATOMW(ATOM atom) noexcept
{
    return MAKEINTATOM(atom);
}

#undef LPCTSTR
#undef LPTSTR


#pragma push_macro("MAKEINTRESOURCE")

#ifdef MAKEINTRESOURCEW
#undef MAKEINTRESOURCE
#define MAKEINTRESOURCE MAKEINTRESOURCEW

[[nodiscard]]
constexpr LPCWSTR _IDC_ARROWW() noexcept
{
    return IDC_ARROW;
}

#endif

#pragma pop_macro("MAKEINTRESOURCE")


#define IDC_ARROWW _IDC_ARROWW()
