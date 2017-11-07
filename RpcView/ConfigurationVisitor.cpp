#include "ConfigurationVisitor.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "ProceduresWidget.h"

//------------------------------------------------------------------------------
ConfigurationVisitor_C::ConfigurationVisitor_C(ConfigurationType_T ConfigurationType, QSettings* pSettings)
{
	this->pSettings			= pSettings;
	this->ConfigurationType = ConfigurationType;
}


//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(EndpointsWidget_C* pEndpointsWidget)
{
	switch (ConfigurationType)
	{
		case Load:
			pEndpointsWidget->LoadConfiguration(this->pSettings);
		break;
		//--
		case Save:
			pEndpointsWidget->SaveConfiguration(this->pSettings);
		break;
		//--
		case UpdateColumns:
			pEndpointsWidget->UpdateColumnsVisibility();
		break;
		//
		default:
		break;
	}
}


//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(InterfacesWidget_C* pInterfacesWidget)
{
	switch (ConfigurationType)
	{
		case Load:
			pInterfacesWidget->LoadConfiguration(this->pSettings);
		break;
		//--
		case Save:
			pInterfacesWidget->SaveConfiguration(this->pSettings);
		break;
		//--
		case UpdateColumns:
			pInterfacesWidget->UpdateColumnsVisibility();
		break;
		//--
		case AddressRVA:
			pInterfacesWidget->SetAddressRepresentation(AddressRepresentation_RVA);
		break;
		//--
		case AddressAbsolute:
			pInterfacesWidget->SetAddressRepresentation(AddressRepresentation_Absolute);
		break;
		//--
		default:
		break;
	}
}

//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(InterfaceInfoWidget_C* pInterfaceInfoWidget)
{
	switch (ConfigurationType)
	{
		case AddressRVA:
			pInterfaceInfoWidget->SetAddressRepresentation(AddressRepresentation_RVA);
		break;
		//--
		case AddressAbsolute:
			pInterfaceInfoWidget->SetAddressRepresentation(AddressRepresentation_Absolute);
		break;
		//--
	default:
		break;
	}
}

//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(ProcessInfoWidget_C* pProcessInfoWidget)
{
	//nothing to do here
    UNREFERENCED_PARAMETER(pProcessInfoWidget);
}

//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(ProceduresWidget_C* pProceduresWidget)
{
	switch (ConfigurationType)
	{
		case Load:
			pProceduresWidget->LoadConfiguration(this->pSettings);
		break;
		//--
		case Save:
			pProceduresWidget->SaveConfiguration(this->pSettings);
		break;
		//--
		case UpdateColumns:
			pProceduresWidget->UpdateColumnsVisibility();
		break;
		//--
		case AddressRVA:
			pProceduresWidget->SetAddressRepresentation(AddressRepresentation_RVA);
		break;
		//--
		case AddressAbsolute:
			pProceduresWidget->SetAddressRepresentation(AddressRepresentation_Absolute);
		break;
		//--
		default:
		break;
	}
}

//------------------------------------------------------------------------------
void ConfigurationVisitor_C::Visit(ProcessWidget_C* pProcessWidget)
{
	switch (ConfigurationType)
	{
		case Load:
			pProcessWidget->LoadConfiguration(this->pSettings);
		break;
		//--
		case Save:
			pProcessWidget->SaveConfiguration(this->pSettings);
		break;
		//--
		case UpdateColumns:
			pProcessWidget->UpdateColumnsVisibility();
		break;
		//
		default:
		break;
	}
}