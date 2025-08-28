// Файл совместимости для разных версий MinGW
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

// Заглушка для __ms_vsnprintf (нужна для IUP)
int __ms_vsnprintf(char *buffer, size_t count, const char *format, va_list argptr) {
    return vsnprintf(buffer, count, format, argptr);
}

// Условные определения для старых версий MinGW (только для 32-бит)
#ifndef _WIN64
// Для 32-битной версии добавляем недостающие функции
void __stdcall DisableProcessWindowsGhosting(void) {
    // Заглушка
}

DWORD __stdcall GetLayout(HDC hdc) {
    UNREFERENCED_PARAMETER(hdc);
    return 0;
}

BOOL __stdcall IsWow64Process(HANDLE hProcess, PBOOL Wow64Process) {
    UNREFERENCED_PARAMETER(hProcess);
    if (Wow64Process) {
        *Wow64Process = FALSE;
    }
    return TRUE;
}

// Альтернативные символы для линкера (только 32-бит)
void* _imp__DisableProcessWindowsGhosting = &DisableProcessWindowsGhosting;
void* _imp__GetLayout = &GetLayout;
#endif