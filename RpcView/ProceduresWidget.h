#ifndef _PROCEDURES_WIDGET_H_
#define _PROCEDURES_WIDGET_H_

#include "..\Qt\Qt.h"
#include "..\RpcCore\RpcCore.h"
#include "View.h"
#include "..\..\RpcCommon\\RpcCommon.h"

typedef enum _ProceduresWigetColumn_T{
	ProceduresWigetColumn_Index,
	ProceduresWigetColumn_Name,
	ProceduresWigetColumn_Address,
	ProceduresWigetColumn_FormatString,
	//...
	ProceduresWigetColumn_Last
}ProceduresWigetColumn_T;

//------------------------------------------------------------------------------
class ProceduresWidget_C : public QDockWidget, public View_I
{
    Q_OBJECT

public:
    ProceduresWidget_C(QWidget* pParent);
	virtual void AcceptVisitor(ViewVisitor_C* pVisitor);

	void		resizeColumnsToContents();
	void		reset(ULONG Pid);
	UINT		GetPid();
	QWidget*	GetColumnsSelectionWidget();
	void		UpdateColumnsVisibility();
	void		SaveConfiguration(QSettings*);
	void		LoadConfiguration(QSettings*);
	void		SetAddressRepresentation(AddressRepresentation_T AddressRepresentation);
public slots:
	bool AddProcedure(quint32 ProcIdx, WCHAR* pSymbolName, VOID* pBase, ULONG Rva, VOID* pProcFormatString);

private:
	UINT					Pid;
	QTreeWidget*			pProcedures;
	QGroupBox*				pColumnsSelectionWidget;
	QCheckBox*				CheckBoxArray[ProceduresWigetColumn_Last];
	void*					pCurrentIntefaceBase;
	AddressRepresentation_T	AddressRepresentation;
	void					CreateColumnsSelectionWidget();
};

#endif// _PROCEDURES_WIDGET_H_
