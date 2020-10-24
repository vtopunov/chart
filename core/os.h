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

#undef OpenFile
#undef LoadBitmap
#undef MessageBox
#undef GetObject
#undef CreateFile
#undef CreateEvent
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef PathFileExists
#undef CreateFileMapping
#undef FindFirstFile
#undef FindNextFile
#undef GetTempPath
#undef GetCurrentTime
#undef SendMessage
#undef GetMessage
#undef OutputDebugString
#undef OVERFLOW

