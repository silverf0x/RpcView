#include "RpcDecompileIt.h"

#include <iostream>
#include <TlHelp32.h>
#include "../RpcDecompiler/IdlInterface.h"

#if _WIN64
	#define RPC_CORE_IS_WOW64 false
#else
	#define RPC_CORE_IS_WOW64 true
#endif

#pragma region TO_DELETE

#include "..\RpcCommon\Misc.h"
#include "..\RpcView\Pdb.h"


#if (RPC_CORE_IS_WOW64==TRUE)
#define REG_PREFIX	L"Wow6432Node\\"
#else
#define REG_PREFIX L""
#endif

//BOOL WINAPI GetRegValueData(HKEY hRootKey, WCHAR* pSubkeyName, WCHAR* pValueName, VOID* pData, UINT DataLength)
//{
//	DWORD	Size;
//	HKEY	hKey = NULL;
//	BOOL	bResult = FALSE;
//
//	if (RegOpenKeyExW(hRootKey, pSubkeyName, 0, KEY_READ, &hKey) != ERROR_SUCCESS) goto End;
//	Size = DataLength;
//	if (RegQueryValueExW(hKey, pValueName, NULL, NULL, (LPBYTE)pData, &Size) != ERROR_SUCCESS) goto End;
//	bResult = TRUE;
//End:
//	if (hKey != NULL) RegCloseKey(hKey);
//	return (bResult);
//}

//------------------------------------------------------------------------------
BOOL NTAPI GetComInterfaceName(const RPC_IF_ID* pInterfaceId, WCHAR* pName, UINT NameLength)
{
	WCHAR		SubKeyName[MAX_PATH];
	RPC_WSTR	pUuidString = NULL;
	BOOL		bResult = FALSE;

	if (UuidToStringW(&pInterfaceId->Uuid, &pUuidString) != RPC_S_OK) goto End;
	swprintf_s(SubKeyName, sizeof(SubKeyName), REG_PREFIX L"Interface\\{%ws}", (wchar_t*) pUuidString);
	if (GetRegValueData(HKEY_CLASSES_ROOT, SubKeyName, NULL, pName, NameLength) == FALSE) goto End;

	bResult = TRUE;
End:
	if (pUuidString != NULL) RpcStringFreeW(&pUuidString);
	return (bResult);
}

IfType_T WINAPI GetInterfaceType(const RPC_INTERFACE_T* pRpcInterface)
{
	WCHAR	TmpPath[MAX_PATH];

	if (pRpcInterface->Flags & RPC_IF_OLE)
	{
		return (IfType_OLE);
	}

	if (GetComInterfaceName(&pRpcInterface->RpcServerInterface.InterfaceId, TmpPath, sizeof(TmpPath)) == FALSE)
	{
		return (IfType_RPC);
	}
	return (IfType_DCOM);
}

BOOL __fastcall GetInterfaceName(RPC_IF_ID* pInterfaceId, WCHAR* pName, UINT NameLength)
{
	//WCHAR		FullPath[MAX_PATH];
	RPC_WSTR	pUuidString = NULL;
	BOOL		bResult = FALSE;

	if (pInterfaceId == NULL) goto End;

	pName[0] = 0;
	bResult = GetComInterfaceName(pInterfaceId, pName, NameLength);
	/*if (bResult == FALSE)
	{
		UuidToStringW(&pInterfaceId->Uuid, &pUuidString);
		GetFullPathNameW(gInterfaceFile, _countof(FullPath), FullPath, NULL);
		GetPrivateProfileStringW((LPCWSTR)pUuidString, L"Name", NULL, pName, NameLength / sizeof(WCHAR), FullPath);
	}*/
End:
	if (pUuidString != NULL) RpcStringFreeW(&pUuidString);
	return (bResult);
}

void NTAPI InitDecompilerInfo(_In_ RpcInterfaceInfo_T* pRpcInterfaceInfo, _Out_ RpcDecompilerInfo_T* pRpcDecompilerInfo)
{
	UINT	i;
	UINT	SymboleLength;
	HANDLE	hProcess = NULL;
	void*	hPdb = NULL;
	WCHAR	SymboleName[RPC_MAX_LENGTH];

	if (pRpcDecompilerInfo == NULL) goto End;
	if (pRpcInterfaceInfo == NULL)	goto End;

	pRpcDecompilerInfo->pModuleBase = (UINT64)pRpcInterfaceInfo->pLocationBase;
	pRpcDecompilerInfo->pIfId = &pRpcInterfaceInfo->If;
	pRpcDecompilerInfo->Pid = pRpcInterfaceInfo->Pid;
	swprintf_s(pRpcDecompilerInfo->InterfaceName, sizeof(pRpcDecompilerInfo->InterfaceName), L"%ws", pRpcInterfaceInfo->Name);
	pRpcDecompilerInfo->pSyntaxId = &pRpcInterfaceInfo->TransfertSyntax;

	pRpcDecompilerInfo->MIDLVersion = pRpcInterfaceInfo->NdrInfo.MIDLVersion;
	pRpcDecompilerInfo->NDRFags = (UINT) pRpcInterfaceInfo->NdrInfo.mFlags;
	pRpcDecompilerInfo->NDRVersion = pRpcInterfaceInfo->NdrInfo.Version;

	pRpcDecompilerInfo->NumberOfProcedures = pRpcInterfaceInfo->NumberOfProcedures;
	pRpcDecompilerInfo->ppProcAddressTable = pRpcInterfaceInfo->ppProcAddressTable;
	pRpcDecompilerInfo->pTypeFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pTypeFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);

	pRpcDecompilerInfo->pFormatStringOffsetTable = pRpcInterfaceInfo->pFormatStringOffsetTable;
	pRpcDecompilerInfo->pProcFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pProcFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);

	pRpcDecompilerInfo->apfnExprEval = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->apfnExprEval - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	pRpcDecompilerInfo->bIsInlined = FALSE;

	pRpcDecompilerInfo->pExprOffset = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pExprOffset - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	pRpcDecompilerInfo->pExprFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pExprFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	//
	// Cannot decompile if we cannot get the ppProcAddressTable value!!!
	//
	if (pRpcDecompilerInfo->ppProcAddressTable == NULL)
	{
		printf("*** No procedure: %u\n", pRpcDecompilerInfo->NumberOfProcedures);
		//	ExitProcess(0);
		//	goto End;
	}

	pRpcDecompilerInfo->ppProcNameTable = (WCHAR**)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(UCHAR*));
	if (pRpcDecompilerInfo->ppProcNameTable == NULL) goto End;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pRpcInterfaceInfo->Pid);
	if (hProcess == NULL) goto End;
#ifdef _WIN64
	pRpcDecompilerInfo->bIs64Bits = !pRpcInterfaceInfo->bWow64Process;
#else
	pRpcDecompilerInfo->bIs64Bits = FALSE;
#endif
	//
	// Creates and initialiaze the pbFunctionInterpreted bool table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->pbFunctionInterpreted = (BOOL*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(BOOL));
	if (pRpcDecompilerInfo->pbFunctionInterpreted == NULL) goto End;
	//
	// Creates and initialiaze the ppProcFormatInlined RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppProcFormatInlined = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppProcFormatInlined == NULL) goto End;
	//
	// Creates and initialiaze the ppDispatchProcAddressTable RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppDispatchProcAddressTable = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppDispatchProcAddressTable == NULL) goto End;

	hPdb = PdbInit(hProcess, pRpcInterfaceInfo->pLocationBase, (UINT) pRpcInterfaceInfo->LocationSize);
	if (hPdb == NULL) goto End;
	for (i = 0; i<pRpcDecompilerInfo->NumberOfProcedures; i++)
	{
		SymboleName[0] = 0;
		if (PdbGetSymbolName(hPdb, (UCHAR*)pRpcInterfaceInfo->pLocationBase + pRpcDecompilerInfo->ppProcAddressTable[i], SymboleName, sizeof(SymboleName)))
		{
			SymboleLength = ((UINT)wcslen(SymboleName) + 1) * sizeof(WCHAR);
			pRpcDecompilerInfo->ppProcNameTable[i] = (WCHAR*)OS_ALLOC(SymboleLength);
			memcpy(pRpcDecompilerInfo->ppProcNameTable[i], SymboleName, SymboleLength);
		}
	}
	PdbUninit(hPdb);
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return;
}
#pragma endregion TO_DELETE

// copied from ../RpcCore/RpcCore.c
RpcInterfaceInfo_T*	WINAPI InternalGetInterfaceInfo(PVOID Context, HANDLE hProcess, VOID* pRpcInterface, DWORD Pid, ULONG InterfaceInfoMask)
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

    UNREFERENCED_PARAMETER(Context);
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
	pRpcInterfaceInfo->bWow64Process = (sizeof(PVOID) != sizeof(uint64_t)) /*RPC_CORE_IS_WOW64*/;
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
		sprintf_s((char*) pRpcInterfaceInfo->Annotation,sizeof(pRpcInterfaceInfo->Annotation),"%s",(const char*)RpcInterface.Annotation);
	}
	else
	{
		pRpcInterfaceInfo->bIsRegistered=FALSE;
	}
End:
	return (pRpcInterfaceInfo);
}

int
DecompileInit(
	_Inout_ PDECOMPILE_IT_CTXT Context
)
{
	WCHAR RefModuleName[MAX_PATH];
	HMODULE hMods[1024];
	DWORD 	ModulesSize;

	HANDLE hProcess = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		Context -> TargetPID
	);

	if (hProcess==NULL)
	{
		return GetLastError();
	}
	Context -> hTargetProcess = hProcess;


	if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &ModulesSize))
    {
		return GetLastError();
    }

	// stop at the first if ModuleName is not set
	if (!Context->ModuleName)
	{
		Context->ModuleBaseAddress = (uintptr_t) hMods[0];
	}
	else
	{
		swprintf_s(RefModuleName, sizeof(RefModuleName) / sizeof(WCHAR), L"%hs", Context->ModuleName);

		for (auto ModuleIndex = 0; (unsigned int) ModuleIndex < (ModulesSize / sizeof(HMODULE)); ModuleIndex++ )
		{
        	WCHAR ModuleName[MAX_PATH];


        	if (GetModuleBaseNameW(
        		hProcess, 
        		hMods[ModuleIndex], 
				ModuleName,
        		sizeof(ModuleName) / sizeof(WCHAR)
        	))
            {
	        	if (!_wcsicmp(ModuleName, RefModuleName))
	        	{
	        		Context->ModuleBaseAddress = (uintptr_t) hMods[ModuleIndex];
					break;
	        	}
	        }
        }		
	}

	if (!Context->ModuleBaseAddress)
    {
		return GetLastError();
    }
		

	Context->DescriptorAddress = Context->DescriptorOffset;
	Context->FormatStrAddress = Context->FormatStrOffset;
	if (!Context->bAbsoluteAddress)
	{
		Context->DescriptorAddress += Context->ModuleBaseAddress;
		Context->FormatStrAddress += Context->ModuleBaseAddress;
	}

	return 0;
}



int 
DecompileIt(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	uintptr_t 				DecompilerHelperAddr = NULL;
	RpcDecompilerHelper_T* 	DecompilerHelper = NULL;
	//
	//RpcInterfaceInfo_T*		RpcInterfaceInfo = NULL;
	//void* 					RpcDecompilerContext;
	MIDL_STUB_DESC			MidlStubDesc;
	size_t					StubDescBytesRead;
	size_t					RpcInterfaceInformationStructSize;
	RPC_CLIENT_INTERFACE    RpcClientInterface;
	//RPC_SERVER_INTERFACE	RpcServerInterface;
	RPC_IF_ID				RpcInterfaceId;
	ASSIGN_RPC_VIEW_STUB(RpcViewHelperStub, &Context);


	// resolve RpcDecompilerHelper address
	HMODULE hRpcDecompiler = LoadLibrary("RpcDecompiler");
	if (!hRpcDecompiler)
	{
		return -1;
	}

	DecompilerHelperAddr = (uintptr_t) GetProcAddress(hRpcDecompiler, "RpcDecompilerHelper");
	if (!DecompilerHelperAddr)
	{
		return -1;
	}
	DecompilerHelper = (RpcDecompilerHelper_T*) DecompilerHelperAddr;

	if (!ReadProcessMemory(
		Context.hTargetProcess,
		(LPCVOID)Context.DescriptorAddress,
		&MidlStubDesc,
		sizeof(MidlStubDesc),
		(SIZE_T*) &StubDescBytesRead
	))
	{
		return -1;
	}

	if (!ReadProcessMemory(
		Context.hTargetProcess,
		MidlStubDesc.RpcInterfaceInformation,
		&RpcInterfaceInformationStructSize,
		sizeof(RpcInterfaceInformationStructSize),
		(SIZE_T*) &StubDescBytesRead
	))
	{
		return -1;
	}

	switch (RpcInterfaceInformationStructSize)
	{
	case sizeof(RPC_CLIENT_INTERFACE):

		if (!ReadProcessMemory(
			Context.hTargetProcess,
			MidlStubDesc.RpcInterfaceInformation,
			&RpcClientInterface,
			sizeof(RpcClientInterface),
			(SIZE_T*) &StubDescBytesRead
		))
		{
			return -1;
		}
		
		RpcInterfaceId.Uuid = RpcClientInterface.InterfaceId.SyntaxGUID;
		RpcInterfaceId.VersMajor = RpcClientInterface.InterfaceId.SyntaxVersion.MajorVersion;
		RpcInterfaceId.VersMinor= RpcClientInterface.InterfaceId.SyntaxVersion.MinorVersion;

		//RpcDecompilerInfoStub.pSyntaxId = &RpcClientInterface.InterfaceId;
		break;

	//case sizeof(RPC_SERVER_INTERFACE):
	default:
		return -1;
	}

	
	/*RpcDecompilerInfoStub.Pid = Context.TargetPID;
	RpcDecompilerInfoStub->pIfId = &RpcInterfaceId;
	RpcDecompilerInfoStub->pModuleBase = Context.ModuleBaseAddress;
	RpcDecompilerInfoStub->NDRVersion = MidlStubDesc.Version;
	RpcDecompilerInfoStub->MIDLVersion = MidlStubDesc.MIDLVersion;
	RpcDecompilerInfoStub->NDRFags = (UINT) MidlStubDesc.mFlags;*/

	//RpcInterfaceInfo = InternalGetInterfaceInfo((PVOID) &Context, Context.hTargetProcess, (VOID*)MidlStubDesc.RpcInterfaceInformation, Context.TargetPID, RPC_INTERFACE_INFO_ALL);
	//InitDecompilerInfo(RpcInterfaceInfo, &RpcDecompilerInfoStub);

	//// Init RpcDecompiler and decompile idl
	//RpcDecompilerContext = DecompilerHelper->RpcDecompilerInitFn(
	//	&RpcViewHelperStub,
	//	&RpcDecompilerInfoStub
	//);
	//if (!RpcDecompilerContext)
	//{
	//	return -1;
	//}

	//DecompilerHelper->RpcDecompilerPrintAllProceduresFn(
	//	RpcDecompilerContext
	//);

	std::string IfaceName("DecompileItInterface");
	IdlInterface Interface(IfaceName, RpcInterfaceId, 1);

	
	// Init stubs for RpcDecompiler
	RpcDecompilerInfo_T		RpcDecompilerInfoStub;
	RpcDecompilerInfoStub.ppProcNameTable = new WCHAR*[1];
	RpcDecompilerInfoStub.ppProcNameTable[0] = NULL;

	RpcDecompilerInfoStub.pFormatStringOffsetTable = new USHORT[1];
	RpcDecompilerInfoStub.pFormatStringOffsetTable[0] = 0;
	RpcDecompilerInfoStub.pProcFormatString = (RVA_T) Context.FormatStrOffset;

	RpcDecompilerInfoStub.pTypeFormatString = (RVA_T) (MidlStubDesc.pFormatTypes - Context.ModuleBaseAddress);

	RpcModuleInfo_T ModuleInfoStub = {
		/*Pid = */Context.TargetPID,
		/*pModuleBase = */Context.ModuleBaseAddress
	};

	RpcDecompilerCtxt_T DecompilerStubContext;
	DecompilerStubContext.pRpcDecompilerInfo = &RpcDecompilerInfoStub;
	DecompilerStubContext.pRpcModuleInfo = &ModuleInfoStub;
	DecompilerStubContext.pRpcViewHelper = &RpcViewHelperStub;

	if (DS_SUCCESS == Interface.decode((PVOID)&DecompilerStubContext))
	{
		//std::ostringstream		ossIf;
		std::cout << Interface;

	}
	
	return 0;
}

int
DecompileUninit(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	if (Context.hTargetProcess == INVALID_HANDLE_VALUE)
	{
		CloseHandle(Context.hTargetProcess);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	DECOMPILE_IT_CTXT Context = {0};
	char*  	EndPtr;
	int 	status;

	if (argc < 7)
	{
		printf("Usage: %s --pid PID [--module MODULE] --descriptor DESC_OFFSET --format-str FORMAT_STRING_OFFSET [--absolute]\n", argv[0]);
		printf("  --pid : PID of the target process. %s must be able to open a handle to read the target process memory.\n", argv[0]);
		printf("  --module : module name to read memory from. If not set, %s read the target executable own module. Ignored if --absolute is set.\n", argv[0]);
		printf("  --descriptor : offset to the rpc header descriptor for the interface. If --absolute is set, --descriptor is interpreted as a virtual address.\n");
		printf("  --format-str : offset to the rpc format string for the chosen proc. If --absolute is set, --format-str is interpreted as a virtual address.\n");
		printf("  --absolute : treat descriptor and format-str as absolute virtual addresses instead of offsets.\n");

		return 0;
	}


	for (auto ArgIndex = 0; ArgIndex < argc; ArgIndex++) 
	{
		char *CurrentArgument = argv[ArgIndex];

		if (!_stricmp(CurrentArgument, "--pid"))
		{
			Context.TargetPID = (DWORD) strtoumax(argv[ArgIndex + 1], &EndPtr, 10);
		}
		if (!_stricmp(CurrentArgument, "--module"))
		{
			Context.ModuleName = argv[ArgIndex + 1];
		}
		else if (!_stricmp(CurrentArgument, "--descriptor"))
		{
			Context.DescriptorOffset = (size_t) strtoumax(argv[ArgIndex + 1], &EndPtr, 16);
		}
		else if (!_stricmp(CurrentArgument, "--format-str"))
		{
			Context.FormatStrOffset = (size_t) strtoumax(argv[ArgIndex + 1], &EndPtr, 16);
		}
		else if (!_stricmp(CurrentArgument, "--absolute"))
		{
			Context.bAbsoluteAddress = true;
		}
	}


	status = DecompileInit(
		&Context
	);
	if (status)
	{
		printf("Could not init the DecompileIt context : %d.\n", GetLastError());
		return status;
	}

	status = DecompileIt(Context);


	DecompileUninit(Context);
	return status;
}