#ifndef _RPC_INTERNALS_H_
#define _RPC_INTERNALS_H_

#include <windows.h>
#include <Rpc.h>


static UINT64 RPC_CORE_RUNTIME_VERSION[] = {
	0x600011DB04001LL,	//6.1.7600.16385
	0x600011DB1446ALL,	//6.1.7601.17514
	0x600011DB1471DLL,	//6.1.7601.18205
	0x600011DB14864LL,	//6.1.7601.18532
	0x600011DB149E0LL,	//6.1.7601.18912
	0x600011DB149F5LL,	//6.1.7601.18933
	0x600011DB149FBLL,	//6.1.7601.18939
	0x600011DB14ABFLL,	//6.1.7601.19135
	0x600011DB15B7BLL,	//6.1.7601.23419
	0x600011DB15CA2LL,	//6.1.7601.23714
        0x600011DB15D08LL,	//6.1.7601.23816
        0x600011DB15D6BLL,	//6.1.7601.23915
	0x600011DB15E35LL, 	//6.1.7601.24117
	0x600011DB15EC4LL, 	//6.1.7601.24260
	0x600011DB15EE3LL	//6.1.7601.24291 
};

#ifdef _WIN64
#define RPC_CORE_DESCRIPTION					"Windows 7 SP1 64bits Wow64 runtime core"
#define RPC_CORE_IS_WOW64						TRUE
#define PTR_T *__ptr32	//WOW64!!!
#define ULONG_PTR_T ULONG
#else
#define RPC_CORE_DESCRIPTION					"Windows 7 SP1 32bits runtime core"
#define RPC_CORE_IS_WOW64						FALSE
#define ULONG_PTR_T ULONG_PTR
#define PTR_T *
#endif


#include "../RpcInternalsCommon.h"

#define RPC_ADDRESS_TYPE_DG		0x400000
#define RPC_ADDRESS_TYPE_LRPC	0x800000
#define RPC_ADDRESS_TYPE_OSF	0x800

#pragma pack(1)
typedef struct _RPC_SERVER_T {
	MUTEX_T						Mutex;
	ULONG						_bIslistening;
	ULONG 						bIsListening;
	ULONG 						MinimumCallThreads;
	ULONG 						bWait;
	ULONG 						OutCalls;
	ULONG 						Unk1;
	ULONG 						InCalls;
	SIMPLE_DICT_T 				AddressDict;
	ULONG 						lAvailableCalls;
	SIMPLE_DICT_T				_ProtSeqQueue;
	ULONG 						Unk2[4];
	ULONG 						OutPackets;
	MUTEX_T	 					Mutex2;
	ULONG 						MaxCalls;
	VOID PTR_T					hEvent;
	ULONG 						Unk3[4];
	SIMPLE_DICT_T 				InterfaceDict;
	ULONG 						__bIslistening;
	ULONG 						bIsMaxCalls1234;
	ULONG 						Unk4[6];
	ULONG 						InPackets;
	RPC_FORWARD_FUNCTION PTR_T	pRpcForwardFunction;
	ULONG 						Unk5[6];
	SIMPLE_DICT_T 				AuthenInfoDict;
}RPC_SERVER_T, PTR_T PRPC_SERVER_T;

typedef struct _RPC_INTERFACE_T
{
	PRPC_SERVER_T				pRpcServer;
	ULONG						Flags;
	ULONG						EpMapperFlags;
	RPC_MGR_EPV PTR_T			pMgrEpv;
	RPC_IF_CALLBACK_FN PTR_T	IfSecurityCallbackFn;
	RPC_SERVER_INTERFACE_T		RpcServerInterface;
	ULONG 						unk2[2];
	BOOL 						bInterfaceSupportMultipleTransferSyntaxes;
	ULONG 						unk3;
	ULONG 						NbTypeManager;
	ULONG 						MaxRpcSize;
	UUID_VECTOR PTR_T 			pUuidVector;
	SIMPLE_DICT_T 				RpcInterfaceManagerDict;
	UCHAR		 				Annotation[MAX_RPC_INTERFACE_ANNOTATION];
	RPC_IF_CALLBACK_FN PTR_T 	IfCallbackFn;
	ULONG 						CurrentNullManagerCalls;
	ULONG 						CurrentAutoListenCalls;
	ULONG 						__Field_DC;
	ULONG 						unk5;
	ULONG 						SecurityCallbackInProgress;
	ULONG 						__Field_E8;
	VOID PTR_T 					pSecurityEntryHashTable;
	SIMPLE_DICT_T 				FwEpDict;
	ULONG 						field_10C;
}RPC_INTERFACE_T, PTR_T PRPC_INTERFACE_T;

typedef struct _RPC_ADDRESS_T {
	VOID PTR_T PTR_T	pVTable;
	ULONG				Magic;
	ULONG				TypeOfAddress;
	ULONG				ReferenceCounter;
	ULONG				unk1;
	WCHAR PTR_T			Name;
	WCHAR PTR_T			Protocole;
	WCHAR PTR_T			Address;
	ULONG				bNamed;
	ULONG				EpAddAddressFlag;
	SIMPLE_DICT_T		__LRPCSassociationDict;
	ULONG				unk3[2];
	ULONG				NbActiveCalls;
	ULONG				unk4[4];
	MUTEX_T				Mutex;
}RPC_ADDRESS_T;

#pragma pack()

#endif // _RPC_INTERNALS_H_
