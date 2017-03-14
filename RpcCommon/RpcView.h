#ifndef	_RPC_VIEW_H_
#define _RPC_VIEW_H_

#include "RpcCommon.h"
#include <Windows.h>

typedef ULONG	RVA_T;

typedef struct _RpcModuleInfo_T
{
	UINT	Pid;
	UINT64	pModuleBase;
}RpcModuleInfo_T;

typedef VOID* (__fastcall*	RpcAllocFn_T)(SIZE_T Size);
typedef VOID  (__fastcall*	RpcFreeFn_T)(VOID* pMem);
typedef BOOL  (__fastcall*	RpcGetProcessDataFn_T)(RpcModuleInfo_T* pRpcModuleInfo, RVA_T rva, VOID* pBuffer, UINT BufferLength);
typedef VOID  (__cdecl*		RpcPrintFn_T)(void* pContext, const char* pTxt);
typedef VOID  (__cdecl*		RpcDebugFn_T)(const char* pFunction, ULONG Line, const char* pFormatString,...);
typedef BOOL  (__fastcall*	RpcGetInterfaceNameFn_T)(GUID* pIfId,UCHAR* pName,ULONG NameLength);

//Framework helper for decompilation plugin
typedef struct _RpcViewHelper_T{
	void*					pContext;
	RpcAllocFn_T			RpcAlloc;
	RpcFreeFn_T				RpcFree;
	RpcGetProcessDataFn_T	RpcGetProcessData;
	RpcPrintFn_T			RpcPrint;
	RpcDebugFn_T			RpcDebug;
	RpcGetInterfaceNameFn_T	RpcGetInterfaceName;
}RpcViewHelper_T;

#endif //_RPC_VIEW_H_