#ifndef _INTERFACE_SELECTED_VISITOR_H_
#define _INTERFACE_SELECTED_VISITOR_H_

#include "../Qt/Qt.h"
#include "ViewVisitor.h"
#include "..\RpcCore\RpcCore.h"

//------------------------------------------------------------------------------
class InterfaceSelectedVisitor_C : public ViewVisitor_C
{
private:
	quint32		        Pid;
	RPC_IF_ID*	        pIf;
	RpcCore_T*	        pRpcCore;
	VOID*		        pRpcCoreCtxt;
	RpcInterfaceInfo_T*	pRpcInterfaceInfo;
public:
	InterfaceSelectedVisitor_C(quint32 Pid, RPC_IF_ID* pIf,RpcCore_T* pRpcCore,void* pRpcCoreCtxt);
	~InterfaceSelectedVisitor_C();
	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);
};

#endif