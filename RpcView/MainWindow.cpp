#include "MainWindow.h"
#include "RpcViewResource.h"
#include "../RpcCommon/Misc.h"
#include <Psapi.h>
#include <Shlobj.h>
#include <Dbghelp.h>
#include <strsafe.h>
#include <conio.h>

#include "ConfigurationVisitor.h"
#include "ProcessSelectedVisitor.h"
#include "EndpointSelectedVisitor.h"
#include "InterfaceSelectedVisitor.h"
#include "InitViewsVisitor.h"
#include "RefreshVisitor.h"

#define FAST_REFRESH_SPEED			500
#define NORMAL_REFRESH_SPEED		1000
#define BELOW_NORMAL_REFRESH_SPEED	2000
#define SLOW_REFRESH_SPEED			5000
#define VERY_SLOW_REFRESH_SPEED		10000
#define MANUAL_REFRESH_SPEED		0
#define SHELL_EXECUTE_SUCCESS		((HINSTANCE)42)		// According to the doc, welcome the 16-bit compatibilty

#ifdef __cplusplus
extern "C" {
#endif

	extern    RpcCore_T	gRpcCoreManager;

#ifdef __cplusplus
}
#endif

extern ULONG	NTAPI DecompilerExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers);
extern HMODULE	NTAPI LoadDecompilerEngine(RpcDecompilerHelper_T** ppRpcDecompilerHelper);
extern void		NTAPI InitDecompilerInfo(_In_ RpcInterfaceInfo_T* pRpcInterfaceInfo, _Out_ RpcDecompilerInfo_T* pRpcDecompilerInfo);
extern void		NTAPI UninitDecompilerInfo(RpcDecompilerInfo_T* pRpcDecompilerInfo);

static const char WidgetName[] = "RpcView";

//------------------------------------------------------------------------------
void MainWindow_C::InterfaceSelected(quint32 Pid, RPC_IF_ID* pIf)
{
	CHAR SymbolPath;
	
	if (GetEnvironmentVariableA("RpcViewSymbolPath",&SymbolPath,sizeof(SymbolPath))==0)
	{
		StatusBar.showMessage("Symbol path not configured.");
	}
	
	InterfaceSelectedVisitor_C	InterfaceSelectedVisitor(
										Pid,
										pIf,
										pRpcCore,
										pRpcCoreCtxt
										);
	
	this->SendVisitor(InterfaceSelectedVisitor);
}


//------------------------------------------------------------------------------
void MainWindow_C::EndpointSelected(quint32 Pid)
{
	EndpointSelectedVisitor_C	EndpointSelectedVisitor(
										Pid,
										pRpcCore,
										pRpcCoreCtxt
										);
	this->SendVisitor(EndpointSelectedVisitor);
}


//------------------------------------------------------------------------------
void MainWindow_C::ProcessSelected(quint32 Pid)
{
	ProcessSelectedVisitor_C	ProcessSelectedVisitor(
										Pid,
										pRpcCore,
										pRpcCoreCtxt
										);
	this->SendVisitor(ProcessSelectedVisitor);
}


//------------------------------------------------------------------------------
void MainWindow_C::InitColumnsDialog()
{
	ColumnsDialog		= new QDialog(this);
	ColumnsVLayout		= new QVBoxLayout(ColumnsDialog);
	ColumnsHLayout		= new QHBoxLayout();
	ColumnsVLayout1		= new QVBoxLayout();
	ColumnsButtonBox	= new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	pProcessColumns		= pProcessWidget->GetColumnsSelectionWidget();
	pEndpointsColumns	= pEndpointsWidget->GetColumnsSelectionWidget();
	pInterfacesColumns	= pInterfacesWidget->GetColumnsSelectionWidget();
	pProceduresColumns	= pProceduresWidget->GetColumnsSelectionWidget();

	ColumnsHLayout->addWidget(pProcessColumns);
	ColumnsVLayout1->addWidget(pEndpointsColumns);
	ColumnsVLayout1->addWidget(pProceduresColumns);
	ColumnsHLayout->addLayout(ColumnsVLayout1);
	ColumnsHLayout->addWidget(pInterfacesColumns);

	ColumnsVLayout->addLayout(ColumnsHLayout);
	ColumnsVLayout->addWidget(ColumnsButtonBox);

	connect(ColumnsButtonBox, SIGNAL(rejected()), ColumnsDialog, SLOT(reject()));
	connect(ColumnsButtonBox, SIGNAL(accepted()), ColumnsDialog, SLOT(accept()));
	connect(ColumnsDialog, SIGNAL(accepted()), this, SLOT(UpdateColumns()));

	ColumnsDialog->setWindowTitle("Select Columns");
	ColumnsDialog->setLayout(ColumnsVLayout);
	ColumnsDialog->setMaximumSize(0, 0);
}


//------------------------------------------------------------------------------
void MainWindow_C::ShowColumnsDialog()
{
	pProcessColumns		= pProcessWidget->GetColumnsSelectionWidget();
	pEndpointsColumns	= pEndpointsWidget->GetColumnsSelectionWidget();
	pInterfacesColumns	= pInterfacesWidget->GetColumnsSelectionWidget();
	pProceduresColumns	= pProceduresWidget->GetColumnsSelectionWidget();

	ColumnsDialog->exec();
}


//------------------------------------------------------------------------------
void MainWindow_C::UpdateColumns()
{
	ConfigurationVisitor_C	ConfigurationVisitor(ConfigurationVisitor_C::UpdateColumns, NULL);
	SendVisitor(ConfigurationVisitor);
}


//------------------------------------------------------------------------------
bool MainWindow_C::eventFilter(QObject* pObject, QEvent* pEvent)
{
	if (pEvent->type() == QEvent::ContextMenu)
	{
		QPoint position = QCursor::pos();
		//
		// Show the context menu associatd to columns
		//
		QMenu ColumnsMenu;

		QAction* pActionSelectColumns = ColumnsMenu.addAction("Select Columns...");
		QAction* pActionFitAll = ColumnsMenu.addAction("Size All Columns to fit");
		//
		QAction* selectedItem = ColumnsMenu.exec(position);
		if (selectedItem == pActionFitAll)
		{
			delete pActionSelectColumns;
			static_cast<QHeaderView*>(pObject)->resizeSections(QHeaderView::ResizeToContents);
			
		}
		else if (selectedItem == pActionSelectColumns)
		{
			delete pActionFitAll;
			ShowColumnsDialog();
		}
		if (selectedItem != NULL) delete selectedItem;
	}
	return QWidget::eventFilter(pObject, pEvent);
}
 

//------------------------------------------------------------------------------
VOID* __fastcall RpcAlloc(SIZE_T Size)
{
	return OS_ALLOC(Size);
}


//------------------------------------------------------------------------------
VOID __fastcall RpcFree(VOID* pMem)
{
	OS_FREE(pMem);
}


//------------------------------------------------------------------------------
BOOL __fastcall	RpcGetProcessData(RpcModuleInfo_T* pRpcModuleInfo, RVA_T Rva, VOID* pBuffer, UINT BufferLength)
{
	BOOL	bResult		= FALSE;
	HANDLE	hProcess	= NULL;
	VOID*	pAddress	= NULL; 
	
	if (pRpcModuleInfo == NULL) goto End;
	pAddress = (VOID*)(pRpcModuleInfo->pModuleBase + Rva);

	hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pRpcModuleInfo->Pid);
	if (hProcess == NULL) goto End;
	bResult = ReadProcessMemory(hProcess, pAddress, pBuffer, BufferLength, NULL);
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return (bResult);
}


//------------------------------------------------------------------------------
VOID __cdecl RpcPrint(void* pContext, const char* pTxt)
{
	DecompilationWidget_C* pDecompilationWidget = (DecompilationWidget_C*)pContext;
	pDecompilationWidget->InsertText(pTxt);
#ifdef _DEBUG
	printf("%s\n", pTxt);
#endif
}


//------------------------------------------------------------------------------
VOID __cdecl RpcDebug(const char* pFunction, ULONG Line, const char* pFormatString, ...)
{
	va_list	Arg;
    UNREFERENCED_PARAMETER(pFunction);
    UNREFERENCED_PARAMETER(Line);
    va_start(Arg, pFormatString);
	_vcprintf(pFormatString, Arg);
}


//------------------------------------------------------------------------------
BOOL __fastcall	RpcGetInterfaceName(GUID* pUuid, UCHAR* pName, ULONG NameLength)
{
	HKEY		hKey = NULL;
	ULONG		DataLength;
	UCHAR		SubKeyName[MAX_PATH];
	RPC_CSTR	pUuidString = NULL;
	BOOL		bResult = FALSE;

	if (UuidToStringA(pUuid, &pUuidString) != RPC_S_OK) goto End;
	StringCbPrintfA((STRSAFE_LPSTR)SubKeyName, sizeof(SubKeyName), "Interface\\{%s}", pUuidString);

	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, (LPCSTR)SubKeyName, 0, KEY_READ, &hKey) != ERROR_SUCCESS) goto End;
	DataLength = NameLength;
	if (RegQueryValueExA(hKey, NULL, NULL, NULL, pName, &DataLength) != ERROR_SUCCESS) goto End;

	bResult = TRUE;
End:
	if (hKey != NULL) RegCloseKey(hKey);
	if (pUuidString != NULL) RpcStringFreeA(&pUuidString);
	return (bResult);
}



//------------------------------------------------------------------------------
void MainWindow_C::SlotDecompileInterface(quint32 Pid, RPC_IF_ID* pIf)
{
	RpcDecompilerInfo_T		RpcDecompilerInfo;
	void*					pDecompilerCtxt			= NULL;
	RpcInterfaceInfo_T*		pRpcInterfaceInfo		= NULL;
	RpcDecompilerHelper_T*	pRpcDecompilerHelper	= NULL;
	HMODULE					hDecompiler				= NULL;

	ZeroMemory(&RpcDecompilerInfo, sizeof(RpcDecompilerInfo));
	//
	// Get the decompiler
	//
	hDecompiler = LoadDecompilerEngine(&pRpcDecompilerHelper);
	if (hDecompiler==NULL) goto End;
	//
	// Get the Interface info
	//
	pRpcInterfaceInfo = pRpcCore->RpcCoreGetInterfaceInfoFn( pRpcCoreCtxt, Pid, pIf, RPC_INTERFACE_INFO_ALL );
    if (pRpcInterfaceInfo == NULL) goto End;
	InitDecompilerInfo(pRpcInterfaceInfo, &RpcDecompilerInfo);
	__try{
		RpcViewHelper_T	LocalRpcViewHelper = {
			this->pDecompilationWidget,
			&RpcAlloc,
			&RpcFree,
			&RpcGetProcessData,
			&RpcPrint,
			&RpcDebug,
			&RpcGetInterfaceName
		};
		pDecompilerCtxt = pRpcDecompilerHelper->RpcDecompilerInitFn(&LocalRpcViewHelper, &RpcDecompilerInfo);
		if (pDecompilerCtxt!=NULL)
		{
			pRpcDecompilerHelper->RpcDecompilerPrintAllProceduresFn( pDecompilerCtxt );
			pRpcDecompilerHelper->RpcDecompilerUninitFn(pDecompilerCtxt);
		}
	}__except( DecompilerExceptionFilter(GetExceptionInformation()) )
	{
        //Failure
        goto End;
	}
End:
	UninitDecompilerInfo(&RpcDecompilerInfo);
	if (hDecompiler!=NULL) FreeLibrary(hDecompiler);
	if (pRpcInterfaceInfo!=NULL)
	{
		pRpcCore->RpcCoreFreeInterfaceInfoFn(pRpcCoreCtxt,pRpcInterfaceInfo);
	}
	return;
}


//------------------------------------------------------------------------------
void MainWindow_C::ViewDetailsForAllProcesses()
{
	HINSTANCE	hInstance;
	UCHAR		FilePath[MAX_PATH];
	
	GetModuleFileNameA(NULL,(LPSTR)FilePath,_countof(FilePath));
	if (gRpcCoreManager.bForceLoading)
		hInstance = ShellExecuteA(NULL, "runas", (LPCSTR)FilePath, "/f", 0, SW_SHOWNORMAL);
	else
		hInstance = ShellExecuteA(NULL, "runas", (LPCSTR)FilePath, 0, 0, SW_SHOWNORMAL);
	if ( hInstance == SHELL_EXECUTE_SUCCESS)
	{
		Exit();
	}
}

//------------------------------------------------------------------------------
void MainWindow_C::About()
{
	Version_T				Version;
	QString					AboutText;
	HMODULE					hDecompiler;
	RpcDecompilerHelper_T*	pRpcDecompilerHelper	= NULL;
	LocationInfo_T			LocationInfo;

	AboutText.append("RpcView");

	if (!GetLocationInfo(GetCurrentProcess(), GetModuleHandle(NULL), &LocationInfo)) goto End;
	Version.As64BitsValue = GetModuleVersion(LocationInfo.Location);

	AboutText.append( QString(" v%4.%3.%2.%1\n\n").arg(Version.As16BitsValues.Part1).arg(Version.As16BitsValues.Part2).arg(Version.As16BitsValues.Part3).arg(Version.As16BitsValues.Part4) );
    /*
	Version.As64BitsValue = pRpcCoreNativeHelper->RuntimeVersion;
	AboutText.append(" - ");
	AboutText.append(pRpcCoreNativeHelper->pDescription);
	AboutText.append( QString(" for v%4.%3.%2.%1\n").arg(Version.As16BitsValues.Part1).arg(Version.As16BitsValues.Part2).arg(Version.As16BitsValues.Part3).arg(Version.As16BitsValues.Part4) );
	Version.As64BitsValue = pRpcCoreNativeHelper->RuntimeVersion;
#ifdef _WIN64
	Version.As64BitsValue = pRpcCoreWow64Helper->RuntimeVersion;
	AboutText.append(" - ");
	AboutText.append(pRpcCoreWow64Helper->pDescription);
	AboutText.append( QString(" for v%4.%3.%2.%1\n").arg(Version.As16BitsValues.Part1).arg(Version.As16BitsValues.Part2).arg(Version.As16BitsValues.Part3).arg(Version.As16BitsValues.Part4) );
	Version.As64BitsValue = pRpcCoreWow64Helper->RuntimeVersion;
#endif
    */
	hDecompiler = LoadDecompilerEngine(&pRpcDecompilerHelper);
	if (hDecompiler==NULL) goto End;

	if (!GetLocationInfo(GetCurrentProcess(), hDecompiler, &LocationInfo)) goto End;
	Version.As64BitsValue = GetModuleVersion(LocationInfo.Location);
	FreeLibrary(hDecompiler);

	AboutText.append(" - Decompiler engine");
	AboutText.append( QString(" v%4.%3.%2.%1\n").arg(Version.As16BitsValues.Part1).arg(Version.As16BitsValues.Part2).arg(Version.As16BitsValues.Part3).arg(Version.As16BitsValues.Part4) );
		
End:
	QMessageBox::about(
					this,
					"About RpcView",
					AboutText
					);
}


//------------------------------------------------------------------------------
void MainWindow_C::Help()
{
	QMessageBox::information(
					this,
					"RpcView Help",
					"RpcView presents a complete view of RPC interfaces on a system.\n"
					"See http://rpcview.org"
					);
}


//------------------------------------------------------------------------------
void MainWindow_C::SendVisitor(ViewVisitor_C& Visitor)
{
	//
	// todo: a vector of elements (maybe a view factory???) 
	//
	pProcessWidget->AcceptVisitor(&Visitor);
	pEndpointsWidget->AcceptVisitor(&Visitor);
	pInterfacesWidget->AcceptVisitor(&Visitor);
	pProceduresWidget->AcceptVisitor(&Visitor);

	pInterfaceInfoWidget->AcceptVisitor(&Visitor);
	pProcessInfoWidget->AcceptVisitor(&Visitor);
}


//------------------------------------------------------------------------------
 void MainWindow_C::closeEvent(QCloseEvent *event)
 {
    UNREFERENCED_PARAMETER(event);
    Exit();
 }


 //------------------------------------------------------------------------------
void MainWindow_C::Exit()
{
	pSettings->setValue("MainWindow/windowState", saveState());
	pSettings->setValue("MainWindow/geometry", saveGeometry());
	pSettings->setValue("RefreshSpeed", this->RefreshSpeedInMs);
	pSettings->setValue("AddressRepresentation", this->AddressRepresentation);
	
	ConfigurationVisitor_C	ConfigurationVisitor(ConfigurationVisitor_C::Save, pSettings);
	SendVisitor(ConfigurationVisitor);
	//
	// Uninit the RpcCore
	//
	pRpcCore->RpcCoreUninitFn(pRpcCoreCtxt);
	delete this;
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	ExitProcess(EXIT_SUCCESS);
}


//------------------------------------------------------------------------------
void MainWindow_C::RefreshViews()
{
	RefreshVisitor_C	RefreshVisitor(
							pRpcCore,
							pRpcCoreCtxt
							);

	SendVisitor(RefreshVisitor);

	pEndpointsCountLabel->setText(	QString("Endpoints: %1/%2").arg(RefreshVisitor.GetEndpoints()).arg(RefreshVisitor.GetTotalEndpoints()) );
	pProcessesCountLabel->setText(	QString("Processes: %1/%2").arg(RefreshVisitor.GetProcesses()).arg(RefreshVisitor.GetTotalProcesses()) );
	pInterfacesCountLabel->setText(	QString("Interfaces: %1/%2").arg(RefreshVisitor.GetInterfaces()).arg(RefreshVisitor.GetTotalInterfaces()) );
}


//------------------------------------------------------------------------------
void MainWindow_C::ConfigureSymbols()
{
	bool			bOk;
	QString			Txt;
	QString			CurrentSymbolsPath;
	QString			NewSymbolsPath;

	CurrentSymbolsPath = pSettings->value("SymbolsPath").toString();

	NewSymbolsPath = QInputDialog::getText(
							this,
							"Configure Symbols",
                            "RpcView uses symbols to resolve function names when displaying RPC procedures names.\n\nIf you do not require that information you do not need to configure symbols",
							QLineEdit::Normal,
							CurrentSymbolsPath,
							&bOk
							);
	 if ( bOk )
	 {
		 pSettings->setValue("SymbolsPath",NewSymbolsPath);
		 SetEnvironmentVariableA("RpcViewSymbolPath",NewSymbolsPath.toLatin1());
	 }
}


//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsFast()
{
	this->RefreshSpeedInMs = FAST_REFRESH_SPEED;
    pRefreshTimer->start();
	pRefreshTimer->setInterval(this->RefreshSpeedInMs);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsNormal()
{
	this->RefreshSpeedInMs = NORMAL_REFRESH_SPEED;
    pRefreshTimer->start();
	pRefreshTimer->setInterval(this->RefreshSpeedInMs);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsBelowNormal()
{
	this->RefreshSpeedInMs = BELOW_NORMAL_REFRESH_SPEED;
    pRefreshTimer->start();
	pRefreshTimer->setInterval(this->RefreshSpeedInMs);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsSlow()
{
	this->RefreshSpeedInMs = SLOW_REFRESH_SPEED;
    pRefreshTimer->start();
	pRefreshTimer->setInterval(this->RefreshSpeedInMs);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsVerySlow()
{
	this->RefreshSpeedInMs = VERY_SLOW_REFRESH_SPEED;
    pRefreshTimer->start();
	pRefreshTimer->setInterval(this->RefreshSpeedInMs);
}

//------------------------------------------------------------------------------
void MainWindow_C::SetUpdateSpeedAsManual()
{
    this->RefreshSpeedInMs = MANUAL_REFRESH_SPEED;
    pRefreshTimer->stop();
}


//------------------------------------------------------------------------------
void MainWindow_C::InvokeFindShortcut()
{
	QKeyEvent FindEvent(QKeyEvent::KeyPress, Qt::Key_F, Qt::ControlModifier);
	QApplication::sendEvent(this, &FindEvent);
}


//------------------------------------------------------------------------------
void MainWindow_C::FilterProcesses()
{
	pProcessWidget->setFocus(Qt::ActiveWindowFocusReason);
	InvokeFindShortcut();
}


//------------------------------------------------------------------------------
void MainWindow_C::FilterEndpoints()
{
	pEndpointsWidget->setFocus(Qt::ActiveWindowFocusReason);
	InvokeFindShortcut();
}


//------------------------------------------------------------------------------
void MainWindow_C::FilterInterfaces()
{
	pInterfacesWidget->setFocus(Qt::ActiveWindowFocusReason);
	InvokeFindShortcut();
}


//------------------------------------------------------------------------------
void MainWindow_C::SetAbsolute()
{
	this->AddressRepresentation = AddressRepresentation_Absolute;

	ConfigurationVisitor_C	ConfigurationVisitor(ConfigurationVisitor_C::AddressAbsolute, NULL);
	SendVisitor(ConfigurationVisitor);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetRVA()
{
	this->AddressRepresentation = AddressRepresentation_RVA;

	ConfigurationVisitor_C	ConfigurationVisitor(ConfigurationVisitor_C::AddressRVA, NULL);
	SendVisitor(ConfigurationVisitor);
}


//------------------------------------------------------------------------------
void MainWindow_C::SetupMenu()
{
	HICON		hUacIcon;
	QMenuBar*	pMenuBar					=	new QMenuBar(this);
	//
	// File
	//
	QMenu*		pMenuFile					=	pMenuBar->addMenu("&File");
	QAction*	pActionAllProcessesDetails	=	pMenuFile->addAction("Show &Details for All Processes",this,SLOT(ViewDetailsForAllProcesses()));
	pMenuFile->addAction("E&xit",this,SLOT(Exit()));
	//
	// Option
	//
	QMenu*		pMenuOptions				=	pMenuBar->addMenu("&Options");
	pMenuOptions->addAction("Configure Sym&bols",this,SLOT(ConfigureSymbols()));
	QMenu*		pSubMenupdateSpeed			=	pMenuOptions->addMenu("&Refresh Speed");
	QAction*	pActionRefresh				=	pMenuOptions->addAction("Refresh &Now", this, SLOT(RefreshViews()));
	pMenuOptions->addAction("Select Col&umns", this, SLOT(ShowColumnsDialog()));
	QMenu*		pSubMenuAddress				=	pMenuOptions->addMenu("&Address");
	pActionRefresh->setShortcut(Qt::Key_F5);

					pAddressAbsolute	= pSubMenuAddress->addAction("Absolute", this, SLOT(SetAbsolute()));
					pAddressRva			= pSubMenuAddress->addAction("RVA", this, SLOT(SetRVA()));
	QActionGroup*	pAddressGroup	= new QActionGroup(this);

	pAddressGroup->addAction(pAddressAbsolute);
	pAddressGroup->addAction(pAddressRva);

	pAddressAbsolute->setCheckable(true);
	pAddressRva->setCheckable(true);

	pActionSpeedFast		= pSubMenupdateSpeed->addAction(".5 seconds", this, SLOT(SetUpdateSpeedAsFast()));
	pActionSpeedNormal		= pSubMenupdateSpeed->addAction("1 second", this, SLOT(SetUpdateSpeedAsNormal()));
	pActionSpeedBelowNormal = pSubMenupdateSpeed->addAction("2 seconds", this, SLOT(SetUpdateSpeedAsBelowNormal()));
	pActionSpeedSlow		= pSubMenupdateSpeed->addAction("5 seconds", this, SLOT(SetUpdateSpeedAsSlow()));
	pActionSpeedVerySlow	= pSubMenupdateSpeed->addAction("10 seconds", this, SLOT(SetUpdateSpeedAsVerySlow()));
    pActionSpeedManual      = pSubMenupdateSpeed->addAction("manual", this, SLOT(SetUpdateSpeedAsManual()));

	pActionSpeedFast->setCheckable(true);
	pActionSpeedNormal->setCheckable(true);
	pActionSpeedBelowNormal->setCheckable(true);
	pActionSpeedSlow->setCheckable(true);
	pActionSpeedVerySlow->setCheckable(true);
    pActionSpeedManual->setCheckable(true);

	QActionGroup*	pSpeedActionGroup = new QActionGroup(this);

	pSpeedActionGroup->addAction(pActionSpeedFast);
	pSpeedActionGroup->addAction(pActionSpeedNormal);
	pSpeedActionGroup->addAction(pActionSpeedBelowNormal);
	pSpeedActionGroup->addAction(pActionSpeedSlow);
	pSpeedActionGroup->addAction(pActionSpeedVerySlow);
    pSpeedActionGroup->addAction(pActionSpeedManual);
	//
	// View
	//
	QMenu*		pMenuView =	pMenuBar->addMenu("&View");
				pMenuView->addAction(pInterfacesWidget->toggleViewAction());
				pMenuView->addAction(pEndpointsWidget->toggleViewAction());	
				pMenuView->addAction(pProceduresWidget->toggleViewAction());
				pMenuView->addAction(pProcessInfoWidget->toggleViewAction());
				pMenuView->addAction(pInterfaceInfoWidget->toggleViewAction());
				pMenuView->addAction(pDecompilationWidget->toggleViewAction());

				pInterfacesWidget->toggleViewAction()->setText("&Interfaces");
				pEndpointsWidget->toggleViewAction()->setText("&Endpoints");
				pProceduresWidget->toggleViewAction()->setText("&Procedures");
				pProcessInfoWidget->toggleViewAction()->setText("Proce&ss Properties");
				pInterfaceInfoWidget->toggleViewAction()->setText("Inter&faces Properties");
				pDecompilationWidget->toggleViewAction()->setText("&Decompilation");
	//
	// Filter
	//
	QMenu*		pMenuFilter = pMenuBar->addMenu("Fil&ter");
	pMenuFilter->addAction("&Process", this, SLOT(FilterProcesses()));
	pMenuFilter->addAction("&Endpoints", this, SLOT(FilterEndpoints()));
	pMenuFilter->addAction("&Interfaces", this, SLOT(FilterInterfaces()));
	//
	// Help
	//
	QMenu*		pMenuHelp = pMenuBar->addMenu("&Help");
	QAction*	pActionHelp = pMenuHelp->addAction("&Help...", this, SLOT(Help()));
				pActionHelp->setShortcut(Qt::Key_F1);
	//--
	pMenuHelp->addSeparator();
	//--
	pMenuHelp->addAction("&About", this, SLOT(About()));
	pMenuHelp->addAction("About &Qt", qApp, SLOT(aboutQt()));

	if (IsUserAnAdmin()) pActionAllProcessesDetails->setEnabled(false);

	hUacIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_UAC_ICON));
	if (hUacIcon!=NULL)
	{
		pActionAllProcessesDetails->setIcon(QtWin::fromHICON(hUacIcon));
		DestroyIcon(hUacIcon);
	}
	setMenuBar(pMenuBar);
}


//------------------------------------------------------------------------------
void MainWindow_C::InitMenuRefreshSpeed()
{
	switch(RefreshSpeedInMs)
	{
		case FAST_REFRESH_SPEED			:	pActionSpeedFast->setChecked(true);			break;
		case NORMAL_REFRESH_SPEED		:	pActionSpeedNormal->setChecked(true);		break;
		case BELOW_NORMAL_REFRESH_SPEED	:	pActionSpeedBelowNormal->setChecked(true);	break;
		case SLOW_REFRESH_SPEED			:	pActionSpeedSlow->setChecked(true);			break;
		case VERY_SLOW_REFRESH_SPEED	:	pActionSpeedVerySlow->setChecked(true);		break;
        case MANUAL_REFRESH_SPEED       :	pActionSpeedManual->setChecked(true);		break;
		//--
		default:
		break;
	}
}


//------------------------------------------------------------------------------
void MainWindow_C::InitMenuAddressRepresentation()
{
	switch (AddressRepresentation)
	{
		case AddressRepresentation_Absolute	:	pAddressAbsolute->setChecked(true);		break;
		case AddressRepresentation_RVA		:	pAddressRva->setChecked(true);			break;
		//--
		default:
		break;
	}
}


//------------------------------------------------------------------------------
MainWindow_C::MainWindow_C(RpcCore_T* pRpcCore)
{
	setObjectName(WidgetName);	//required to save the window geometry/state
	setWindowTitle(WidgetName);
	//
	// Inits Helpers: RpcCore and RpcDecompiler
	//
	this->pRpcCore = pRpcCore;
	
	setDockOptions(
		QMainWindow::AllowNestedDocks|
		QMainWindow::AllowTabbedDocks|
		QMainWindow::AnimatedDocks
	);
	//
	// Creates all the views
	//
	pInterfacesWidget	= new InterfacesWidget_C(this);
	pProceduresWidget	= new ProceduresWidget_C(this);
	pInterfaceInfoWidget= new InterfaceInfoWidget_C(this);
	pProcessWidget		= new ProcessWidget_C(this);
	pProcessInfoWidget	= new ProcessInfoWidget_C(this);
	pEndpointsWidget	= new EndpointsWidget_C(this);
	pDecompilationWidget= new DecompilationWidget_C(this);
	//
	// Add the Process view as the main one
	//
	setCentralWidget(pProcessWidget);
	//
	// All other views are optional DockWidgets:
	//
	addDockWidget(Qt::RightDockWidgetArea, pProcessInfoWidget);
	addDockWidget(Qt::RightDockWidgetArea, pInterfaceInfoWidget);
	addDockWidget(Qt::BottomDockWidgetArea, pInterfacesWidget);
	addDockWidget(Qt::LeftDockWidgetArea, pEndpointsWidget);
	addDockWidget(Qt::BottomDockWidgetArea, pProceduresWidget);
	addDockWidget(Qt::LeftDockWidgetArea, pDecompilationWidget);
	//
	// Create the menu
	//
	SetupMenu();

	pProcessesCountLabel	= new QLabel("Processes: -",this);
	pInterfacesCountLabel	= new QLabel("Interfaces: -",this);
	pEndpointsCountLabel	= new QLabel("Enpoints: -",this);

	StatusBar.showMessage("Ready.");

	StatusBar.addPermanentWidget(pEndpointsCountLabel);
	StatusBar.addPermanentWidget(pInterfacesCountLabel);
	StatusBar.addPermanentWidget(pProcessesCountLabel);

	setStatusBar(&StatusBar);

#ifndef _DEBUG
    HANDLE hIcon = LoadImageA(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MAIN_ICON), IMAGE_ICON, 0, 0, 0);
	QSplashScreen SplashScreen(QtWin::fromHICON((HICON)hIcon),Qt::WindowStaysOnTopHint);
	SplashScreen.showMessage(QString("RpcView"), Qt::AlignCenter, QColor(Qt::lightGray));
    QFont font("Helvetica", 20, QFont::Bold);
	SplashScreen.setFont(font);
	SplashScreen.show();
#endif
	//
	// Init all the views
	//
	InitViewsVisitor_C InitViewsVisitor(
							pRpcCore,
							&pRpcCoreCtxt
							);
	SendVisitor(InitViewsVisitor);
	//
	// Restore saved settings 
	//
    pSettings = new QSettings(RPC_VIEW_ORGANIZATION_NAME, RPC_VIEW_APPLICATION_NAME,this);

    QVariant    RefreshSpeedValue;

    RefreshSpeedValue = pSettings->value("RefreshSpeed");
    if (RefreshSpeedValue.isNull())
    {
        this->RefreshSpeedInMs = NORMAL_REFRESH_SPEED;
    }
    else
    {
        this->RefreshSpeedInMs = RefreshSpeedValue.toUInt();
    }
    InitMenuRefreshSpeed();
	
	QVariant    AddressRepresentationValue;

	AddressRepresentationValue = pSettings->value("AddressRepresentation");
	if (AddressRepresentationValue.isNull())
	{
		this->AddressRepresentation = AddressRepresentation_Absolute;
	}
	else
	{
		this->AddressRepresentation = (AddressRepresentation_T)AddressRepresentationValue.toUInt();
	}
	InitMenuAddressRepresentation();
	show();

	restoreGeometry( pSettings->value("MainWindow/geometry").toByteArray() );
	restoreState( pSettings->value("MainWindow/windowState").toByteArray() );
	ConfigurationVisitor_C	ConfigurationVisitorLoad(ConfigurationVisitor_C::Load,pSettings);
	SendVisitor(ConfigurationVisitorLoad);
	if (AddressRepresentation == AddressRepresentation_RVA)
	{ 
		ConfigurationVisitor_C	ConfigurationVisitorAddr(ConfigurationVisitor_C::AddressRVA, pSettings);
		SendVisitor(ConfigurationVisitorAddr);
	}
	
	SetEnvironmentVariableA( "RpcViewSymbolPath",pSettings->value("SymbolsPath").toByteArray() );
	
	pInterfacesCountLabel->setText( QString("Interfaces: %1").arg(InitViewsVisitor.GetInterfaces()) );
	pEndpointsCountLabel->setText(  QString("Endpoints: %1").arg(InitViewsVisitor.GetEndpoints()) );
	//
	// Start auto refresh
	//
	pRefreshTimer = new QTimer(this);
    connect(pRefreshTimer, SIGNAL(timeout()), this, SLOT(RefreshViews()));
	if (this->RefreshSpeedInMs) pRefreshTimer->start(this->RefreshSpeedInMs);
	
	InitColumnsDialog();
}