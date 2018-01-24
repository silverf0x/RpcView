#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include "..\Qt\Qt.h"

#include "..\RpcCommon\RpcView.h"
#include "ProcessWidget.h"
#include "ProcessInfoWidget.h"
#include "EndpointsWidget.h"
#include "InterfacesWidget.h"
#include "InterfaceInfoWidget.h"
#include "ProceduresWidget.h"
#include "DecompilationWidget.h"
#include "..\RpcCore\RpcCore.h"
#include "..\RpcDecompiler\RpcDecompiler.h"
#include "ViewVisitor.h" 

#define RPC_VIEW_ORGANIZATION_NAME  "RpcView Team"
#define RPC_VIEW_APPLICATION_NAME   "RpcView"


//------------------------------------------------------------------------------
class MainWindow_C : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow_C(RpcCore_T* pRpcCore);

public slots:
	void ConfigureSymbols();
	void ProcessSelected(quint32 Pid);
	void EndpointSelected(quint32 Pid);
	void InterfaceSelected(quint32 Pid, RPC_IF_ID* pIf);
	void SlotDecompileInterface(quint32, RPC_IF_ID*);
	void SetAbsolute();
	void SetRVA();
	void Exit();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void About();
	void Help();
	void RefreshViews();
	void ViewDetailsForAllProcesses();
	//--
	void SetUpdateSpeedAsFast();
	void SetUpdateSpeedAsNormal();
	void SetUpdateSpeedAsBelowNormal();
	void SetUpdateSpeedAsSlow();
	void SetUpdateSpeedAsVerySlow();
    void SetUpdateSpeedAsManual();
	void ShowColumnsDialog();
	void UpdateColumns();
	//--
	void FilterProcesses();
	void FilterEndpoints();
	void FilterInterfaces();

private:
	void*					pRpcCoreCtxt;
	RpcCore_T*	        	pRpcCore;
	AddressRepresentation_T	AddressRepresentation;

	QStatusBar				StatusBar;
	ProcessWidget_C*		pProcessWidget;
	ProcessInfoWidget_C*	pProcessInfoWidget;
	ProceduresWidget_C*		pProceduresWidget;
	EndpointsWidget_C*		pEndpointsWidget;
	InterfacesWidget_C*		pInterfacesWidget;
	InterfaceInfoWidget_C*	pInterfaceInfoWidget;
	DecompilationWidget_C*	pDecompilationWidget;

	QLabel*					pProcessesCountLabel;
	QLabel*					pInterfacesCountLabel;
	QLabel*					pEndpointsCountLabel;

	QAction*				pActionViewInterfaces;
	QAction*				pActionViewEndpoints;
	QAction*				pActionViewProcedures;
	QAction*				pActionViewInterfaceInfo;
	QAction*				pActionViewProcessInfo;
	QAction*				pActionSpeedFast;		
	QAction*				pActionSpeedNormal;		
	QAction*				pActionSpeedBelowNormal;	
	QAction*				pActionSpeedSlow;
	QAction*				pActionSpeedVerySlow;
    QAction*				pActionSpeedManual;
	QAction*				pAddressAbsolute;
	QAction*				pAddressRva;

	QSettings*				pSettings;
	QTimer*					pRefreshTimer ;
	//--
	uint					RefreshSpeedInMs;
	//--
	// Columns Dialog
	QDialog*			ColumnsDialog;
	QVBoxLayout*		ColumnsVLayout;
	QHBoxLayout*		ColumnsHLayout;
	QVBoxLayout*		ColumnsVLayout1;
	QDialogButtonBox*	ColumnsButtonBox;
	QWidget*			pProcessColumns;
	QWidget*			pEndpointsColumns;
	QWidget*			pInterfacesColumns;
	QWidget*			pProceduresColumns;


	void SendVisitor(ViewVisitor_C& pVisitor);
	void InitMenuRefreshSpeed();
	void InitMenuAddressRepresentation();
	void InitColumnsDialog();
	void SetupMenu();
	void InvokeFindShortcut();
	bool eventFilter(QObject* pObject, QEvent* pEvent);
};

#endif// _PROCESS_LOGGER_WINDOW_H_
