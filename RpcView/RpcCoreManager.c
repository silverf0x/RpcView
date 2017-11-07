#include <../../RpcCore/RpcCore.h>
#include "..\RpcCommon\Misc.h"
#include "..\RpcCommon\RpcCommon.h"
#include <conio.h>

typedef struct _RpcCoreManager_T{
    void*       pNativeCoreCtxt;
    RpcCore_T*  pNativeCore;
#ifdef _WIN64
    void*       pWow64CoreCtxt;
    RpcCore_T*  pWow64Core;
#endif
}RpcCoreManager_T; 

// RpcCore
VOID*				__fastcall RpcCoreInit();						//returns a private context for the RpcCoreEngine
VOID				__fastcall RpcCoreUninit(VOID* pRpcCoreCtxt);
RpcProcessInfo_T*	__fastcall RpcCoreGetProcessInfo(void* pRpcCoreCtxt, DWORD Pid, DWORD Ppid, ULONG ProcessInfoMask);
VOID				__fastcall RpcCoreFreeProcessInfo(void* pRpcCoreCtxt, RpcProcessInfo_T* pRpcProcessInfo);
RpcInterfaceInfo_T*	__fastcall RpcCoreGetInterfaceInfo(void* pRpcCoreCtxt, DWORD Pid, RPC_IF_ID* pIf, ULONG InterfaceInfoMask);
VOID				__fastcall RpcCoreFreeInterfaceInfo(void* pRpcCoreCtxt, RpcInterfaceInfo_T* pRpcInterfaceInfo);
BOOL				__fastcall RpcCoreEnumProcessInterfaces(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessInterfacesCallbackFn_T RpcCoreEnumProcessInterfacesCallbackFn, void* pCallbackCtxt, ULONG InterfaceInfoMask);
BOOL				__fastcall RpcCoreEnumProcessEndpoints(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessEndpointsCallbackFn_T RpcCoreEnumProcessEndpointsCallbackFn, void* pCallbackCtxt);
BOOL				__fastcall RpcCoreEnumProcessAuthInfo(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessAuthInfoCallbackFn_T RpcCoreEnumProcessAuthInfoCallbackFn, void* pCallbackCtxt);


RpcCore_T	gRpcCoreManager =
{
    0,
    //"Generic RpcCore Manager",
    FALSE,
    &RpcCoreInit,
    &RpcCoreUninit,
    &RpcCoreGetProcessInfo,
    &RpcCoreFreeProcessInfo,
    &RpcCoreGetInterfaceInfo,
    &RpcCoreFreeInterfaceInfo,
    &RpcCoreEnumProcessInterfaces,
    &RpcCoreEnumProcessEndpoints,
    &RpcCoreEnumProcessAuthInfo,
};

//------------------------------------------------------------------------------
BOOL NTAPI LoadCoreEngine(RpcCore_T** ppRpcCoreHelper, void** ppRpcCoreCtxt, BOOL bWow64Helper)
{
    WIN32_FIND_DATAA	Win32FindData;
    HMODULE				hLib;
   // Version_T			Version;
    HANDLE				hFindFile = INVALID_HANDLE_VALUE;
    RpcCore_T*	        pRpcCoreHelper = NULL;
    BOOL				bResult = FALSE;

    hFindFile = FindFirstFileA("*.dll", &Win32FindData);
    if (hFindFile == INVALID_HANDLE_VALUE) goto End;
    do
    {
        hLib = LoadLibraryA(Win32FindData.cFileName);
        if (hLib != NULL)
        {
            pRpcCoreHelper = (RpcCore_T*)(ULONG_PTR)GetProcAddress(hLib, RPC_CORE_EXPORT_SYMBOL);
            if (pRpcCoreHelper != NULL)
            {
                *ppRpcCoreCtxt = pRpcCoreHelper->RpcCoreInitFn();
                if (*ppRpcCoreCtxt != NULL)
                {
                    pRpcCoreHelper->RpcCoreUninitFn(*ppRpcCoreCtxt);
                    if (bWow64Helper == pRpcCoreHelper->bWow64Helper)
                    {
                        _cprintf("RpcCore    : %s\n", Win32FindData.cFileName);
                        _cprintf("Helper     : 0x%p\n", pRpcCoreHelper);
                        _cprintf("Wow64      : ");
                        if (pRpcCoreHelper->bWow64Helper) _cprintf("TRUE\n"); else _cprintf("FALSE\n");
                        *ppRpcCoreHelper = pRpcCoreHelper;
                        bResult = TRUE;
                        goto End;
                    }
                }
            }
            FreeLibrary(hLib);
        }
    } while (FindNextFileA(hFindFile, &Win32FindData));
End:
    if (hFindFile != INVALID_HANDLE_VALUE) FindClose(hFindFile);
    return (bResult);
}


//-----------------------------------------------------------------------------
VOID* __fastcall RpcCoreInit()
{
    RpcCoreManager_T*   pRpcCoreManager;

    pRpcCoreManager = (RpcCoreManager_T*)OS_ALLOC(sizeof(RpcCoreManager_T));

    if (!LoadCoreEngine(&pRpcCoreManager->pNativeCore, &pRpcCoreManager->pNativeCoreCtxt, FALSE))
    {
		const char Caption[]	= "Unsupported runtime version";
#ifdef _WIN64
		const char Msg[]		= "Please send us a mail to contact@rpcview.org with your rpcrt4.dll files (in System32 and SysWOW64 directories)";
#else
		const char Msg[]		= "Please send us a mail to contact@rpcview.org with your rpcrt4.dll file (in System32 directory)";
#endif
#ifdef _DEBUG
        _cprintf("** %s **\n%s\n",Caption,Msg);
#else
		MessageBoxA(NULL, Msg, Caption, MB_OK);
#endif
		ExitProcess(0);
    }
    pRpcCoreManager->pNativeCoreCtxt = pRpcCoreManager->pNativeCore->RpcCoreInitFn();
#ifdef _WIN64
    if (!LoadCoreEngine(&pRpcCoreManager->pWow64Core, &pRpcCoreManager->pWow64CoreCtxt, TRUE)) goto Cleanup;
    pRpcCoreManager->pWow64CoreCtxt = pRpcCoreManager->pWow64Core->RpcCoreInitFn();
#endif
End:
    return (pRpcCoreManager);
#ifdef _WIN64
Cleanup:
#endif
    OS_FREE(pRpcCoreManager);
    pRpcCoreManager = NULL;
    goto End;
}

//-----------------------------------------------------------------------------
VOID __fastcall RpcCoreUninit(VOID* pRpcCoreCtxt)
{
    RpcCoreManager_T*   pRpcCoreManager;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

    pRpcCoreManager->pNativeCore->RpcCoreUninitFn(pRpcCoreManager->pNativeCoreCtxt);
#ifdef _WIN64
	pRpcCoreManager->pWow64Core->RpcCoreUninitFn(pRpcCoreManager->pWow64CoreCtxt);
#endif
	OS_FREE(pRpcCoreManager);
End:
    return;
}

//-----------------------------------------------------------------------------
RpcProcessInfo_T*  __fastcall RpcCoreGetProcessInfo(void* pRpcCoreCtxt, DWORD Pid, DWORD Ppid, ULONG ProcessInfoMask)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;
    RpcProcessInfo_T*   pRpcProcessInfo = NULL;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

#ifdef _WIN64
    if (IsProcessWow64(Pid))
    {
        pRpcProcessInfo = pRpcCoreManager->pWow64Core->RpcCoreGetProcessInfoFn(pRpcCoreManager->pWow64CoreCtxt, Pid, Ppid, ProcessInfoMask);
    }
    else
#endif
    {
        pRpcProcessInfo = pRpcCoreManager->pNativeCore->RpcCoreGetProcessInfoFn(pRpcCoreManager->pNativeCoreCtxt, Pid, Ppid, ProcessInfoMask);
    }
End:
    return (pRpcProcessInfo);
}

//-----------------------------------------------------------------------------
VOID __fastcall RpcCoreFreeProcessInfo(void* pRpcCoreCtxt, RpcProcessInfo_T* pRpcProcessInfo)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager != NULL)
    {
#ifdef _WIN64
		if (IsProcessWow64(pRpcProcessInfo->Pid))
		{
			pRpcCoreManager->pWow64Core->RpcCoreFreeProcessInfoFn(pRpcCoreManager->pWow64CoreCtxt, pRpcProcessInfo);
		}
		else
#endif
		{
			pRpcCoreManager->pNativeCore->RpcCoreFreeProcessInfoFn(pRpcCoreManager->pNativeCoreCtxt, pRpcProcessInfo);
		}
    }
}

//-----------------------------------------------------------------------------
RpcInterfaceInfo_T*	__fastcall RpcCoreGetInterfaceInfo(void* pRpcCoreCtxt, DWORD Pid, RPC_IF_ID* pIf, ULONG InterfaceInfoMask)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;
    RpcInterfaceInfo_T* pRpcInterfaceInfo = NULL;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

#ifdef _WIN64
    if (IsProcessWow64(Pid))
    {
        pRpcInterfaceInfo = pRpcCoreManager->pWow64Core->RpcCoreGetInterfaceInfoFn(pRpcCoreManager->pWow64CoreCtxt, Pid, pIf, InterfaceInfoMask);
    }
    else
#endif
    {
        pRpcInterfaceInfo = pRpcCoreManager->pNativeCore->RpcCoreGetInterfaceInfoFn(pRpcCoreManager->pNativeCoreCtxt, Pid, pIf, InterfaceInfoMask);
    }
End:
    return (pRpcInterfaceInfo);
}

//-----------------------------------------------------------------------------
VOID __fastcall RpcCoreFreeInterfaceInfo(void* pRpcCoreCtxt, RpcInterfaceInfo_T* pRpcInterfaceInfo)
{
	RpcCoreManager_T*   pRpcCoreManager = NULL;

	if (pRpcInterfaceInfo == NULL) return;
	pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
	if (pRpcCoreManager != NULL)
	{
#ifdef _WIN64
		if (IsProcessWow64(pRpcInterfaceInfo->Pid))
		{
			pRpcCoreManager->pWow64Core->RpcCoreFreeInterfaceInfoFn(pRpcCoreManager->pWow64CoreCtxt, pRpcInterfaceInfo);
		}
		else
#endif
		{
			pRpcCoreManager->pNativeCore->RpcCoreFreeInterfaceInfoFn(pRpcCoreManager->pNativeCoreCtxt, pRpcInterfaceInfo);
		}
	}
}

//-----------------------------------------------------------------------------
BOOL __fastcall RpcCoreEnumProcessInterfaces(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessInterfacesCallbackFn_T RpcCoreEnumProcessInterfacesCallbackFn, void* pCallbackCtxt, ULONG InterfaceInfoMask)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;
    BOOL                bResult         = FALSE;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

#ifdef _WIN64
    if (IsProcessWow64(Pid))
    {
        bResult = pRpcCoreManager->pWow64Core->RpcCoreEnumProcessInterfacesFn(pRpcCoreManager->pWow64CoreCtxt, Pid, RpcCoreEnumProcessInterfacesCallbackFn, pCallbackCtxt, InterfaceInfoMask);
    }
    else
#endif
    {
        bResult = pRpcCoreManager->pNativeCore->RpcCoreEnumProcessInterfacesFn(pRpcCoreManager->pNativeCoreCtxt, Pid, RpcCoreEnumProcessInterfacesCallbackFn, pCallbackCtxt, InterfaceInfoMask);
    }
End:
    return (bResult);
}

//-----------------------------------------------------------------------------
BOOL __fastcall RpcCoreEnumProcessEndpoints(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessEndpointsCallbackFn_T RpcCoreEnumProcessEndpointsCallbackFn, void* pCallbackCtxt)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;
    BOOL                bResult = FALSE;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

#ifdef _WIN64
    if (IsProcessWow64(Pid))
    {
        bResult = pRpcCoreManager->pWow64Core->RpcCoreEnumProcessEndpointsFn(pRpcCoreManager->pWow64CoreCtxt, Pid, RpcCoreEnumProcessEndpointsCallbackFn, pCallbackCtxt);
    }
    else
#endif
    {
        bResult = pRpcCoreManager->pNativeCore->RpcCoreEnumProcessEndpointsFn(pRpcCoreManager->pNativeCoreCtxt, Pid, RpcCoreEnumProcessEndpointsCallbackFn, pCallbackCtxt);
    }
End:
    return (bResult);
}

//-----------------------------------------------------------------------------
BOOL __fastcall RpcCoreEnumProcessAuthInfo(void* pRpcCoreCtxt, DWORD Pid, RpcCoreEnumProcessAuthInfoCallbackFn_T RpcCoreEnumProcessAuthInfoCallbackFn, void* pCallbackCtxt)
{
    RpcCoreManager_T*   pRpcCoreManager = NULL;
    BOOL                bResult = FALSE;

    pRpcCoreManager = (RpcCoreManager_T*)pRpcCoreCtxt;
    if (pRpcCoreManager == NULL) goto End;

#ifdef _WIN64
    if (IsProcessWow64(Pid))
    {
        bResult = pRpcCoreManager->pWow64Core->RpcCoreEnumProcessAuthInfoFn(pRpcCoreManager->pWow64CoreCtxt, Pid, RpcCoreEnumProcessAuthInfoCallbackFn, pCallbackCtxt);
    }
    else
#endif
    {
        bResult = pRpcCoreManager->pNativeCore->RpcCoreEnumProcessAuthInfoFn(pRpcCoreManager->pNativeCoreCtxt, Pid, RpcCoreEnumProcessAuthInfoCallbackFn, pCallbackCtxt);
    }
End:
    return (bResult);
}
