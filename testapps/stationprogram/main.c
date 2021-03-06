/*
 *  KernelEx
 *  Copyright (C) 2013, Ley0k
 *
 *  This file is part of KernelEx source code.
 *
 *  KernelEx is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; version 2 of the License.
 *
 *  KernelEx is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "main.h"
#include <stdio.h>

PSESSION Session;

VOID DEBUG(const char *string)
{
    /*LPTSTR buffer;
    va_list args;
    va_start(args, format);

    //sprintf(buffer, format, args);
    FormatMessage(FORMAT_MESSAGE_FROM_STRING |
				  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  format,
                  0,
                  0,
                  (LPTSTR)&buffer,
                  0,
                  &args);

    va_end(args);
    char buffer[80];
    sprintf(buffer, string);

    MessageBox(NULL, buffer, NULL, 0);*/
    MessageBox(NULL, string, NULL, 0);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd,
    LPARAM lParam
)
{
    DWORD dwResult = -1;
    DWORD dwProcessId = 0;
    LRESULT Message = 0;
	HANDLE hProcess = NULL;

    GetWindowThreadProcessId(hwnd, &dwProcessId);

	if(dwProcessId == GetCurrentProcessId())
		return TRUE;

	if(lParam & EWX_FORCE)
		goto _terminate;

    Message = SendMessageTimeout(hwnd,
                                WM_QUERYENDSESSION,
                                0,
                                ENDSESSION_LOGOFF,
                                SMTO_NORMAL,
                                3000,
                                &dwResult);

    if(!Message || !dwResult)
		return FALSE;

_terminate:
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);

	Sleep(100);
    return TRUE;
}

BOOL CALLBACK EnumDesktopProc(LPTSTR lpszDesktop, LPARAM lParam)
{
    HDESK hDesktop;
    TCHAR Buffer[255];
    BOOL result;
    hDesktop = OpenDesktop(lpszDesktop, 0, FALSE, DESKTOP_ENUMERATE | DESKTOP_SWITCHDESKTOP);
    if(!hDesktop)
    {
        wsprintf(Buffer, "OpenDesktop failed, GetLastError = 0x%X", GetLastError());
        MessageBox(NULL, Buffer, NULL, 0);
        return TRUE;
    }
    SwitchDesktop(hDesktop);
    result = EnumDesktopWindows(hDesktop, EnumWindowsProc, lParam);

    if(!result)
    {
        wsprintf(Buffer, "EnumDesktopWindows failed, GetLastError = 0x%X", GetLastError());
        MessageBox(NULL, Buffer, NULL, 0);
        return TRUE;
    }

	return TRUE;
}

BOOL ExitWindowStation(HWINSTA hWinSta, DWORD dwFlags)
{
    TCHAR Buffer[255];
    BOOL result;
    result = EnumDesktops(hWinSta, EnumDesktopProc, (LPARAM) dwFlags);
    if(!result)
    {
        wsprintf(Buffer, "EnumDesktops failed, GetLastError = 0x%X", GetLastError());
        MessageBox(NULL, Buffer, NULL, 0);
    }
	return TRUE;
}

void UnloadEverything(void)
{
    if(!SetProcessWindowStation(Session->WindowStation))
        goto Cleanup;

    ExitWindowStation(Session->WindowStation, EWX_FORCE);

    SetProcessWindowStation(Session->InteractiveWindowStation);
    SwitchDesktop(Session->DefaultDesktop);

Cleanup:
    CloseDesktop(Session->Desktop);
    CloseWindowStation(Session->WindowStation);
    ExitProcess(0);
}

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    SECURITY_ATTRIBUTES sa;
    USEROBJECTFLAGS uoi;
    BOOL Result;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    uoi.fInherit = TRUE;
    uoi.fReserved = 0;
    uoi.dwFlags = WSF_VISIBLE;

    Session = (PSESSION)malloc(sizeof(SESSION));

    Session->InteractiveWindowStation = OpenWindowStation(TEXT("WinSta0"), FALSE, GENERIC_ALL);

    if(!Session->InteractiveWindowStation)
    {
        UnloadEverything(); // Unload if fail
        return 0;
    }

    Session->DefaultDesktop = GetThreadDesktop(GetCurrentThreadId());

    Session->WindowStationAccessRights = WINSTA_CREATEDESKTOP  |
                                         WINSTA_ENUMDESKTOPS   |
                                         WINSTA_READSCREEN     |
                                         WINSTA_READATTRIBUTES |
                                         WINSTA_WRITEATTRIBUTES;

    /*
     * Create a window station, remember that each window station contains their
     * own input, atoms, clipboard and desktop, each desktop include
     * one or more windows
     */

    SetProcessWindowStation(Session->InteractiveWindowStation);

    Session->WindowStation = CreateWindowStation(TEXT("WinSta1"), 0, Session->WindowStationAccessRights, &sa);
    SetLastError(0);
    if(!Session->WindowStation)
    {
        UnloadEverything();
        return 0;
    }

    /*if(!SetUserObjectInformation(Session.WinStation, UOI_FLAGS, &uoi, sizeof(USEROBJECTFLAGS)))
    {
        UnloadEverything();
        return 0;
    }*/

    //DEBUG("#4");
    if(!SetProcessWindowStation(Session->WindowStation))
    {
        Result = GetLastError();
        UnloadEverything();
        return 0;
    }

    //DEBUG("#5");
    Session->Desktop = CreateDesktop(TEXT("Default"), NULL, NULL, 0, GENERIC_ALL, NULL);
    if(!Session->Desktop)
    {
        UnloadEverything();
        return 0;
    }

    //DEBUG("#6");
    if(!SetProcessWindowStation(Session->InteractiveWindowStation))
    {
        UnloadEverything();
        return 0;
    }

    if(!CreateMultiSessionWindow())
    {
        UnloadEverything();
        return 0;
    }
    UnloadEverything();
    return 0;
}
