#include "..\Qt\Qt.h"

#include <windows.h>
#include <locale.h>
#include <Psapi.h>
#include <strsafe.h>
#include <Dbghelp.h>

#include "MainWindow.h"
#include "EulaDialog.h"
#include "DecompilationWidget.h"
#include "Pdb.h"
#include "RpcViewResource.h"
#include "..\RpcDecompiler\RpcDecompiler.h"
#include "..\RpcCore\RpcCore.h"
#include "..\RpcCommon\Misc.h"

#include <conio.h>

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Imm32.lib")
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"Ws2_32.lib")

//
// Set the entry point to WinMain in relase configuration
//
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif

VOID*	__fastcall	RpcAlloc(SIZE_T Size);
VOID	__fastcall	RpcFree(VOID* pMem);
BOOL	__fastcall	RpcGetProcessData(RpcModuleInfo_T* pRpcModuleInfo, RVA_T rva, VOID* pBuffer, UINT BufferLength);	//Size is IN (pPorcNameBuffer size)/OUT (read size) 
VOID	__cdecl		RpcPrint(void* pContext,const char* pTxt);
VOID	__cdecl		RpcDebug(const char* pFunction, ULONG Line, const char* pFormatString, ...);
BOOL	__fastcall	RpcGetInterfaceName(GUID* pIfId,UCHAR* pName,ULONG NameLength);

#ifdef __cplusplus
extern "C" {
#endif

extern    RpcCore_T	gRpcCoreManager;

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------
ULONG NTAPI DecompilerExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
	UCHAR	ModulePath[RPC_MAX_LENGTH];
	HMODULE	hModule;

	ModulePath[0]=0;
	_cprintf("Exception catched.\n");
	GetMappedFileNameA(GetCurrentProcess(), pExceptionPointers->ExceptionRecord->ExceptionAddress, (LPSTR)ModulePath, sizeof(ModulePath));
	_cprintf("Code   : 0x%X\n", pExceptionPointers->ExceptionRecord->ExceptionCode);
	_cprintf("Module : %s\n", ModulePath);
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pExceptionPointers->ExceptionRecord->ExceptionAddress, &hModule);
	_cprintf("Address: 0x%p (0x%p + 0x%X)\n", pExceptionPointers->ExceptionRecord->ExceptionAddress, hModule, (UINT_PTR)pExceptionPointers->ExceptionRecord->ExceptionAddress - (UINT_PTR)hModule );
	return (EXCEPTION_EXECUTE_HANDLER);
}


//------------------------------------------------------------------------------
HMODULE NTAPI LoadDecompilerEngine(RpcDecompilerHelper_T** ppRpcDecompilerHelper)
{
	WIN32_FIND_DATAA		Win32FindData;
	HMODULE					hLib				 = NULL;
	HANDLE					hFindFile			 = INVALID_HANDLE_VALUE;
	RpcDecompilerHelper_T*	pRpcDecompilerHelper = NULL;
	BOOL					bResult				 = FALSE;

	hFindFile = FindFirstFileA("*.dll", &Win32FindData);
	if ( hFindFile==INVALID_HANDLE_VALUE ) goto End;
	do
	{
        __try{
            hLib = LoadLibraryA(Win32FindData.cFileName);
        }
        __except( DecompilerExceptionFilter(GetExceptionInformation()) )
        {
           goto End;
        }
		if (hLib!=NULL)
		{
			pRpcDecompilerHelper = (RpcDecompilerHelper_T*)GetProcAddress(hLib, RPC_DECOMPILER_EXPORT_SYMBOL);
			if (pRpcDecompilerHelper!=NULL) 
			{
				_cprintf("Found RpcDecompilerHelper %p\n", pRpcDecompilerHelper);
				*ppRpcDecompilerHelper	= pRpcDecompilerHelper;
				bResult = TRUE;
				goto End;
			}
			else
			{
				FreeLibrary(hLib);
				hLib=NULL;
			}
		} 
	}while( FindNextFileA(hFindFile, &Win32FindData) );
End:
	if ( hFindFile!=INVALID_HANDLE_VALUE ) FindClose(hFindFile);
	return (hLib);
}


//------------------------------------------------------------------------------
void NTAPI InitDecompilerInfo(_In_ RpcInterfaceInfo_T* pRpcInterfaceInfo, _Out_ RpcDecompilerInfo_T* pRpcDecompilerInfo)
{
	UINT	i;
	UINT	SymboleLength;
	HANDLE	hProcess	= NULL;
	void*	hPdb		= NULL;
	WCHAR	SymboleName[RPC_MAX_LENGTH];

	if (pRpcDecompilerInfo == NULL) goto End;
	if (pRpcInterfaceInfo == NULL)	goto End;

	pRpcDecompilerInfo->pModuleBase = (UINT64)pRpcInterfaceInfo->pLocationBase;
	pRpcDecompilerInfo->pIfId = &pRpcInterfaceInfo->If;
	pRpcDecompilerInfo->Pid = pRpcInterfaceInfo->Pid;
	StringCbPrintfW(pRpcDecompilerInfo->InterfaceName, sizeof(pRpcDecompilerInfo->InterfaceName), L"%s", pRpcInterfaceInfo->Name);
	pRpcDecompilerInfo->pSyntaxId = &pRpcInterfaceInfo->TransfertSyntax;

	pRpcDecompilerInfo->MIDLVersion = pRpcInterfaceInfo->NdrInfo.MIDLVersion;
	pRpcDecompilerInfo->NDRFags = pRpcInterfaceInfo->NdrInfo.mFlags;
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

	pRpcDecompilerInfo->ppProcNameTable = (WCHAR**)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures*sizeof(UCHAR*));
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
	pRpcDecompilerInfo->pbFunctionInterpreted = (BOOL*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures*sizeof(BOOL));
	if (pRpcDecompilerInfo->pbFunctionInterpreted == NULL) goto End;
	//
	// Creates and initialiaze the ppProcFormatInlined RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppProcFormatInlined = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures*sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppProcFormatInlined == NULL) goto End;
	//
	// Creates and initialiaze the ppDispatchProcAddressTable RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppDispatchProcAddressTable = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures*sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppDispatchProcAddressTable == NULL) goto End;

	hPdb = PdbInit(hProcess, pRpcInterfaceInfo->pLocationBase, pRpcInterfaceInfo->LocationSize);
	if (hPdb == NULL) goto End;
	for (i = 0; i<pRpcDecompilerInfo->NumberOfProcedures; i++)
	{
		SymboleName[0] = 0;
		if (PdbGetSymbolName(hPdb, (UCHAR*)pRpcInterfaceInfo->pLocationBase + pRpcDecompilerInfo->ppProcAddressTable[i], SymboleName, sizeof(SymboleName)))
		{
			SymboleLength = ((UINT)wcslen(SymboleName) + 1)*sizeof(WCHAR);
			pRpcDecompilerInfo->ppProcNameTable[i] = (WCHAR*)OS_ALLOC(SymboleLength);
			memcpy(pRpcDecompilerInfo->ppProcNameTable[i], SymboleName, SymboleLength);
		}
	}
	PdbUninit(hPdb);
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return;
}


//------------------------------------------------------------------------------
void NTAPI UninitDecompilerInfo(RpcDecompilerInfo_T* pRpcDecompilerInfo)
{
	UINT i;

	if (pRpcDecompilerInfo->ppProcNameTable != NULL)
	{
		for (i = 0; i<pRpcDecompilerInfo->NumberOfProcedures; i++)
		{
			if (pRpcDecompilerInfo->ppProcNameTable[i] != NULL) OS_FREE(pRpcDecompilerInfo->ppProcNameTable[i]);
		}
		OS_FREE(pRpcDecompilerInfo->ppProcNameTable);
	}
	if (pRpcDecompilerInfo->pbFunctionInterpreted != NULL) OS_FREE(pRpcDecompilerInfo->pbFunctionInterpreted);
	if (pRpcDecompilerInfo->ppProcFormatInlined != NULL) OS_FREE(pRpcDecompilerInfo->ppProcFormatInlined);
	if (pRpcDecompilerInfo->ppDispatchProcAddressTable != NULL) OS_FREE(pRpcDecompilerInfo->ppDispatchProcAddressTable);
}


#ifdef _DEBUG

typedef struct _EnumCtxt_T{ 
	void*			        pRpcCoreCtxt;
	RpcCore_T*		        pRpcCore;
	RpcDecompilerHelper_T*	pRpcDecompilerHelper;
}EnumCtxt_T;


//------------------------------------------------------------------------------
VOID __cdecl RpcDbgPrint(void* pContext, const char* pTxt)
{
    UNREFERENCED_PARAMETER(pContext);
    printf("%s\n", pTxt);
}


//------------------------------------------------------------------------------
static BOOL __fastcall EnumInterfaces(RpcInterfaceInfo_T* pRpcInterfaceInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	RpcDecompilerInfo_T	RpcDecompilerInfo;
	void*				pDecompilerCtxt;

	UNREFERENCED_PARAMETER(pbContinue);

	ZeroMemory(&RpcDecompilerInfo, sizeof(RpcDecompilerInfo_T));
	if (pRpcInterfaceInfo->IfType != IfType_RPC) goto End;
	InitDecompilerInfo(pRpcInterfaceInfo, &RpcDecompilerInfo);
	__try{
        RpcViewHelper_T	LocalRpcViewHelper = {
            NULL,
            &RpcAlloc,
            &RpcFree,
            &RpcGetProcessData,
            &RpcDbgPrint,
            &RpcDebug,
            &RpcGetInterfaceName
        };
        pDecompilerCtxt = pEnumCtxt->pRpcDecompilerHelper->RpcDecompilerInitFn(&LocalRpcViewHelper, &RpcDecompilerInfo);
		if (pDecompilerCtxt!=NULL)
		{
			pEnumCtxt->pRpcDecompilerHelper->RpcDecompilerPrintAllProceduresFn( pDecompilerCtxt );
			pEnumCtxt->pRpcDecompilerHelper->RpcDecompilerUninitFn(pDecompilerCtxt);
		}
	}__except( DecompilerExceptionFilter(GetExceptionInformation()) )
	{
        //Failure
        goto End;
	}
	
End:
	UninitDecompilerInfo(&RpcDecompilerInfo);
	return (TRUE);
}


//------------------------------------------------------------------------------
static BOOL WINAPI EnumProc(DWORD Pid, DWORD Ppid, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{ 
	RpcProcessInfo_T*	pRpcProcessInfo;

	UNREFERENCED_PARAMETER(pbContinue);

	pRpcProcessInfo=pEnumCtxt->pRpcCore->RpcCoreGetProcessInfoFn(pEnumCtxt->pRpcCoreCtxt, Pid, Ppid,RPC_PROCESS_INFO_ALL);
	if (pRpcProcessInfo==NULL) goto End;
	pEnumCtxt->pRpcCore->RpcCoreFreeProcessInfoFn(pEnumCtxt->pRpcCoreCtxt, pRpcProcessInfo);
	pEnumCtxt->pRpcCore->RpcCoreEnumProcessInterfacesFn(pEnumCtxt->pRpcCoreCtxt, Pid, (RpcCoreEnumProcessInterfacesCallbackFn_T)&EnumInterfaces, pEnumCtxt, RPC_INTERFACE_INFO_ALL);
End:
	return (TRUE);
}


//------------------------------------------------------------------------------
int DecompileAllInterfaces(RpcCore_T* pRpcCore)
{
    EnumCtxt_T				EnumCtxt = {0};
	RpcDecompilerHelper_T*	pRpcDecompilerHelper;
	HMODULE					hDecompiler = NULL;

	hDecompiler=LoadDecompilerEngine(&pRpcDecompilerHelper);
	if (hDecompiler==NULL) goto End;

	EnumCtxt.pRpcDecompilerHelper	= pRpcDecompilerHelper;
    EnumCtxt.pRpcCore               = pRpcCore;
    EnumCtxt.pRpcCoreCtxt           = pRpcCore->RpcCoreInitFn();
    if (EnumCtxt.pRpcCoreCtxt==NULL) goto End;

	_cprintf("Start scanning...\n");
	EnumProcess( (EnumProcessCallbackFn_T)&EnumProc, &EnumCtxt );
	_cprintf("Done\n");
End:
	if (EnumCtxt.pRpcCoreCtxt != NULL) pRpcCore->RpcCoreUninitFn(EnumCtxt.pRpcCoreCtxt);
	if (hDecompiler!=NULL) FreeLibrary(hDecompiler);
	return (0);
}

#endif


//------------------------------------------------------------------------------
#ifdef _DEBUG
	int main(int argc, char* argv[])
#else
	int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#endif
{
	MainWindow_C*	pMainWindow			= NULL;
	HICON			hMainIcon;
	UCHAR			CurrentDirectory[MAX_PATH];
	UCHAR*			pSeparator;

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#else
	int				argc		= 0;
	//char*			pCmdLineA	= NULL;
	char*			argv[100] = { 0 };

    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);
	pCmdLine = GetCommandLineW();
	LPWSTR* argvw = CommandLineToArgvW(pCmdLine, &argc);
#endif
	QApplication app(argc, argv);
    QSettings   Settings(RPC_VIEW_ORGANIZATION_NAME, RPC_VIEW_APPLICATION_NAME);
 	//
	// Set the current directory according to the given EXE path
	//
	StringCbPrintfA((STRSAFE_LPSTR)CurrentDirectory,sizeof(CurrentDirectory),"%s",argv[0]);
	pSeparator = (UCHAR*)strrchr((const char*)CurrentDirectory,'\\');
	if (pSeparator!=NULL)
	{
		*pSeparator = 0;
		_cprintf("%s\n",CurrentDirectory);
		SetCurrentDirectoryA((LPCSTR)CurrentDirectory);
	}
#ifdef _DEBUG
	if (argc>1)
	{
		if (!_stricmp(argv[1],"/DA"))
		{
            DecompileAllInterfaces(&gRpcCoreManager);
		}
		else
		{
			_cprintf("Usage %s: [/DA]\n",argv[0]);
			_cprintf("  /DA : decompile all interfaces\n");
		}
		_CrtDumpMemoryLeaks();
		return 0;
	}
#else
	//argc is corrupted ? 
	//if (argc>1)
	{
		if (argvw[1] && !wcsncmp(argvw[1], L"/f", 2))
		{
			gRpcCoreManager.bForceLoading = TRUE;
		}
		else
		{
			_cprintf("Usage %s: [/f]\n", argv[0]);
			_cprintf("  /f : force loading for unsupported runtime versions \n");
		}
	}
#endif
    pMainWindow = new MainWindow_C(&gRpcCoreManager);

	hMainIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MAIN_ICON));
	if (hMainIcon!=NULL)
	{
		pMainWindow->setWindowIcon(QtWin::fromHICON(hMainIcon));
		DestroyIcon(hMainIcon);
	}
	return app.exec();
}