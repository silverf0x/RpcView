#include "InterfaceSelectedVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"
#include "RefreshVisitor.h"
#include "../RpcCommon/Misc.h"
#include "Pdb.h"
#include <Dbghelp.h>


typedef struct _EnumCtxt_T{
	RefreshVisitor_C*	pRefreshVisitor;
	EndpointsWidget_C*	pEndpointsWidget;
	InterfacesWidget_C*	pInterfacesWidget;
	RpcCore_T*	        pRpcCore;
	VOID*				pRpcCoreCtxt;
}EnumCtxt_T;


//------------------------------------------------------------------------------
static BOOL WINAPI EnumProc(DWORD Pid, DWORD Ppid, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
    UNREFERENCED_PARAMETER(pbContinue);
	ProcessEntry_C* pProcessEntry = new ProcessEntry_C(Ppid,Pid);

	pEnumCtxt->pRefreshVisitor->ProcessVector.push_back(pProcessEntry);
	return (TRUE);
}

//------------------------------------------------------------------------------
RefreshVisitor_C::RefreshVisitor_C(RpcCore_T* pRpcCore,void* pRpcCoreCtxt)
{
	EnumCtxt_T	EnumCtxt;

    this->pRpcCore      = pRpcCore;
	this->pRpcCoreCtxt	= pRpcCoreCtxt;

	this->NumberOfEndpoints = 0;
	this->NumberOfInterfaces = 0;

	EnumCtxt.pRefreshVisitor = this;
	EnumProcess( (EnumProcessCallbackFn_T)&EnumProc, &EnumCtxt );
}


//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetEndpoints()
{
	return (this->NumberOfEndpoints);
}


//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetInterfaces()
{
	return (this->NumberOfInterfaces);
}


//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetProcesses()
{
	return (this->NumberOfProcesses);
}

//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetTotalEndpoints()
{
	return (this->TotalEndpoints);
}

//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetTotalInterfaces()
{
	return (this->TotalInterfaces);
}

//------------------------------------------------------------------------------
ULONG RefreshVisitor_C::GetTotalProcesses()
{
	return (this->TotalProcesses);
}

//------------------------------------------------------------------------------
RefreshVisitor_C::~RefreshVisitor_C()
{
	for (UINT32 i = 0; i < ProcessVector.size(); i++)
	{
		delete ProcessVector[i];
	}
	ProcessVector.clear();
}


//------------------------------------------------------------------------------
static BOOL __fastcall EnumEndpoints(DWORD Pid, RpcEndpointInfo_T* pRpcEndpointInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(pbContinue);

	if (!pEnumCtxt->pEndpointsWidget->IsEndpointsPresent(Pid,pRpcEndpointInfo->pName,pRpcEndpointInfo->pProtocole))
	{
		pEnumCtxt->pEndpointsWidget->AddEndpoint(Pid, pRpcEndpointInfo );
	}
	return (TRUE);
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	EnumCtxt_T								EnumCtxt;
	std::vector<ProcessEntry_C*>::iterator	Iter;

	pEndpointsWidget->SnapEndpoint();

	EnumCtxt.pRefreshVisitor	= this;
	EnumCtxt.pEndpointsWidget	= pEndpointsWidget;
	for (Iter=ProcessVector.begin();Iter!=ProcessVector.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			pRpcCore->RpcCoreEnumProcessEndpointsFn(
								pRpcCoreCtxt,
								(*Iter)->Pid,
								(RpcCoreEnumProcessEndpointsCallbackFn_T) &EnumEndpoints,
								&EnumCtxt);
		}
	}
	pEndpointsWidget->RemoveUnregisteredEndpoints();
	pEndpointsWidget->UpdateUserFilter();
	this->NumberOfEndpoints = pEndpointsWidget->GetEndpoints();
	this->TotalEndpoints = pEndpointsWidget->GetTotalEndpoints();
}


//------------------------------------------------------------------------------
static BOOL __fastcall EnumInterfaces(RpcInterfaceInfo_T* pRpcInterfaceInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(pbContinue);
	RpcInterfaceInfo_T* pRpcInterfaceInfoFull = NULL;

	if (!pEnumCtxt->pInterfacesWidget->IsInterfacePresent(pRpcInterfaceInfo->Pid,&pRpcInterfaceInfo->If))
	{
		pRpcInterfaceInfoFull = pEnumCtxt->pRpcCore->RpcCoreGetInterfaceInfoFn(pEnumCtxt->pRpcCoreCtxt,pRpcInterfaceInfo->Pid,&pRpcInterfaceInfo->If,RPC_INTERFACE_INFO_ALL);
		if (pRpcInterfaceInfoFull!=NULL)
		{
			pEnumCtxt->pInterfacesWidget->AddInterfaces(pRpcInterfaceInfoFull);
			pEnumCtxt->pRpcCore->RpcCoreFreeInterfaceInfoFn(pEnumCtxt->pRpcCoreCtxt,pRpcInterfaceInfoFull);
		}
	}
	return (TRUE);
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	EnumCtxt_T								EnumCtxt;
	std::vector<ProcessEntry_C*>::iterator	Iter;

	pInterfacesWidget->SnapInterfaces();

	EnumCtxt.pRefreshVisitor	= this;
	EnumCtxt.pInterfacesWidget	= pInterfacesWidget;
	EnumCtxt.pRpcCoreCtxt		= this->pRpcCoreCtxt;
	EnumCtxt.pRpcCore           = this->pRpcCore;
	for (Iter=ProcessVector.begin();Iter!=ProcessVector.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			pRpcCore->RpcCoreEnumProcessInterfacesFn(
								pRpcCoreCtxt,
								(*Iter)->Pid,
								(RpcCoreEnumProcessInterfacesCallbackFn_T) &EnumInterfaces,
								&EnumCtxt,
								RPC_INTERFACE_INFO_DEFAULT);
		}
	}
	pInterfacesWidget->RemoveUnregisteredInterfaces();
	pInterfacesWidget->UpdateUserFilter();
	this->NumberOfInterfaces = pInterfacesWidget->GetInterfaces();
	this->TotalInterfaces = pInterfacesWidget->GetTotalInterfaces();
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	//nothing to do here
    UNREFERENCED_PARAMETER(pInterfaceInfoWidget);
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	RpcProcessInfo_T*	pRpcProcessInfo	= NULL;

	pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn(pRpcCoreCtxt,pProcessInfoWidget->GetPid(),0,RPC_PROCESS_INFO_ALL);
	if (pRpcProcessInfo!=NULL)
	{
		pProcessInfoWidget->UpdateProcessInfo(pRpcProcessInfo);
		pRpcCore->RpcCoreFreeProcessInfoFn(pRpcCoreCtxt,pRpcProcessInfo);
	}
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	//nothing to do
    UNREFERENCED_PARAMETER(pProceduresWidget);
}


//------------------------------------------------------------------------------
void RefreshVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	std::vector<ProcessEntry_C*>::iterator	Iter;
	RpcProcessInfo_T*						pRpcProcessInfo	= NULL;
	BOOL									bWow64;
	bool									bIsProcessPresent;
	ULONG									ProcessInfoMask;

	pProcessWidget->SnapProcesses();

	for (Iter=ProcessVector.begin();Iter!=ProcessVector.end();Iter++)
	{
		if ( (*Iter!=NULL) )
		{
			bWow64 = IsProcessWow64((*Iter)->Pid);
			bIsProcessPresent = pProcessWidget->IsProcessPresent((*Iter)->Pid);
			if (bIsProcessPresent) ProcessInfoMask = 0;
			else ProcessInfoMask = RPC_PROCESS_INFO_ALL;

            pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn(pRpcCoreCtxt,(*Iter)->Pid,(*Iter)->Ppid,ProcessInfoMask);

			if (pRpcProcessInfo!=NULL) 
			{
				if (bIsProcessPresent) pProcessWidget->UpdateProcess(pRpcProcessInfo);
				else pProcessWidget->AddProcess(pRpcProcessInfo);

				pRpcCore->RpcCoreFreeProcessInfoFn(pRpcCoreCtxt, pRpcProcessInfo);
			}
		}
	}
	pProcessWidget->RemoveTerminatedProcesses();
	this->NumberOfProcesses = pProcessWidget->GetProcesses();
	this->TotalProcesses	= pProcessWidget->GetTotalProcesses();
}