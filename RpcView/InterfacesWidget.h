#ifndef _INTERFACES_WIDGET_H_
#define _INTERFACES_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCore\RpcCore.h"
#include "FilterWidget.h"
#include "View.h"
#include "..\RpcCommon\RpcCommon.h"


//------------------------------------------------------------------------------
class InterfacesWidget_C : public QDockWidget, public View_I
{
    Q_OBJECT

public:

	typedef enum{
		Column_Pid,
		Column_Uuid,
		Column_Version,
		Column_IfType,
		Column_NumberOfProcedures,
		Column_HasMarshallingInfo,
		Column_IfCallbackFn,
		Column_Name,
		Column_Base,
		Column_Location,
		Column_Flags,
		Column_Description,
		Column_IsRegistered,
		Column_Annotation,
		Column_TransfertSyntax,
		Column_Last	//Must be the last one
	}Column_T; 

    InterfacesWidget_C(QWidget* pParent);
	virtual void	AcceptVisitor(ViewVisitor_C* pVisitor);
	ULONG			GetInterfaces();
	ULONG			GetTotalInterfaces();
	bool			IsInterfacePresent(quint32 Pid, RPC_IF_ID* pIfIp);
	void			RemoveUnregisteredInterfaces();
	void			SnapInterfaces();
	void			SaveConfiguration(QSettings*);
	void			LoadConfiguration(QSettings*);
	void			UpdateUserFilter();
	QWidget*		GetColumnsSelectionWidget();
	void			UpdateColumnsVisibility();
	void			CreateColumnsSelectionWidget();
	
	void			ApplyProcessFilter(quint32 Pid);
	void			resizeColumnsToContents();
	bool			AddInterfaces(RpcInterfaceInfo_T* pRpcInterfaceInfo);
	void			SetAddressRepresentation(AddressRepresentation_T AddressRepresentation);

public slots:
	void	ApplyUserFilter(const QString &);

private slots:
	void	InterfaceSelected(const QModelIndex& Index);
	void	SetRowColor(int Index,const QColor& Color);
	void	ShowUserFilter();
	void	HideUserFilter();

signals:
	void	InterfaceSelected(quint32 Pid, RPC_IF_ID* pIf);
	void	SigDecompileInterface(quint32 Pid, RPC_IF_ID* pIf);

private:
	FilterWidget_C*			pFilterWidget;
	QTreeView*				pInterfaces;
	QSortFilterProxyModel*	pProxyModel;
	QStandardItemModel*		pModel;
	QLineEdit*				pUserFilter;
	QLabel*					pMatchingItems;
	QList<QStandardItem*>	PrivateItemList;
	QGroupBox*				pColumnsSelectionWidget;
	QCheckBox*				ColumnsCheckBoxArray[Column_Last];
	QWidget*				pColorSelectionWidget;
	QSignalMapper*			SignalMapper;
	AddressRepresentation_T	AddressRepresentation;

	QString GetColumName(Column_T Column);
};

#endif// _INTERFACES_WIDGET_H_
