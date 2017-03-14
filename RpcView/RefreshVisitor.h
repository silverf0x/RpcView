#ifndef _REFRESH_VISITOR_H_
#define _REFRESH_VISITOR_H_

#include "../Qt/Qt.h"
#include "ViewVisitor.h"
#include "ProcessEntry.h"
#include "..\RpcCore\RpcCore.h"

//------------------------------------------------------------------------------
class RefreshVisitor_C : public ViewVisitor_C
{
public:
	std::vector <ProcessEntry_C*>	ProcessVector;

	RefreshVisitor_C(RpcCore_T* pRpcCore,void* pRpcCoreCtxt);
	~RefreshVisitor_C();

	// FIXE ME
	// NumberOfEndpoints as well as NumberOfInterfaces SHOULD be private members
	// but we have to change the RpcCore API to avoid enum in order to achieve this goal!
	//
	ULONG	NumberOfEndpoints;
	ULONG	NumberOfInterfaces;
	ULONG	NumberOfProcesses;

	ULONG	TotalEndpoints;
	ULONG	TotalInterfaces;
	ULONG	TotalProcesses;

	ULONG GetEndpoints();
	ULONG GetInterfaces();
	ULONG GetProcesses();
	ULONG GetTotalEndpoints();
	ULONG GetTotalInterfaces();
	ULONG GetTotalProcesses();

	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);

private:
	RpcCore_T*	pRpcCore;
	void*		pRpcCoreCtxt;
};

#endif //_REFRESH_VISITOR_H_