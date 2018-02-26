#ifndef _RPC_CORE_H_
#define _RPC_CORE_H_

#include <windows.h>

#define RPC_CORE_EXPORT_SYMBOL			"RpcCoreHelper"
#define MAX_RPC_INTERFACE_ANNOTATION	64
#define MAX_GUID						40
#define MAX_CLSID_NAME					128

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

#define RPC_PROCESS_INFO_DEFAULT	0x0
#define	RPC_PROCESS_INFO_MISC		0x1
#define	RPC_PROCESS_INFO_RPC		0x2
#define	RPC_PROCESS_INFO_ALL		(RPC_PROCESS_INFO_MISC | RPC_PROCESS_INFO_RPC)

typedef enum _RpcProcessType_T{
	RpcProcessType_UNKNOWN = 0,
	RpcProcessType_RPC,
	RpcProcessType_DCOM,
	RpcProcessType_HYBRID
}RpcProcessType_T;

typedef struct _RpcProcessInfo_T{
	//Default
	DWORD				Pid;
	DWORD				ParentPid;
#ifdef _WIN64
	BOOL				bIsWow64;
#endif
	//Misc
	WCHAR				Name[MAX_PATH];
	WCHAR				Path[MAX_PATH];
	WCHAR				CmdLine[MAX_PATH];
	WCHAR				User[MAX_PATH];
	WCHAR				Desktop[MAX_PATH];
	DWORD				Session;
	UINT64				Version;
	HICON				hIcon;
	WCHAR				Description[MAX_PATH];
	FILETIME			CreationTime;
	//Rpc
	RpcProcessType_T	RpcProcessType;
	DWORD				MaxCalls;
	BOOL				bIsServer;
	BOOL				bIsListening;
	UINT				InterfacesCount;
	UINT				EndpointsCount;
	UINT				SspCount;
	DWORD				InCalls;
	DWORD				OutCalls;
	DWORD				InPackets;
	DWORD				OutPackets;
	//...
}RpcProcessInfo_T;


typedef struct _NdrInfo_T{
	ULONG		Version;		// NDR version required for the stub.
	ULONG		MIDLVersion;	// Version of the MIDL compiler used to compile the .idl file
	ULONG_PTR	mFlags;			// Flag describing the attributes of the stub (RPCFLG_HAS_MULTI_SYNTAXES, RPCFLG_HAS_CALLBACK, or RPC_INTERFACE_HAS_PIPES)
}NdrInfo_T;


typedef enum _TypeOfStub_T{
	TypeOfStub_Unknown = 0,
	TypeOfStub_Interpreted,
	TypeOfStub_Inlined,
	TypeOfStub_TypeLib,
	TypeOfStub_Hybrid
}TypeOfStub_T;

typedef enum _IfType_T{
	IfType_Unknown = 0,
	IfType_RPC,
	IfType_DCOM,
	IfType_OLE,
}IfType_T;

#define RPC_INTERFACE_INFO_DEFAULT	0x0
#define RPC_INTERFACE_INFO_MISC		0x1
#define RPC_INTERFACE_INFO_RPC		0x2
#define RPC_INTERFACE_INFO_NDR		0x4
#define RPC_INTERFACE_INFO_DCOM		0x8
#define RPC_INTERFACE_INFO_ALL		(RPC_INTERFACE_INFO_MISC | RPC_INTERFACE_INFO_RPC | RPC_INTERFACE_INFO_NDR |RPC_INTERFACE_INFO_DCOM)


typedef struct _RpcInterfaceInfo_T{
	//Default
	DWORD					Pid;
#ifdef _WIN64
	BOOL					bWow64Process;
#endif
	RPC_IF_ID				If;
	RPC_SYNTAX_IDENTIFIER	TransfertSyntax;
	UINT					Flags;
	IfType_T				IfType;
	//Misc
	WCHAR					Name[MAX_PATH];
	WCHAR					Location[MAX_PATH];
	DWORD					LocationState;
	DWORD					LocationType;
	VOID*					pLocationBase;
	SIZE_T					LocationSize;
	WCHAR					Description[MAX_PATH];
	//RPC
	BOOL					bIsRegistered;									//EP mapper
	UCHAR					Annotation[MAX_RPC_INTERFACE_ANNOTATION];		//annotation is natively in UCHAR
	RPC_IF_CALLBACK_FN*		IfCallbackFn;									//security callback if any
	//NDR
	UINT					NumberOfProcedures;
	NdrInfo_T				NdrInfo;
	TypeOfStub_T			TypeOfStub;
	ULONG*					ppProcAddressTable;								//Table containing the address of each function
	USHORT*					pFormatStringOffsetTable;						//Table containing the offset of each function encoding
	UCHAR*					pProcFormatString;			// Midl server info
	UCHAR*					pTypeFormatString;			// Midl stub desc
	VOID*					apfnExprEval;				// Midl stub desc
	USHORT*					pExprOffset;				// Midl stub desc
	UCHAR*					pExprFormatString;			// Midl stub desc
	//DCOM information
	WCHAR					ProxyStubClsid32[MAX_GUID];
	WCHAR					TypeLib[MAX_GUID];
	WCHAR					TypeLibName[MAX_PATH];
	WCHAR					ClsidName[MAX_CLSID_NAME];
	WCHAR					InprocServer32[MAX_PATH];
	WCHAR					TypeLibVersion[20];
	WCHAR					TypeLibPath[MAX_PATH];
	//...
}RpcInterfaceInfo_T;


typedef struct _RpcEndpointInfo_T{
	WCHAR*	pName;
	WCHAR*	pProtocole;
	//...
}RpcEndpointInfo_T;


//Private name: unnamed in PDB file
typedef struct _RPC_AUTH_INFO_T{
	WCHAR*	pPrincipalName;
	ULONG	AuthSvc;
	VOID*	pGetKeyFn;
	VOID*	pArg;
}RPC_AUTH_INFO_T;


typedef struct _RpcAuthInfo_T{
	ULONG	Capabilities;
	ULONG	Version;
	WCHAR	PrincipalName[MAX_PATH];
	ULONG	AuthSvc;
	WCHAR	Name[MAX_PATH];
	WCHAR	Comment[MAX_PATH];
	WCHAR	DllName[MAX_PATH];
	VOID*	pGetKeyFn;
	VOID*	pArg;
}RpcAuthInfo_T;


////////////////////////////////////////////////////////////////////////////////
// Callback function type definition
////////////////////////////////////////////////////////////////////////////////
typedef BOOL (__fastcall* RpcCoreEnumProcessInterfacesCallbackFn_T)(RpcInterfaceInfo_T* pRpcInterfaceInfo, VOID* pContext, BOOL* pbContinue);
typedef BOOL (__fastcall* RpcCoreEnumProcessEndpointsCallbackFn_T)(DWORD Pid, RpcEndpointInfo_T* pRpcEndpointInfo, VOID* pContext, BOOL* pbContinue);
typedef BOOL (__fastcall* RpcCoreEnumProcessAuthInfoCallbackFn_T)(DWORD Pid, RpcAuthInfo_T* pRpcAuthInfo, VOID* pContext, BOOL* pbContinue);


////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

typedef VOID*				(__fastcall* RpcCoreInitFn_T)(BOOL bForce);
typedef VOID				(__fastcall* RpcCoreUninitFn_T)(VOID* pRpcCoreCtxt);
typedef RpcProcessInfo_T*	(__fastcall* RpcCoreGetProcessInfoFn_T)(void* pRpcCoreCtxt, DWORD Pid, DWORD Ppid,ULONG ProcessInfoMask);
typedef VOID				(__fastcall* RpcCoreFreeProcessInfoFn_T)(void* pRpcCoreCtxt, RpcProcessInfo_T* pRpcProcessInfo);
typedef RpcInterfaceInfo_T*	(__fastcall* RpcCoreGetInterfaceInfoFn_T)(void* pRpcCoreCtxt, DWORD Pid, RPC_IF_ID* pIf,ULONG InterfaceInfoMask);
typedef VOID				(__fastcall* RpcCoreFreeInterfaceInfoFn_T)(void* pRpcCoreCtxt, RpcInterfaceInfo_T* pRpcInterfaceInfo);
typedef BOOL				(__fastcall* RpcCoreEnumProcessInterfacesFn_T)(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessInterfacesCallbackFn_T RpcCoreEnumProcessInterfacesCallbackFn, void* pCallbackCtxt,ULONG InterfaceInfoMask);
typedef BOOL				(__fastcall* RpcCoreEnumProcessEndpointsFn_T)(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessEndpointsCallbackFn_T RpcCoreEnumProcessEndpointsCallbackFn, void* pCallbackCtxt);
typedef BOOL				(__fastcall* RpcCoreEnumProcessAuthInfoFn_T)(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessAuthInfoCallbackFn_T RpcCoreEnumProcessAuthInfoCallbackFn, void* pCallbackCtxt);


typedef struct _RpcCore_T{
	UINT64*								RuntimeVersion;		//the supported version (forx example 0x600011DB04001LL (6.1.7600.16385) for Windows 7 64bits )
	//const char*							pDescription;
	BOOL								bWow64Helper;
	BOOL								bForceLoading;
	RpcCoreInitFn_T						RpcCoreInitFn;
	RpcCoreUninitFn_T					RpcCoreUninitFn;
	RpcCoreGetProcessInfoFn_T			RpcCoreGetProcessInfoFn;
	RpcCoreFreeProcessInfoFn_T			RpcCoreFreeProcessInfoFn;
	RpcCoreGetInterfaceInfoFn_T			RpcCoreGetInterfaceInfoFn;
	RpcCoreFreeInterfaceInfoFn_T		RpcCoreFreeInterfaceInfoFn;
	RpcCoreEnumProcessInterfacesFn_T	RpcCoreEnumProcessInterfacesFn;
	RpcCoreEnumProcessEndpointsFn_T		RpcCoreEnumProcessEndpointsFn;
	RpcCoreEnumProcessAuthInfoFn_T		RpcCoreEnumProcessAuthInfoFn;
}RpcCore_T;

#ifdef __cplusplus
}
#endif

#endif //_RPC_CORE_H_