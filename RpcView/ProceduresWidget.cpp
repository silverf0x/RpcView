#include "ProceduresWidget.h"
#include "..\..\RpcCommon\RpcCommon.h"

static const char WidgetName[] = "Procedures";
static const char ConfigPath[] = "Procedures/geometry";


//------------------------------------------------------------------------------
QString GetProceduresWidgetColumName(ProceduresWigetColumn_T ProceduresWigetColumn)
{
	switch(ProceduresWigetColumn)
	{
		case ProceduresWigetColumn_Index		: return (QString("Index"));
		case ProceduresWigetColumn_Name			: return (QString("Name")); 
		case ProceduresWigetColumn_Address		: return (QString("Address"));
		case ProceduresWigetColumn_FormatString	: return (QString("Format"));
		default: return (QString("Unknown"));
	}
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::reset(ULONG pid)
{
	this->Pid = pid;
	pProcedures->clear();
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::resizeColumnsToContents()
{
	int	Index;

	for( Index=0; Index < ProceduresWigetColumn_Last; Index++)
	{
		pProcedures->resizeColumnToContents(Index);
	}
}


//------------------------------------------------------------------------------
bool ProceduresWidget_C::AddProcedure(quint32 ProcIdx, WCHAR* pSymbolName, VOID* pBase, ULONG Rva, VOID* pProcFormatString)
{
	QTreeWidgetItem*	pProcedure;

	pCurrentIntefaceBase	= pBase;
	pProcedure				= new QTreeWidgetItem;
	
	pProcedure->setData(ProceduresWigetColumn_Index, Qt::DisplayRole, ProcIdx );
	pProcedure->setData(ProceduresWigetColumn_Name, Qt::DisplayRole, QString::fromUtf16((const ushort*)pSymbolName));
	pProcedure->setData(ProceduresWigetColumn_Address, Qt::UserRole, (quintptr)pBase + (quint32)Rva);
	
	if (pProcFormatString == NULL)
	{
		pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::DisplayRole, "");
	}
	else
	{
		pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::UserRole, (quintptr)pProcFormatString);
	}
	
	switch (AddressRepresentation)
	{
		case AddressRepresentation_Absolute:
			pProcedure->setData(ProceduresWigetColumn_Address, Qt::DisplayRole, QString("0x%1").arg((quintptr)pBase + Rva, 16, 16, QLatin1Char('0')));
			pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::DisplayRole, QString("0x%1").arg((quintptr)pProcFormatString, 16, 16, QLatin1Char('0')));
		break;
		//--
		case AddressRepresentation_RVA:
			pProcedure->setData(ProceduresWigetColumn_Address, Qt::DisplayRole, QString("+0x%1").arg((quintptr)Rva, 8, 16, QLatin1Char('0')));
			pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::DisplayRole, QString("+0x%1").arg((quintptr)pProcFormatString - (quintptr)pBase, 8, 16, QLatin1Char('0')));
		break;
		//--
		default:
		break;
	}

	pProcedures->addTopLevelItem(pProcedure);
	return (true);
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::SetAddressRepresentation(AddressRepresentation_T AddrRepresentation)
{
	QTreeWidgetItem*	pProcedure;
	quintptr			AbsoluteAddr;

	this->AddressRepresentation = AddrRepresentation;

	switch (AddrRepresentation)
	{
		case AddressRepresentation_RVA:
			for (int i = 0; i < pProcedures->topLevelItemCount(); i++)
			{
				pProcedure = pProcedures->topLevelItem(i);
				AbsoluteAddr = pProcedure->data(ProceduresWigetColumn_Address, Qt::UserRole).toULongLong();
				pProcedure->setData(ProceduresWigetColumn_Address, Qt::DisplayRole, QString("+0x%1").arg(AbsoluteAddr - (quintptr)pCurrentIntefaceBase, 8, 16, QLatin1Char('0')));
				AbsoluteAddr = pProcedure->data(ProceduresWigetColumn_FormatString, Qt::UserRole).toULongLong();
				pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::DisplayRole, QString("+0x%1").arg(AbsoluteAddr - (quintptr)pCurrentIntefaceBase, 8, 16, QLatin1Char('0')));
			}
		break;
		//--
		case AddressRepresentation_Absolute:
			for (int i = 0; i < pProcedures->topLevelItemCount(); i++)
			{
				pProcedure = pProcedures->topLevelItem(i);
				AbsoluteAddr = pProcedure->data(ProceduresWigetColumn_Address, Qt::UserRole).toULongLong();
				pProcedure->setData(ProceduresWigetColumn_Address, Qt::DisplayRole, QString("0x%1").arg(AbsoluteAddr, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
				AbsoluteAddr = pProcedure->data(ProceduresWigetColumn_FormatString, Qt::UserRole).toULongLong();
				pProcedure->setData(ProceduresWigetColumn_FormatString, Qt::DisplayRole, QString("0x%1").arg(AbsoluteAddr, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
			}
		break;
		//--
		default:
		break;
	}
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}


//------------------------------------------------------------------------------
UINT ProceduresWidget_C::GetPid()
{
	return (this->Pid);
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::SaveConfiguration(QSettings* pSettings)
{
	pSettings->setValue(ConfigPath, pProcedures->header()->saveState());
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::LoadConfiguration(QSettings* pSettings)
{
	pProcedures->header()->restoreState(pSettings->value(ConfigPath).toByteArray());
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::CreateColumnsSelectionWidget()
{
	pColumnsSelectionWidget = new QGroupBox(WidgetName,this);
	QVBoxLayout*	pLayout = new QVBoxLayout(pColumnsSelectionWidget);

	for (int i = 0; i < ProceduresWigetColumn_Last; i++)
	{
		CheckBoxArray[i] = new QCheckBox(GetProceduresWidgetColumName((ProceduresWigetColumn_T)i), pColumnsSelectionWidget);
		pLayout->addWidget(CheckBoxArray[i]);
	}
	pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	pColumnsSelectionWidget->setLayout(pLayout);
}


//------------------------------------------------------------------------------
QWidget* ProceduresWidget_C::GetColumnsSelectionWidget()
{
	for (int i = 0; i < ProceduresWigetColumn_Last; i++)
	{
		CheckBoxArray[i]->setChecked(!pProcedures->header()->isSectionHidden(i));
	}
	return pColumnsSelectionWidget;
}


//------------------------------------------------------------------------------
void ProceduresWidget_C::UpdateColumnsVisibility()
{
	for (int i = 0; i < ProceduresWigetColumn_Last; i++)
	{
		pProcedures->header()->setSectionHidden(i, !CheckBoxArray[i]->isChecked());
	}
}


//------------------------------------------------------------------------------
ProceduresWidget_C::ProceduresWidget_C(QWidget* pParent):QDockWidget(WidgetName)
{
	UINT			Idx;
	QGridLayout*	pGridLayout;
	QGroupBox*		pGroupBox;

	this->Pid = 0;

	setObjectName(WidgetName);

	pProcedures	= new QTreeWidget(this);
	pGridLayout = new QGridLayout();
	pGroupBox = new QGroupBox();

	QTreeWidgetItem* pHeaderItem = pProcedures->headerItem();
	for (Idx = 0; Idx < ProceduresWigetColumn_Last; Idx++)
	{ 
		pHeaderItem->setText( Idx, GetProceduresWidgetColumName((ProceduresWigetColumn_T)Idx) );
	} 
	
	pProcedures->setColumnCount(ProceduresWigetColumn_Last);
	pProcedures->setSortingEnabled(true);
	pProcedures->sortByColumn(-1);
	pProcedures->setAnimated(true);
	pProcedures->expandAll();
	pProcedures->setAlternatingRowColors(true);
	pProcedures->setRootIsDecorated(false);
	pProcedures->setMouseTracking(true);
	pProcedures->header()->installEventFilter(pParent);

	pGridLayout->addWidget(pProcedures,0,0);
	pGroupBox->setLayout(pGridLayout);
	setWidget(pGroupBox);
	AddressRepresentation = AddressRepresentation_Absolute;
	//
	// Create the Widget for column selection
	//
	CreateColumnsSelectionWidget();
}