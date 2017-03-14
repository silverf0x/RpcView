#ifndef _INIT_VIEWS_VISITOR_H_
#define _INIT_VIEWS_VISITOR_H_

#include "ViewVisitor.h"
#include "..\RpcCore\RpcCore.h"
#include "ProcessEntry.h"
#include <vector>


//------------------------------------------------------------------------------
class InitViewsVisitor_C : public ViewVisitor_C
{
private:
	RpcCore_T*	pRpcCore;
	VOID*		pRpcCoreCtxt;
	ULONG		NbOfEndpoints;
	ULONG		NbOfProcesses;
	ULONG		NbOfInterfaces;

public:
	ULONG	GetEndpoints();
	ULONG	GetInterfaces();
public:
	std::vector <ProcessEntry_C*>	ProcessVector;

	InitViewsVisitor_C(RpcCore_T* pRpcCore,void** ppRpcCoreCtxt);
	~InitViewsVisitor_C();
	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);
};

#endif