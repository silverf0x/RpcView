#include "InitViewsVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"
#include "../RpcCommon/Misc.h"


typedef struct _EnumCtxt_T{
	InitViewsVisitor_C*	pInitViewsVisitor;
	EndpointsWidget_C*	pEndpointsWidget;
	InterfacesWidget_C*	pInterfacesWidget;
}EnumCtxt_T;


//------------------------------------------------------------------------------
static BOOL WINAPI EnumProc(DWORD Pid, DWORD Ppid, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
    UNREFERENCED_PARAMETER(pbContinue);
	ProcessEntry_C* pProcessEntry = new ProcessEntry_C(Ppid,Pid);

	pEnumCtxt->pInitViewsVisitor->ProcessVector.push_back(pProcessEntry);
	return (TRUE);
}


//------------------------------------------------------------------------------
InitViewsVisitor_C::InitViewsVisitor_C(RpcCore_T* pRpcCore,void** ppRpcCoreCtxt)
{
	EnumCtxt_T	EnumCtxt;

	this->pRpcCore= pRpcCore;

	this->NbOfInterfaces = 0;
	this->pRpcCoreCtxt = pRpcCore->RpcCoreInitFn(pRpcCore->bForceLoading);
	if (this->pRpcCoreCtxt==NULL) goto End;
	*ppRpcCoreCtxt = this->pRpcCoreCtxt;

    EnumCtxt.pInitViewsVisitor = this;
	EnumProcess( (EnumProcessCallbackFn_T)&EnumProc, &EnumCtxt );
End:
	;
}


//------------------------------------------------------------------------------
InitViewsVisitor_C::~InitViewsVisitor_C()
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

	pEnumCtxt->pEndpointsWidget->AddEndpoint(Pid, pRpcEndpointInfo );
	return (TRUE);
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	EnumCtxt_T								EnumCtxt;
	std::vector<ProcessEntry_C*>::iterator	Iter;

	EnumCtxt.pInitViewsVisitor	= this;
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
	NbOfEndpoints = pEndpointsWidget->GetEndpoints();
}


//------------------------------------------------------------------------------
static BOOL __fastcall EnumInterfaces(RpcInterfaceInfo_T* pRpcInterfaceInfo, EnumCtxt_T* pEnumCtxt, BOOL* pbContinue)
{
	UNREFERENCED_PARAMETER(pbContinue);

	pEnumCtxt->pInterfacesWidget->AddInterfaces(pRpcInterfaceInfo);
	return (TRUE);
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	EnumCtxt_T								EnumCtxt;
	std::vector<ProcessEntry_C*>::iterator	Iter;

	EnumCtxt.pInitViewsVisitor	= this;
	EnumCtxt.pInterfacesWidget	= pInterfacesWidget;
	for (Iter=ProcessVector.begin();Iter!=ProcessVector.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			pRpcCore->RpcCoreEnumProcessInterfacesFn(
								pRpcCoreCtxt,
								(*Iter)->Pid,
								(RpcCoreEnumProcessInterfacesCallbackFn_T) &EnumInterfaces,
								&EnumCtxt,
								RPC_INTERFACE_INFO_ALL);			
		}
	}
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	//nothing to do here
    UNREFERENCED_PARAMETER(pInterfaceInfoWidget);
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	//nothing to do here
    UNREFERENCED_PARAMETER(pProcessInfoWidget);
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	//nothing to do here
    UNREFERENCED_PARAMETER(pProceduresWidget);
}


//------------------------------------------------------------------------------
void InitViewsVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	std::vector<ProcessEntry_C*>::iterator	Iter;
	RpcProcessInfo_T*						pRpcProcessInfo	= NULL;

	for (Iter=ProcessVector.begin();Iter!=ProcessVector.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			pRpcProcessInfo = pRpcCore->RpcCoreGetProcessInfoFn(pRpcCoreCtxt,(*Iter)->Pid,(*Iter)->Ppid,RPC_PROCESS_INFO_ALL);
			if (pRpcProcessInfo!=NULL)
			{
				pProcessWidget->AddProcess(pRpcProcessInfo);
				this->NbOfInterfaces+=pRpcProcessInfo->InterfacesCount;
				pRpcCore->RpcCoreFreeProcessInfoFn(pRpcCoreCtxt,pRpcProcessInfo);
			}
		}
	}
	pProcessWidget->resizeColumnsToContents();
	NbOfProcesses=pProcessWidget->GetProcesses();
}


//------------------------------------------------------------------------------
ULONG InitViewsVisitor_C::GetEndpoints()
{
	return (this->NbOfEndpoints);
}


//------------------------------------------------------------------------------
ULONG InitViewsVisitor_C::GetInterfaces()
{
	return (this->NbOfInterfaces);
}