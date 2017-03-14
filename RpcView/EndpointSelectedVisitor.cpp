#include "EndpointSelectedVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"
#include "../RpcCommon/Misc.h"


typedef struct _EnumCtxt_T{
	ProcessInfoWidget_C* pProcessInfoWidget;
}EnumCtxt_T;


//------------------------------------------------------------------------------
EndpointSelectedVisitor_C::EndpointSelectedVisitor_C(quint32 Pid,RpcCore_T* pRpcCore,void* pRpcCoreCtxt)
{
	this->Pid			= Pid;
	this->pRpcCore      = pRpcCore;
	this->pRpcCoreCtxt	= pRpcCoreCtxt;
}


//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	//nothing to do here
}


//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	//
	// Apply process filter
	//
	pInterfacesWidget->ApplyProcessFilter(Pid);
	pInterfacesWidget->resizeColumnsToContents();
}

//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	//
	// if the Pid of the selected endpoint is different from the current 
	// interface one we ave to reset the InterfaceInfoWidget
	//
	if (this->Pid != pInterfaceInfoWidget->GetPid())
	{
		pInterfaceInfoWidget->Reset();
	}
}


//------------------------------------------------------------------------------
static BOOL __fastcall EnumProcessAuth(DWORD Pid, RpcAuthInfo_T* pRpcAuthInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(Pid);
	UNREFERENCED_PARAMETER(pbContinue);

	pEnumCtxt->pProcessInfoWidget->AddAuthInfo(pRpcAuthInfo);
	return (TRUE);
}


//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	RpcProcessInfo_T*	pRpcProcessInfo;
	EnumCtxt_T			EnumCtxt;

	EnumCtxt.pProcessInfoWidget = pProcessInfoWidget;
	pProcessInfoWidget->reset();

	pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn( pRpcCoreCtxt, Pid, 0, RPC_PROCESS_INFO_ALL );
	if (pRpcProcessInfo != NULL)
	{
		pProcessInfoWidget->UpdateProcessInfo(pRpcProcessInfo);
		pRpcCore->RpcCoreFreeProcessInfoFn( pRpcCoreCtxt, pRpcProcessInfo );
		pRpcCore->RpcCoreEnumProcessAuthInfoFn( pRpcCoreCtxt, Pid, (RpcCoreEnumProcessAuthInfoCallbackFn_T)EnumProcessAuth,&EnumCtxt);
	}
}

//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	//
	// if the Pid of the selected endpoint is different from the current 
	// procedure one we ave to reset the ProcedureWidget
	//
	if (this->Pid != pProceduresWidget->GetPid())
	{
		pProceduresWidget->reset(this->Pid);
	}
}

//------------------------------------------------------------------------------
void EndpointSelectedVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	pProcessWidget->SelectProcess(Pid);
}