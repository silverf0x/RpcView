#include "ntdll.h"

NtQueryInformationProcessFn_T	NtQueryInformationProcessFn	= NULL;


//-----------------------------------------------------------------------------
NTSTATUS NTAPI NtQueryInformationProcess(
	IN	HANDLE				ProcessHandle,
	IN	PROCESSINFOCLASS	ProcessInformationClass,
	OUT	PVOID				ProcessInformation,
	IN	ULONG				ProcessInformationLength,
	OUT	PULONG				ReturnLength OPTIONAL
	)
{
	if (NtQueryInformationProcessFn == NULL)
	{
		NtQueryInformationProcessFn = (NtQueryInformationProcessFn_T)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQueryInformationProcess");
	}
	return NtQueryInformationProcessFn(
		ProcessHandle,
		ProcessInformationClass,
		ProcessInformation,
		ProcessInformationLength,
		ReturnLength);
}