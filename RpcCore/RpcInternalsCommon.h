#ifndef _RPC_INTERNALS_COMMON_H_
#define _RPC_INTERNALS_COMMON_H_

// Microsoft Remote Procedure Call (RPC) internals
// Undocumented structure from rpcrt4.dll
//
// global pointers :
// RPC_SERVER*				GlobalRpcServer
// RPC_INTERFACE*			GlobalManagementInterface
// ENDPOINT_MANAGER*		EndpointManager
// LRPC_SERVER* 			GlobalLrpcServer
// SECURITY_PROVIDER_INFO*	ProviderList
//
// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Rpc\
//   - the "ClientsProtocols" key contains protocole sequence accepted
//   - the "Extensions" key ???
//   - the "SecurityService" key contains AuthnSvc constants and the corresponding DLLs
//        the "InitSecurityInterfaceW" function is called as entry-point to initialize the Authentication service
//

#include <windows.h>
#include "..\RpcCommon\ntdll.h"

#define MAX_RPC_INTERFACE_ANNOTATION			64
#define SIMPLE_DICT_SMALL_ARRAY					4


//==============================================================================
// From Winnt.h
// The following structures are redefined to support Wow64 ptr
//
struct _RTL_CRITICAL_SECTION_T;

typedef struct _LIST_ENTRY_T {
   struct _LIST_ENTRY PTR_T Flink;
   struct _LIST_ENTRY PTR_T Blink;
} LIST_ENTRY_T, *PLIST_ENTRY_T;


typedef struct _RTL_CRITICAL_SECTION_DEBUG_T {
    WORD   									Type;
    WORD   									CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION_T PTR_T	CriticalSection;
    LIST_ENTRY_T							ProcessLocksList;
    DWORD 									EntryCount;
    DWORD 									ContentionCount;
    DWORD 									Flags;
    WORD   									CreatorBackTraceIndexHigh;
    WORD   									SpareWORD;
} RTL_CRITICAL_SECTION_DEBUG_T, PTR_T PRTL_CRITICAL_SECTION_DEBUG_T;

typedef struct _RTL_CRITICAL_SECTION_T {
    PRTL_CRITICAL_SECTION_DEBUG_T	DebugInfo;
    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //
    LONG 						LockCount;
    LONG 						RecursionCount;
    VOID PTR_T					OwningThread;        // from the thread's ClientId->UniqueThread
    VOID PTR_T					LockSemaphore;
    VOID PTR_T					SpinCount;			// force size on 64-bit systems when packed
} RTL_CRITICAL_SECTION_T, PTR_T PRTL_CRITICAL_SECTION_T;

//==============================================================================
// From RpcDceP.h
//
typedef struct _RPC_DISPATCH_TABLE_T{
    UINT							DispatchTableCount;
    RPC_DISPATCH_FUNCTION  PTR_T	DispatchTable;
    ULONG_PTR_T                      Reserved;
} RPC_DISPATCH_TABLE_T, PTR_T PRPC_DISPATCH_TABLE_T;

typedef struct _RPC_PROTSEQ_ENDPOINT_T{
    UCHAR PTR_T RpcProtocolSequence;
    UCHAR PTR_T Endpoint;
} RPC_PROTSEQ_ENDPOINT_T, PTR_T PRPC_PROTSEQ_ENDPOINT_T;

typedef struct _RPC_SERVER_INTERFACE_T{
    UINT					Length;
    RPC_IF_ID				InterfaceId;
    RPC_IF_ID				TransferSyntax;
    PRPC_DISPATCH_TABLE_T	DispatchTable;
    UINT					RpcProtseqEndpointCount;
    PRPC_PROTSEQ_ENDPOINT_T RpcProtseqEndpoint;
    RPC_MGR_EPV PTR_T		DefaultManagerEpv;
    void const PTR_T		InterpreterInfo;
    UINT					Flags ;
} RPC_SERVER_INTERFACE_T, PTR_T PRPC_SERVER_INTERFACE_T;


typedef struct _NDR_EXPR_DESC_T
{
    const unsigned short PTR_T	pOffset;
    const unsigned char	PTR_T	pFormatExpr;
} NDR_EXPR_DESC_T;


/*
 * MIDL Stub Descriptor
 */
typedef struct _MIDL_STUB_DESC_T{
    void  PTR_T						RpcInterfaceInformation;
    void  PTR_T						pfnAllocate;				
    void  PTR_T						pfnFree;
	void  PTR_T						pAutoHandle;				
    const VOID  PTR_T				apfnNdrRundownRoutines;		
    const VOID  PTR_T				aGenericBindingRoutinePairs;
    const VOID  PTR_T				apfnExprEval;				
    const VOID  PTR_T				aXmitQuintuple;				
    const unsigned char  PTR_T		pFormatTypes;
    int								fCheckBounds;
    /* Ndr library version. */
    unsigned long					Version;
    VOID PTR_T						pMallocFreeStruct;
    long							MIDLVersion;
    const COMM_FAULT_OFFSETS  PTR_T	CommFaultOffsets;		
    // New fields for version 3.0+
    const VOID PTR_T				aUserMarshalQuadruple;	
    // Notify routines - added for NT5, MIDL 5.0
    const VOID PTR_T				NotifyRoutineTable;		
    /*
     * Reserved for future use.
     */
    ULONG_PTR_T						mFlags;
    // International support routines - added for 64bit post NT5
    const VOID	PTR_T				CsRoutineTables;			
    void  PTR_T						ProxyServerInfo;
    const NDR_EXPR_DESC_T	PTR_T	pExprInfo;					
    // Fields up to now present in win2000 release.
} MIDL_STUB_DESC_T, PTR_T PMIDL_STUB_DESC_T;


/*
 * Server Interpreter's information strucuture.
 */
typedef struct  _MIDL_SERVER_INFO_T{
    PMIDL_STUB_DESC_T				pStubDesc;
    const VOID	PTR_T	PTR_T		DispatchTable;		
    const unsigned char		PTR_T	ProcString;
    const unsigned short	PTR_T	FmtStringOffset;
    const VOID PTR_T PTR_T			ThunkTable;			
    RPC_IF_ID PTR_T					pTransferSyntax;
    ULONG_PTR_T						nCount;
    VOID PTR_T						pSyntaxInfo;		
} MIDL_SERVER_INFO_T, PTR_T PMIDL_SERVER_INFO_T;

//==============================================================================
// Common private structures from rpctr4.dll.
// These structures seems to be constant on all the runtime versions.
//
#pragma pack(1)
typedef struct _SIMPLE_DICT_T{
	VOID PTR_T PTR_T	pArray;
	UINT				ArraySizeInBytes;	//to change : countof array elements
	UINT				NumberOfEntries;
	VOID PTR_T			SmallArray[SIMPLE_DICT_SMALL_ARRAY];
}SIMPLE_DICT_T, PTR_T PSIMPLE_DICT_T;

typedef struct _QUEUE_T{
	VOID PTR_T	Tail;
	VOID PTR_T	Head;
	ULONG		Lentgh;
	VOID PTR_T	SmallArray[SIMPLE_DICT_SMALL_ARRAY];
}QUEUE_T;

typedef struct _MUTEX_T{
	RTL_CRITICAL_SECTION_T	CriticalSection;
}MUTEX_T;

typedef struct _EVENT_T{
	ULONG	hEvent;
} EVENT_T;

#pragma pack()

#define RPC_ADDRESS_TYPE_DG		0x400000
#define RPC_ADDRESS_TYPE_LRPC	0x800000
#define RPC_ADDRESS_TYPE_OSF	0x800

#endif //_RPC_INTERNALS_COMMON_H_