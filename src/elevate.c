// clumsy needs to be run as admin to work properly.
// here's routines for checking admin state and self elevate
// from the example CppSelfElevate
// http://code.msdn.microsoft.com/windowsdesktop/CppUACSelfElevation-981c0160
#include <Windows.h>
#include <VersionHelpers.h>
#include "common.h"

// 
//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as 
//   administrator. In other words, it dictates whether the primary access 
//   token of the process belongs to user account that is a member of the 
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//   * changed to not throw and return false *
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID, 
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 0, 0, 0, 0, 0, 
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        return FALSE;
    }

    return fIsRunAsAdmin;
}

// pasta from:
// http://stackoverflow.com/questions/8046097/how-to-check-if-a-process-has-the-admin-rights
BOOL IsElevated( ) {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess( ),TOKEN_QUERY,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return fRet;
}

// try elevate and error out when can't happen
// is silent then no message boxes are shown
// return whether to close the program
BOOL tryElevate(HWND hWnd, BOOL silent) {
    // Check the current process's "run as administrator" status.
    BOOL fIsRunAsAdmin;
    if (!IsWindowsVistaOrGreater()) {
        if (!silent) MessageBox(hWnd, (LPCSTR)"Unsupported Windows version. clumsy only supports Windows Vista or above.",
            (LPCSTR)"Aborting", MB_OK);
        return TRUE;
    }

    fIsRunAsAdmin = IsRunAsAdmin();
    if (fIsRunAsAdmin) {
        return FALSE;
    }

    // when not silent then trying to reinvoke to elevate
    if (!silent) {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileName(NULL, (LPSTR)szPath, ARRAYSIZE(szPath)))
        {
            // Launch itself as administrator.
            SHELLEXECUTEINFO sei = { sizeof(sei) };
            sei.lpVerb = (LPSTR)"runas";
            sei.lpFile = (LPSTR)szPath;
            sei.hwnd = hWnd;
            sei.nShow = SW_NORMAL;

            // here it is restartintg the program using run as
            LOG("Try elevating by runas");
            if (!ShellExecuteEx(&sei))
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_CANCELLED)
                {
                    // The user refused the elevation.
                    // alert and exit
                    MessageBox(hWnd, (LPCSTR)"clumsy needs to be elevated to work. Run as Administrator or click Yes in promoted UAC dialog",
                        (LPCSTR)"Aborting", MB_OK);
                }
            }
            // runas executed.
        } else {
            MessageBox(hWnd, (LPCSTR)"Failed to get clumsy path. Please place the executable in a normal directory.",
                (LPCSTR)"Aborting", MB_OK);
        }
    }

    // exit when not run as admin
    return TRUE;
}