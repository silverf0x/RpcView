#ifndef _VIEW_VISITOR_H_
#define _VIEW_VISITOR_H_

class EndpointsWidget_C;
class InterfacesWidget_C;
class InterfaceInfoWidget_C;
class ProcessInfoWidget_C;
class ProceduresWidget_C;
class ProcessWidget_C;

class ViewVisitor_C
{
public:
    virtual void Visit(EndpointsWidget_C* pEndpointsWidget) = 0;
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget) = 0;
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget) = 0;
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget) = 0;
	virtual void Visit(ProceduresWidget_C* pProceduresWidget) = 0;
	virtual void Visit(ProcessWidget_C* pProcessWidget) = 0;
};

#endif