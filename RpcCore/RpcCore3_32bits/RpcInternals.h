#ifndef _RPC_INTERNALS_H_
#define _RPC_INTERNALS_H_

#include <windows.h>
#include <Rpc.h>


static UINT64 RPC_CORE_RUNTIME_VERSION[] = {
	0x6000223f04000LL,	//6.2.9200.16384
	0x6000223f040eeLL	//6.2.9200.16622
};

#ifdef _WIN64
#define RPC_CORE_DESCRIPTION					"Windows 8 64bits Wow64 runtime core"
#define RPC_CORE_IS_WOW64						TRUE
#define PTR_T *__ptr32	//WOW64!!!
#define ULONG_PTR_T ULONG
#else
#define RPC_CORE_DESCRIPTION					"Windows 8 32bits runtime core"
#define RPC_CORE_IS_WOW64						FALSE
#define ULONG_PTR_T ULONG_PTR
#define PTR_T *
#endif

#include "../RpcInternalsCommon.h"


#pragma pack(1)
typedef struct _RPC_SERVER_T {
	MUTEX_T						MUTEX;
	BOOL						_bIslistening;
	BOOL						bIsListening;
	ULONG						MinimumCallThreads;
	BOOL						bWait;
	ULONG						OutCalls;
	ULONG						Unk1;
	ULONG						InCalls;
	SIMPLE_DICT_T				AddressDict;
	ULONG						lAvailableCalls;
	ULONG						Unk2[11];
	ULONG						OutPackets;
	MUTEX_T						Mutex2;
	ULONG						MaxCalls;
	VOID PTR_T					hEvent;
	ULONG						Unk4[4];
	SIMPLE_DICT_T				InterfaceDict;
	BOOL						__bIslistening;
	BOOL						bIsMaxCall1234;
	ULONG						Unk5[6];
	ULONG						InPackets;
	RPC_FORWARD_FUNCTION PTR_T	pRpcForwardFunction;
	ULONG						Unk6[6];
	SIMPLE_DICT_T				AuthenInfoDict;
	LIST_ENTRY_T				RpcIfGroupListEntry;
	ULONG	PTR_T				__SRWLock;
	LIST_ENTRY_T				__138;
}RPC_SERVER_T, PTR_T PRPC_SERVER_T;

typedef struct _RPC_INTERFACE_T
{
	PRPC_SERVER_T				pRpcServer;
	ULONG						Flags;
	MUTEX_T						Mutex;
	ULONG						EpMapperFlags;
	RPC_MGR_EPV PTR_T			pMgrEpv;
	RPC_IF_CALLBACK_FN PTR_T	IfSecurityCallback;
	RPC_SERVER_INTERFACE_T		RpcServerInterface;
	MIDL_SYNTAX_INFO PTR_T		pSyntaxInfo;
	VOID PTR_T					pTransfertSyntaxes;
	ULONG						TransfertSyntaxesCount;
	ULONG						Unk1;
	ULONG						NbTypeManager;
	ULONG						MaxRpcSize;
	UUID_VECTOR PTR_T			pUuidVector;
	SIMPLE_DICT_T				RpcInterfaceManagerDict;
	UCHAR						Annotation[64];
	RPC_IF_CALLBACK_FN PTR_T	IfCallbackFn;
	ULONG						IsCallSizeLimitReached;
	ULONG						currentNullManagerCalls;
	ULONG						currentAutoListenCalls;
	ULONG						Unk2[2];
	ULONG						SecurityCallbackInProgress;
	ULONG						SecurityCacheEntry;
	ULONG						__SecurityCacheEntries[16];
	SIMPLE_DICT_T				FwEpDict;
	ULONG						Unk3[4];
}RPC_INTERFACE_T, PTR_T PRPC_INTERFACE_T;

#define RPC_ADDRESS_TYPE_DG		0x400000
#define RPC_ADDRESS_TYPE_LRPC	0x800000
#define RPC_ADDRESS_TYPE_OSF	0x800

typedef struct _RPC_ADDRESS_T {
	VOID PTR_T PTR_T pVTable;
	ULONG					Magic;
	ULONG					TypeOfAddress;
	ULONG					ReferenceCounter;
	ULONG					Unk1;
	WCHAR PTR_T				Name;
	WCHAR PTR_T				Protocole;
	WCHAR PTR_T				Address;
	BOOL					bNamed;
	ULONG					EpAddAddressFlag;
	SIMPLE_DICT_T			__LRPCSassociationSimpleDict;
	ULONG					__Field_68;
	ULONG					Unk3;
	ULONG					NbActiveCalls;
	ULONG					__Field_74;
	ULONG					Unk4[6];
	ULONG					__Field_90;
	MUTEX_T					Mutex;
}RPC_ADDRESS_T;

#pragma pack()

#endif // _RPC_INTERNALS_H_