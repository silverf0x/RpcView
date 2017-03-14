#ifndef _RPC_INTERNALS_H_
#define _RPC_INTERNALS_H_

#include <windows.h>
#include <Rpc.h>

static UINT64 RPC_CORE_RUNTIME_VERSION[] = {
	0x500010a280884LL,	//5.1.2600.2180
	0x500010a281588LL,	//5.1.2600.5512
	0x500010a281786LL,	//5.1.2600.6022
	0x500010a28194dLL,	//5.1.2600.6477
	0x500020ECE0F77LL	//5.2.3790.3959
};

#ifdef _WIN64
	#define RPC_CORE_IS_WOW64						TRUE
	#define PTR_T *__ptr32	//WOW64!!!
	#define ULONG_PTR_T ULONG
#else
	#define RPC_CORE_IS_WOW64						FALSE
	#define ULONG_PTR_T ULONG_PTR
	#define PTR_T *
#endif

#include "../RpcInternalsCommon.h"
 
#define RPC_ADDRESS_TYPE_DG		0x400000
#define RPC_ADDRESS_TYPE_LRPC	0x800000
#define RPC_ADDRESS_TYPE_OSF	0x800

#pragma pack(1)
typedef struct _RPC_SERVER_T{
	MUTEX_T						MUTEX;
	ULONG						Unk1;
	ULONG						Unk2;
	BOOL						bIsListening;
	ULONG						MinimumCallThreads;
	ULONG						bWait;
	ULONG						OutCalls;
	ULONG						Unk3;
	ULONG						InCalls;
	SIMPLE_DICT_T				AddressDict;
	ULONG						Unk3_;
	SIMPLE_DICT_T				_ProtSeqQueue;
	ULONG						Unk4[4];
	ULONG						OutPackets;
	ULONG						Unk5;
	MUTEX_T						Mutex2;
	ULONG						MaxCalls;
	VOID PTR_T					hEvent;
	ULONG						Unk6[6];
	SIMPLE_DICT_T				InterfaceDict;
	BOOL						___bIsListening;
	BOOL						bIsListenMaxCallsDefault;
	ULONG						Unk7[6];
	ULONG						InPackets;
	RPC_FORWARD_FUNCTION PTR_T	pRpcForwardFunction;
	ULONG						Unk8[6];
	SIMPLE_DICT_T				AuthenInfoDict;
}RPC_SERVER_T, PTR_T PRPC_SERVER_T;

typedef struct _RPC_INTERFACE_T
{
	RPC_SERVER_T PTR_T				pRpcServer;
	ULONG						Flags;
	ULONG						EpMapperFlags;
	RPC_MGR_EPV PTR_T			pMgrEpv;
	RPC_IF_CALLBACK_FN PTR_T	IfCallbackFn;
	RPC_SERVER_INTERFACE_T		RpcServerInterface;
	ULONG						pSyntaxInfo;
	ULONG						pTransfertSyntaxes;
	ULONG						pTransfertSyntaxesCount;
	ULONG						Unk1;
	ULONG						NbTypeManager;
	ULONG						MaxRpcSize;
	ULONG						Unk2;
	ULONG						pUuidVector;
	SIMPLE_DICT_T				RpcInterfaceManagerDict;
	UCHAR		 				Annotation[MAX_RPC_INTERFACE_ANNOTATION];
	SIMPLE_DICT_T				FwEpDict;
	ULONG						Unk3[6];
}RPC_INTERFACE_T, PTR_T PRPC_INTERFACE_T;

typedef struct _RPC_ADDRESS_T{
	VOID PTR_T PTR_T	pVTable;
	ULONG				Magic;
	ULONG				TypeOfAddress;
	ULONG				ReferenceCounter;
	WCHAR PTR_T			Name;
	WCHAR PTR_T			Protocole;
	WCHAR PTR_T			Address;
	ULONG				bNamed;
	ULONG				EpAddAddressFlag;
	ULONG				unk4[6];
	MUTEX_T				Mutex;
}RPC_ADDRESS_T;

#pragma pack()
#endif // _RPC_INTERNALS_H_