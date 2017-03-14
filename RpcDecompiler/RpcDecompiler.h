#ifndef _RPC_DECOMPILER_H_
#define _RPC_DECOMPILER_H_

#include <Rpc.h>
#include <Rpcndr.h>

#include "..\RpcCommon\RpcView.h"

#define RPC_DECOMPILER_EXPORT_SYMBOL	"RpcDecompilerHelper"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////



typedef struct _RpcDecompilerInfo_T{
	UINT					Pid;
	RPC_IF_ID*				pIfId;
	UINT64					pModuleBase;
	UINT					NDRVersion;
	UINT					MIDLVersion;
	UINT					NDRFags;
	UINT					NumberOfProcedures;
	RVA_T*					ppProcAddressTable;			//A table containing the address of each function
	RVA_T*					ppDispatchProcAddressTable;
	USHORT*					pFormatStringOffsetTable;	//A table containing the FormatStringOffset of each function
	WCHAR**					ppProcNameTable;			//A table containing the name of each function if possible or a NULL pointer else
	RVA_T/*UCHAR* */		pTypeFormatString;			//Just a pointer to the type format string: must be read using 
	RVA_T/*UCHAR* */		pProcFormatString;
	//RVA_T /*VOID* */		pExprInfo;					// FC_EXPR : to be removed (replaced by following 2 elements)
	RVA_T					pExprOffset;
	RVA_T					pExprFormatString;
	RVA_T					apfnExprEval;				// Callbacks
	BOOL					bIs64Bits;
	WCHAR					InterfaceName[RPC_MAX_LENGTH];
	RPC_SYNTAX_IDENTIFIER*	pSyntaxId;
	BOOL					bIsInlined;
	RVA_T*					ppProcFormatInlined;
	BOOL*					pbFunctionInterpreted;		// array containing for every function a boolean indicating if function is interpreted
	//...
}RpcDecompilerInfo_T;

typedef VOID*	(__fastcall* RpcDecompilerInitFn_T)(RpcViewHelper_T* pRpcViewHelper, RpcDecompilerInfo_T* pDecompilerInfo);	//returns NULL in case of failure
typedef VOID	(__fastcall* RpcDecompilerUninitFn_T)(VOID* pRpcDecompilerCtxt);
typedef BOOL	(__fastcall* RpcDecompilerPrintProcedureFn_T)(VOID* pRpcDecompilerCtxt, UINT ProcIndex);
typedef BOOL	(__fastcall* RpcDecompilerPrintAllProceduresFn_T)(VOID* pRpcDecompilerCtxt);

typedef struct _RpcDecompilerHelper_T{
	RpcDecompilerInitFn_T				RpcDecompilerInitFn;
	RpcDecompilerUninitFn_T				RpcDecompilerUninitFn;
	RpcDecompilerPrintProcedureFn_T		RpcDecompilerPrintProcedureFn;
	RpcDecompilerPrintAllProceduresFn_T	RpcDecompilerPrintAllProceduresFn;
}RpcDecompilerHelper_T;


////////////////////////////////////////////////////////////////////////////////
// STATUS DECLARATION
////////////////////////////////////////////////////////////////////////////////
typedef enum _DECOMP_STATUS
{
	DS_SUCCESS									= 0,
	DS_ERR										,
	DS_ERR_UNABLE_TO_READ_MEMORY				,
	DS_ERR_INVALID_DATA							,
	DS_ERR_INVALID_PARAM						,
	DS_ERR_FUNCTION_NOT_PROPERLY_INITIALIZED	,
	DS_ERR_IN_SIMPLE_TYPE						,
	DS_ERR_IN_COMPLEX_TYPE						,
	DS_ERR_IN_DECODE_PROC_HEADER				,
	DS_ERR_IN_DECODE_PROC_PARAMS				,
	DS_ERR_INVALID_INDEX						,
	DS_ERR_UNABLE_TO_DECODE_COMPLEX_TYPE		,
}DECOMP_STATUS;


#ifdef __cplusplus
}
#endif

#endif//_RPC_DECOMPILER_H_