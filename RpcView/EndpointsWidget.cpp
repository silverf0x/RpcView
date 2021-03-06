#include "EndpointsWidget.h"
#include "..\RpcCommon\RpcCommon.h"


static const char WidgetName[] = "Endpoints";


//------------------------------------------------------------------------------
QString EndpointsWidget_C::GetEndpointsWidgetColumnName(Column_T Column)
{
	switch(Column)
	{
		case Column_Pid		: return (QString("Pid"));
		case Column_Protocol: return (QString("Protocol"));
		case Column_Name	: return (QString("Name"));
		default				: return (QString("Unknown"));
	}
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::EndpointSelected(const QModelIndex& Index)
{
	quint32 Pid;

	Pid = pProxyModel->data(pProxyModel->index(Index.row(), Column_Pid)).toUInt();
	//
	// Emit the EndpointSelected signal so that the MainWindow sends 
	// the appropriate visitor
	//
	emit EndpointSelected(Pid);
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::ApplyProcessFilter(quint32 Pid)
{
	if (Pid == INVALID_PID_VALUE)
	{
		pProxyModel->setFilterRegExp( QRegExp(".*") );
	}
	else
	{
		pProxyModel->setFilterRegExp( QString("^%1$").arg(Pid) );
		if (pProxyModel->rowCount() == 0) pProxyModel->setFilterRegExp( QRegExp(".*") );
	}
	pFilterWidget->Reset();
}


//------------------------------------------------------------------------------
ULONG EndpointsWidget_C::GetEndpoints()
{
	return (ULONG)pProxyModel->rowCount();
}


//------------------------------------------------------------------------------
ULONG EndpointsWidget_C::GetTotalEndpoints()
{
	return (ULONG)pModel->rowCount();
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::SnapEndpoint()
{
	PrivateItemList = pModel->findItems(".*", Qt::MatchRegularExpression, Column_Name);
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::RemoveUnregisteredEndpoints()
{
	for (auto Iter=PrivateItemList.begin();Iter!=PrivateItemList.end();Iter++)
	{
		if (*Iter!=NULL) pModel->removeRow( (*Iter)->row());
	}
}


//------------------------------------------------------------------------------
bool EndpointsWidget_C::IsEndpointsPresent(quint32 Pid, WCHAR* pName,WCHAR* pProtocole)
{
	QList<QStandardItem*>	ItemList;
	bool					bResult = false;

    UNREFERENCED_PARAMETER(Pid);

	ItemList = pModel->findItems(QString::fromUtf16((const ushort*)pName), Qt::MatchFixedString, Column_Name);
	if (ItemList.isEmpty()) goto End;
	
	for (auto Iter=ItemList.begin();Iter!=ItemList.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			if (pModel->data(pModel->index((*Iter)->row(), Column_Protocol)).toString() == QString::fromUtf16((const ushort*)pProtocole))
			{
				PrivateItemList.removeOne(*Iter);

				bResult = true;
				goto End;
			}
		}
	}
End:
	return (bResult);
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::SaveConfiguration(QSettings* pSettings)
{
	pSettings->setValue("Endpoints/geometry",pEndpoints->header()->saveState());
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::LoadConfiguration(QSettings* pSettings)
{
	pEndpoints->header()->restoreState( pSettings->value("Endpoints/geometry").toByteArray() );
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::resizeColumnsToContents()
{
	int	Index;

	for (Index = 0; Index < Column_Last; Index++)
	{
		pEndpoints->resizeColumnToContents(Index);
	}
}


//------------------------------------------------------------------------------
bool EndpointsWidget_C::AddEndpoint(quint32 Pid, RpcEndpointInfo_T* pRpcEndpointInfo)
{
	int Index;

	Index = pModel->rowCount();
	pModel->setRowCount(Index+1);
	pModel->setData(pModel->index(Index, EndpointsWidget_C::Column_Pid), Pid);
	pModel->setData(pModel->index(Index, EndpointsWidget_C::Column_Protocol), QString::fromUtf16((const ushort*)pRpcEndpointInfo->pProtocole));
	pModel->setData(pModel->index(Index, EndpointsWidget_C::Column_Name), QString::fromUtf16((const ushort*)pRpcEndpointInfo->pName));
	return (true);
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::UpdateUserFilter()
{
	if (pFilterWidget->GetText()!="") ApplyUserFilter( pFilterWidget->GetText() );
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::ApplyUserFilter(const QString & FilterText)
{
	QRegExp	FilterRegExp;

	FilterRegExp.setPattern( FilterText );
	FilterRegExp.setCaseSensitivity( Qt::CaseInsensitive );

	pProxyModel->setFilterKeyColumn(-1);		//filter all columns: UGLY Qt
	pProxyModel->setFilterRegExp( FilterRegExp );
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::CreateColumnsSelectionWidget()
{
	pColumnsSelectionWidget = new QGroupBox(WidgetName,this);
	QVBoxLayout*	pLayout = new QVBoxLayout(pColumnsSelectionWidget);

	for (int i = 0; i < Column_Last; i++)
	{
		CheckBoxArray[i] = new QCheckBox(GetEndpointsWidgetColumnName((Column_T)i),this);
		pLayout->addWidget(CheckBoxArray[i]);
	}
	pColumnsSelectionWidget->setLayout(pLayout);
}


//------------------------------------------------------------------------------
QWidget* EndpointsWidget_C::GetColumnsSelectionWidget()
{
	for (int i = 0; i < Column_Last; i++)
	{
		CheckBoxArray[i]->setChecked(!pEndpoints->header()->isSectionHidden(i));
	}
	return pColumnsSelectionWidget;
}


//------------------------------------------------------------------------------
void EndpointsWidget_C::UpdateColumnsVisibility()
{
	for (int i = 0; i < Column_Last; i++)
	{
		pEndpoints->header()->setSectionHidden(i, !CheckBoxArray[i]->isChecked());
	}
}


//------------------------------------------------------------------------------
EndpointsWidget_C::EndpointsWidget_C(QWidget* pParent):QDockWidget(WidgetName)
{
	QGridLayout*	pGridLayout;
	QGroupBox*		pGroupBox;

	setObjectName(WidgetName);

	pGroupBox		= new QGroupBox(this);
	pGridLayout		= new QGridLayout;

	pProxyModel = new QSortFilterProxyModel(this);
	pProxyModel->setDynamicSortFilter(true);
	pProxyModel->setFilterKeyColumn(Column_Pid);
	pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	
	pModel = new QStandardItemModel(0, Column_Last, this);
	
	for (uint i = 0; i<Column_Last; i++)
	{
		pModel->setHeaderData(i, Qt::Horizontal, GetEndpointsWidgetColumnName( (EndpointsWidget_C::Column_T)i ) );
	}

	pEndpoints = new QTreeView;
	pEndpoints->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pEndpoints->setSelectionBehavior(QAbstractItemView::SelectRows);
	pEndpoints->setSelectionMode(QAbstractItemView::SingleSelection);
	pEndpoints->setRootIsDecorated(false);
    pEndpoints->setSortingEnabled(true);
	pEndpoints->setAlternatingRowColors(true);
	pEndpoints->setAnimated(true);
	pEndpoints->setModel(pProxyModel);
	pEndpoints->sortByColumn(Column_Protocol, Qt::AscendingOrder);
	pEndpoints->header()->installEventFilter(pParent);

	pProxyModel->setSourceModel(pModel);

	connect( pEndpoints, SIGNAL(pressed(const QModelIndex&)), this, SLOT(EndpointSelected(const QModelIndex&)));
	connect( pEndpoints, SIGNAL(activated(const QModelIndex&)), this, SLOT(EndpointSelected(const QModelIndex&)));
	connect( this, SIGNAL(EndpointSelected(quint32)), pParent, SLOT(EndpointSelected(quint32)) );
	//
	// Add user filtering (CTL+F) support
	//
	pFilterWidget = new FilterWidget_C(this);

	pGridLayout->addWidget(pEndpoints,0,0);
	pGridLayout->addWidget(pFilterWidget,1,0);
	pGroupBox->setLayout(pGridLayout);
	setWidget(pGroupBox);
	//
	// Create the Widget for column selection
	//
	CreateColumnsSelectionWidget();
}