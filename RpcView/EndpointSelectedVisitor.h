#ifndef _ENDPOINT_SELECTED_VISITOR_H_
#define _ENDPOINT_SELECTED_VISITOR_H_

#include "..\Qt\Qt.h"
#include "ViewVisitor.h"
#include "..\RpcCore\RpcCore.h"

//------------------------------------------------------------------------------
class EndpointSelectedVisitor_C : public ViewVisitor_C
{
private:
	RpcCore_T*	        pRpcCore;
	void*				pRpcCoreCtxt;
    quint32             Pid;
public:
	EndpointSelectedVisitor_C(quint32 Pid,RpcCore_T* pRpcCore,void* pRpcCoreCtxt);
	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);
};

#endif