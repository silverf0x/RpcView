#ifndef _PROCESS_INFO_WIDGET_H_
#define _PROCESS_INFO_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCore\RpcCore.h"
#include "View.h"

/*
///GLOBAL///
- Name
- Path
- CmdLine
- Current Directory
- Image 32/64 bits
- Description
- Icon
- Version
- Signed or not
- Parent
- User
...

///RPC///
 - Authentication-Service & principal name (PROVIDER_INFO)
 - number of interfaces
 - number of endpoints
 - max call
 - is listening
 - GetKeyFn and its Arg (see RpcServerRegisterAuthInfo)
 ...
*/


//------------------------------------------------------------------------------
class ProcessInfoWidget_C : public QDockWidget, public View_I
{
    Q_OBJECT

public:
    ProcessInfoWidget_C(QWidget* pParent);
	virtual void AcceptVisitor(ViewVisitor_C* pVisitor);

	bool	AddAuthInfo(RpcAuthInfo_T* pRpcAuthInfo);
	void	UpdateProcessInfo(RpcProcessInfo_T* pRpcProcessInfo);
	void	resizeColumnsToContents();
	void	reset();
	ULONG	GetPid();
private:
	QLabel*					pIconLabel;
	QLineEdit*				pCmdLine;
	QLineEdit*				pPath;
	QLineEdit*				pUser;
	QLineEdit*				pDesktop;
	QLineEdit*				pName;
	QLabel*					pImage;
	QLabel*					pEndpoints;
	QLabel*					pInterfaces;
	QLabel*					pMaxCall;
	//QLabel*					pState;
	QLabel*					pVersion;
	QSortFilterProxyModel*	pProxyModel;
	QStandardItemModel*		pModel;
	
	QTabWidget*				pTabWidget;
	//tabs
	QWidget*				pImageWidget;
	QWidget*				pRpcWidget;
	QTreeView*				pAuthTreeView;

	QLabel*					pInCalls;
	QLabel*					pOutCalls;
	QLabel*					pInPackets;
	QLabel*					pOutPackets;

	ULONG					Pid;
};

#endif// _PROCESS_INFO_WIDGET_H_
