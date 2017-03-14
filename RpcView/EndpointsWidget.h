#ifndef _ENDPOINTS_WIDGET_H_
#define _ENDPOINTS_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCore\RpcCore.h"
#include "FilterWidget.h"
#include "View.h"

//-----------------------------------------------------------------------------
class EndpointsWidget_C : public QDockWidget, public View_I
{
    Q_OBJECT

public:
    EndpointsWidget_C(QWidget* pParent);

	virtual void	AcceptVisitor(ViewVisitor_C* pVisitor);
	void			ApplyProcessFilter(quint32 Pid);
	ULONG			GetEndpoints();
	ULONG			GetTotalEndpoints();
	bool			IsEndpointsPresent(quint32 Pid, WCHAR* pName,WCHAR* pProtocol);
	void			RemoveUnregisteredEndpoints();
	void			SnapEndpoint();
	void			resizeColumnsToContents();
	bool			AddEndpoint(quint32 Pid, RpcEndpointInfo_T* pRpcEndpointInfo);
	void			UpdateUserFilter();
	void			SaveConfiguration(QSettings*);
	void			LoadConfiguration(QSettings*);
	QWidget*		GetColumnsSelectionWidget();
	void			UpdateColumnsVisibility();
	void			CreateColumnsSelectionWidget();

public slots:
	void	ApplyUserFilter(const QString &);

private slots:
	void	EndpointSelected(const QModelIndex&);

signals:
	void	EndpointSelected(quint32 Pid);

private:
	
	typedef enum{
		Column_Pid,
		Column_Protocol,
		Column_Name,
		Column_Last
	}Column_T;

	FilterWidget_C*			pFilterWidget;
	QTreeView*				pEndpoints;
	QSortFilterProxyModel*	pProxyModel;
	QStandardItemModel*		pModel;
	QList<QStandardItem*>	PrivateItemList;
	//--
	QGroupBox*				pColumnsSelectionWidget;
	QCheckBox*				CheckBoxArray[Column_Last];

	QString GetEndpointsWidgetColumnName(Column_T Column);
};

#endif// _ENDPOINTS_WIDGET_H_
