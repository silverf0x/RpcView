#define SECURITY_WIN32

#include <windows.h>
#include <strsafe.h>
#include <Sspi.h>
#include "RpcCore.h"
#include "..\RpcCommon\ntdll.h"
#include <Tlhelp32.h>
#include <conio.h>
#include <Userenv.h>
#include <RpcInternals.h>
#include "..\..\RpcCommon\RpcCommon.h"
#include "..\..\RpcCommon\Misc.h"
#include <Psapi.h>

#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"Secur32.lib")
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Userenv.lib")

#define MAX_SIMPLE_DICT_ENTRIES			0x200
#define RPC_MAX_ENDPOINT_PROTOCOL_SIZE	0x100
#define RPC_MAX_ENDPOINT_NAME_SIZE		0x100
#define RPC_MAX_DLL_NAME_SIZE			0x100
#define RPC_IS_EPMAPPER_REGISTERED		0x20


////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////
typedef struct _RpcCoreInternalCtxt_T{
	VOID*	pGlobalRpcServer;
	UINT64	ModuleVersion;
}RpcCoreInternalCtxt_T;

typedef struct _ModuleSectionInfo_T{
	VOID*	pBase;
	UINT	Size;
}ModuleSectionInfo_T;


////////////////////////////////////////////////////////////////////////////////
// Functions declarations
////////////////////////////////////////////////////////////////////////////////

// Misc
BOOL	WINAPI GetModuleDataSection(HANDLE hProcess,VOID* pModule,ModuleSectionInfo_T* pModuleSectionInfo);
VOID*	WINAPI GetProcessInterface(void* pRpcCoreCtxt,HANDLE hProcess,RPC_IF_ID* pIf);

typedef BOOL (WINAPI* EnumSimpleDictCallbackFn_T)(HANDLE hProcess, UINT Index, VOID PTR_T pSimpleDictEntry, VOID* pContext, BOOL* pbContinue);
BOOL	WINAPI EnumSimpleDict(HANDLE hProcess, SIMPLE_DICT_T* pSimpleDict, EnumSimpleDictCallbackFn_T EnumSimpleDictCallbackFn, VOID* pContext);

// RpcCore
VOID*				__fastcall RpcCoreInit(BOOL bForce);						//returns a private context for the RpcCoreEngine
VOID				__fastcall RpcCoreUninit(VOID* pRpcCoreCtxt);
RpcProcessInfo_T*	__fastcall RpcCoreGetProcessInfo(void* pRpcCoreCtxt,DWORD Pid,DWORD Ppid,ULONG ProcessInfoMask);
VOID				__fastcall RpcCoreFreeProcessInfo(void* pRpcCoreCtxt,RpcProcessInfo_T* pRpcProcessInfo);
RpcInterfaceInfo_T*	__fastcall RpcCoreGetInterfaceInfo(void* pRpcCoreCtxt,DWORD Pid,RPC_IF_ID* pIf,ULONG InterfaceInfoMask);
VOID				__fastcall RpcCoreFreeInterfaceInfo(void* pRpcCoreCtxt,RpcInterfaceInfo_T* pRpcInterfaceInfo);
BOOL				__fastcall RpcCoreEnumProcessInterfaces(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessInterfacesCallbackFn_T RpcCoreEnumProcessInterfacesCallbackFn,void* pCallbackCtxt,ULONG InterfaceInfoMask);
BOOL				__fastcall RpcCoreEnumProcessEndpoints(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessEndpointsCallbackFn_T RpcCoreEnumProcessEndpointsCallbackFn,void* pCallbackCtxt);
BOOL				__fastcall RpcCoreEnumProcessAuthInfo(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessAuthInfoCallbackFn_T RpcCoreEnumProcessAuthInfoCallbackFn,void* pCallbackCtxt);

////////////////////////////////////////////////////////////////////////////////
// Global declarations
////////////////////////////////////////////////////////////////////////////////

WCHAR		gInterfaceFile[]			= L"RpcView.ini";
const char	gRpcCoreDataSectionName[]	= ".data";

RpcCore_T	RpcCoreHelper =
{
	RPC_CORE_RUNTIME_VERSION,
	RPC_CORE_IS_WOW64,
	FALSE,
	&RpcCoreInit,
	&RpcCoreUninit,
	&RpcCoreGetProcessInfo,
	&RpcCoreFreeProcessInfo,
	&RpcCoreGetInterfaceInfo,
	&RpcCoreFreeInterfaceInfo,
	&RpcCoreEnumProcessInterfaces,
	&RpcCoreEnumProcessEndpoints,
	&RpcCoreEnumProcessAuthInfo,
};


const RPC_IF_ID	gIorCallback =
{
	{ 0x18f70770, 0x8e64, 0x11cf, { 0x9a, 0xf1, 0x00, 0x20, 0xAF, 0x6E, 0x72, 0xF4 } },	// Uuid
	0,																					// VersMajor
	0																					// VersMinor
};

//------------------------------------------------------------------------------
BOOL WINAPI EnumSimpleDict(HANDLE hProcess, SIMPLE_DICT_T* pSimpleDict, EnumSimpleDictCallbackFn_T EnumSimpleDictCallbackFn, VOID* pContext)
{
	//WARNING : in case of WOW64 core, pTable is an array of 32-bit pointers for which each address is itself a 64-bit pointer
	VOID PTR_T *	pTable=NULL;
	UINT			i;
	UINT			Size;
	BOOL			bResult = TRUE;
	BOOL			bContinue = TRUE; 	//Assume continue

	if ( pSimpleDict->NumberOfEntries > MAX_SIMPLE_DICT_ENTRIES)
	{
		goto End;
	}

	Size=pSimpleDict->NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;
	ZeroMemory(pTable,Size);

	if (!ReadProcessMemory(hProcess,pSimpleDict->pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<pSimpleDict->NumberOfEntries; i++)
	{
		bResult = EnumSimpleDictCallbackFn(hProcess,i,pTable[i],pContext,&bContinue);
		if (!bResult) goto End;
		if (!bContinue) break;
	}
	bResult=TRUE;
End:
	if (pTable!=NULL) OS_FREE(pTable);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetModuleDataSection(HANDLE hProcess,VOID* pModule,ModuleSectionInfo_T* pModuleSectionInfo)
{
	IMAGE_DOS_HEADER		ImageDosHeader;
	IMAGE_NT_HEADERS		ImageNtHeaders;
	IMAGE_SECTION_HEADER*	pImageSectionHeader;
	UINT					SectionIdx;
	IMAGE_SECTION_HEADER	ImageSectionHeader;
	UCHAR*					pBase			= (UCHAR*)pModule;
	BOOL					bResult			= FALSE;

	if (!ReadProcessMemory(hProcess,pBase,&ImageDosHeader,sizeof(ImageDosHeader), NULL)) goto End;
	if (ImageDosHeader.e_magic!=IMAGE_DOS_SIGNATURE) goto End;
	if (!ReadProcessMemory(hProcess,pBase+ImageDosHeader.e_lfanew,&ImageNtHeaders,sizeof(ImageNtHeaders), NULL)) goto End;
	if (ImageNtHeaders.Signature!=IMAGE_NT_SIGNATURE) goto End;
	//
	// For 64bits, add the size of IMAGE_NT_HEADERS64 to its own base to get the base of the IMAGE_SECTION_HEADER table
	//
#ifdef _WIN64
#pragma warning(push)
#pragma warning(disable:4127)
	if (RPC_CORE_IS_WOW64==TRUE) pImageSectionHeader = (IMAGE_SECTION_HEADER*) ((UINT_PTR)pBase+ImageDosHeader.e_lfanew+sizeof(IMAGE_NT_HEADERS32));
#pragma warning(pop)
	else 
#endif		
	pImageSectionHeader = (IMAGE_SECTION_HEADER*) ((UINT_PTR)pBase+ImageDosHeader.e_lfanew+sizeof(IMAGE_NT_HEADERS));
	
	for(SectionIdx=0; SectionIdx<ImageNtHeaders.FileHeader.NumberOfSections; SectionIdx++)
	{
		if (!ReadProcessMemory(hProcess,&pImageSectionHeader[SectionIdx],&ImageSectionHeader,sizeof(ImageSectionHeader),NULL)) goto End;
		if (strncmp((const char*)ImageSectionHeader.Name,gRpcCoreDataSectionName,sizeof(gRpcCoreDataSectionName))==0)
		{
			pModuleSectionInfo->pBase	= pBase + ImageSectionHeader.VirtualAddress;
			pModuleSectionInfo->Size	= ImageSectionHeader.Misc.VirtualSize;
			bResult=TRUE;
			break;
		}
	}
End:
	return (bResult);
}


typedef struct{
	BOOL            bFound;
    PRPC_SERVER_T   pRpcServer;
}GetRpcServerAddressCallbackCtxt_T;


//8a885d04-1ceb-11c9-9fe8-08002b104860 V2.0
RPC_SYNTAX_IDENTIFIER	DceRpcSyntaxUuid =
{
	{ 0x8a885d04,0x1ceb,0x11c9,{ 0x9f,0xe8,0x08,0x00,0x2b,0x10,0x48,0x60 } },
	{ 2,0 }
};


//------------------------------------------------------------------------------
BOOL WINAPI GetRpcServerAddressCallback(HANDLE hProcess, UINT Index, VOID PTR_T pSimpleDictEntry, VOID* pContext, BOOL* pbContinue)
{
	RPC_INTERFACE_T						RpcInterface;
	GetRpcServerAddressCallbackCtxt_T*	pGetRpcServerAddressCallbackCtxt=(GetRpcServerAddressCallbackCtxt_T*)pContext;

    UNREFERENCED_PARAMETER(Index);

	if (!ReadProcessMemory(hProcess,pSimpleDictEntry,&RpcInterface,sizeof(RpcInterface),NULL)) goto End;
	
	if ( (RpcInterface.RpcServerInterface.Length==sizeof(RPC_SERVER_INTERFACE_T)) &&
		(!memcmp(&RpcInterface.RpcServerInterface.TransferSyntax, &DceRpcSyntaxUuid, sizeof(DceRpcSyntaxUuid))) &&
        RpcInterface.pRpcServer == pGetRpcServerAddressCallbackCtxt->pRpcServer)
	{
		pGetRpcServerAddressCallbackCtxt->bFound = TRUE;
		*pbContinue=FALSE;
	}
End:
	return (TRUE);
}


//------------------------------------------------------------------------------
BOOL WINAPI GetRpcServerAddressInProcess(DWORD Pid,RpcCoreInternalCtxt_T* pRpcCoreInternalCtxt)
{
	VOID PTR_T PTR_T					pCandidate;
	VOID PTR_T							pRpcServer;
	RPC_SERVER_T						RpcServer;
	ModuleSectionInfo_T					ModuleSectionInfo;
	HANDLE								hProcess = NULL;
	GetRpcServerAddressCallbackCtxt_T	GetRpcServerAddressCallbackCtxt;
	DWORD								cbSize;
	HMODULE*							pHmodule = NULL;
	CHAR								ModuleFileName[MAX_PATH];
	BOOL								bResult=FALSE;

	hProcess = ProcexpOpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
	if (hProcess == NULL) goto End;

    if (!EnumProcessModulesEx(hProcess, NULL, 0, &cbSize, LIST_MODULES_ALL)) goto End;
	pHmodule = (HMODULE*)malloc(cbSize);
    if (pHmodule == NULL) goto End;
    if (!EnumProcessModulesEx(hProcess, pHmodule, cbSize, &cbSize, LIST_MODULES_ALL)) goto End;

	for(ULONG i=0;i<cbSize/sizeof(*pHmodule);i++)
	{
		ModuleFileName[0] = 0;
		GetModuleFileNameExA(hProcess, pHmodule[i], ModuleFileName, sizeof(ModuleFileName));
		if (strstr(ModuleFileName,"RPCRT4.dll")==NULL) goto NextModule;

		if (!GetModuleDataSection(hProcess, pHmodule[i],&ModuleSectionInfo)) goto End;
#pragma warning(push)
#pragma warning(disable:4305)
		pCandidate=(VOID PTR_T PTR_T)ModuleSectionInfo.pBase;
#pragma warning(pop)
        GetRpcServerAddressCallbackCtxt.bFound=FALSE;
		
		for (i=0; i<ModuleSectionInfo.Size; i+=sizeof(VOID PTR_T))
		{
			if (!ReadProcessMemory(hProcess,pCandidate,&pRpcServer,sizeof(VOID PTR_T),NULL))	goto NextCandidate;
			if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL))		goto NextCandidate;
            GetRpcServerAddressCallbackCtxt.pRpcServer = pRpcServer;
			if (!EnumSimpleDict(hProcess,&RpcServer.InterfaceDict,&GetRpcServerAddressCallback,&GetRpcServerAddressCallbackCtxt)) goto End;
			if (GetRpcServerAddressCallbackCtxt.bFound==TRUE)
			{
				_cprintf("gRpcServer localized at address %p in process %u\n",pCandidate,Pid);
				pRpcCoreInternalCtxt->pGlobalRpcServer=pCandidate;
				bResult=TRUE;
				break;
			}
NextCandidate:		
			pCandidate++;
		}
		CloseHandle(hProcess);
		hProcess=NULL;
		goto End;
NextModule: ;
	}
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	if (pHmodule !=NULL) free(pHmodule);
	return (bResult);
}



//------------------------------------------------------------------------------
VOID* __fastcall RpcCoreInit(BOOL bForce)
{
	UINT64					RuntimVersion;
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt=NULL;
	WCHAR					RpcRuntimePath[MAX_PATH];
	UINT					i;
	BOOL					bFound = FALSE;

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#endif
	//
	// Get the RPC runtime version (rpcrt4.dll) to verify if it is supported by this Core
	//
	if (GetSystemDirectoryW(RpcRuntimePath,_countof(RpcRuntimePath))==0) goto End;
	StringCbPrintfW(RpcRuntimePath,sizeof(RpcRuntimePath),L"%s\\rpcrt4.dll",RpcRuntimePath);
	RuntimVersion=GetModuleVersion(RpcRuntimePath);
	for (i = 0; i < _countof(RPC_CORE_RUNTIME_VERSION); i++)
	{
		if (bForce && ((RuntimVersion & 0xFFFFFFFF00000000) == (RPC_CORE_RUNTIME_VERSION[i] & 0xFFFFFFFF00000000)))
		{
			bFound = TRUE;
			break;
		}
		if (RuntimVersion == RPC_CORE_RUNTIME_VERSION[i])
		{
			bFound = TRUE;
			break;
		}
	}
	if (!bFound) goto End;
	//
	// Invoke CoInitialize, required by SHGetFileInfo
	//
	CoInitialize(NULL);
	//
	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)OS_ALLOC(sizeof(RpcCoreInternalCtxt_T));
	if (pRpcCoreInternalCtxt==NULL) goto End;
	
	pRpcCoreInternalCtxt->ModuleVersion = RuntimVersion;
	//
	// Try to activate the DEBUG privilege to access all processes
	//
	AdjustPrivilege(SE_DEBUG_NAME,TRUE);
End:
	return (pRpcCoreInternalCtxt);
}


//------------------------------------------------------------------------------
VOID __fastcall	RpcCoreUninit(VOID* pRpcCoreCtxt)
{
	OS_FREE(pRpcCoreCtxt);
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
}


//------------------------------------------------------------------------------
RpcProcessType_T __fastcall GetProcessType(HANDLE hProcess, RPC_SERVER_T* pRpcServer)
{
	SIZE_T				Size;
	ULONG				i;
	RPC_INTERFACE_T		RpcInterface;
	RpcProcessType_T	RpcProcessType		= RpcProcessType_UNKNOWN;
	VOID PTR_T *		pTable				= NULL;

	if (pRpcServer==NULL) goto End;

	Size=pRpcServer->InterfaceDict.NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;

	if (!ReadProcessMemory(hProcess,pRpcServer->InterfaceDict.pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<pRpcServer->InterfaceDict.NumberOfEntries; i++)
	{
		if (!ReadProcessMemory(hProcess,pTable[i],&RpcInterface,sizeof(RpcInterface),NULL)) goto End;
		if ((RpcInterface.Flags & RPC_IF_OLE) || (memcmp(&RpcInterface.RpcServerInterface.InterfaceId, &gIorCallback, sizeof(RPC_IF_ID)) == 0))
		{
			//The interface is DCOM
			if ( (RpcProcessType==RpcProcessType_UNKNOWN) || (RpcProcessType==RpcProcessType_DCOM) )
			{
				RpcProcessType = RpcProcessType_DCOM;
			}
			else
			{
				RpcProcessType = RpcProcessType_HYBRID;
				goto End; //we can stop now
			}
		}
		else
		{
			//The interface is RPC
			if ( (RpcProcessType==RpcProcessType_UNKNOWN) || (RpcProcessType==RpcProcessType_RPC) )
			{
				RpcProcessType = RpcProcessType_RPC;
			}
			else
			{
				RpcProcessType = RpcProcessType_HYBRID;
				goto End; //we can stop now
			}
		}
	}
End:
	if (pTable!=NULL) OS_FREE(pTable);
	return (RpcProcessType);
}


//------------------------------------------------------------------------------
RpcProcessInfo_T* __fastcall RpcCoreGetProcessInfo(void* pRpcCoreCtxt,DWORD Pid,DWORD Ppid,ULONG ProcessInfoMask)
{
	SHFILEINFOW				ShFileInfo;
	HANDLE					hProcess;
	VOID PTR_T				pRpcServer;
	RPC_SERVER_T			RpcServer;
	RpcProcessInfo_T*		pRpcProcessInfo=NULL;
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)pRpcCoreCtxt;

	pRpcProcessInfo=(RpcProcessInfo_T*)OS_ALLOC(sizeof(RpcProcessInfo_T));
	if (pRpcProcessInfo==NULL) return (NULL);
	//
	// Process minimal info
	//
	pRpcProcessInfo->Pid			= Pid;
	pRpcProcessInfo->ParentPid		= Ppid;
	pRpcProcessInfo->RpcProcessType = RpcProcessType_UNKNOWN;

	hProcess=ProcexpOpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,FALSE,Pid);
	if (hProcess!=NULL)
	{
#ifdef _WIN64
	IsWow64Process(hProcess, &pRpcProcessInfo->bIsWow64);
#endif
	}
	//
	// Process general information
	//
	if (ProcessInfoMask & RPC_PROCESS_INFO_MISC)
	{
		GetProcessNameFromPid(Pid,pRpcProcessInfo->Name,sizeof(pRpcProcessInfo->Name));	
		GetProcessPath(Pid,pRpcProcessInfo->Path,sizeof(pRpcProcessInfo->Path));
		pRpcProcessInfo->Version=GetModuleVersion(pRpcProcessInfo->Path);
		GetModuleDescription(pRpcProcessInfo->Path,pRpcProcessInfo->Description,sizeof(pRpcProcessInfo->Description));
		GetUserAndDomainName(pRpcProcessInfo->Pid,pRpcProcessInfo->User,sizeof(pRpcProcessInfo->User));
		GetProcessPebInfo(hProcess,pRpcProcessInfo->CmdLine,sizeof(pRpcProcessInfo->CmdLine),pRpcProcessInfo->Desktop,sizeof(pRpcProcessInfo->Desktop));
		// Get icon
		if (hProcess!=NULL)
		{
			ZeroMemory(&ShFileInfo,sizeof(ShFileInfo));
			if (SHGetFileInfoW(pRpcProcessInfo->Path,0,&ShFileInfo,sizeof(ShFileInfo),SHGFI_ICON|SHGFI_LARGEICON))
			{
				pRpcProcessInfo->hIcon=ShFileInfo.hIcon;
			}
		}
	}
	//
	// Process RPC information
	//
	if (ProcessInfoMask & RPC_PROCESS_INFO_RPC)
	{
		if (pRpcCoreInternalCtxt->pGlobalRpcServer==NULL)
		{
			GetRpcServerAddressInProcess(pRpcProcessInfo->Pid,pRpcCoreInternalCtxt);
			if (pRpcCoreInternalCtxt->pGlobalRpcServer==NULL) goto End;
		}
		if (!ReadProcessMemory(hProcess,pRpcCoreInternalCtxt->pGlobalRpcServer,&pRpcServer,sizeof(VOID PTR_T),NULL)) goto End;
		if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL)) goto End;

		//If the number of endpoints is correct we have a RPC server
		if (RpcServer.AddressDict.NumberOfEntries!=0)
		{
			pRpcProcessInfo->RpcProcessType		= GetProcessType(hProcess,&RpcServer);
			pRpcProcessInfo->bIsServer			= TRUE;
			pRpcProcessInfo->EndpointsCount		= RpcServer.AddressDict.NumberOfEntries;
			pRpcProcessInfo->SspCount			= RpcServer.AuthenInfoDict.NumberOfEntries;
			pRpcProcessInfo->InterfacesCount	= RpcServer.InterfaceDict.NumberOfEntries;
			pRpcProcessInfo->InCalls			= RpcServer.InCalls;
			pRpcProcessInfo->OutCalls			= RpcServer.OutCalls;
			pRpcProcessInfo->InPackets			= RpcServer.InPackets;
			pRpcProcessInfo->OutPackets			= RpcServer.OutPackets;
			pRpcProcessInfo->bIsListening		= RpcServer.bIsListening;
			pRpcProcessInfo->MaxCalls			= RpcServer.MaxCalls;
		}
	}
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	return (pRpcProcessInfo);
}


//------------------------------------------------------------------------------
VOID __fastcall RpcCoreFreeProcessInfo(void* pRpcCoreCtxt,RpcProcessInfo_T* pRpcProcessInfo)
{
    UNREFERENCED_PARAMETER(pRpcCoreCtxt);
	OS_FREE(pRpcProcessInfo);
}


//------------------------------------------------------------------------------
void GetInlineStubInfo(RpcCoreInternalCtxt_T* pRpcCoreInternalCtxt,HANDLE hProcess,VOID* pRpcInterface,RpcInterfaceInfo_T* pRpcInterfaceInfo)
{
	RPC_INTERFACE_T			RpcInterface;
	RPC_DISPATCH_TABLE_T	RpcDispatchTbl;
	VOID PTR_T				pProc;
	UINT					i;

    UNREFERENCED_PARAMETER(pRpcCoreInternalCtxt);
	
    if (pRpcInterfaceInfo==NULL) goto End;
	if (!ReadProcessMemory(hProcess,pRpcInterface,&RpcInterface,sizeof(RpcInterface),NULL)) goto End;
	//
	// Get the interface procedures count
	//
	if (!ReadProcessMemory(hProcess,RpcInterface.RpcServerInterface.DispatchTable,&RpcDispatchTbl,sizeof(RpcDispatchTbl),NULL)) goto End;
	
	if (pRpcInterfaceInfo->IfType==IfType_RPC)
	{
		pRpcInterfaceInfo->NumberOfProcedures=RpcDispatchTbl.DispatchTableCount;
		pRpcInterfaceInfo->TypeOfStub=TypeOfStub_Inlined;
		//
		// Get the interface procedure addresses table
		//
		pRpcInterfaceInfo->ppProcAddressTable=(ULONG*)OS_ALLOC(pRpcInterfaceInfo->NumberOfProcedures * sizeof(VOID *));
		if (pRpcInterfaceInfo->ppProcAddressTable==NULL) goto End;

		pRpcInterfaceInfo->pFormatStringOffsetTable = (USHORT*)OS_ALLOC(pRpcInterfaceInfo->NumberOfProcedures * sizeof(USHORT));
		if (pRpcInterfaceInfo->pFormatStringOffsetTable==NULL) goto End;

		for(i=0;i<pRpcInterfaceInfo->NumberOfProcedures;i++)
		{
			pProc=NULL;
			if (!ReadProcessMemory(hProcess,((VOID PTR_T PTR_T)RpcDispatchTbl.DispatchTable)+i,&pProc,sizeof(VOID PTR_T),NULL)) goto End;
			pRpcInterfaceInfo->ppProcAddressTable[i]=(ULONG)((ULONG_PTR)pProc - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
		}
	}
End:
	return;
}

#if (RPC_CORE_IS_WOW64==TRUE)
#define REG_PREFIX	L"Wow6432Node\\"
#else
#define REG_PREFIX L""
#endif


//------------------------------------------------------------------------------
BOOL NTAPI GetComInterfaceName(const RPC_IF_ID* pInterfaceId, WCHAR* pName, UINT NameLength)
{
    WCHAR		SubKeyName[MAX_PATH];
    RPC_WSTR	pUuidString = NULL;
    BOOL		bResult = FALSE;

    if (UuidToStringW(&pInterfaceId->Uuid, &pUuidString) != RPC_S_OK) goto End;
    StringCbPrintfW(SubKeyName, sizeof(SubKeyName), REG_PREFIX L"Interface\\{%s}", pUuidString);
    if (GetRegValueData(HKEY_CLASSES_ROOT, SubKeyName, NULL, pName, NameLength) == FALSE) goto End;

    bResult = TRUE;
End:
    if (pUuidString != NULL) RpcStringFreeW(&pUuidString);
    return (bResult);
}


//------------------------------------------------------------------------------
IfType_T WINAPI GetInterfaceType(const RPC_INTERFACE_T* pRpcInterface)
{
	WCHAR	TmpPath[MAX_PATH];

	if ( pRpcInterface->Flags & RPC_IF_OLE )
	{
		return (IfType_OLE);
	}
	
	if ( GetComInterfaceName(&pRpcInterface->RpcServerInterface.InterfaceId,TmpPath,sizeof(TmpPath)) == FALSE )
	{
		return (IfType_RPC);
	}
	return (IfType_DCOM);
}


//------------------------------------------------------------------------------
BOOL __fastcall GetInterfaceName(RPC_IF_ID* pInterfaceId,WCHAR* pName,UINT NameLength)
{
	WCHAR		FullPath[MAX_PATH];
	RPC_WSTR	pUuidString=NULL;
	BOOL		bResult=FALSE;

	if (pInterfaceId==NULL) goto End;

	pName[0]=0;
	bResult = GetComInterfaceName(pInterfaceId,pName,NameLength);
	if (bResult==FALSE)
	{
		UuidToStringW(&pInterfaceId->Uuid,&pUuidString);
		GetFullPathNameW(gInterfaceFile,_countof(FullPath),FullPath,NULL);
		GetPrivateProfileStringW((LPCWSTR)pUuidString,L"Name",NULL,pName,NameLength/sizeof(WCHAR),FullPath);
	}
End:
	if (pUuidString!=NULL) RpcStringFreeW(&pUuidString);
	return (bResult);
}


//------------------------------------------------------------------------------
RpcInterfaceInfo_T*	WINAPI InternalGetInterfaceInfo(RpcCoreInternalCtxt_T* pRpcCoreInternalCtxt, HANDLE hProcess, VOID* pRpcInterface, DWORD Pid, ULONG InterfaceInfoMask)
{
	MIDL_SERVER_INFO_T		MidlServerInfo;
	MIDL_STUB_DESC_T		MidlStubDesc;
	RPC_DISPATCH_TABLE_T	RpcDispatchTbl;
	RPC_INTERFACE_T			RpcInterface;
	MODULEENTRY32W			ModuleEntry;
	UINT					i;
	NDR_EXPR_DESC_T			NdrExprDesc;
	VOID PTR_T				pProc = NULL;
	WCHAR*					pModuleName = NULL;
	HANDLE					hModulesSnapshot = NULL;
	RpcInterfaceInfo_T*		pRpcInterfaceInfo = NULL;
	LocationInfo_T			LocationInfo = {0};

    UNREFERENCED_PARAMETER(pRpcCoreInternalCtxt);
	//
	// Get the RPC_INTERFACE
	//
	if (!ReadProcessMemory(hProcess, pRpcInterface, &RpcInterface, sizeof(RpcInterface), NULL))
	{
		//printf("ReadProcessMemory failed RpcInterface\n");
		goto End; 
	}

	pRpcInterfaceInfo=(RpcInterfaceInfo_T*)OS_ALLOC(sizeof(RpcInterfaceInfo_T));
	if (pRpcInterfaceInfo == NULL) { DEBUG_BREAK(); goto End; }
	//
	// Get Default interface information
	// - Pid
	// - Flags
	// - Type
	// - UUID
	// - Transfert Syntax GUID
	// - Name
	//
#ifdef _WIN64
	pRpcInterfaceInfo->bWow64Process = RPC_CORE_IS_WOW64;
#endif
	pRpcInterfaceInfo->Pid		= Pid;
	pRpcInterfaceInfo->Flags	= RpcInterface.Flags;
	pRpcInterfaceInfo->IfType	= GetInterfaceType(&RpcInterface);
	//
	memcpy(&pRpcInterfaceInfo->If,&RpcInterface.RpcServerInterface.InterfaceId,sizeof(pRpcInterfaceInfo->If));
	memcpy(&pRpcInterfaceInfo->TransfertSyntax,&RpcInterface.RpcServerInterface.TransferSyntax,sizeof(pRpcInterfaceInfo->TransfertSyntax));
	//
	GetInterfaceName(&RpcInterface.RpcServerInterface.InterfaceId,pRpcInterfaceInfo->Name,sizeof(pRpcInterfaceInfo->Name));
	
	//!!!!!!to change!!!!!!
	if (InterfaceInfoMask == 0) goto End;

	switch(pRpcInterfaceInfo->IfType)
	{
		case IfType_DCOM:
		case IfType_OLE:
			goto End;	//NO DCOM now!!
		break;
		//
		case IfType_RPC:
		{
			if (GetLocationInfo(hProcess, (void*)RpcInterface.RpcServerInterface.DispatchTable, &LocationInfo))
			{
				memcpy(pRpcInterfaceInfo->Location, LocationInfo.Location, sizeof(pRpcInterfaceInfo->Location));
			}
			pRpcInterfaceInfo->pLocationBase = LocationInfo.pBaseAddress;
			pRpcInterfaceInfo->LocationState = LocationInfo.State;
			pRpcInterfaceInfo->LocationType = LocationInfo.Type;
			pRpcInterfaceInfo->LocationSize = LocationInfo.Size;
		}
		break;
		//
		default:
		break;
	}
	//
	// If we have a module path then we can get its real base and size
	//
	if (LocationInfo.Location[0] != 0)
	{
		pModuleName = wcsrchr(pRpcInterfaceInfo->Location, '\\');
		if (pModuleName == NULL) pModuleName = pRpcInterfaceInfo->Location;
		else pModuleName++;
		//
		// Get module base and size
		//
		hModulesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, Pid);
		if (hModulesSnapshot != INVALID_HANDLE_VALUE)
		{
			ModuleEntry.dwSize = sizeof(ModuleEntry);
			if (Module32FirstW(hModulesSnapshot, &ModuleEntry))
			do
			{
				if (_wcsicmp(pModuleName, ModuleEntry.szModule) == 0)
				{
					pRpcInterfaceInfo->pLocationBase	= ModuleEntry.modBaseAddr;
					pRpcInterfaceInfo->LocationSize		= ModuleEntry.modBaseSize;
					break;
				}
			} while (Module32NextW(hModulesSnapshot, &ModuleEntry));
			CloseHandle(hModulesSnapshot);
		}
		else
		{
            ULONG_PTR			Base = 0;
			WORD				Magic = 0;
			IMAGE_DOS_HEADER	ImageDosHeader;
			IMAGE_NT_HEADERS	ImageNtHeaders;
			//
			// Last chance approach
			//
			Base = ((ULONG_PTR)pRpcInterfaceInfo->pLocationBase) & (~0xFFFF);
			while (Magic != IMAGE_DOS_SIGNATURE)
			{
				ReadProcessMemory(hProcess, (PVOID)Base, &Magic, sizeof(Magic), NULL);
				Base -= 0x1000;
			}
			pRpcInterfaceInfo->pLocationBase = (PVOID)(Base + 0x1000);
			ReadProcessMemory(hProcess, pRpcInterfaceInfo->pLocationBase, &ImageDosHeader, sizeof(ImageDosHeader), NULL);
			ReadProcessMemory(hProcess, (PVOID)((ULONG_PTR)pRpcInterfaceInfo->pLocationBase + ImageDosHeader.e_lfanew), &ImageNtHeaders, sizeof(ImageNtHeaders), NULL);
		}
	}
	//
	// Get module display name
	//
	GetModuleDescription(pRpcInterfaceInfo->Location,pRpcInterfaceInfo->Description,sizeof(pRpcInterfaceInfo->Description));
	//
	// Get the interface procedures count ony for RPC interfaces
	//
	if (pRpcInterfaceInfo->IfType == IfType_RPC)
	{
		if (ReadProcessMemory(hProcess,RpcInterface.RpcServerInterface.DispatchTable,&RpcDispatchTbl,sizeof(RpcDispatchTbl),NULL))
		{
			pRpcInterfaceInfo->NumberOfProcedures=RpcDispatchTbl.DispatchTableCount;
		}
		else
		{
			ZeroMemory(pRpcInterfaceInfo, sizeof(*pRpcInterfaceInfo));
			goto End;
		}
	}
	//
	// Interpreted stub
	//
	if (RpcInterface.RpcServerInterface.InterpreterInfo!=NULL)
	{
		if (pRpcInterfaceInfo->IfType==IfType_RPC)
		{
			pRpcInterfaceInfo->TypeOfStub=TypeOfStub_Interpreted;
		}
		//
		// Get NDR info
		//
		ZeroMemory(&MidlServerInfo,sizeof(MidlServerInfo));
		ZeroMemory(&MidlStubDesc,sizeof(MidlStubDesc));
		if (!ReadProcessMemory(hProcess, RpcInterface.RpcServerInterface.InterpreterInfo, &MidlServerInfo, sizeof(MidlServerInfo), NULL))
		{
			goto End;
		}
		if (!ReadProcessMemory(hProcess, MidlServerInfo.pStubDesc, &MidlStubDesc, sizeof(MidlStubDesc), NULL))
		{
			goto End;
		}
		//
		pRpcInterfaceInfo->NdrInfo.Version		= MidlStubDesc.Version;
		pRpcInterfaceInfo->NdrInfo.MIDLVersion	= MidlStubDesc.MIDLVersion;
		pRpcInterfaceInfo->NdrInfo.mFlags		= MidlStubDesc.mFlags;
		pRpcInterfaceInfo->pTypeFormatString	= (UCHAR*)MidlStubDesc.pFormatTypes;
		pRpcInterfaceInfo->apfnExprEval			= (void*)MidlStubDesc.apfnExprEval;

		if (MidlStubDesc.pExprInfo != NULL)
		{
			if (ReadProcessMemory(hProcess, MidlStubDesc.pExprInfo, &NdrExprDesc, sizeof(NdrExprDesc), NULL))
			{
				pRpcInterfaceInfo->pExprOffset = (USHORT*)NdrExprDesc.pOffset;
				pRpcInterfaceInfo->pExprFormatString = (UCHAR*)NdrExprDesc.pFormatExpr;
			}
			else
			{
				//printf("Unable to read the pExprInfo at address %p in process %u\n", MidlStubDesc.pExprInfo, pRpcInterfaceInfo->Pid);
			}
		}

		pRpcInterfaceInfo->pProcFormatString	= (UCHAR*)MidlServerInfo.ProcString;
		//
		// Get the interface procedure addresses table
		//
		pRpcInterfaceInfo->ppProcAddressTable=(ULONG*)OS_ALLOC(pRpcInterfaceInfo->NumberOfProcedures * sizeof(VOID*));
		if (pRpcInterfaceInfo->ppProcAddressTable == NULL) { DEBUG_BREAK(); goto End; }

		for(i=0;i<pRpcInterfaceInfo->NumberOfProcedures;i++)
		{
			if (ReadProcessMemory(hProcess,((VOID PTR_T PTR_T)MidlServerInfo.DispatchTable)+i,&pProc,sizeof(VOID PTR_T),NULL))
			{
				pRpcInterfaceInfo->ppProcAddressTable[i]= (ULONG) ((ULONG64)pProc - (ULONG64)pRpcInterfaceInfo->pLocationBase);
			}
		}
		//
		// Get the interface format string offset table
		//
		pRpcInterfaceInfo->pFormatStringOffsetTable=(USHORT*)OS_ALLOC(pRpcInterfaceInfo->NumberOfProcedures * sizeof(USHORT));
		if (pRpcInterfaceInfo->pFormatStringOffsetTable!=NULL)
		{
			ReadProcessMemory(hProcess,MidlServerInfo.FmtStringOffset,pRpcInterfaceInfo->pFormatStringOffsetTable,pRpcInterfaceInfo->NumberOfProcedures*sizeof(USHORT),NULL);
		}
	}
	else
	{
		//!!!todo!!!
		//GetInlineStubInfo(pRpcCoreInternalCtxt,hProcess,pRpcInterface,pRpcInterfaceInfo);
	}
	//
	// Test if DCOM or simple RPC
	//
	if (pRpcInterfaceInfo->IfType==IfType_RPC)
	{
		pRpcInterfaceInfo->IfCallbackFn	= RpcInterface.IfCallbackFn;
	}
	
	if (RpcInterface.EpMapperFlags & RPC_IS_EPMAPPER_REGISTERED)
	{
		pRpcInterfaceInfo->bIsRegistered=TRUE;
		StringCbPrintfA((STRSAFE_LPSTR)pRpcInterfaceInfo->Annotation,sizeof(pRpcInterfaceInfo->Annotation),"%s",(const char*)RpcInterface.Annotation);
	}
	else
	{
		pRpcInterfaceInfo->bIsRegistered=FALSE;
	}
End:
	return (pRpcInterfaceInfo);
}


//------------------------------------------------------------------------------
RpcInterfaceInfo_T*	__fastcall RpcCoreGetInterfaceInfo(void* pRpcCoreCtxt,DWORD Pid,RPC_IF_ID* pIf,ULONG InterfaceInfoMask)
{
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt;
	VOID*					pRpcInterface;
	HANDLE					hProcess				= NULL;
	RpcInterfaceInfo_T*		pRpcInterfaceInfo		= NULL;

	pRpcCoreInternalCtxt = (RpcCoreInternalCtxt_T*)pRpcCoreCtxt;
	hProcess = ProcexpOpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, Pid);
	if (hProcess == NULL) { DEBUG_BREAK(); goto End; }

	pRpcInterface = GetProcessInterface(pRpcCoreInternalCtxt, hProcess, pIf);
	if (pRpcInterface == NULL) { goto End; }

	pRpcInterfaceInfo = InternalGetInterfaceInfo(pRpcCoreInternalCtxt, hProcess, pRpcInterface, Pid, InterfaceInfoMask);
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return (pRpcInterfaceInfo);
}


//------------------------------------------------------------------------------
VOID __fastcall RpcCoreFreeInterfaceInfo(void* pRpcCoreCtxt,RpcInterfaceInfo_T* pRpcInterfaceInfo)
{
    UNREFERENCED_PARAMETER(pRpcCoreCtxt);

	if (pRpcInterfaceInfo==NULL) goto End;
	if (pRpcInterfaceInfo->ppProcAddressTable!=NULL) OS_FREE(pRpcInterfaceInfo->ppProcAddressTable);
	if (pRpcInterfaceInfo->pFormatStringOffsetTable!=NULL) OS_FREE(pRpcInterfaceInfo->pFormatStringOffsetTable);

	OS_FREE(pRpcInterfaceInfo);
End:
	return;
}


//------------------------------------------------------------------------------
BOOL __fastcall RpcCoreEnumProcessInterfaces(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessInterfacesCallbackFn_T RpcCoreEnumProcessInterfacesCallbackFn,void* pCallbackCtxt,ULONG InterfaceInfoMask)
{
	HANDLE					hProcess;
	BOOL					bResult=FALSE;
	RPC_SERVER_T			RpcServer;
	UINT					i;
	UINT					Size;
	VOID PTR_T *			pTable=NULL;
	VOID PTR_T				pRpcServer;
	BOOL					bContinue=TRUE;
	RpcInterfaceInfo_T*		pRpcInterfaceInfo = NULL;
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)pRpcCoreCtxt;

	hProcess=ProcexpOpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,FALSE,Pid);
	if (hProcess==NULL) goto End;

	if (!ReadProcessMemory(hProcess,pRpcCoreInternalCtxt->pGlobalRpcServer,&pRpcServer,sizeof(VOID PTR_T),NULL)) goto End;
	if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL)) goto End;

	if (RpcServer.InterfaceDict.NumberOfEntries > MAX_SIMPLE_DICT_ENTRIES)
	{
		goto End;
	}

	Size=RpcServer.InterfaceDict.NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;

	if (!ReadProcessMemory(hProcess,RpcServer.InterfaceDict.pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<RpcServer.InterfaceDict.NumberOfEntries; i++)
	{
		pRpcInterfaceInfo=InternalGetInterfaceInfo(pRpcCoreInternalCtxt,hProcess,pTable[i],Pid,InterfaceInfoMask); 
		if (pRpcInterfaceInfo!=NULL)
		{
			bResult=RpcCoreEnumProcessInterfacesCallbackFn(pRpcInterfaceInfo,pCallbackCtxt,&bContinue);
			RpcCoreFreeInterfaceInfo(pRpcCoreCtxt,pRpcInterfaceInfo);
		}
		if (!bResult) goto End;
		if (!bContinue) break;
	}
	bResult=TRUE;
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	if (pTable!=NULL) OS_FREE(pTable);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL __fastcall	RpcCoreEnumProcessEndpoints(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessEndpointsCallbackFn_T RpcCoreEnumProcessEndpointsCallbackFn,void* pCallbackCtxt)
{
	HANDLE					hProcess;
	BOOL					bResult=FALSE;
	RPC_SERVER_T			RpcServer;
	UINT					i;
	UINT					Size;
	VOID PTR_T *			pTable=NULL;
	VOID PTR_T				pRpcServer;
	RPC_ADDRESS_T			RpcAddress;
	WCHAR					ProtocoleW[RPC_MAX_ENDPOINT_PROTOCOL_SIZE];
	WCHAR					NameW[RPC_MAX_ENDPOINT_NAME_SIZE];
	RpcEndpointInfo_T		RpcEndpointInfo;
	BOOL					bContinue=TRUE;
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)pRpcCoreCtxt;

	hProcess=ProcexpOpenProcess(PROCESS_VM_READ,FALSE,Pid);
	if (hProcess==NULL) goto End;

	if (!ReadProcessMemory(hProcess,pRpcCoreInternalCtxt->pGlobalRpcServer,&pRpcServer,sizeof(VOID PTR_T),NULL)) goto End;
	if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL)) goto End;

	if (RpcServer.AddressDict.NumberOfEntries > MAX_SIMPLE_DICT_ENTRIES)
	{
		goto End;
	}

	Size=RpcServer.AddressDict.NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;

	if (!ReadProcessMemory(hProcess,RpcServer.AddressDict.pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<RpcServer.AddressDict.NumberOfEntries; i++)
	{
		if (!ReadProcessMemory(hProcess,pTable[i],&RpcAddress,sizeof(RpcAddress),NULL)) goto End;
		if (!ReadProcessMemory(hProcess,RpcAddress.Protocole,ProtocoleW,sizeof(ProtocoleW),NULL)) goto End;
		if (!ReadProcessMemory(hProcess,RpcAddress.Name,NameW,sizeof(NameW),NULL)) goto End;
		RpcEndpointInfo.pName		= NameW;
		RpcEndpointInfo.pProtocole	= ProtocoleW;
		bResult=RpcCoreEnumProcessEndpointsCallbackFn(Pid,&RpcEndpointInfo,pCallbackCtxt,&bContinue);
		if (!bResult) goto End;
		if (!bContinue) break;
	}
	bResult=TRUE;
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	if (pTable!=NULL)	OS_FREE(pTable);
	return (bResult);
}


//------------------------------------------------------------------------------
VOID* WINAPI GetProcessInterface(void* pRpcCoreCtxt,HANDLE hProcess,RPC_IF_ID* pIf)
{
	RPC_INTERFACE_T					RpcInterface;
	RPC_SERVER_INTERFACE_T PTR_T	pRpcServerInterface;
	RPC_SERVER_T					RpcServer;
	UINT							Size;
	RpcCoreInternalCtxt_T*			pRpcCoreInternalCtxt;
	VOID PTR_T						pRpcServer;
	UINT							i;
	VOID PTR_T *					pTable			= NULL;
	VOID PTR_T						pRpcInterface	= NULL;

	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)pRpcCoreCtxt;	
	if (!ReadProcessMemory(hProcess,pRpcCoreInternalCtxt->pGlobalRpcServer,&pRpcServer,sizeof(VOID PTR_T),NULL)) goto End;
	if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL)) goto End;

	if (RpcServer.InterfaceDict.NumberOfEntries > MAX_SIMPLE_DICT_ENTRIES) goto End;

	Size=RpcServer.InterfaceDict.NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;

	if (!ReadProcessMemory(hProcess,RpcServer.InterfaceDict.pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<RpcServer.InterfaceDict.NumberOfEntries; i++)
	{
		if (!ReadProcessMemory(hProcess,pTable[i],&RpcInterface,sizeof(RpcInterface),NULL)) goto End;
		//if match interface --> OK
		if (memcmp(&RpcInterface.RpcServerInterface.InterfaceId,pIf,sizeof(pRpcServerInterface->InterfaceId)) == 0)
		{
			pRpcInterface=pTable[i];
			break;
		}
	}
End:
	if (pTable!=NULL) OS_FREE(pTable);
	return(pRpcInterface);
}


//------------------------------------------------------------------------------
BOOL __fastcall RpcCoreEnumProcessAuthInfo(void* pRpcCoreCtxt,DWORD Pid,RpcCoreEnumProcessAuthInfoCallbackFn_T RpcCoreEnumProcessAuthInfoCallbackFn,void* pCallbackCtxt)
{
	RPC_SERVER_T			RpcServer;
	UINT					i;
	UINT					j;
	DWORD					Size;
	RPC_AUTH_INFO_T			RpcAuth;
	RpcAuthInfo_T			RpcAuthInfo;
	ULONG					PackagesCount;
	VOID PTR_T				pRpcServer				= NULL;
	HANDLE					hProcess				= NULL;
	VOID PTR_T *			pTable					= NULL;
	PSecPkgInfoW			SecurityPackageInfoTbl	= NULL;
	HKEY					hKey					= NULL;
	BOOL					bContinue				= TRUE;
	BOOL					bResult					= FALSE;
	WCHAR					ValueName[RPC_MAX_DLL_NAME_SIZE];
	RpcCoreInternalCtxt_T*	pRpcCoreInternalCtxt=(RpcCoreInternalCtxt_T*)pRpcCoreCtxt;

	ZeroMemory(&RpcAuthInfo,sizeof(RpcAuthInfo_T));
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Rpc\\SecurityService",0,KEY_READ,&hKey)!=ERROR_SUCCESS) goto End;
	if (EnumerateSecurityPackagesW(&PackagesCount,&SecurityPackageInfoTbl)!=SEC_E_OK) goto End;

	hProcess=ProcexpOpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,FALSE,Pid);
	if (hProcess==NULL) goto End;

	if (!ReadProcessMemory(hProcess,pRpcCoreInternalCtxt->pGlobalRpcServer,&pRpcServer,sizeof(VOID PTR_T), NULL)) goto End;
	if (!ReadProcessMemory(hProcess,pRpcServer,&RpcServer,sizeof(RpcServer),NULL)) goto End;

	if (RpcServer.AuthenInfoDict.NumberOfEntries > MAX_SIMPLE_DICT_ENTRIES) goto End;

	Size=RpcServer.AuthenInfoDict.NumberOfEntries*sizeof(VOID PTR_T);
	pTable=(VOID PTR_T *)OS_ALLOC(Size);
	if (pTable==NULL) goto End;

	if (!ReadProcessMemory(hProcess,RpcServer.AuthenInfoDict.pArray,pTable,Size,NULL)) goto End;
	for (i=0; i<RpcServer.AuthenInfoDict.NumberOfEntries; i++)
	{
		if (!ReadProcessMemory(hProcess,pTable[i],&RpcAuth,sizeof(RPC_AUTH_INFO_T),NULL)) goto End;
		ZeroMemory(&RpcAuthInfo,sizeof(RpcAuthInfo_T));
		for (j=0;j<PackagesCount;j++)
		{
			if (SecurityPackageInfoTbl[j].wRPCID==RpcAuth.AuthSvc)
			{
				if (!ReadProcessMemory(hProcess,RpcAuth.pPrincipalName,RpcAuthInfo.PrincipalName,sizeof(RpcAuthInfo.PrincipalName),NULL)) goto End;
				StringCbPrintfW(RpcAuthInfo.Name,sizeof(RpcAuthInfo.Name),L"%s",SecurityPackageInfoTbl[j].Name);
				StringCbPrintfW(RpcAuthInfo.Comment,sizeof(RpcAuthInfo.Comment),L"%s",SecurityPackageInfoTbl[j].Comment);
				RpcAuthInfo.Capabilities=SecurityPackageInfoTbl[j].fCapabilities;
				RpcAuthInfo.Version		=SecurityPackageInfoTbl[j].wVersion;
				break;
			}
		}
		RpcAuthInfo.AuthSvc		= RpcAuth.AuthSvc;
		RpcAuthInfo.pGetKeyFn	= RpcAuth.pGetKeyFn;
		RpcAuthInfo.pArg		= RpcAuth.pArg;
		//
		// get the SSP dll name
		//
		Size=sizeof(RpcAuthInfo.DllName);
		StringCbPrintfW(ValueName,sizeof(ValueName),L"%u",RpcAuthInfo.AuthSvc);
		RegQueryValueExW(hKey,ValueName,NULL,NULL,(BYTE*)RpcAuthInfo.DllName,&Size);
		//
		// Invoke the callback
		//
		bResult=RpcCoreEnumProcessAuthInfoCallbackFn(Pid,&RpcAuthInfo,pCallbackCtxt,&bContinue);
		if (!bResult) goto End;
		if (!bContinue) break;
	}
	bResult=TRUE;
End:
	if (hKey!=NULL)		RegCloseKey(hKey);
	if (pTable!=NULL)	OS_FREE(pTable);
	if (hProcess!=NULL) CloseHandle(hProcess);
	if (SecurityPackageInfoTbl!=NULL) FreeContextBuffer(SecurityPackageInfoTbl);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL WINAPI DllMain(HANDLE hInstDLL,DWORD dwReason,LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hInstDLL);
    UNREFERENCED_PARAMETER(dwReason);
    UNREFERENCED_PARAMETER(lpvReserved);

	return (TRUE);
}