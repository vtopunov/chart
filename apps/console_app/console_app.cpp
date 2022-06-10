#include <Windows.h>

struct tagMSG_
{
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
    DWORD       time;
    POINT       pt;
    DWORD       lPrivate;
};

int main() noexcept
{
    constexpr tagMSG_ msg{};
    constexpr auto sz = sizeof(msg);
    constexpr auto align = alignof(tagMSG_);
    return sz && align;
}
