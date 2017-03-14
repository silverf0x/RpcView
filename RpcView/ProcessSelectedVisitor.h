#ifndef _PROCESS_SELECTED_VISITOR_H_
#define _PROCESS_SELECTED_VISITOR_H_

#include "../Qt/Qt.h"
#include "ViewVisitor.h"
#include "..\RpcCore\RpcCore.h"

//------------------------------------------------------------------------------
class ProcessSelectedVisitor_C : public ViewVisitor_C
{
private:
	RpcProcessInfo_T*	pRpcProcessInfo;
	RpcCore_T*      	pRpcCore;
	void*				pRpcCoreCtxt;
	ULONG				NbOfEndpoints;
	ULONG				NbOfProcesses;
	ULONG				NbOfInterfaces;
	quint32				Pid;
public:
	ProcessSelectedVisitor_C(quint32 Pid,RpcCore_T* pRpcCore,void* pRpcCoreCtxt);
	ProcessSelectedVisitor_C::~ProcessSelectedVisitor_C();
	ULONG	GetEndpoints();
	ULONG	GetInterfaces();
	ULONG	GetProcesses();
	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);
};

#endif