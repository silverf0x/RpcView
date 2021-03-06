#include <conio.h>
#include "InterfacesWidget.h"
#include "..\RpcCommon\RpcCommon.h"

static const char WidgetName[] = "Interfaces";

//8a885d04-1ceb-11c9-9fe8-08002b104860 V2.0
RPC_SYNTAX_IDENTIFIER	DceRpcSyntaxUuid = 
{
	{0x8a885d04,0x1ceb,0x11c9,{0x9f,0xe8,0x08,0x00,0x2b,0x10,0x48,0x60}},
	{2,0}
};

//71710533-beba-4937-8319-b5dbef9ccc36 V1.0
RPC_SYNTAX_IDENTIFIER	Ndr64SyntaxUuid = 
{
	{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},
	{1,0}
};

//------------------------------------------------------------------------------
QString InterfacesWidget_C::GetColumName(Column_T Column)
{
	switch(Column)
	{
		case Column_Uuid				: return (QString("Uuid")); 
		case Column_Pid					: return (QString("Pid"));
		case Column_Version				: return (QString("Ver"));
		case Column_Flags				: return (QString("Flags"));
		case Column_Name				: return (QString("Name"));
		case Column_Base				: return (QString("Base"));
		case Column_Location			: return (QString("Location"));
		case Column_Description			: return (QString("Description"));
		case Column_NumberOfProcedures	: return (QString("Procs"));
		case Column_IfType				: return (QString("Type"));
		case Column_IsRegistered		: return (QString("EpMapper"));
		case Column_Annotation			: return (QString("Annotation"));
		case Column_HasMarshallingInfo	: return (QString("Stub"));
		case Column_IfCallbackFn		: return (QString("Callback"));
		case Column_TransfertSyntax		: return (QString("Syntax"));
		default							: return (QString("Unknown"));
	}
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::InterfaceSelected(const QModelIndex& Index)
{
	QStringList			PidStringList;
	QStringList			VersionStringList;
    QByteArray 			UuidStringARef;
	RPC_IF_ID			RpcIfId;
	UCHAR*				pUuidStringA;
	
	QString	PidString			= pProxyModel->data( pProxyModel->index(Index.row(), Column_Pid) ).toString();
            UuidStringARef      = pProxyModel->data(pProxyModel->index(Index.row(), Column_Uuid)).toString().toLatin1();
			pUuidStringA		= (UCHAR*)UuidStringARef.data();
	QString	VersionString		= pProxyModel->data( pProxyModel->index(Index.row(), Column_Version) ).toString();
			VersionStringList	= VersionString.split(".", Qt::SkipEmptyParts, Qt::CaseSensitive);
	
	if (VersionStringList.isEmpty())
	{
		RpcIfId.VersMajor = 0;
		RpcIfId.VersMinor = 0;
	}
	else
	{
		RpcIfId.VersMajor= VersionStringList.at(0).toUShort();
		RpcIfId.VersMinor= VersionStringList.at(1).toUShort();
	}

	UuidFromStringA( pUuidStringA, &RpcIfId.Uuid);
	
	if (QApplication::mouseButtons() & Qt::RightButton)
	{
		QMenu myMenu;

		QAction* pActionDecompile	= myMenu.addAction("Decompile");
		QAction* selectedItem = myMenu.exec(QCursor::pos());
		if (selectedItem == pActionDecompile)
		{
			emit SigDecompileInterface(PidString.toUInt(), &RpcIfId);
			delete selectedItem;
		}
		else
		{
			delete selectedItem;
			delete pActionDecompile;
		}
	}
	else
	{
		//Select the interface
		emit InterfaceSelected(PidString.toUInt(), &RpcIfId);
	}
	return;
}

 
//------------------------------------------------------------------------------
void InterfacesWidget_C::ApplyProcessFilter(quint32 Pid)
{
	pProxyModel->setFilterRegExp( QString("^%1$").arg(Pid) );
	if (pProxyModel->rowCount() == 0) pProxyModel->setFilterRegExp( QRegExp(".*") );
	
	pFilterWidget->Reset();
}

//------------------------------------------------------------------------------
void InterfacesWidget_C::SnapInterfaces()
{
	PrivateItemList = pModel->findItems(".*", Qt::MatchRegularExpression, Column_Uuid);
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::RemoveUnregisteredInterfaces()
{
	for (auto Iter=PrivateItemList.begin();Iter!=PrivateItemList.end();Iter++)
	{
		if (*Iter!=NULL) pModel->removeRow( (*Iter)->row());
	}
}

//------------------------------------------------------------------------------
bool InterfacesWidget_C::IsInterfacePresent(quint32 Pid, RPC_IF_ID* pIfId)
{
	QList<QStandardItem*>	ItemList;
	UCHAR*					pUuidString = NULL;
	bool					bResult = false;

	if ( UuidToStringA(&pIfId->Uuid,&pUuidString)!=RPC_S_OK ) goto End;

	ItemList = pModel->findItems(QString::fromLatin1((const char*)pUuidString), Qt::MatchFixedString, Column_Uuid);
	if (ItemList.isEmpty()) goto End;
	
	for (auto Iter=ItemList.begin();Iter!=ItemList.end();Iter++)
	{
		if (*Iter!=NULL)
		{
			if (pModel->data(pModel->index((*Iter)->row(), Column_Pid)).toUInt() == Pid)
			{
				PrivateItemList.removeOne(*Iter);

				bResult = true;
				goto End;
			}
		}
	}
End:
	RpcStringFreeA(&pUuidString);
	return (bResult);
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::UpdateUserFilter()
{
	if (pFilterWidget->GetText()!="") ApplyUserFilter( pFilterWidget->GetText() );
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::ApplyUserFilter(const QString & FilterText)
{
	QRegExp	FilterRegExp;

	FilterRegExp.setPattern( FilterText );
	FilterRegExp.setCaseSensitivity( Qt::CaseInsensitive );

	pProxyModel->setFilterKeyColumn(-1);
	pProxyModel->setFilterRegExp( FilterRegExp );
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::ShowUserFilter()
{
	pFilterWidget->show();
	pUserFilter->setFocus();
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::HideUserFilter()
{
	pFilterWidget->hide();
}


//------------------------------------------------------------------------------
ULONG InterfacesWidget_C::GetInterfaces()
{
	return (pProxyModel->rowCount());
}


//------------------------------------------------------------------------------
ULONG InterfacesWidget_C::GetTotalInterfaces()
{
	return (pModel->rowCount());
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::resizeColumnsToContents()
{
	int	Index;

	for( Index=0; Index < Column_Last; Index++)
	{
		pInterfaces->resizeColumnToContents(Index);
	}
}


//------------------------------------------------------------------------------
QString GetInterfaceFlagsString(quint32 Flags)
{
	QString	FlagsString;

	if (Flags & RPC_IF_AUTOLISTEN)						FlagsString.append("RPC_IF_AUTOLISTEN\n");
	if (Flags & RPC_IF_OLE)								FlagsString.append("RPC_IF_OLE\n");
	if (Flags & RPC_IF_ALLOW_UNKNOWN_AUTHORITY)			FlagsString.append("RPC_IF_ALLOW_UNKNOWN_AUTHORITY\n");
	if (Flags & RPC_IF_ALLOW_SECURE_ONLY)				FlagsString.append("RPC_IF_ALLOW_SECURE_ONLY\n");
	if (Flags & RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH)	FlagsString.append("RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH\n");
	if (Flags & RPC_IF_ALLOW_LOCAL_ONLY)				FlagsString.append("RPC_IF_ALLOW_LOCAL_ONLY\n");
	if (Flags & RPC_IF_SEC_NO_CACHE)					FlagsString.append("RPC_IF_SEC_NO_CACHE\n");
	if (Flags & RPC_IF_SEC_CACHE_PER_PROC)				FlagsString.append("RPC_IF_SEC_CACHE_PER_PROC\n");
	if (Flags & RPC_IF_ASYNC_CALLBACK)					FlagsString.append("RPC_IF_ASYNC_CALLBACK\n");
	return (FlagsString);
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::SetRowColor(int Index, const QColor& Color)
{
	int	i;

	for (i=0;i<Column_Last;i++)
	{
		pModel->setData(pModel->index(Index,i), QBrush(Color), Qt::BackgroundRole);
	}
}


//------------------------------------------------------------------------------
bool InterfacesWidget_C::AddInterfaces(RpcInterfaceInfo_T* pRpcInterfaceInfo)
{
	int			Index;
	QString		PidString;
	WCHAR*		pUuidString				= NULL;
	WCHAR*		pTypeW					= NULL;
	WCHAR*		pStubW					= NULL;
	
	if (pRpcInterfaceInfo==NULL) goto End;
	
	//=====
	if (pRpcInterfaceInfo->IfType!=IfType_RPC) goto End;	 //NO DCOM!!!!
	//====

	if ( UuidToStringW(&pRpcInterfaceInfo->If.Uuid,(RPC_WSTR*)&pUuidString)!=RPC_S_OK ) goto End;
	Index = pModel->rowCount();
	pModel->setRowCount(Index+1);
	pModel->setData(pModel->index(Index, Column_Pid), QString("%1").arg(pRpcInterfaceInfo->Pid) );
	pModel->setData(pModel->index(Index, Column_Uuid), QString::fromUtf16((const ushort*)pUuidString));
	
	pModel->setData(pModel->index(Index, Column_Flags), QString("0x%1").arg(pRpcInterfaceInfo->Flags,0,16));
	pModel->setData(pModel->index(Index, Column_Flags), GetInterfaceFlagsString(pRpcInterfaceInfo->Flags), Qt::ToolTipRole );

	pModel->setData(pModel->index(Index, Column_Name), QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Name));
	pModel->setData(pModel->index(Index, Column_Base), QString("0x%1").arg((quintptr)pRpcInterfaceInfo->pLocationBase, 16, 16, QLatin1Char('0')));
	pModel->setData(pModel->index(Index, Column_Base), (quintptr)pRpcInterfaceInfo->pLocationBase, Qt::UserRole);
	pModel->setData(pModel->index(Index, Column_Location), QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Location));
	pModel->setData(pModel->index(Index, Column_Description), QString::fromUtf16((const ushort*)pRpcInterfaceInfo->Description));

	if (pRpcInterfaceInfo->NumberOfProcedures==INVALID_PROC_COUNT) pModel->setData(pModel->index(Index, Column_NumberOfProcedures), 0);
	else pModel->setData(pModel->index(Index, Column_NumberOfProcedures), pRpcInterfaceInfo->NumberOfProcedures);
	
	switch(pRpcInterfaceInfo->IfType)
	{
		case IfType_RPC:
			//Only RPC interfaces have versions
			pTypeW = L"RPC";
			pModel->setData(pModel->index(Index, Column_Version), QString("%1.%2").arg(pRpcInterfaceInfo->If.VersMajor).arg(pRpcInterfaceInfo->If.VersMinor));
		break;
		
		case IfType_OLE:
			pTypeW = L"OLE";
			if (pRpcInterfaceInfo->TypeOfStub==TypeOfStub_TypeLib) SetRowColor(Index,QColor(255, 200, 255, 180));
			else SetRowColor(Index,QColor(255, 255, 200, 180));
		break;
		case IfType_DCOM:
			pTypeW = L"DCOM";
			SetRowColor(Index,QColor(200, 200, 255, 180));
		break;
	}

	pModel->setData(pModel->index(Index, Column_IfType), QString::fromUtf16((const ushort*)pTypeW));
	if (!pRpcInterfaceInfo->bIsRegistered)
	{
		pModel->setData(pModel->index(Index, Column_IsRegistered), "");
	}
	else
	{
		pModel->setData(pModel->index(Index, Column_IsRegistered), QString("Registered"));
	}
	pModel->setData(pModel->index(Index, Column_Annotation), QString::fromLatin1((const char*)pRpcInterfaceInfo->Annotation));
	switch(pRpcInterfaceInfo->TypeOfStub) 
	{
		case TypeOfStub_Interpreted:pStubW = L"Interpreted";	break;
		case TypeOfStub_Inlined:	pStubW = L"Inlined";		break;
		case TypeOfStub_TypeLib:	pStubW = L"TypeLib";		break;
		case TypeOfStub_Hybrid:		pStubW = L"Hybrid";			break;
		default:					pStubW = NULL;		break;
	}
	pModel->setData(pModel->index(Index, Column_HasMarshallingInfo), QString::fromUtf16((const ushort*)pStubW));
	if ((pRpcInterfaceInfo->IfCallbackFn != 0) && (pRpcInterfaceInfo->IfCallbackFn != (RPC_IF_CALLBACK_FN*)INVALID_IF_CALLBACK_ADDRESS))
	{
		pModel->setData(pModel->index(Index, Column_IfCallbackFn), QString("0x%1").arg((quintptr)pRpcInterfaceInfo->IfCallbackFn, 16, 16, QLatin1Char('0')));
		pModel->setData(pModel->index(Index, Column_IfCallbackFn), (quintptr)pRpcInterfaceInfo->IfCallbackFn,Qt::UserRole);
	}
 
	if (pRpcInterfaceInfo->bIsRegistered) SetRowColor(Index,QColor(200, 255, 200, 180));
	if (pRpcInterfaceInfo->LocationState & MEM_FREE) SetRowColor(Index, QColor(200, 200, 200, 180));

	if (!memcmp(&pRpcInterfaceInfo->TransfertSyntax,&DceRpcSyntaxUuid,sizeof(DceRpcSyntaxUuid)))
		pModel->setData(pModel->index(Index, Column_TransfertSyntax), QString::fromLatin1("DCE"));
	else if (!memcmp(&pRpcInterfaceInfo->TransfertSyntax,&Ndr64SyntaxUuid,sizeof(Ndr64SyntaxUuid)))
		pModel->setData(pModel->index(Index, Column_TransfertSyntax), QString::fromUtf16((const ushort*)L"NDR64"));
	else 
		pModel->setData(pModel->index(Index, Column_TransfertSyntax), QString::fromUtf16((const ushort*)L"unknown"));
	
End:
	if (pUuidString!=NULL) RpcStringFreeW((RPC_WSTR*)&pUuidString);

	return (true);
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::AcceptVisitor(ViewVisitor_C* pVisitor)
{
	pVisitor->Visit(this);
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::SaveConfiguration(QSettings* pSettings)
{
	pSettings->setValue("Interfaces/geometry",pInterfaces->header()->saveState());
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::LoadConfiguration(QSettings* pSettings)
{
	pInterfaces->header()->restoreState( pSettings->value("Interfaces/geometry").toByteArray() );
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::CreateColumnsSelectionWidget()
{
	pColumnsSelectionWidget = new QGroupBox(WidgetName,this);
	QVBoxLayout*	pLayout = new QVBoxLayout(pColumnsSelectionWidget);

	for (int i = 0; i < Column_Last; i++)
	{
		ColumnsCheckBoxArray[i] = new QCheckBox(GetColumName((Column_T)i), pColumnsSelectionWidget);
		pLayout->addWidget(ColumnsCheckBoxArray[i]);
	}
	pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	pColumnsSelectionWidget->setLayout(pLayout);
}


//------------------------------------------------------------------------------
QWidget* InterfacesWidget_C::GetColumnsSelectionWidget()
{
	for (int i = 0; i < Column_Last; i++)
	{
		ColumnsCheckBoxArray[i]->setChecked(!pInterfaces->header()->isSectionHidden(i));
	}
	return pColumnsSelectionWidget;
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::UpdateColumnsVisibility()
{
	for (int i = 0; i < Column_Last; i++)
	{
		pInterfaces->header()->setSectionHidden(i, !ColumnsCheckBoxArray[i]->isChecked());
	}
}


//------------------------------------------------------------------------------
void InterfacesWidget_C::SetAddressRepresentation(AddressRepresentation_T AddrRepresentation)
{
	switch (AddrRepresentation)
	{
		case AddressRepresentation_RVA:
			for (int i = 0; i < pModel->rowCount(); i++)
			{
				pModel->setData(pModel->index(i, Column_IfCallbackFn), "");
				quintptr IfCallback = pModel->data(pModel->index(i, Column_IfCallbackFn), Qt::UserRole).toULongLong();
				if ((IfCallback != 0) && (IfCallback != INVALID_IF_CALLBACK_ADDRESS))
				{
					quintptr Base = pModel->data(pModel->index(i, Column_Base), Qt::UserRole).toULongLong();
					pModel->setData(pModel->index(i, Column_IfCallbackFn), QString("+0x%1").arg(IfCallback - Base, 8, 16, QLatin1Char('0')));
				}
			}
		break;
		//--
		case AddressRepresentation_Absolute:
			for (int i = 0; i < pModel->rowCount(); i++)
			{
				pModel->setData(pModel->index(i, Column_IfCallbackFn), "");
				quintptr IfCallback = pModel->data(pModel->index(i, Column_IfCallbackFn), Qt::UserRole).toULongLong();
				if ((IfCallback != 0) && (IfCallback != INVALID_IF_CALLBACK_ADDRESS))
				{
					pModel->setData(pModel->index(i, Column_IfCallbackFn), QString("0x%1").arg(IfCallback, HEX_SIZE_OF_PTR, 16, QLatin1Char('0')));
				}
			}
		break;
		//--
		default:
		break;
	}
}


//------------------------------------------------------------------------------
InterfacesWidget_C::InterfacesWidget_C(QWidget* pParent):QDockWidget(WidgetName)
{
	unsigned int	i;

	QGridLayout*	pGridLayout;
	QGroupBox*		pGroupBox;

	pGroupBox	= new QGroupBox(this);
	pGridLayout = new QGridLayout(pGroupBox);

	setObjectName(WidgetName);

	pProxyModel = new QSortFilterProxyModel(this);
	pProxyModel->setDynamicSortFilter(true);
	pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	
	pModel = new QStandardItemModel(0, Column_Last, this);

	for(i=0;i<Column_Last;i++)
	{
		pModel->setHeaderData(i, Qt::Horizontal, GetColumName( (Column_T)i ) );
	}
 
	pInterfaces = new QTreeView(this);
	pInterfaces->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pInterfaces->setSelectionBehavior(QAbstractItemView::SelectRows);
	pInterfaces->setSelectionMode(QAbstractItemView::SingleSelection);
	pInterfaces->setRootIsDecorated(false);
    pInterfaces->setSortingEnabled(true);
	pInterfaces->setAnimated(true);
	pInterfaces->setModel(pProxyModel);
	pInterfaces->sortByColumn(Column_Uuid, Qt::AscendingOrder);
	pInterfaces->header()->installEventFilter(pParent);
	
	pProxyModel->setSourceModel(pModel);

	connect( pInterfaces, SIGNAL(pressed(const QModelIndex&)), this, SLOT(InterfaceSelected(const QModelIndex&)));
	connect( pInterfaces, SIGNAL(activated(const QModelIndex&)), this, SLOT(InterfaceSelected(const QModelIndex&)));
	connect( this, SIGNAL(InterfaceSelected(quint32, RPC_IF_ID*)), pParent, SLOT(InterfaceSelected(quint32, RPC_IF_ID*)) );
	connect( this, SIGNAL(SigDecompileInterface(quint32, RPC_IF_ID*)), pParent, SLOT(SlotDecompileInterface(quint32, RPC_IF_ID*)) );
	//
	// Add user filtering (CTL+F) support
	//
	pFilterWidget = new FilterWidget_C(this);

	pGridLayout->addWidget(pInterfaces,0,0);
	pGridLayout->addWidget(pFilterWidget,1,0);
	pGroupBox->setLayout(pGridLayout);
	setWidget(pGroupBox);
	//
	// Create the widgets for column selection
	//
	CreateColumnsSelectionWidget();
	AddressRepresentation = AddressRepresentation_Absolute;
}