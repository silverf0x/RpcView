#include "ProcessInfoWidget.h"
#include "../RpcCore/RpcCore.h"
#include "../RpcCommon/Misc.h"

static const char WidgetName[] = "Processes Properties";

typedef enum _AuthInfoColumn_T{
	AuthInfoColumn_AuthSvc,
	AuthInfoColumn_Name,
	AuthInfoColumn_Comment,
	AuthInfoColumn_PrincipalName,
	//...
	AuthInfoColumn_Last
}AuthInfoColumn_T;


typedef enum _ProcessInfoTab_T{
	ProcessInfoTab_Image,
	ProcessInfoTab_RPC,
	ProcessInfoTab_SSP
}ProcessInfoTab_T;


//------------------------------------------------------------------------------
QString GetProcessInfoTabName(ProcessInfoTab_T ProcessInfoTab)
{
	switch(ProcessInfoTab)
	{
		case ProcessInfoTab_Image	: return QString("Image");
		case ProcessInfoTab_RPC		: return QString("RPC");
		case ProcessInfoTab_SSP		: return QString("SSP");
		default:return QString("Unknown");
	}
}


//------------------------------------------------------------------------------
QString GetAuthInfoColumnName(AuthInfoColumn_T AuthInfoColumn)
{
	switch(AuthInfoColumn)
	{
		case AuthInfoColumn_AuthSvc: return QString("Id");
		case AuthInfoColumn_Name: return QString("Name");
		case AuthInfoColumn_Comment: return QString("Comment");
		case AuthInfoColumn_PrincipalName: return QString("PrincName");
		default:return QString("Unknown");
	}
}


//------------------------------------------------------------------------------
bool ProcessInfoWidget_C::AddAuthInfo(RpcAuthInfo_T* pRpcAuthInfo)
{
	int	Index;
	
	Index = pModel->rowCount();
	pModel->setRowCount(Index+1);
	pModel->setData(pModel->index(Index, AuthInfoColumn_AuthSvc), (quint32)pRpcAuthInfo->AuthSvc );
	pModel->setData(pModel->index(Index, AuthInfoColumn_Name), QString::fromUtf16((const ushort*)pRpcAuthInfo->Name));
	pModel->setData(pModel->index(Index, AuthInfoColumn_Comment), QString::fromUtf16((const ushort*)pRpcAuthInfo->Comment));
	pModel->setData(pModel->index(Index, AuthInfoColumn_PrincipalName), QString::fromUtf16((const ushort*)pRpcAuthInfo->PrincipalName));
	return (true);
}


//------------------------------------------------------------------------------
void ProcessInfoWidget_C::reset()
{
		pModel->setRowCount(0);
}

//------------------------------------------------------------------------------
void ProcessInfoWidget_C::resizeColumnsToContents()
{
	int	Index;

	for( Index=0; Index < AuthInfoColumn_Last; Index++)
	{
		pAuthTreeView->resizeColumnToContents(Index);
	}
}


//------------------------------------------------------------------------------
ULONG ProcessInfoWidget_C::GetPid()
{
	return (this->Pid);
}


//------------------------------------------------------------------------------
void ProcessInfoWidget_C::UpdateProcessInfo(RpcProcessInfo_T* pRpcProcessInfo)
{
	Version_T	Version;
	bool		bEnable;
	
	if (pRpcProcessInfo==NULL) return;

	this->Pid = pRpcProcessInfo->Pid;

	pTabWidget->setTabEnabled(pTabWidget->indexOf(pImageWidget),true);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),pRpcProcessInfo->bIsServer);

	if (pRpcProcessInfo->SspCount != 0) pTabWidget->setTabEnabled(pTabWidget->indexOf(pAuthTreeView),true);
	else pTabWidget->setTabEnabled(pTabWidget->indexOf(pAuthTreeView),false);

	Version.As64BitsValue = pRpcProcessInfo->Version;

	if ( pRpcProcessInfo->Path[0] == 0) bEnable=false;
	else bEnable=true;
	pPath->setEnabled(bEnable);
	pPath->setText( QString::fromUtf16((const ushort*)pRpcProcessInfo->Path) );
	pPath->setToolTip( pPath->text() );
	pPath->setCursorPosition(0);

	if ( pRpcProcessInfo->CmdLine[0] == 0) bEnable=false;
	else bEnable=true;
	pCmdLine->setEnabled(bEnable);
	pCmdLine->setText( QString::fromUtf16((const ushort*)pRpcProcessInfo->CmdLine) );
	pCmdLine->setToolTip( pCmdLine->text() );
	pCmdLine->setCursorPosition(0);

	if (pRpcProcessInfo->hIcon!=NULL)
	{
		pIconLabel->setPixmap( QtWin::fromHICON( pRpcProcessInfo->hIcon ) );
		pIconLabel->show();
		DestroyIcon( pRpcProcessInfo->hIcon );
	}
	else
	{
		pIconLabel->hide();
	}

	if ( pRpcProcessInfo->User[0] == 0) bEnable=false;
	else bEnable=true;
	pUser->setEnabled(bEnable);
	pUser->setText( QString::fromUtf16((const ushort*)pRpcProcessInfo->User) );
	pCmdLine->setCursorPosition(0);

	pInterfaces->setText( QString("%1").arg(pRpcProcessInfo->InterfacesCount) );
	pEndpoints->setText( QString("%1").arg(pRpcProcessInfo->EndpointsCount) );
	if (pRpcProcessInfo->MaxCalls==RPC_C_LISTEN_MAX_CALLS_DEFAULT) pMaxCall->setText( "Default" );
	else pMaxCall->setText( QString("%1").arg(pRpcProcessInfo->MaxCalls) );
	/*
	if (pRpcProcessInfo->bIsListening) pState->setText( "Listening" );
	else pState->setText( "Stopped" );
	*/	
	pName->setText( QString::fromUtf16((const ushort*)pRpcProcessInfo->Description) );
	if (pRpcProcessInfo->Desktop[0] == NULL)  bEnable=false;
	else bEnable=true;
	pDesktop->setEnabled(bEnable);
	pDesktop->setText( QString::fromUtf16((const ushort*)pRpcProcessInfo->Desktop) );
	pDesktop->setCursorPosition(0);

	pInCalls->setText( QString("%1").arg(pRpcProcessInfo->InCalls) );
	pOutCalls->setText( QString("%1").arg(pRpcProcessInfo->OutCalls) );
	pInPackets->setText( QString("%1").arg(pRpcProcessInfo->InPackets) );
	pOutPackets->setText( QString("%1").arg(pRpcProcessInfo->OutPackets) );
	
	if (Version.As64BitsValue==0) pVersion->setText("Unknown");
	else pVersion->setText( QString("%4.%3.%2.%1").arg(Version.As16BitsValues.Part1).arg(Version.As16BitsValues.Part2).arg(Version.As16BitsValues.Part3).arg(Version.As16BitsValues.Part4) );
#ifdef _WIN64	
	if (pRpcProcessInfo->bIsWow64) pImage->setText( QString("32-bits") );
	else pImage->setText( QString("64-bits") );
#else
	pImage->setText( QString("32-bits") );
#endif
}


//------------------------------------------------------------------------------
void ProcessInfoWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}


//------------------------------------------------------------------------------
ProcessInfoWidget_C::ProcessInfoWidget_C(QWidget* pParent):QDockWidget(WidgetName)
{

    UNREFERENCED_PARAMETER(pParent);

	setObjectName(WidgetName);

	pTabWidget	= new QTabWidget(this);
	pTabWidget->setMovable(true);
	setWidget(pTabWidget);
	//
	// Creates the Image properties tab
	//
	QFormLayout*	pImageLayout = new QFormLayout;
	pImageWidget	= new QWidget(this);
	pIconLabel		= new QLabel(this);
	pVersion		= new QLabel(this);
	pName			= new QLineEdit(this);
	pUser			= new QLineEdit(this);
	pCmdLine		= new QLineEdit(this);
	pPath			= new QLineEdit(this);
	pDesktop		= new QLineEdit(this);
	pImage			= new QLabel(this);

	pName->setFrame(false);
	pName->setReadOnly(true);
	pImageLayout->addRow(pIconLabel, pName);
	pImageLayout->addRow("Version:", pVersion);
	pImageLayout->addRow("Path:", pPath);
	pPath->setReadOnly(true);
	pPath->setEnabled(false);
	pImageLayout->addRow("CmdLine:", pCmdLine);
	pCmdLine->setReadOnly(true);
	pCmdLine->setEnabled(false);
	pImageLayout->addRow("User:", pUser);
	pUser->setReadOnly(true);
	pUser->setEnabled(false);
	pImageLayout->addRow("Desktop:", pDesktop);
	pDesktop->setReadOnly(true);
	pDesktop->setEnabled(false);
	pImageLayout->addRow("Image:", pImage);

	pImageWidget->setLayout(pImageLayout);
	pTabWidget->insertTab(ProcessInfoTab_Image,pImageWidget,GetProcessInfoTabName(ProcessInfoTab_Image));
	//
	// Creates the RPC properties tab
	//
	pRpcWidget = new QWidget(this);
	QFormLayout*	pRpcLayout = new QFormLayout;
	//pState			= new QLabel(this);
	pMaxCall		= new QLabel(this);
	pEndpoints		= new QLabel(this);
	pInterfaces		= new QLabel(this);
	pInCalls		= new QLabel(this);
	pOutCalls		= new QLabel(this);
	pInPackets		= new QLabel(this);
	pOutPackets		= new QLabel(this);
	
	pRpcLayout->addRow("Interfaces:", pInterfaces);
	pRpcLayout->addRow("Endpoints:", pEndpoints);
	//pRpcLayout->addRow("State:", pState);
	pRpcLayout->addRow("MaxCalls:", pMaxCall);

	pRpcLayout->addRow("In  Calls:", pInCalls);
	pRpcLayout->addRow("Out Calls:", pOutCalls);
	pRpcLayout->addRow("In  Packets:", pInPackets);
	pRpcLayout->addRow("Out Packets:", pOutPackets);

	pRpcWidget->setLayout(pRpcLayout);
	pTabWidget->insertTab(ProcessInfoTab_RPC,pRpcWidget,GetProcessInfoTabName(ProcessInfoTab_RPC));
	//
	// Creates the Security package properties tab
	//
	pProxyModel = new QSortFilterProxyModel;
	pProxyModel->setDynamicSortFilter(true);
	pProxyModel->setFilterKeyColumn(AuthInfoColumn_AuthSvc);
	pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	
	pModel = new QStandardItemModel(0, AuthInfoColumn_Last, this);
	
	for(int i=0;i<AuthInfoColumn_Last;i++)
	{
		pModel->setHeaderData(i, Qt::Horizontal, GetAuthInfoColumnName( (AuthInfoColumn_T)i ) );
	}

	pAuthTreeView = new QTreeView;
	pAuthTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pAuthTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pAuthTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	pAuthTreeView->setRootIsDecorated(false);
    pAuthTreeView->setSortingEnabled(true);
	pAuthTreeView->setAnimated(true);
	pAuthTreeView->setModel(pProxyModel);
	pAuthTreeView->sortByColumn(AuthInfoColumn_AuthSvc, Qt::AscendingOrder);

	pProxyModel->setSourceModel(pModel);
	pTabWidget->insertTab(ProcessInfoTab_SSP,pAuthTreeView,GetProcessInfoTabName(ProcessInfoTab_SSP));
	//
	// By default: all tabs are disable
	//
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pImageWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pAuthTreeView),false);
}