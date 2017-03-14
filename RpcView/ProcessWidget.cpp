#include "ProcessWidget.h"

static const char WidgetName[] = "Processes";

//------------------------------------------------------------------------------
QString ProcessWidget_C::GetColumName(Column_T Column)
{
	switch(Column)
	{
		case Column_Name		: return (QString("Name"));
		case Column_Pid			: return (QString("Pid")); 
		case Column_Path		: return (QString("Path"));
		case Column_Description	: return (QString("Description"));
		case Column_CmdLine		: return (QString("CmdLine"));
		case Column_UserName	: return (QString("User"));
		case Column_Desktop		: return (QString("Desktop"));
		case Column_IsRpcServer	: return (QString("RpcServer"));
		//case Column_IsListening	: return (QString("Listening"));
#ifdef _WIN64
		case Column_IsWow64		: return (QString("Image"));
#endif
		case Column_MaxCalls	: return (QString("MaxCalls"));
		default: return (QString("Unknown"));
	}
}

//------------------------------------------------------------------------------
ULONG ProcessWidget_C::GetProcesses()
{
	return ( pProxyModel->rowCount() );
}


//------------------------------------------------------------------------------
ULONG ProcessWidget_C::GetTotalProcesses()
{
	return ( pModel->rowCount() );
}

//------------------------------------------------------------------------------
void ProcessWidget_C::resizeColumnsToContents()
{
	int	Index;

	for( Index=0; Index < Column_Last; Index++)
	{
		pProcessTree->resizeColumnToContents(Index);
		pProcessView->resizeColumnToContents(Index);
	}
}


//------------------------------------------------------------------------------
void ProcessWidget_C::SelectProcess(quint32 Pid)
{
	QList<QTreeWidgetItem *>	TreeItemList;
	QList<QStandardItem*>		ViewItemList;

	TreeItemList= pProcessTree->findItems( QString("%1").arg(Pid), Qt::MatchFixedString|Qt::MatchRecursive, Column_Pid);
	if (!TreeItemList.isEmpty())
	{
		pProcessTree->setCurrentItem( TreeItemList.first() );
		pProcessTree->scrollToItem( TreeItemList.first() );
	}
	ViewItemList = pModel->findItems( QString("%1").arg(Pid), Qt::MatchFixedString, Column_Pid);
	if (!ViewItemList.isEmpty())
	{
		QModelIndex	ModelIndex = pProxyModel->mapFromSource( ViewItemList.first()->index() );
		pProcessView->selectionModel()->select( ModelIndex,  QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		pProcessView->scrollTo( ModelIndex );
	}
	pFilterWidget->Reset();
	this->ApplyUserFilter("");
}


//------------------------------------------------------------------------------
void ProcessWidget_C::SnapProcesses()
{
	PrivateTreeItemList = pProcessTree->findItems(".*", Qt::MatchRegExp|Qt::MatchRecursive, Column_Pid);
	PrivateViewItemList = pModel->findItems(".*", Qt::MatchRegExp, Column_Pid);
}


//------------------------------------------------------------------------------
void ProcessWidget_C::RemoveTerminatedProcesses()
{
	QTreeWidgetItem*	pParent;
	//
	// Remove processes from the TreeWidget
	// 1. Only child processes
	//
	for (auto Iter=PrivateTreeItemList.begin();Iter!=PrivateTreeItemList.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			pParent = (*Iter)->parent();
			if (pParent != NULL) pParent->removeChild(*Iter);
		}
	}
	//
	// 2. Parent processes
	//
	for (auto Iter = PrivateTreeItemList.begin(); Iter != PrivateTreeItemList.end(); Iter++)
	{
		if (*Iter != NULL)
		{
			pParent = (*Iter)->parent();
			if (pParent == NULL) delete *Iter;
		}
	}
	//
	// Remove processes from the TreeView
	//
	for (auto Iter=PrivateViewItemList.begin();Iter!=PrivateViewItemList.end();Iter++)
	{
		if (*Iter!=NULL) pModel->removeRow( (*Iter)->row());
	}
}
	

//------------------------------------------------------------------------------
bool ProcessWidget_C::IsProcessPresent(quint32 Pid)
{
	QList<QTreeWidgetItem *>	TreeItemList;
	QList<QStandardItem*>		ViewItemList;
	bool						bProcessPresent = false;

	TreeItemList = pProcessTree->findItems( QString("%1").arg(Pid), Qt::MatchFixedString|Qt::MatchRecursive, Column_Pid);
	if (TreeItemList.isEmpty()) goto End;
	
	PrivateTreeItemList.removeOne(TreeItemList.first());
	
	ViewItemList = pModel->findItems( QString("%1").arg(Pid), Qt::MatchFixedString, Column_Pid);
	if (ViewItemList.isEmpty()) goto End;
	
	PrivateViewItemList.removeOne(ViewItemList.first());
	bProcessPresent = true;
End:
	return (bProcessPresent);
}


//------------------------------------------------------------------------------
void ProcessWidget_C::ProcessSelected(QTreeWidgetItem* pItem, int Column)
{
	quint32	Pid;

	Pid = pItem->data(Column_Pid,0).toInt();
	emit ProcessSelected(Pid);
}

//------------------------------------------------------------------------------
void ProcessWidget_C::ProcessSelected(const QModelIndex& Index)
{
	quint32 Pid;

	Pid = pProxyModel->data( pProxyModel->index(Index.row(), Column_Pid) ).toUInt();
	emit ProcessSelected(Pid);
}


//------------------------------------------------------------------------------
void ProcessWidget_C::SetRowColor(QTreeWidgetItem* pProcess,int Index,const QColor& Color)
{
	for (int i=0;i<Column_Last;i++)
	{
		pProcess->setBackground(i, QBrush(Color) );
		pModel->setData(pModel->index(Index,i), QBrush(Color), Qt::BackgroundRole);
	}
}


//------------------------------------------------------------------------------
void ProcessWidget_C::UpdateProcess(RpcProcessInfo_T* pRpcProcessInfo)
{
	QList<QTreeWidgetItem *>	TreeItemList;
	QList<QStandardItem*>		ViewItemList;
	QTreeWidgetItem*			pProcess;
	int							Index;

	if (pRpcProcessInfo->bIsServer)
	{
		TreeItemList = pProcessTree->findItems( QString("%1").arg(pRpcProcessInfo->Pid), Qt::MatchFixedString|Qt::MatchRecursive, Column_Pid);
		if (TreeItemList.isEmpty()) return;
	
		ViewItemList = pModel->findItems(  QString("%1").arg(pRpcProcessInfo->Pid), Qt::MatchFixedString, Column_Pid);
		if (ViewItemList.isEmpty()) return;

		pProcess= TreeItemList.first();
		if (pProcess==NULL) return;

		Index = ViewItemList.first()->row();

		if ( pRpcProcessInfo->MaxCalls == RPC_C_LISTEN_MAX_CALLS_DEFAULT)
		{
			AddProcessItem( pProcess, Index, Column_MaxCalls, Qt::DisplayRole, QString("Default") );
		}
		else
		{
			AddProcessItem( pProcess, Index, Column_MaxCalls, Qt::DisplayRole, (quint32)pRpcProcessInfo->MaxCalls );
		}
		//--
		AddProcessItem( pProcess, Index, Column_IsRpcServer, Qt::DisplayRole, true );

		switch(pRpcProcessInfo->RpcProcessType)
		{
			case RpcProcessType_RPC:
				SetRowColor(pProcess,Index,QColor(200, 200, 255, 180));
			break;
			//--
			case RpcProcessType_DCOM:
				SetRowColor(pProcess,Index,QColor(255, 255, 200, 180));
				//SetRowColor(pProcess, Index, QColor(200, 200, 255, 180));
			break;
			//--
			case RpcProcessType_HYBRID:
				//SetRowColor(pProcess,Index,QColor(255, 200, 200, 180));
				SetRowColor(pProcess, Index, QColor(200, 200, 255, 180));
			break;
			//--
			default:
			break;
		}
	}
}


//------------------------------------------------------------------------------
void ProcessWidget_C::AddProcessItem(QTreeWidgetItem* pProcess,int Index, Column_T Column, Qt::ItemDataRole Role,const QVariant& Value)
{
	pProcess->setData( Column, Role, Value );
	pModel->setData(pModel->index(Index, Column), Value, Role );
}


//------------------------------------------------------------------------------
bool ProcessWidget_C::AddProcess(RpcProcessInfo_T* pRpcProcessInfo)
{
	QList<QTreeWidgetItem*> ParentList;
	QTreeWidgetItem*		pParent = NULL;
	QTreeWidgetItem*		pProcess;
	int						Index;

	Index = pModel->rowCount();
	pModel->setRowCount(Index+1);

	pProcess = new QTreeWidgetItem;
 
	if ( pRpcProcessInfo->hIcon!=NULL )	
	{	
		AddProcessItem(pProcess,Index,Column_Name, Qt::DecorationRole, QIcon( QPixmap::fromWinHICON( pRpcProcessInfo->hIcon ) ) );
		DestroyIcon( pRpcProcessInfo->hIcon );
	}

	if (pRpcProcessInfo->Pid == 0)	
	{
		AddProcessItem(pProcess,Index,Column_Name, Qt::DisplayRole, QString("[System Idle Process]") );
		AddProcessItem(pProcess,Index,Column_Pid, Qt::DisplayRole, (quint32) pRpcProcessInfo->Pid );
		pProcessTree->addTopLevelItem(pProcess);
		return true;
	}
	AddProcessItem(pProcess,Index,Column_Name, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->Name) );

	AddProcessItem(
			pProcess,
			Index,
			Column_Name, 
			Qt::ToolTipRole,
			QString("Path:\n").append(QString::fromUtf16((const ushort*)pRpcProcessInfo->Path)).append("\nCmdLine:\n").append(QString::fromUtf16((const ushort*)pRpcProcessInfo->CmdLine)) 
			);

	AddProcessItem(pProcess,Index,Column_Pid, Qt::DisplayRole, (quint32) pRpcProcessInfo->Pid );
	AddProcessItem(pProcess,Index,Column_UserName, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->User) );
	AddProcessItem(pProcess,Index,Column_Path, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->Path) );
	AddProcessItem(pProcess,Index,Column_Description, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->Description) );
	AddProcessItem(pProcess,Index,Column_CmdLine, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->CmdLine) );
	AddProcessItem(pProcess,Index,Column_Desktop, Qt::DisplayRole, QString::fromUtf16((const ushort*)pRpcProcessInfo->Desktop) );
#ifdef _WIN64
	if (pRpcProcessInfo->bIsWow64) AddProcessItem(pProcess,Index,Column_IsWow64, Qt::DisplayRole, QString("32-bit") );
	else AddProcessItem(pProcess,Index,Column_IsWow64, Qt::DisplayRole, QString("64-bit") );
#endif

	QList<QTreeWidgetItem *>	ItemList;

	ItemList = pProcessTree->findItems( QString("%1").arg(pRpcProcessInfo->ParentPid), Qt::MatchFixedString|Qt::MatchRecursive, Column_Pid);
	if (!ItemList.isEmpty())
	{
		pParent = ItemList.first();
		pParent->addChild(pProcess);
		pProcessTree->expandItem(pParent);
	}
	else
	{
		pProcessTree->addTopLevelItem(pProcess);
	}
	//
	// Add warning: when a non rpc server is selected -> view all endpoints and interfaces of the system
	//
	if (!pRpcProcessInfo->bIsServer)
	{
		for(int i=0;i<Column_Last;i++)
		{
			pProcess->setStatusTip(i,"WARNING: view all the system endpoints and interfaces.");
		}
	}
	UpdateProcess(pRpcProcessInfo);
	return (true);
}


//------------------------------------------------------------------------------
void ProcessWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}

//------------------------------------------------------------------------------
void ProcessWidget_C::Reset()
{
	pProcessTree->clear();
	pProcessView->reset();
}


//------------------------------------------------------------------------------
void ProcessWidget_C::SaveConfiguration(QSettings* pSettings)
{
	pSettings->setValue("Processes/View/geometry",pProcessView->header()->saveState());
	pSettings->setValue("Processes/Tree/geometry",pProcessTree->header()->saveState());
}


//------------------------------------------------------------------------------
void ProcessWidget_C::LoadConfiguration(QSettings* pSettings)
{
	pProcessView->header()->restoreState( pSettings->value("Processes/View/geometry").toByteArray() );
	pProcessTree->header()->restoreState( pSettings->value("Processes/Tree/geometry").toByteArray() );
	//
	// Force Tree sorting by PID
	//
	pProcessTree->sortByColumn(Column_Pid);
	pProcessTree->sortByColumn(-1);
}

//------------------------------------------------------------------------------
void ProcessWidget_C::InitProcessTreeWidget(QWidget* pParent)
{
	pProcessTree = new QTreeWidget(this); 

	QTreeWidgetItem* pHeaderItem = pProcessTree->headerItem();
	for (quint32 Idx = 0; Idx < Column_Last; Idx++)
	{ 
		pHeaderItem->setText( Idx, GetColumName((Column_T)Idx) );
	}
	pProcessTree->setColumnCount(Column_Last);
	pProcessTree->sortByColumn(Column_Pid);
	pProcessTree->sortByColumn(-1);
	pProcessTree->setAnimated(true);
	pProcessTree->setSortingEnabled(true);

	pProcessTree->expandAll();
	pProcessTree->setRootIsDecorated(false);
	pProcessTree->setMouseTracking(true);

	pProcessTree->hideColumn(Column_IsRpcServer);
	pProcessTree->hideColumn(Column_Desktop);
	
	connect( pProcessTree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(ProcessSelected(QTreeWidgetItem*, int)));
	connect( pProcessTree, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(ProcessSelected(QTreeWidgetItem*, int)));
}


//------------------------------------------------------------------------------
void ProcessWidget_C::InitProcessTreeView(QWidget* pParent)
{
	pProxyModel = new QSortFilterProxyModel(this);
	pProxyModel->setDynamicSortFilter(true);
	pProxyModel->setFilterKeyColumn(Column_Pid);
	pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	
	pModel = new QStandardItemModel(0, Column_Last, this);
	
	for(uint i=0;i<Column_Last;i++)
	{
		pModel->setHeaderData(i, Qt::Horizontal, GetColumName((Column_T)i) );
	}

	pProcessView = new QTreeView(this);
	pProcessView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pProcessView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pProcessView->setSelectionMode(QAbstractItemView::SingleSelection);
	pProcessView->setRootIsDecorated(false);
    pProcessView->setSortingEnabled(true);
	pProcessView->setAlternatingRowColors(true);
	pProcessView->setAnimated(true);
	pProcessView->setModel(pProxyModel);
	pProcessView->sortByColumn(Column_Pid, Qt::AscendingOrder);

	pProcessView->hideColumn(Column_IsRpcServer);
	pProcessView->hideColumn(Column_Desktop);

	pProxyModel->setSourceModel(pModel);
	connect( pProcessView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(ProcessSelected(const QModelIndex&)));
	connect( pProcessView, SIGNAL(activated(const QModelIndex&)), this, SLOT(ProcessSelected(const QModelIndex&)));
}


//------------------------------------------------------------------------------
void ProcessWidget_C::UpdateUserFilter()
{
	if (pFilterWidget->GetText()!="") ApplyUserFilter( pFilterWidget->GetText() );
}


//------------------------------------------------------------------------------
void ProcessWidget_C::ApplyUserFilter(const QString & FilterText)
{
	QRegExp	FilterRegExp;

	FilterRegExp.setPattern( FilterText );
	FilterRegExp.setCaseSensitivity( Qt::CaseInsensitive );

	pProxyModel->setFilterKeyColumn(-1);		//filter all columns: UGLY Qt
	pProxyModel->setFilterRegExp( FilterRegExp );

	if ( pProxyModel->rowCount()==pModel->rowCount() )
	{
		// 
		// In case of view switching, we have to update the selected process in the other view!
		//
		if ( (pStackedWidget->currentWidget()!=pProcessTree) && 
             (pProcessTree->header()->sortIndicatorSection()==-1) )
		{
			pStackedWidget->setCurrentWidget(pProcessTree);
			pProcessTree->header()->restoreState(pProcessView->header()->saveState());
		}
		else
		{
			pProxyModel->setFilterRegExp( QRegExp(".*") );
		}
	}
	else
	{
		if (pStackedWidget->currentWidget()!=pProcessView)
		{
			pStackedWidget->setCurrentWidget(pProcessView);
			pProcessView->header()->restoreState(pProcessTree->header()->saveState());
		}
	}
}


//------------------------------------------------------------------------------
void ProcessWidget_C::CreateColumnsSelectionWidget()
{
	pColumnsSelectionWidget = new QGroupBox(WidgetName,this);
	QVBoxLayout*	pLayout = new QVBoxLayout(pColumnsSelectionWidget);

	for (int i = 0; i < Column_Last; i++)
	{
		CheckBoxArray[i] = new QCheckBox(GetColumName((Column_T)i), pColumnsSelectionWidget);
		pLayout->addWidget(CheckBoxArray[i]);
	}
	pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	pColumnsSelectionWidget->setLayout(pLayout);
}


//------------------------------------------------------------------------------
QWidget* ProcessWidget_C::GetColumnsSelectionWidget()
{
	for (int i = 0; i < Column_Last; i++)
	{
		CheckBoxArray[i]->setChecked(!pProcessTree->header()->isSectionHidden(i));
	}
	return pColumnsSelectionWidget;
}


//------------------------------------------------------------------------------
void ProcessWidget_C::UpdateColumnsVisibility()
{
	for (int i = 0; i < Column_Last; i++)
	{
		pProcessTree->header()->setSectionHidden(i, !CheckBoxArray[i]->isChecked() );
		pProcessView->header()->setSectionHidden(i, !CheckBoxArray[i]->isChecked());
	}
}


//------------------------------------------------------------------------------
// Switch to the view header
void ProcessWidget_C::TreeHeaderClicked(int logicalIndex)
{
	if (pStackedWidget->currentWidget()!=pProcessView)
	{
		pStackedWidget->setCurrentWidget(pProcessView);
		pProcessView->header()->restoreState(pProcessTree->header()->saveState());
		pProcessView->scrollTo(pProcessView->currentIndex());
	}
}


//------------------------------------------------------------------------------
// Switch to the Tree after ascending and descending order
void ProcessWidget_C::ViewHeaderClicked(int logicalIndex)
{
	if (logicalIndex!=Column_Name) return;

	if (pProcessView->header()->sortIndicatorOrder() == Qt::DescendingOrder) return;

	if (pStackedWidget->currentWidget()!=pProcessTree)
	{
		pFilterWidget->Reset();
		ApplyUserFilter("");

		pStackedWidget->setCurrentWidget(pProcessTree);
		pProcessTree->header()->restoreState(pProcessView->header()->saveState());
		pProcessTree->sortByColumn(Column_Pid);
		pProcessTree->sortByColumn(-1);
		pProcessTree->scrollToItem(pProcessTree->currentItem());
	}
}


//------------------------------------------------------------------------------
ProcessWidget_C::ProcessWidget_C(QWidget* pParent):QGroupBox(WidgetName)
{
	QGridLayout*	pGridLayout;

	setObjectName(WidgetName);

	pStackedWidget = new QStackedWidget(this);
	pGridLayout = new QGridLayout(this);

	InitProcessTreeWidget(pParent);
	InitProcessTreeView(pParent);
	pProcessView->header()->installEventFilter(pParent);
	pProcessTree->header()->installEventFilter(pParent);

	pStackedWidget->addWidget(pProcessTree);
	pStackedWidget->addWidget(pProcessView);
	//
	// Add user filtering (CTL+F) support
	//
	pFilterWidget = new FilterWidget_C(this);
	
	pGridLayout->addWidget(pStackedWidget,0,0);
	pGridLayout->addWidget(pFilterWidget,1,0);
	setLayout(pGridLayout);
	connect( this, SIGNAL(ProcessSelected(quint32)), pParent, SLOT(ProcessSelected(quint32)) );
	connect( pProcessTree->header(),SIGNAL(sectionClicked(int)), this , SLOT(TreeHeaderClicked(int) ));
	connect( pProcessView->header(),SIGNAL(sectionClicked(int)), this , SLOT(ViewHeaderClicked(int) ));
	//
	// Create the Widget for column selection
	//
	CreateColumnsSelectionWidget();
}
