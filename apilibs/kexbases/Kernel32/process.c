/*
 *  KernelEx
 *  Copyright (C) 2009, Xeno86
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

#include "common.h"
#include "..\user32\desktop.h"
#include "_kernel32_apilist.h"

/* MAKE_EXPORT CreateProcessA_fix=CreateProcessA */
BOOL WINAPI CreateProcessA_fix(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    PTDB98 Thread = get_tdb();
	PPDB98 Process = get_pdb();
	PCHAR pszDesktop = NULL;
	BOOL result = FALSE;
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS pNTHeader;

    if(IsBadWritePtr(lpStartupInfo, sizeof(STARTUPINFO)) || IsBadWritePtr(lpProcessInformation, sizeof(PROCESS_INFORMATION)))
        return FALSE;

    /* Remove unsupported flags (and fix them) */
    if(dwCreationFlags & CREATE_NO_WINDOW)
    {
        if(!lpStartupInfo->dwFlags & STARTF_USESHOWWINDOW)
            lpStartupInfo->dwFlags |= STARTF_USESHOWWINDOW;

        lpStartupInfo->wShowWindow = SW_HIDE;
        dwCreationFlags &= ~CREATE_NO_WINDOW;
    }

    if(dwCreationFlags & CREATE_SEPARATE_WOW_VDM)
        dwCreationFlags &= ~CREATE_SEPARATE_WOW_VDM;

    if(dwCreationFlags & CREATE_SHARED_WOW_VDM)
        dwCreationFlags &= ~CREATE_SHARED_WOW_VDM;

    if(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT)
        dwCreationFlags &= ~CREATE_UNICODE_ENVIRONMENT;

	hFile = CreateFileA_fix(lpApplicationName != NULL ? lpApplicationName : lpCommandLine,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							0);
                    
	if(hFile == INVALID_HANDLE_VALUE)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	hFileMapping = CreateFileMapping(hFile,
									NULL,
									PAGE_READONLY,
									0,
									0,
									NULL);

	if(hFileMapping == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	lpFileBase = MapViewOfFile(hFileMapping,
							FILE_MAP_READ,
							0,
							0,
							0);

	if(lpFileBase == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	__try
	{
		pNTHeader = (PIMAGE_NT_HEADERS)dosHeader->e_lfanew;

		if(pNTHeader->FileHeader.Characteristics == IMAGE_FILE_DLL)
		{
			SetLastError(ERROR_BAD_EXE_FORMAT);
			return FALSE;
		}

		if(pNTHeader->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_GUI &&
			pNTHeader->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CUI)
		{
			/* Set the famous "The %1 application cannot be run in Win32 mode" error */
			SetLastError(ERROR_CHILD_NOT_COMPLETE);
			return FALSE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		SetLastError(ERROR_BAD_EXE_FORMAT);
		return FALSE;
	}

	__try
	{
		if(lpStartupInfo->lpDesktop == NULL)
		{
			lpStartupInfo->lpDesktop = Process->Win32Process->rpdeskStartup->lpName;
		}

		/* Make the desktop name a shared string */
		pszDesktop = (PCHAR)kexAllocObject(strlen(lpStartupInfo->lpDesktop));
		strcpy(pszDesktop, lpStartupInfo->lpDesktop);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		pszDesktop = NULL;
	}

	lpStartupInfo->lpDesktop = pszDesktop;

	result = CreateProcessA(lpApplicationName,
						 lpCommandLine,
						 lpProcessAttributes,
						 lpThreadAttributes,
						 bInheritHandles,
						 dwCreationFlags,
						 lpEnvironment,
						 lpCurrentDirectory,
						 lpStartupInfo,
						 lpProcessInformation);

	if(!result)
	{
		free(pszDesktop);
		return FALSE;
	}

	return TRUE;
}

/* MAKE_EXPORT CreateProcessAsUserA_new=CreateProcessAsUserA*/
BOOL WINAPI CreateProcessAsUserA_new(HANDLE hToken, 
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	return CreateProcessA_fix(lpApplicationName,
							lpCommandLine,
							lpProcessAttributes,
							lpThreadAttributes,
							bInheritHandles,
							dwCreationFlags,
							lpEnvironment,
							lpCurrentDirectory,
							lpStartupInfo,
							lpProcessInformation);
}

/* MAKE_EXPORT ExitProcess_fix=ExitProcess */
VOID WINAPI ExitProcess_fix( UINT uExitCode )
{
	PDB98* pdb = get_pdb();
	
	//process is already terminating. we would badly crash 
	if (pdb->Flags & fTerminating)
	{
		//so instead silently fail
		SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
		RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
	}

	//set calling thread priority to the lowest possible. this way 
	//we greatly improve chance for thread which calls ExitProcess
	//to finish latest and call dll process detach like NT
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
	ExitProcess(uExitCode);
}

/* MAKE_EXPORT GetProcessId_new=GetProcessId */
DWORD WINAPI GetProcessId_new(
  HANDLE hProcess
)
{
	PPDB98 Process = (PPDB98)kexGetHandleObject(hProcess, WIN98_K32OBJ_PROCESS, 0);
	DWORD Obfuscator = (DWORD)get_pdb() ^ GetCurrentProcessId();

	if(Process == NULL)
		return 0;

	return (DWORD)Process ^ Obfuscator;
}

/* This is less reliable */
#if 0
DWORD WINAPI GetProcessId_new(
  HANDLE hProcess
)
{
	typedef DWORD (WINAPI *MPH) (HANDLE hProcess);
	static MPH MapProcessHandle = 0;
	
	if (!MapProcessHandle)
	{
		DWORD *faddr;
		DWORD addr;
		
		faddr = (DWORD *) ( (DWORD)SetFilePointer + 0x1D ); //there is jmp _SetFilePointer	
		addr = (DWORD) faddr + *faddr + 4 - 0x16; //0x16 bytes before _SetFilePointer there is MapProcessHandle, just what we need	
		faddr = (DWORD *) addr;
		if (*faddr != 0x206A006A)
			fatal_error("GetProcessId: ASSERTION FAILED"); //push 0; push 0x20
		MapProcessHandle = (MPH) addr;
	}
	return MapProcessHandle(hProcess);
}
#endif

/* MAKE_EXPORT GetProcessShutdownParameters_new=GetProcessShutdownParameters */
BOOL WINAPI GetProcessShutdownParameters_new(LPDWORD lpdwLevel, LPDWORD lpdwFlags)
{
	PPDB98 Process = get_pdb();
	PPROCESSINFO ppi = Process->Win32Process;

	if(IsBadWritePtr(lpdwLevel, sizeof(DWORD)) || IsBadWritePtr(lpdwFlags, sizeof(DWORD)))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(ppi == NULL)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	*lpdwLevel = ppi->ShutdownLevel;
	*lpdwFlags = ppi->ShutdownFlags;

	return TRUE;
}

/* MAKE_EXPORT IsWow64Process_new=IsWow64Process */
BOOL WINAPI IsWow64Process_new(HANDLE hProcess, PBOOL Wow64Process)
{
	if (!Wow64Process) 
		return FALSE;
	*Wow64Process = FALSE;
	return TRUE;
}

/* MAKE_EXPORT ProcessIdToSessionId_new=ProcessIdToSessionId */
BOOL WINAPI ProcessIdToSessionId_new(DWORD dwProcessId, DWORD *pSessionId)
{
	if (!pSessionId)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	/* Process not running under RDS session */
	*pSessionId = 0;
	return TRUE;
}

/* MAKE_EXPORT SetProcessShutdownParameters_new=SetProcessShutdownParameters */
BOOL WINAPI SetProcessShutdownParameters_new(DWORD dwLevel, DWORD dwFlags)
{
	PPDB98 Process = get_pdb();
	PPROCESSINFO ppi = Process->Win32Process;

	if(dwFlags > SHUTDOWN_NORETRY)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if(ppi == NULL)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	ppi->ShutdownLevel = dwLevel;
	ppi->ShutdownFlags = dwFlags;

	return TRUE;
}