#include "InterfaceInfoWidget.h"
#include "..\RpcCommon\RpcCommon.h"

//------------------------------------------------------------------------------
extern QString GetInterfaceFlagsString(quint32 Flags);

static const char WidgetName[] = "Interface Properties";

//------------------------------------------------------------------------------
QString GetInterfaceNdrFlagsString(quint32 Flags)
{
	QString	FlagsString;

	if (Flags & RPCFLG_HAS_MULTI_SYNTAXES)	FlagsString.append("RPCFLG_HAS_MULTI_SYNTAXES\n");
	if (Flags & RPCFLG_HAS_CALLBACK)		FlagsString.append("RPCFLG_HAS_CALLBACK\n");
	if (Flags & RPC_INTERFACE_HAS_PIPES)	FlagsString.append("RPC_INTERFACE_HAS_PIPES\n");
	return (FlagsString);
}


//------------------------------------------------------------------------------
void InterfaceInfoWidget_C::Reset()
{
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pMainWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pNdrWidget),false);
}


//------------------------------------------------------------------------------
UINT InterfaceInfoWidget_C::GetPid()
{
	return (this->Pid);
}


//------------------------------------------------------------------------------
void InterfaceInfoWidget_C::UpdateInterfaceInfo(RpcInterfaceInfo_T* pRpcInterfaceInfo, WCHAR* pIfCallbackName)
{
	WCHAR*	pUuidString = NULL;
	WCHAR*	pTransfertSyntaxString = NULL;
	bool	bEnable;

	if (pRpcInterfaceInfo==NULL) goto End;
	this->Pid = pRpcInterfaceInfo->Pid;

	if ( UuidToStringW(&pRpcInterfaceInfo->If.Uuid,(RPC_WSTR*)&pUuidString)!=RPC_S_OK ) goto End;
	if ( UuidToStringW(&pRpcInterfaceInfo->TransfertSyntax.SyntaxGUID,(RPC_WSTR*)&pTransfertSyntaxString)!=RPC_S_OK ) goto End;

	pTabWidget->setTabEnabled(pTabWidget->indexOf(pMainWidget),true);
	pUuid->setText( QString::fromUtf16((const ushort*)pUuidString) );
	//
	// Only Rpc interfaces have versions
	//
	if (pRpcInterfaceInfo->IfType==IfType_RPC)
	{
		pVersion->setText( QString("%1.%2").arg(pRpcInterfaceInfo->If.VersMajor).arg(pRpcInterfaceInfo->If.VersMinor) );
	}
	else
	{
		pVersion->clear();
	}
	pName->setText( QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Name) );

	if (pRpcInterfaceInfo->IfType!=IfType_RPC)
	{
		pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),false);
	}
	else
	{
		pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),true);
	}

	if ( (pIfCallbackName==NULL) || (*pIfCallbackName==0) ) bEnable=false;
	else bEnable=true;
	pCallbackName->setEnabled(bEnable);
	pCallbackName->setText( QString::fromUtf16((const ushort*)pIfCallbackName) );

	if (pRpcInterfaceInfo->TypeOfStub==TypeOfStub_TypeLib)
	{
		pTabWidget->setTabEnabled(pTabWidget->indexOf(pNdrWidget),false);
	}
	else
	{
		pTabWidget->setTabEnabled(pTabWidget->indexOf(pNdrWidget),true);
	}

	if (pRpcInterfaceInfo->NumberOfProcedures==INVALID_PROC_COUNT) pProcCount->setText( "Unknown" );
	else pProcCount->setText( QString("%1").arg(pRpcInterfaceInfo->NumberOfProcedures) );

	switch(pRpcInterfaceInfo->TypeOfStub) 
	{
		case TypeOfStub_Interpreted:pStub->setText("Interpreted");	break;
		case TypeOfStub_Inlined:	pStub->setText("Inlined");		break;
		case TypeOfStub_TypeLib:	pStub->setText("TypeLib");		break;
		case TypeOfStub_Hybrid:		pStub->setText("Hybrid");		break;
		default:					pStub->setText("Unknown");		break;
	}

	if (pRpcInterfaceInfo->Location[0]==0) bEnable=false;
	else bEnable=true;
	pLocation->setEnabled(bEnable);
	pLocation->setText( QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Location) );
	pLocation->setCursorPosition(0);

	Base					= (quintptr)pRpcInterfaceInfo->pLocationBase;
	IfCallback				= (quintptr)pRpcInterfaceInfo->IfCallbackFn;
	TypeFormatString		= (quintptr)pRpcInterfaceInfo->pTypeFormatString;
	ProcFormatString		= (quintptr)pRpcInterfaceInfo->pProcFormatString;
	ExpressionEvaluation	= (quintptr)pRpcInterfaceInfo->apfnExprEval;

	pBase->setText(QString("0x%1").arg(Base, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
	SetAddressRepresentation(this->AddressRepresentation);
	//
	// Even if Location state is actually a mask, we consider it as 2 separte values
	//
		 if (pRpcInterfaceInfo->LocationState == MEM_FREE)		pState->setText("MEM_FREE");
	else if (pRpcInterfaceInfo->LocationState == MEM_COMMIT)	pState->setText("MEM_COMMIT");

	if (pRpcInterfaceInfo->Annotation[0]==0) bEnable=false;
	else bEnable=true;

	pAnnotation->setEnabled(bEnable);
	pAnnotation->setText( QString::fromLatin1((const char*)pRpcInterfaceInfo->Annotation) );
	pAnnotation->setCursorPosition(0);

	if (!pRpcInterfaceInfo->bIsRegistered)
	{
		pEpMapper->setEnabled(false);
		pEpMapper->setText( "" );
	}
	else
	{
		pEpMapper->setEnabled(true);
		pEpMapper->setText( "Registered" );
	}

	pTransfertSyntax->setText( QString::fromUtf16((const ushort*)pTransfertSyntaxString).append(" V%1.%2").arg(pRpcInterfaceInfo->TransfertSyntax.SyntaxVersion.MajorVersion).arg(pRpcInterfaceInfo->TransfertSyntax.SyntaxVersion.MinorVersion) );
	pNdrVersion->setText( QString("0x%1").arg(pRpcInterfaceInfo->NdrInfo.Version,0,16) );
	pMidlVersion->setText( QString("0x%1").arg(pRpcInterfaceInfo->NdrInfo.MIDLVersion,0,16) );
	pNdrFlags->setText( GetInterfaceNdrFlagsString(pRpcInterfaceInfo->NdrInfo.mFlags) );

	if (pRpcInterfaceInfo->Flags==0) bEnable=false;
	else bEnable=true;

	pFlags->setEnabled(bEnable);
	pFlags->setText( GetInterfaceFlagsString(pRpcInterfaceInfo->Flags) );

	if (pRpcInterfaceInfo->Description[0]==0) bEnable=false;
	else bEnable=true;

	pDescription->setEnabled(bEnable);
	pDescription->setText( QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Description) );
End:
	if (pUuidString!=NULL)				RpcStringFreeW((RPC_WSTR*)&pUuidString);
	if (pTransfertSyntaxString!=NULL)	RpcStringFreeW((RPC_WSTR*)&pTransfertSyntaxString);
	return;
}

//------------------------------------------------------------------------------
void InterfaceInfoWidget_C::SetAddressRepresentation(AddressRepresentation_T AddressRepresentation)
{
	this->AddressRepresentation	= AddressRepresentation;
	
	pCallbackAddress->setText("");
	pTypeFormatString->setText("");
	pProcFormatString->setText("");
	pExpressionEvaluation->setText("");
	switch (AddressRepresentation)
	{
		case AddressRepresentation_RVA:
			if ((IfCallback != 0) && (IfCallback!=INVALID_IF_CALLBACK_ADDRESS)) pCallbackAddress->setText(QString("+0x%1").arg(IfCallback - Base, 8, 16, QLatin1Char('0')));
			if (TypeFormatString != 0) pTypeFormatString->setText(QString("+0x%1").arg(TypeFormatString - Base, 8, 16, QLatin1Char('0')));
			if (ProcFormatString != 0) pProcFormatString->setText(QString("+0x%1").arg(ProcFormatString - Base, 8, 16, QLatin1Char('0')));
			if (ExpressionEvaluation != 0) pExpressionEvaluation->setText(QString("+0x%1").arg(ExpressionEvaluation - Base, 8, 16, QLatin1Char('0')));
		break;
		//--
		case AddressRepresentation_Absolute:
			if ((IfCallback != 0) && (IfCallback != INVALID_IF_CALLBACK_ADDRESS))pCallbackAddress->setText(QString("0x%1").arg(IfCallback, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
			if (TypeFormatString != 0) pTypeFormatString->setText(QString("0x%1").arg(TypeFormatString, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
			if (ProcFormatString != 0) pProcFormatString->setText(QString("0x%1").arg(ProcFormatString, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
			if (ExpressionEvaluation != 0) pExpressionEvaluation->setText(QString("0x%1").arg(ExpressionEvaluation, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
		break;
		//--
		default:
		break;
	}
}


//------------------------------------------------------------------------------
void InterfaceInfoWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}


//------------------------------------------------------------------------------
InterfaceInfoWidget_C::InterfaceInfoWidget_C(QWidget* pParent):QDockWidget(WidgetName)
{
	this->Pid = 0;
	
	setObjectName(WidgetName);

	pTabWidget	= new QTabWidget(this);
	pTabWidget->setMovable(true);
	setWidget(pTabWidget);
	//
	// Creates the main properties tab
	//
	pMainWidget	= new QWidget(this);
	QFormLayout*	pMainLayout	= new QFormLayout;
	
	pUuid			= new QLineEdit(this);
	pVersion		= new QLabel(this);
	pName			= new QLineEdit(this);
	pLocation		= new QLineEdit(this);
	pBase			= new QLabel(this);
	pState			= new QLabel(this);
	pStub			= new QLabel(this);
	pProcCount		= new QLabel("0",this);
	pDescription	= new QTextEdit(this);

	pUuid->setReadOnly(true);
	pName->setReadOnly(true);
	pLocation->setEnabled(false);
	pLocation->setReadOnly(true);
	pDescription->setReadOnly(true);
	pDescription->setMinimumHeight(20);

	pMainLayout->addRow("Uuid:", pUuid);
	pMainLayout->addRow("Version:", pVersion);
	pMainLayout->addRow("Name:", pName);
	pMainLayout->addRow("Location:", pLocation);
	pMainLayout->addRow("Base:", pBase);
	pMainLayout->addRow("State:", pState);
	pMainLayout->addRow("Stub:", pStub);
	pMainLayout->addRow("Procedures:", pProcCount);
	pMainLayout->addRow("Description:", pDescription);

	pMainWidget->setLayout(pMainLayout);
	pTabWidget->addTab(pMainWidget,"Main");
	//
	// Creates the RPC tab
	//
	pRpcWidget	= new QWidget(this);
	QFormLayout*	pRpcLayout	= new QFormLayout;

	pCallbackName	= new QLineEdit(this);
	pCallbackAddress= new QLineEdit(this);
	pEpMapper		= new QLabel(this);
	pFlags			= new QTextEdit(this);
	pAnnotation		= new QLineEdit(this);
	
	pFlags->setReadOnly(true);
	pFlags->setMinimumHeight(20);
	pCallbackName->setReadOnly(true);
	pCallbackAddress->setReadOnly(true);
	pAnnotation->setEnabled(false);
	pAnnotation->setReadOnly(true);
	
	pRpcLayout->addRow("Callback Name:", pCallbackName);
	pRpcLayout->addRow("Callback Addr:", pCallbackAddress);
	pRpcLayout->addRow("EpMapper:", pEpMapper);
	pRpcLayout->addRow("Annotation:", pAnnotation);
	pRpcLayout->addRow("Flags:", pFlags);
	
	pRpcWidget->setLayout(pRpcLayout);
	pTabWidget->addTab(pRpcWidget,"RPC");
	//
	// Creates the NDR tab
	//
	pNdrWidget	= new QWidget(this);
	QFormLayout*	pNdrLayout	= new QFormLayout;

	pTransfertSyntax			= new QLineEdit(this);
	pNdrVersion					= new QLabel("0",this);
	pMidlVersion				= new QLabel("0",this);
	pNdrFlags					= new QTextEdit(this);
	pTypeFormatString			= new QLineEdit(this);
	pProcFormatString			= new QLineEdit(this);
	pExpressionEvaluation		= new QLineEdit(this);

	pTransfertSyntax->setReadOnly(true);
	pNdrFlags->setReadOnly(true);
	pNdrFlags->setMinimumHeight(20);
	pTypeFormatString->setReadOnly(true);
	pProcFormatString->setReadOnly(true);
	pExpressionEvaluation->setReadOnly(true);

	pNdrLayout->addRow("Syntax:", pTransfertSyntax);
	pNdrLayout->addRow("NDR Version:", pNdrVersion);
	pNdrLayout->addRow("MIDL Version:", pMidlVersion);
	pNdrLayout->addRow("NDR Flags:", pNdrFlags);
	pNdrLayout->addRow("TypeFormatString:", pTypeFormatString);
	pNdrLayout->addRow("ProcFormatString:", pProcFormatString);
	pNdrLayout->addRow("ExpressionEvaluation:", pExpressionEvaluation);
		
	pNdrWidget->setLayout(pNdrLayout);
	pTabWidget->addTab(pNdrWidget, "NDR");
	//
	// By default: all tabs are disable
	//
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pMainWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pRpcWidget),false);
	pTabWidget->setTabEnabled(pTabWidget->indexOf(pNdrWidget),false);
	//
	this->AddressRepresentation = AddressRepresentation_Absolute;
}