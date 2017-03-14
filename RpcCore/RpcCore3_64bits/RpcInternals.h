#ifndef _RPC_INTERNALS_H_
#define _RPC_INTERNALS_H_

#include <windows.h>
#include <Rpc.h>

static UINT64 RPC_CORE_RUNTIME_VERSION[] = {
	0x6000223f04000LL,	//6.2.9200.16384
	0x6000223f040eeLL	//6.2.9200.16622
};

#define RPC_CORE_DESCRIPTION					"Windows 8 64bits runtime core"
#define RPC_CORE_IS_WOW64						FALSE

#define ULONG_PTR_T ULONG_PTR
#define PTR_T *

#include "../RpcInternalsCommon.h"

#pragma pack(1)
typedef struct _RPC_SERVER_T {
	MUTEX_T						Mutex;
	ULONG							__bIslistening;
	ULONG							bIsListening;
	ULONG							MinimumCallThreads;
	ULONG							Wait;
	ULONG							OutCalls;
	ULONG							Unk1;
	ULONG							InCalls;
	ULONG							Unk2;
	SIMPLE_DICT_T					AddressDict;
	ULONG							lAvailableCalls;
	ULONG							Unk3;
	ULONG							Unk4[20];
	ULONG							OutPackets;
	ULONG							Unk5;
	MUTEX_T						Mutex2;
	ULONG							MaxCalls;
	ULONG							Unk6;
	VOID PTR_T					hEvent;
	ULONG							Unk7[4];
	SIMPLE_DICT_T					InterfaceDict;
	ULONG							_bIsListening;
	ULONG							bIsMaxCalls1234;
	ULONG							Unk8[6];
	ULONG							InPackets;
	ULONG							Unk9;
	RPC_FORWARD_FUNCTION PTR_T	pRpcForwardFunction;
	ULONG							Unk10[6];
	SIMPLE_DICT_T					AuthenInfoDict;
	LIST_ENTRY_T					RpcIfGroupListEntry;
	ULONG	PTR_T					__SRWLock;
	LIST_ENTRY_T					field_1E0;
}RPC_SERVER_T, PTR_T PRPC_SERVER_T;

typedef struct _RPC_INTERFACE_T
{
	PRPC_SERVER_T				pRpcServer;
	ULONG						Flags;
	ULONG						Unk1;
	MUTEX_T					Mutex;
	ULONG						EpMapperFlags;
	ULONG						Unk2;
	RPC_MGR_EPV PTR_T			pMgrEpv;
	RPC_IF_CALLBACK_FN PTR_T	IfCallbackFn;
	RPC_SERVER_INTERFACE_T	RpcServerInterface;
	PMIDL_SYNTAX_INFO			pSyntaxInfo;
	VOID PTR_T				pTransfertSyntaxes;
	ULONG						TransfertSyntaxesCount;
	ULONG						__Field_C4;
	ULONG						NbTypeManager;
	ULONG						MaxRpcSize;
	UUID_VECTOR  PTR_T		pUuidVector;
	SIMPLE_DICT_T				RpcInterfaceManagerDict;
	UCHAR						Annotation[MAX_RPC_INTERFACE_ANNOTATION];
	ULONG						IsCallSizeLimitReached;
	ULONG						currentNullManagerCalls;
	ULONG						__Field_150;
	ULONG						__Field_154;
	ULONG						__Field_158;
	ULONG						SecurityCallbackInProgress;
	ULONG						SecurityCacheEntry;
	ULONG						field_164;
	VOID PTR_T				__SecurityCacheEntries[16];
	SIMPLE_DICT_T				FwEpDict;
	ULONG						Unk3[6];
	struct RPCP_INTERFACE_GROUP PTR_T pRpcpInterfaceGroup;
}RPC_INTERFACE_T, PTR_T PRPC_INTERFACE_T;

#define RPC_ADDRESS_TYPE_DG		0x400000
#define RPC_ADDRESS_TYPE_LRPC	0x800000
#define RPC_ADDRESS_TYPE_OSF	0x800

typedef struct _RPC_ADDRESS_T {
	VOID PTR_T				pVtable;
	ULONG					Magic;
	ULONG					TypeOfAddress;
	ULONG					ReferenceCounter;
	ULONG					Unk1[3];
	WCHAR PTR_T				Name;
	WCHAR PTR_T				Protocole;
	WCHAR PTR_T				Address;
	ULONG					bNamed;
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