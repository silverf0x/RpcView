#ifndef _RPC_INTERNALS_H_
#define _RPC_INTERNALS_H_

#include <windows.h>
#include <Rpc.h>

static UINT64 RPC_CORE_RUNTIME_VERSION[] = {
	0x6000324D70000LL,	//6.3.9431.0000
	0x6000325804000LL,	//6.3.9600.16384
	0x6000325804340LL,	//6.3.9600.17216
	0x6000325804407LL,	//6.3.9600.17415
	0x60003258045FFLL,	//6.3.9600.17919
    0x6000325804774LL,	//6.3.9600.18292

	0xA000028004000LL,	//10.0.10240.16384
	0xA00002800401CLL,  //10.0.10240.16412
	0xA0000295A0000LL,	//10.0.10586.0
	0xA0000295A0132LL,	//10.0.10586.306
	0xA0000380603E8LL,	//10.0.14342.1000
	0xA000038190000LL,	//10.0.14361.0
	0xA000038390000LL,	//10.0.14393.0
	0xA000038390052LL,	//10.0.14393.82
	0xA0000383906EALL,	//10.0.14393.1770
	0xA00003AD70000LL,  //10.0.15063.0
	0xA00003AD701BFLL,  //10.0.15063.447
	0xA00003AD702A2LL,  //10.0.15063.674
	0xA00003F6803E8LL,  //10.0.16232.1000
	0xA00003FAB000FLL,  //10.0.16299.15
	0xA00003FAB00C0LL,  //10.0.16299.192
	0xA00003FAB0135LL,  //10.0.16299.309
	0xA00003FAB0173LL,  //10.0.16299.371
    0xA00003FAB02D6LL,  //10.0.16299.726
    0xA0000427903E8LL,  //10.0.17017.1000
	0xA0000428103E8LL,  //10.0.17025.1000
	0xA000042B203EALL,  //10.0.17074.1002
	0xA000042EE0001LL,  //10.0.17134.1
	0xA000042EE0030LL,  //10.0.17134.48
	0xA000042EE0070LL,  //10.0.17134.112
	0xA000042EE00E4LL,  //10.0.17134.228
	0xA000045630001LL   //10.0.17763.1
};

#ifdef _WIN64
	#define RPC_CORE_DESCRIPTION					"Windows 10 64bits wow64 runtime core"
	#define RPC_CORE_IS_WOW64						TRUE
	#define PTR_T *__ptr32	//WOW64!!!
	#define ULONG_PTR_T ULONG
#else
	#define RPC_CORE_DESCRIPTION					"Windows 10 32bits runtime core"
	#define RPC_CORE_IS_WOW64						FALSE
	#define ULONG_PTR_T ULONG_PTR
	#define PTR_T *
#endif

#include "../RpcInternalsCommon.h"


#pragma pack(1)
typedef struct _RPC_SERVER_T{
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
	SIMPLE_DICT_T				_ProtSeqQueue;
	ULONG						Unk2[4];
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

typedef struct _RPC_ADDRESS_T{
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