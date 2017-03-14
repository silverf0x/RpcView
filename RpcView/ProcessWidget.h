#ifndef _PROCESS_WIDGET_H_
#define _PROCESS_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCommon\RpcView.h"
#include "..\RpcCore\RpcCore.h"
#include "View.h"
#include "FilterWidget.h"


//------------------------------------------------------------------------------
class ProcessWidget_C : public QGroupBox, public View_I
{
    Q_OBJECT

public:

	typedef enum {
		Column_Name,
		Column_Pid,
		Column_Path,
		Column_Description,
#ifdef _WIN64
		Column_IsWow64,
#endif
		Column_UserName,
		Column_CmdLine,
		Column_Desktop,
		Column_IsRpcServer,
		//Column_IsListening,
		Column_MaxCalls,
		Column_Last,
	}Column_T;

    ProcessWidget_C(QWidget* pParent);
	virtual void	AcceptVisitor(ViewVisitor_C* pVisitor);
	void			resizeColumnsToContents();
	void			SelectProcess(quint32 Pid);
	void			SnapProcesses();
	void			RemoveTerminatedProcesses();
	bool			IsProcessPresent(quint32 Pid);
	bool			AddProcess(RpcProcessInfo_T* pRpcProcessInfo);
	void			UpdateProcess(RpcProcessInfo_T* pRpcProcessInfo);
	void			Reset();
	ULONG			GetProcesses();
	ULONG			GetTotalProcesses();
	void			UpdateUserFilter();
	void			SaveConfiguration(QSettings*);
	void			LoadConfiguration(QSettings*);
	QWidget*		GetColumnsSelectionWidget();
	void			UpdateColumnsVisibility();

private slots:
	void ProcessSelected(QTreeWidgetItem*, int);
	void ProcessSelected(const QModelIndex&);
	void ApplyUserFilter(const QString &);

signals:
	void ProcessSelected(quint32);

public slots:
	void TreeHeaderClicked(int logicalIndex);
	void ViewHeaderClicked(int logicalIndex);

private:
	QGridLayout*			pGridLayout;
	QStackedWidget*			pStackedWidget;
	QTreeWidget*			pProcessTree;
	QTreeView*				pProcessView;
	QSortFilterProxyModel*	pProxyModel;
	QStandardItemModel*		pModel;
	QList<QTreeWidgetItem*>	PrivateTreeItemList;
	QList<QStandardItem*>	PrivateViewItemList;
	FilterWidget_C*			pFilterWidget;
	//--
	QGroupBox*				pColumnsSelectionWidget;
	QCheckBox*				CheckBoxArray[Column_Last];

	void	SetRowColor(QTreeWidgetItem* pProcess,int Index,const QColor& Color);
	void	InitProcessTreeWidget(QWidget* pParent);
	void	InitProcessTreeView(QWidget* pParent);
	void	AddProcessItem(QTreeWidgetItem* pProcess,int Index, Column_T Column, Qt::ItemDataRole Role,const QVariant& Value);
	void	CreateColumnsSelectionWidget();
	QString GetColumName(Column_T ProcessWigetColumn);
};

#endif// _PROCESS_WIDGET_H_
