#ifndef _CONFIGURATION_VISITOR_H_
#define _CONFIGURATION_VISITOR_H_

#include "../Qt/Qt.h"
#include "ViewVisitor.h"
#include "ProcessEntry.h"
#include "..\RpcCore\RpcCore.h"


//------------------------------------------------------------------------------
class ConfigurationVisitor_C : public ViewVisitor_C
{
public:
	typedef enum {
		Load,
		Save,
		UpdateColumns,
		AddressAbsolute,
		AddressRVA
	}ConfigurationType_T;

	ConfigurationVisitor_C(ConfigurationType_T, QSettings*);

	virtual void Visit(EndpointsWidget_C* pEndpointsWidget);
	virtual void Visit(InterfacesWidget_C* pInterfacesWidget);
	virtual void Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget);
	virtual void Visit(ProcessInfoWidget_C* pProcessInfoWidget);
	virtual void Visit(ProceduresWidget_C* pProceduresWidget);
	virtual void Visit(ProcessWidget_C* pProcessWidget);

private:
	QSettings*			pSettings;
	ConfigurationType_T	ConfigurationType;
};

#endif //_CONFIGURATION_VISITOR_H_