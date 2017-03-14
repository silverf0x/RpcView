#include "ProcessSelectedVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"
#include "../RpcCommon/Misc.h"

typedef struct _EnumCtxt_T{
	ProcessInfoWidget_C*	pProcessInfoWidget;	
}EnumCtxt_T;

//------------------------------------------------------------------------------
ProcessSelectedVisitor_C::ProcessSelectedVisitor_C(quint32 Pid,RpcCore_T* pRpcCore,void* pRpcCoreCtxt)
{
	this->Pid		    = Pid;
	this->pRpcCore      = pRpcCore;
	this->pRpcCoreCtxt	= pRpcCoreCtxt; 

    pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn(pRpcCoreCtxt, Pid, INVALID_PID_VALUE, RPC_PROCESS_INFO_ALL);
}


//------------------------------------------------------------------------------
ProcessSelectedVisitor_C::~ProcessSelectedVisitor_C()
{
	pRpcCore->RpcCoreFreeProcessInfoFn(pRpcCoreCtxt, pRpcProcessInfo);
}


//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	//Apply process filter
	pEndpointsWidget->ApplyProcessFilter(Pid);
	this->NbOfEndpoints = pEndpointsWidget->GetEndpoints();
}


//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	//Apply process filter
	pInterfacesWidget->ApplyProcessFilter(Pid);
	this->NbOfInterfaces = pInterfacesWidget->GetInterfaces();
}


//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	if (this->Pid!=pInterfaceInfoWidget->GetPid())
	{
		pInterfaceInfoWidget->Reset();
	}
}


//------------------------------------------------------------------------------
BOOL __fastcall EnumProcessAuth(DWORD Pid, RpcAuthInfo_T* pRpcAuthInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(Pid);
	UNREFERENCED_PARAMETER(pbContinue);

	pEnumCtxt->pProcessInfoWidget->AddAuthInfo(pRpcAuthInfo);
	return (TRUE);
}


//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	EnumCtxt_T	EnumCtxt;

	EnumCtxt.pProcessInfoWidget = pProcessInfoWidget;
	
	pProcessInfoWidget->UpdateProcessInfo( pRpcProcessInfo );
	pProcessInfoWidget->reset();	//for authinfo
	pRpcCore->RpcCoreEnumProcessAuthInfoFn(pRpcCoreCtxt, pRpcProcessInfo->Pid,(RpcCoreEnumProcessAuthInfoCallbackFn_T)&EnumProcessAuth,&EnumCtxt);
}

//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	if (pProceduresWidget->GetPid() != this->Pid )
	{
		pProceduresWidget->reset(this->Pid);
	}
}

//------------------------------------------------------------------------------
void ProcessSelectedVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	this->NbOfProcesses = pProcessWidget->GetProcesses();
}


//------------------------------------------------------------------------------
ULONG ProcessSelectedVisitor_C::GetEndpoints()
{
	return (this->NbOfEndpoints);
}


//------------------------------------------------------------------------------
ULONG ProcessSelectedVisitor_C::GetInterfaces()
{
	return (this->NbOfInterfaces);
}


//------------------------------------------------------------------------------
ULONG ProcessSelectedVisitor_C::GetProcesses()
{
	return (this->NbOfProcesses);
}
