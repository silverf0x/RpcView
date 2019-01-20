#include <windows.h>
#include <Psapi.h>
#include <Tlhelp32.h>
#include <conio.h>
#include <strsafe.h>
#include "Misc.h"
#include "RpcCommon.h"
#include "ntdll.h"

#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"strsafe.lib")
#pragma comment(lib,"Version.lib")

#define MAX_DRIVE_INDEX			26

typedef struct _LanguageCodePage_T {
	WORD wLanguage;
	WORD wCodePage;
} LanguageCodePage_T;

#define IOCTL_OPEN_PROCESS 0x8335003C

HANDLE hProcexp = NULL;

HANDLE WINAPI ProcexpOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
    HANDLE  hProcess = NULL;
    HANDLE  Pid = (HANDLE)(uintptr_t)dwProcessId;
    DWORD   Bytes;

    hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    if (hProcess != NULL) goto End;
    if (hProcexp == NULL)
    {
        hProcexp = CreateFileA(
            "\\\\.\\PROCEXP152",
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hProcexp == INVALID_HANDLE_VALUE)
        {
            goto End;
        }
    }
    DeviceIoControl(
        hProcexp,
        IOCTL_OPEN_PROCESS,
        &Pid,
        sizeof(Pid),
        &hProcess,
        sizeof(hProcess),
        &Bytes,
        NULL
    );
End:
    return hProcess;
}

//------------------------------------------------------------------------------
BOOL WINAPI AdjustPrivilege(LPCTSTR lpPrivilegeName,BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES	TokenPrivilege;
    LUID				Luid;
	HANDLE				hToken = NULL;
	BOOL				bResult=FALSE;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) { goto End; }
	if (!LookupPrivilegeValue(NULL,lpPrivilegeName,&Luid)) {goto End;}
    TokenPrivilege.PrivilegeCount    =1;
    TokenPrivilege.Privileges[0].Luid=Luid;
	if (bEnablePrivilege) TokenPrivilege.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    else TokenPrivilege.Privileges[0].Attributes=0;
	if (!AdjustTokenPrivileges(hToken,FALSE,&TokenPrivilege,sizeof(TOKEN_PRIVILEGES),NULL,NULL)) {goto End;}
	if (GetLastError() == ERROR_SUCCESS) bResult=TRUE;
End:
	if (hToken != NULL) CloseHandle(hToken);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI EnumProcess(EnumProcessCallbackFn_T EnumProcessCallbackFn,void* pCallbackCtxt)
{
	BOOL				bResult=FALSE;
	HANDLE				hSnapshot;
	PROCESSENTRY32W		ProcessEntry;
	BOOL				bContinue=TRUE;

	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hSnapshot == INVALID_HANDLE_VALUE) goto End;
	ProcessEntry.dwSize=sizeof(ProcessEntry);
	if (!Process32FirstW(hSnapshot,&ProcessEntry)) goto End;
	do
	{
		bResult=EnumProcessCallbackFn(ProcessEntry.th32ProcessID,ProcessEntry.th32ParentProcessID,pCallbackCtxt,&bContinue);
		if (!bResult) goto End;
		if (!bContinue) break;

	}while(Process32NextW(hSnapshot,&ProcessEntry));
End:
    if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetModuleDescription(WCHAR* pModulePath,WCHAR* pDescription,UINT Bytes)
{
	DWORD				dwHandle;
	DWORD				InfoSize;
	UINT				cbTranslate;
	LanguageCodePage_T*	lpTranslate;
	UINT				Size;
	UINT				i;
	WCHAR				SubBlock[MAX_PATH];
	WCHAR*				lpBuffer;
	BOOL				bResult=FALSE;
	VOID*				pData=NULL;
	//
	// Read the list of languages and code pages.
	//
	InfoSize=GetFileVersionInfoSizeW(pModulePath,&dwHandle);
	pData=OS_ALLOC(InfoSize);
	if (pData==NULL) goto End;

	if (!GetFileVersionInfoW(pModulePath,0,InfoSize,pData)) goto End;

	if (!VerQueryValueW(pData, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate,&cbTranslate)) goto End;
	//
	// Read the file description for each language and code page.
	//
	for(i=0; i < (cbTranslate/sizeof(LanguageCodePage_T)); i++)
	{
		StringCbPrintfW(SubBlock,sizeof(SubBlock),L"\\StringFileInfo\\%04x%04x\\FileDescription",lpTranslate[i].wLanguage,lpTranslate[i].wCodePage);
		//
		// Retrieve file description for language and code page "i". 
		//
		if (VerQueryValueW(pData,SubBlock,(LPVOID*)&lpBuffer,&Size))
		{
			StringCbPrintfW(pDescription,Bytes,L"%s",lpBuffer);
			break;
		}
	}
	bResult=TRUE;
End:
	if (pData!=NULL) OS_FREE(pData);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetLocationInfo(HANDLE hProcess, VOID* pAddress, LocationInfo_T* pLocationInfo)
{
	MEMORY_BASIC_INFORMATION	MemBasicInfo;
	DWORD						DriveMask;
	WCHAR						DriveIdx;
	WCHAR						NativeLocation[RPC_MAX_LENGTH];
	WCHAR						DeviceName[RPC_MAX_LENGTH];
	WCHAR*						pPath;
	WCHAR						DosDevice[]=L"X:";
	BOOL						bResult=FALSE;
	
	if (pLocationInfo == NULL) goto End;
	//
	// Get Memory location informations
	//
	ZeroMemory(pLocationInfo, sizeof(LocationInfo_T));
	if (!VirtualQueryEx(hProcess, pAddress, &MemBasicInfo, sizeof(MemBasicInfo))) goto End;
	pLocationInfo->pBaseAddress = MemBasicInfo.BaseAddress;
	pLocationInfo->State		= MemBasicInfo.State;
	pLocationInfo->Type			= MemBasicInfo.Type;
	pLocationInfo->Size			= MemBasicInfo.RegionSize;
	//
	// Get the native mapped file name containing the specified address
	//
	if (!GetMappedFileNameW(hProcess, pAddress, NativeLocation, _countof(NativeLocation))) goto End;
	//
	// Get the correponding Win32 path
	//
	DriveMask=GetLogicalDrives();
	for (DriveIdx=0; DriveIdx < MAX_DRIVE_INDEX; DriveIdx++)
	{
		if (DriveMask & (1 << DriveIdx))
		{
			DosDevice[0]=L'A'+DriveIdx;
			if (QueryDosDeviceW(DosDevice,DeviceName,_countof(DeviceName))!=0)
			{
				pPath=wcsstr(NativeLocation,DeviceName);
				if (pPath!=NULL)
				{
					StringCbPrintfW(pLocationInfo->Location, sizeof(pLocationInfo->Location), L"%s%s", DosDevice, NativeLocation + wcslen(DeviceName));
					bResult=TRUE;
					break;
				}
			}
		}
	}
End:
	return (bResult);
}

//------------------------------------------------------------------------------
UINT64 WINAPI GetModuleVersion(WCHAR* pModulePath)
{
	DWORD				dwHandle;
	DWORD				VersionInfoSize;
	VS_FIXEDFILEINFO*	pFixedFileInfo;
	LARGE_INTEGER		ModuleVersion;
	UINT				Size;
	VOID*				pVersionData = NULL;

	ModuleVersion.QuadPart = 0;
	VersionInfoSize = GetFileVersionInfoSizeW(pModulePath, &dwHandle);
	pVersionData = OS_ALLOC( VersionInfoSize );
	if (pVersionData==NULL) goto End;

	if (!GetFileVersionInfoW(pModulePath, 0, VersionInfoSize, pVersionData)) goto End;

	if (!VerQueryValueW( pVersionData, L"\\", (VOID**)&pFixedFileInfo, &Size )) goto End;
	ModuleVersion.HighPart	= pFixedFileInfo->dwProductVersionMS;
	ModuleVersion.LowPart	= pFixedFileInfo->dwProductVersionLS;
End:
	if (pVersionData!=NULL) OS_FREE(pVersionData);
	return (ModuleVersion.QuadPart);
}

//------------------------------------------------------------------------------
BOOL WINAPI GetProcessPebInfo(HANDLE hProcess,WCHAR* pCmdLine,UINT CmdLineLength,WCHAR* pDesktop,UINT DesktopLength)
{
	PROCESS_PARAMETERS			ProcessParameters;
	NTSTATUS					Status;
	PEB							Peb;
	WCHAR						TmpBuffer[MAX_PATH];
	BOOL						bResult=FALSE;
	PROCESS_BASIC_INFORMATION	ProcessBasicInfo;
	//
	// Get process Basic Info
	//
	Status=NtQueryInformationProcess(hProcess,ProcessBasicInformation,&ProcessBasicInfo,sizeof(ProcessBasicInfo),NULL);
	if (Status != STATUS_SUCCESS) goto End;
	//
	// Read the PEB
	//
	if (!ReadProcessMemory(hProcess,ProcessBasicInfo.PebBaseAddress,&Peb,sizeof(Peb),NULL)) goto End;
	//
	// Read the process parameters
	//
	ZeroMemory(&ProcessParameters,sizeof(ProcessParameters));
	if (!ReadProcessMemory(hProcess,Peb.ProcessParameters,&ProcessParameters,sizeof(ProcessParameters),NULL)) goto End;
	//
	// Read the CmdLine
	//
	if (!ReadProcessMemory(hProcess,ProcessParameters.CommandLine.Buffer,TmpBuffer,sizeof(TmpBuffer),NULL)) goto End;
	StringCbPrintfW(pCmdLine,CmdLineLength,L"%s",TmpBuffer);
	//
	// Read the Desktop
	//
	if (!ReadProcessMemory(hProcess,ProcessParameters.Desktop.Buffer,TmpBuffer,sizeof(TmpBuffer),NULL)) goto End;
	StringCbPrintfW(pDesktop,DesktopLength,L"%s",TmpBuffer);
	bResult=TRUE;
End:
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetProcessNameFromPid(DWORD Pid,WCHAR* pName,UINT NameSizeInBytes)
{
	HANDLE			hProcessSnapshot	= INVALID_HANDLE_VALUE;
	BOOL			bResult				= FALSE;
	PROCESSENTRY32W	ProcessEntry;
	
	hProcessSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnapshot==INVALID_HANDLE_VALUE) goto End;
	ProcessEntry.dwSize=sizeof(ProcessEntry);
	if (!Process32FirstW(hProcessSnapshot,&ProcessEntry)) goto End;
	do
	{
		if (Pid==ProcessEntry.th32ProcessID)
		{
			StringCbPrintfW(pName,NameSizeInBytes,L"%s",ProcessEntry.szExeFile);
			break;
		}
	}while( Process32NextW(hProcessSnapshot,&ProcessEntry) );
	bResult=TRUE;
End:
	if (hProcessSnapshot!=INVALID_HANDLE_VALUE) CloseHandle(hProcessSnapshot);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetProcessPath(DWORD Pid, WCHAR* pProcessPath, DWORD ProcessPathLength)
{
	HANDLE	hProcess;
	BOOL	bResult = FALSE;
	DWORD	Size;

	hProcess = ProcexpOpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
	if (hProcess == NULL) goto End;
	Size = ProcessPathLength;
	bResult = QueryFullProcessImageNameW(hProcess, 0, pProcessPath, &Size);
End:	
	if (hProcess!=NULL) CloseHandle(hProcess);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetRegValueData(HKEY hRootKey,WCHAR* pSubkeyName,WCHAR* pValueName,VOID* pData,UINT DataLength)
{
	DWORD	Size;
	HKEY	hKey	= NULL;
	BOOL	bResult	= FALSE; 
	
	if (RegOpenKeyExW(hRootKey,pSubkeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS) goto End;
	Size = DataLength;
	if (RegQueryValueExW(hKey,pValueName,NULL,NULL,(LPBYTE)pData,&Size)!=ERROR_SUCCESS) goto End;
	bResult=TRUE;
End:
	if (hKey!=NULL) RegCloseKey(hKey);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetUserAndDomainName(DWORD Pid, WCHAR* Buffer, ULONG BufferLengthInBytes)
{
	HANDLE			hProcess	= NULL;
	HANDLE			hToken		= NULL;
	DWORD			Bytes;
	TOKEN_USER*		pTokenUser=NULL;
	WCHAR			UserName[RPC_MAX_LENGTH];
	WCHAR			DomainName[RPC_MAX_LENGTH];
	DWORD			dwSize;
	SID_NAME_USE	SidType;
	BOOL			bResult = FALSE;

	hProcess = ProcexpOpenProcess(PROCESS_VM_OPERATION|PROCESS_QUERY_INFORMATION,FALSE,Pid);
	if (hProcess==NULL) goto End;

	if (!OpenProcessToken(hProcess,TOKEN_QUERY,&hToken)) goto End;
	GetTokenInformation(hToken,TokenUser,NULL,0,&Bytes);
	pTokenUser=(TOKEN_USER*)OS_ALLOC(Bytes);
	if (pTokenUser==NULL) goto End;
	if (!GetTokenInformation(hToken,TokenUser,pTokenUser,Bytes,&Bytes)) goto End;
	dwSize=_countof(UserName);
	if (!LookupAccountSidW(NULL,pTokenUser->User.Sid,UserName,&dwSize,DomainName,&dwSize,&SidType)) goto End;
	StringCbPrintfW(Buffer,BufferLengthInBytes,L"%s\\%s",DomainName,UserName);
	bResult=TRUE;
End:
	if (pTokenUser!=NULL)	OS_FREE(pTokenUser);
	if (hToken!=NULL)		CloseHandle(hToken);
	if (hProcess!=NULL)		CloseHandle(hProcess);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI IsProcessWow64(ULONG Pid)
{
	BOOL	bWow64		= FALSE;
	HANDLE	hProcess	= NULL;

	hProcess = ProcexpOpenProcess(PROCESS_VM_OPERATION|PROCESS_QUERY_INFORMATION,FALSE,Pid);
	if (hProcess==NULL) goto End;
	IsWow64Process(hProcess,&bWow64);
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	return (bWow64);
}