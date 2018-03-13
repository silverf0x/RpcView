#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <psapi.h>


#include "RpcCore\RpcCore.h"
#include "RpcCommon\RpcView.h"
#include "RpcDecompiler\RpcDecompiler.h"

#pragma warning ( push )
#pragma warning ( disable: 4201 )
typedef struct _DECOMPILE_IT_CTXT {

	DWORD 		TargetPID;
	HANDLE  	hTargetProcess;

	char*		ModuleName;	
	uintptr_t	ModuleBaseAddress;

	bool   		bAbsoluteAddress;
	
	union
	{
		uintptr_t DescriptorArg;		

		struct {
			size_t 		DescriptorOffset;
			uintptr_t	DescriptorAddress;
		};
	};

	union
	{
		uintptr_t FormatStrArg;		

		struct {
			size_t 		FormatStrOffset;
			uintptr_t	FormatStrAddress;
		};
	};

	uint16_t *FormatStrOffsets;

	size_t NumberOfProcedures;

} DECOMPILE_IT_CTXT, *PDECOMPILE_IT_CTXT;
#pragma warning ( pop )


#pragma region RpcViewStub

void
DecompileItInitRpcViewStub
(
	_Inout_ RpcViewHelper_T *RpcViewStub,
	_In_ PVOID Context
);


// #define ASSIGN_RPC_VIEW_STUB(RpcViewHelperStub, Context) \
// RpcViewHelper_T RpcViewHelperStub; \
// DecompileItInitRpcViewStub(&RpcViewHelperStub, (PVOID) Context)


#pragma endregion RpcViewStub
