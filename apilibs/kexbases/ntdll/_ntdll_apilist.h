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

#ifndef _NTDLL_APILIST_H
#define _NTDLL_APILIST_H

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "common.h"
#include <ntsecapi.h>
#include "../kernel32/_kernel32_apilist.h"
#include "../user32/_user32_apilist.h"

#define KEXSHUTDOWNSYSTEM	2

DWORD __fastcall VKernelExCall(DWORD command, DWORD param1);

typedef PLSA_OBJECT_ATTRIBUTES POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID
{
     DWORD UniqueProcessId;
     DWORD UniqueThreadId;
} CLIENT_ID, *PCLIENT_ID;

typedef enum _SHUTDOWN_ACTION {
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
} SHUTDOWN_ACTION, *PSHUTDOWN_ACTION;

//
// Responses for NtRaiseHardError
//
typedef enum _HARDERROR_RESPONSE_OPTION
{
    OptionAbortRetryIgnore,
    OptionOk,
    OptionOkCancel,
    OptionRetryCancel,
    OptionYesNo,
    OptionYesNoCancel,
    OptionShutdownSystem,
    OptionOkNoWait,
    OptionCancelTryContinue
} HARDERROR_RESPONSE_OPTION, *PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE
{
    ResponseReturnToCaller,
    ResponseNotHandled,
    ResponseAbort,
    ResponseCancel,
    ResponseIgnore,
    ResponseNo,
    ResponseOk,
    ResponseRetry,
    ResponseYes,
    ResponseTryAgain,
    ResponseContinue
} HARDERROR_RESPONSE, *PHARDERROR_RESPONSE;

typedef struct _INITIAL_TEB {
	PVOID StackBase;
	PVOID StackLimit;
	PVOID StackCommit;
	PVOID StackCommitMax;
	PVOID StackReserved;
} INITIAL_TEB, *PINITIAL_TEB;


BOOL init_ntdll();
extern const apilib_api_table apitable_ntdll;

/*** AUTOGENERATED APILIST DECLARATIONS BEGIN ***/
STUB NtCreateProcess_stub;
STUB NtCreateThread_stub;
STUB ZwCreateProcess_stub;
STUB ZwCreateThread_stub;
ULONG NTAPI RtlNtStatusToDosError(NTSTATUS Status);
NTSTATUS NTAPI NtRaiseHardError(IN NTSTATUS ErrorStatus, IN ULONG NumberOfParameters, IN ULONG UnicodeStringParameterMask, IN PULONG_PTR Parameters, IN ULONG ValidResponseOptions, OUT PULONG Response);
NTSTATUS NTAPI ZwClose(IN HANDLE Handle);
NTSTATUS NTAPI ZwDuplicateObject(IN HANDLE SourceProcessHandle, IN HANDLE SourceHandle, IN HANDLE TargetProcessHandle OPTIONAL, OUT PHANDLE TargetHandle OPTIONAL, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, IN ULONG Options);
NTSTATUS WINAPI ZwWaitForSingleObject(IN HANDLE Handle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout);
NTSTATUS ZwAllocateVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN ULONG_PTR ZeroBits, IN OUT PSIZE_T RegionSize, IN ULONG AllocationType, IN ULONG Protect);
NTSTATUS ZwFreeVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T RegionSize, IN ULONG FreeType);
NTSTATUS NTAPI ZwOpenProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK AccessMask, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId);
NTSTATUS NTAPI ZwResumeProcess(IN HANDLE ProcessHandle);
NTSTATUS NTAPI ZwSuspendProcess(IN HANDLE ProcessHandle);
NTSTATUS NTAPI ZwTerminateProcess(IN HANDLE ProcessHandle OPTIONAL, IN NTSTATUS ExitStatus);
NTSTATUS NTAPI ZwCreateThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ProcessHandle, OUT PCLIENT_ID ClientId, IN PCONTEXT ThreadContext, IN PINITIAL_TEB InitialTeb, IN BOOLEAN CreateSuspended);
NTSTATUS NTAPI ZwGetContextThread(IN HANDLE ThreadHandle, OUT PCONTEXT pContext);
NTSTATUS NTAPI ZwSetContextThread(IN HANDLE ThreadHandle, IN PCONTEXT Context);
NTSTATUS NTAPI ZwOpenThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK AccessMask, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId);
NTSTATUS NTAPI ZwResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL);
NTSTATUS NTAPI ZwSuspendThread(IN HANDLE ThreadHandle, OUT PULONG PreviousSuspendCount OPTIONAL);
NTSTATUS NTAPI ZwTerminateThread(IN HANDLE ThreadHandle, IN NTSTATUS ExitStatus);
NTSTATUS NTAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *crit);
NTSTATUS NTAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
NTSTATUS NTAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *crit);
NTSTATUS NTAPI RtlInitializeCriticalSectionAndSpinCount(RTL_CRITICAL_SECTION *crit, ULONG spincount);
NTSTATUS NTAPI RtlInitializeCriticalSectionEx(RTL_CRITICAL_SECTION *crit, ULONG spincount, ULONG flags);
NTSTATUS NTAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *crit);
NTSTATUS NTAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *crit);
NTSTATUS NTAPI ZwQuerySystemInformation(IN ULONG SystemInformationClass, IN OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength OPTIONAL);
NTSTATUS NTAPI ZwShutdownSystem(IN SHUTDOWN_ACTION Action);
/*** AUTOGENERATED APILIST DECLARATIONS END ***/

#endif
