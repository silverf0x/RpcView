#include "InterfaceSelectedVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"
#include "../RpcCommon/Misc.h"
#include "Pdb.h"
#include <Dbghelp.h>

//------------------------------------------------------------------------------
InterfaceSelectedVisitor_C::InterfaceSelectedVisitor_C(quint32 Pid, RPC_IF_ID* pIf,RpcCore_T* pRpcCore,void* pRpcCoreCtxt)
{
	this->Pid			= Pid;
	this->pIf			= pIf;
	this->pRpcCore      = pRpcCore;
	this->pRpcCoreCtxt	= pRpcCoreCtxt;

	this->pRpcInterfaceInfo = pRpcCore->RpcCoreGetInterfaceInfoFn( pRpcCoreCtxt, Pid, pIf, RPC_INTERFACE_INFO_ALL );
}


//------------------------------------------------------------------------------
InterfaceSelectedVisitor_C::~InterfaceSelectedVisitor_C()
{
	if (pRpcInterfaceInfo != NULL)
	{
		pRpcCore->RpcCoreFreeInterfaceInfoFn(pRpcCoreCtxt, pRpcInterfaceInfo);
	}
}


//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	pEndpointsWidget->ApplyProcessFilter(this->Pid);
	pEndpointsWidget->resizeColumnsToContents();
}


//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	//nothing todo
    UNREFERENCED_PARAMETER(pInterfacesWidget);
}

//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	HANDLE	hProcess = NULL;
	WCHAR	SymbolName[RPC_MAX_LENGTH];
	void*	hPdb;
	
	if (pRpcInterfaceInfo==NULL) goto End;
	//
	// Get the callback name
	//
	SymbolName[0]=0;
	if (pRpcInterfaceInfo->pLocationBase!=NULL) 
	{
		hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,Pid);
		if (hProcess==NULL) goto End;
		
		hPdb = PdbInit(hProcess, pRpcInterfaceInfo->pLocationBase, pRpcInterfaceInfo->LocationSize);
		if (hPdb!=NULL)
		{
			PdbGetSymbolName(hPdb,pRpcInterfaceInfo->IfCallbackFn, SymbolName, sizeof(SymbolName));
			PdbUninit(hPdb);
		}
	}
	//
	// Update the view
	//
	pInterfaceInfoWidget->UpdateInterfaceInfo( pRpcInterfaceInfo, SymbolName );
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	return;
}


typedef struct _EnumCtxt_T{
	ProcessInfoWidget_C* pProcessInfoWidget;
}EnumCtxt_T;


//------------------------------------------------------------------------------
static BOOL __fastcall EnumProcessAuth(DWORD Pid, RpcAuthInfo_T* pRpcAuthInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(Pid);
	UNREFERENCED_PARAMETER(pbContinue);
	
	pEnumCtxt->pProcessInfoWidget->AddAuthInfo(pRpcAuthInfo);
	return (TRUE);
}


//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	RpcProcessInfo_T*	pRpcProcessInfo;
	EnumCtxt_T			EnumCtxt;

	EnumCtxt.pProcessInfoWidget = pProcessInfoWidget;
	pProcessInfoWidget->reset();
	pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn( pRpcCoreCtxt, Pid, 0, RPC_PROCESS_INFO_ALL );
	if (pRpcProcessInfo!=NULL)
	{
		pProcessInfoWidget->UpdateProcessInfo(pRpcProcessInfo);
		pRpcCore->RpcCoreFreeProcessInfoFn( pRpcCoreCtxt, pRpcProcessInfo );
		pRpcCore->RpcCoreEnumProcessAuthInfoFn( pRpcCoreCtxt, Pid, (RpcCoreEnumProcessAuthInfoCallbackFn_T)EnumProcessAuth,&EnumCtxt);
	}
}

//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	HANDLE	hProcess		= NULL;
	WCHAR	SymbolName[RPC_MAX_LENGTH];
	ULONG	ProcIdx;
	VOID*	ProcFormat		= NULL;
	VOID*	hPdb			= NULL;

	if (pRpcInterfaceInfo==NULL) goto End;
	pProceduresWidget->reset(pRpcInterfaceInfo->Pid);
	
	switch(pRpcInterfaceInfo->IfType)
	{
		//
		// Get proc names from pdb files if present
		//
		case IfType_RPC:
			if (pRpcInterfaceInfo->pLocationBase==NULL) goto End;
			
			hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,Pid);
			if (hProcess==NULL) goto End;
			
			hPdb = PdbInit(hProcess, pRpcInterfaceInfo->pLocationBase, pRpcInterfaceInfo->LocationSize);
			for(ProcIdx=0;ProcIdx<pRpcInterfaceInfo->NumberOfProcedures;ProcIdx++)
			{
				if (pRpcInterfaceInfo->ppProcAddressTable!=NULL)
				{
					SymbolName[0]=0;
					if (hPdb!=NULL)
					{
						PdbGetSymbolName(hPdb, (UCHAR*)pRpcInterfaceInfo->pLocationBase + pRpcInterfaceInfo->ppProcAddressTable[ProcIdx], SymbolName, sizeof(SymbolName));
					}
					if ( (pRpcInterfaceInfo->pFormatStringOffsetTable==NULL)||
						 (pRpcInterfaceInfo->pProcFormatString==NULL)) 
					{
						ProcFormat=NULL;
					}
					else
					{
						ProcFormat=pRpcInterfaceInfo->pProcFormatString + pRpcInterfaceInfo->pFormatStringOffsetTable[ProcIdx];
					}
					pProceduresWidget->AddProcedure(
											ProcIdx,
											SymbolName,
											pRpcInterfaceInfo->pLocationBase,
											pRpcInterfaceInfo->ppProcAddressTable[ProcIdx],
											ProcFormat
											);
				}
			}
			PdbUninit(hPdb);
			
		break;
		
		default:
		break;
	}
	pProceduresWidget->resizeColumnsToContents();
End:
	if (hProcess!=NULL) CloseHandle(hProcess);
	return;
}

//------------------------------------------------------------------------------
void InterfaceSelectedVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	pProcessWidget->SelectProcess(Pid);
}