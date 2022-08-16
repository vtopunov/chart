#pragma once

#include <type_traits>

#include <os/fwd.h>

#if defined(D_OS_WINDOWS)
#include <os/os_windows.h>

static_assert(std::is_same_v<os::handle_t, HANDLE>);
static_assert(std::is_same_v<os::module_handle_t, HMODULE>);
static_assert(std::is_same_v<os::window_handle_t, HWND>);
static_assert(std::is_same_v<os::file_descriptor_t, HANDLE>);

static_assert(std::is_same_v<os::message_t, MSG>);

static_assert(std::is_same_v<os::uint_t, UINT>);
static_assert(std::is_same_v<os::dword_t, DWORD>);
static_assert(std::is_same_v<os::word_t, WORD>);

#elif defined(D_OS_ANDROID)
#include <os/os_android.h>
#endif


#include <os/undef.h>