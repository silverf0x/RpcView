#include "RpcDecompileIt.h"
#include <conio.h>

PVOID
DecompileItRpcAlloc(
	_In_ size_t Size
)
{
	return malloc(Size);
}

void
DecompileItRpcFree(
	_In_ PVOID pMem
)
{
	free(pMem);
}

void
DecompileItRpcPrint(
	_In_ PVOID Context,
	_In_ const char *pText
)
{
	UNREFERENCED_PARAMETER(Context);

	printf(pText);
}

void
DecompileItRpcDebug(
	_In_ const char *pFunction,
	_In_ ULONG Line,
	_In_ const char *pFormat,
	...
)
{
	va_list	Arg;
	UNREFERENCED_PARAMETER(pFunction);
	UNREFERENCED_PARAMETER(Line);
	va_start(Arg, pFormat);
	_vcprintf(pFormat, Arg);
}

bool
DecompileItRpcGetInterfaceName(
	_In_ GUID *pIfId,
	_Out_ UCHAR *pName,
	_Out_ ULONG NameLength
)
{
	HKEY		hKey = NULL;
	ULONG		DataLength;
	UCHAR		SubKeyName[MAX_PATH];
	RPC_CSTR	pUuidString = NULL;
	BOOL		bResult = FALSE;

	if (UuidToStringA(pIfId, &pUuidString) != RPC_S_OK) goto End;
	sprintf_s((char*) SubKeyName, sizeof(SubKeyName), "Interface\\{%s}", pUuidString);

	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, (LPCSTR)SubKeyName, 0, KEY_READ, &hKey) != ERROR_SUCCESS) goto End;
	DataLength = NameLength;
	if (RegQueryValueExA(hKey, NULL, NULL, NULL, pName, &DataLength) != ERROR_SUCCESS) goto End;

	bResult = TRUE;
End:
	if (hKey != NULL) RegCloseKey(hKey);
	if (pUuidString != NULL) RpcStringFreeA(&pUuidString);
	return (bResult);
}


bool __fastcall
DecompileItRpcGetProcessData(
	_In_  RpcModuleInfo_T *Context,
	_In_  RVA_T Rva, 
	_Out_ VOID* pBuffer, 
	_Out_ UINT BufferLength
)
{
	HANDLE  hTargetProcess = INVALID_HANDLE_VALUE;
	BOOL	bResult = FALSE;
	VOID*	pAddress = NULL;

	RpcModuleInfo_T *DecompileContext = (RpcModuleInfo_T *)Context;

	if ((Context == NULL) || (DecompileContext->Pid == 0))
	{
		goto End;
	}

	hTargetProcess = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		DecompileContext->Pid
	);

	if (hTargetProcess == INVALID_HANDLE_VALUE)
	{
		goto End;
	}

	pAddress = (VOID*)(DecompileContext->pModuleBase + Rva);
	bResult = ReadProcessMemory(
		hTargetProcess,
		pAddress, 
		pBuffer, 
		BufferLength, 
		NULL
	);

End:
	if (hTargetProcess != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hTargetProcess);
	}

	return (bResult);
}


void
DecompileItInitRpcViewStub
(
	_Inout_ RpcViewHelper_T *RpcViewStub,
	_In_ PVOID Context
)
{
	RpcViewStub->pContext = &Context;
	RpcViewStub->RpcAlloc = (RpcAllocFn_T) &DecompileItRpcAlloc;
	RpcViewStub->RpcFree = (RpcFreeFn_T) &DecompileItRpcFree;
	RpcViewStub->RpcGetProcessData = (RpcGetProcessDataFn_T)&DecompileItRpcGetProcessData;
	RpcViewStub->RpcPrint = &DecompileItRpcPrint;
	RpcViewStub->RpcDebug = &DecompileItRpcDebug;
	RpcViewStub->RpcGetInterfaceName = (RpcGetInterfaceNameFn_T)&DecompileItRpcGetInterfaceName;
}