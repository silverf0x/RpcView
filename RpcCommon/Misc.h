#ifndef _MISC_H_
#define _MISC_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable:4201)
//To represent a version as either 64bits value or x.x.x.x
typedef struct _Version_T{
	union{
		UINT64	As64BitsValue;
		struct{
			USHORT	Part1;
			USHORT	Part2;
			USHORT	Part3;
			USHORT	Part4;
		}As16BitsValues;
	};
}Version_T;
#pragma warning(pop)

typedef struct _LocationInfo_T{
	WCHAR	Location[MAX_PATH];
	VOID*	pBaseAddress;
	DWORD	State;
	DWORD	Type;
	SIZE_T	Size;
}LocationInfo_T;


BOOL	WINAPI AdjustPrivilege(LPCTSTR lpPrivilegeName,BOOL bEnablePrivilege);
BOOL	WINAPI GetModuleDescription(WCHAR* pModulePath,WCHAR* pDescription,UINT Bytes);
UINT64	WINAPI GetModuleVersion(WCHAR* pModulePath);
BOOL	WINAPI GetLocationInfo(HANDLE hProcess, VOID* pAddress, LocationInfo_T* pLocationInfo);
BOOL	WINAPI GetProcessNameFromPid(DWORD Pid,WCHAR* pName,UINT NameSizeInBytes);
BOOL	WINAPI GetProcessPath(DWORD Pid, WCHAR* pProcessPath, DWORD ProcessPathLength);
BOOL	WINAPI GetProcessPebInfo(HANDLE hProcess,WCHAR* pCmdLine,UINT CmdLineLength,WCHAR* pDesktop,UINT DesktopLength);
BOOL	WINAPI GetRegValueData(HKEY hRootKey,WCHAR* pSubkeyName,WCHAR* pValueName,VOID* pData,UINT DataLength);
BOOL	WINAPI GetUserAndDomainName(DWORD Pid, WCHAR* Buffer, ULONG BufferLengthInBytes);
BOOL	WINAPI IsProcessWow64(ULONG Pid);

typedef BOOL (WINAPI* EnumProcessCallbackFn_T)(DWORD Pid, DWORD Ppid, VOID* pContext, BOOL* pbContinue);
BOOL	WINAPI EnumProcess(EnumProcessCallbackFn_T EnumProcessCallbackFn, void* pCallbackCtxt);

#ifdef __cplusplus
}
#endif

#endif //_MISC_H_