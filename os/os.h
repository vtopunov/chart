#pragma once

#include <os/fwd.h>

#if defined(D_OS_WINDOWS)
#include <os/os_windows.h>

static_assert(std::is_same_v<wchar_t, WCHAR>);
static_assert(std::is_same_v<wchar_t*, LPWSTR>);
static_assert(std::is_same_v<const wchar_t*, LPCWSTR>);
static_assert(std::is_same_v<os::handle_t, HANDLE>);
static_assert(std::is_same_v<os::gdi_object_handle_t, HGDIOBJ>);
static_assert(std::is_same_v<os::brush_handle_t, HBRUSH>);
static_assert(std::is_same_v<os::window_handle_t, HWND>);
static_assert(std::is_same_v<os::file_descriptor_t, HANDLE>);

static_assert(std::is_same_v<os::message_t, MSG>);

static_assert(std::is_same_v<os::uint_t, UINT>);
static_assert(std::is_same_v<os::dword_t, DWORD>);
static_assert(std::is_same_v<os::word_t, WORD>);
static_assert(std::is_same_v<os::word_parameter_t, WPARAM>);
static_assert(std::is_same_v<os::long_parameter_t, LPARAM>);
static_assert(std::is_same_v<os::long_result_t, LRESULT>);

static_assert(std::is_same_v<os::wndproc_t, WNDPROC>);

namespace os
{
    inline HMODULE current_module() noexcept
    {
        return GetModuleHandleW(nullptr);
    }
}

#elif defined(D_OS_ANDROID)
#include <os/os_android.h>
#endif