#ifndef _INTERFACE_INFO_WIDGET_H_
#define _INTERFACE_INFO_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCore\RpcCore.h"
#include "..\RpcCommon\RpcCommon.h"
#include "View.h"

//------------------------------------------------------------------------------
class InterfaceInfoWidget_C : public QDockWidget, public View_I
{
    Q_OBJECT

public:
    InterfaceInfoWidget_C(QWidget* pParent);
	virtual void AcceptVisitor(ViewVisitor_C* pVisitor);

	UINT GetPid();	
	void Reset();
	void UpdateInterfaceInfo(RpcInterfaceInfo_T* pRpcInterfaceInfo, WCHAR* pCallbackName);
	void SetAddressRepresentation(AddressRepresentation_T AddressRepresentation);
private:
	UINT					Pid;
	AddressRepresentation_T	AddressRepresentation;
	quintptr				Base;
	quintptr				IfCallback;
	quintptr				TypeFormatString;
	quintptr				ProcFormatString;
	quintptr				ExpressionEvaluation;
	//Global
	QTabWidget*	pTabWidget;
	//Main
	QWidget*	pMainWidget;
	QLineEdit*	pUuid;
	QLabel*		pVersion;
	QLineEdit*	pName;
	QLineEdit*	pLocation;
	QLabel*		pBase;
	QLabel*		pState;
	QLabel*		pStub;
	QLabel*		pProcCount;
	QTextEdit*	pDescription;
	//RPC
	QWidget*	pRpcWidget;
	QLineEdit*	pCallbackName;
	QLineEdit*	pCallbackAddress;
	QLabel*		pEpMapper;
	QLineEdit*	pAnnotation;
	QTextEdit*	pFlags;
	//NDR
	QWidget*	pNdrWidget;
	QLineEdit*	pTransfertSyntax;
	QLabel*		pNdrVersion;
	QLabel*		pMidlVersion;
	QTextEdit*	pNdrFlags;
	QLineEdit*	pTypeFormatString;
	QLineEdit*	pProcFormatString;
	QLineEdit*	pExpressionEvaluation;
};

#endif// _INTERFACE_INFO_WIDGET_H_
