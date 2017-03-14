#ifndef _RPC_COMMON_H_
#define _RPC_COMMON_H_

#include <windows.h>
#include <stdio.h>

#define INVALID_PID_VALUE			((UINT)-1)
#define INVALID_PROC_COUNT			((UINT)-1)
#define RPC_MAX_LENGTH				260
#define INVALID_IF_CALLBACK_ADDRESS	0x400000
#define HEX_SIZE_OF_PTR				(2*sizeof(void*))

typedef enum _AddressRepresentation_T{
	AddressRepresentation_Unknown = 0,
	AddressRepresentation_Absolute,
	AddressRepresentation_RVA
}AddressRepresentation_T;

#ifdef _DEBUG
	//
	// DEBUG
	//
	#ifndef DBG_NEW      
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )      
		#define new DBG_NEW   
	#endif

	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

	#define OS_FREE(pMem)			free(pMem)
	#define OS_ALLOC(Size)			calloc(Size,1);
	#define OS_DEBUG(...)			_cprintf(__VA_ARGS__)
	#define DEBUG_BREAK()			__debugbreak()
#else
	//
	// RELEASE
	//
	#define OS_FREE(pMem)			HeapFree(GetProcessHeap(), 0, pMem)
	#define OS_ALLOC(Size)			HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size)
	#define OS_DEBUG(...)			_cprintf(__VA_ARGS__)
	#define DEBUG_BREAK()
#endif  // _DEBUG

#endif//_RPC_COMMON_H_