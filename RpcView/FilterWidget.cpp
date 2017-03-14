#include "FilterWidget.h"
#include "RpcViewResource.h"


//------------------------------------------------------------------------------
void FilterWidget_C::Show()
{
	this->show();
	pUserFilter->setFocus();
}


//------------------------------------------------------------------------------
void FilterWidget_C::Reset()
{
	pUserFilter->clear();
	this->hide();
}


//------------------------------------------------------------------------------
const QString FilterWidget_C::GetText()
{
	return ( pUserFilter->text() );
}


//------------------------------------------------------------------------------
void FilterWidget_C::Hide()
{
	this->hide();
}


//------------------------------------------------------------------------------
FilterWidget_C::FilterWidget_C(QWidget* pParent)
{
	QAction* pFindAction	= new QAction(this);
	QAction* pEchapAction	= new QAction(this);
	QLabel*	 pFilterLabel	= new QLabel("Filter: ",this);

	pFindAction->setShortcut(QKeySequence::Find);
	pFindAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	pParent->addAction( pFindAction );
	
	pEchapAction->setShortcut(QKeySequence(tr("Esc")));
	pEchapAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	pParent->addAction( pEchapAction );
	
	connect( pFindAction,  SIGNAL(triggered(bool)), this, SLOT(Show()));
	connect( pEchapAction, SIGNAL(triggered(bool)), this, SLOT(Hide()));

	QGridLayout*	pFilterLayout	= new QGridLayout(this);

	pUserFilter		= new QLineEdit(this);
	pMatchingItems	= new QLabel(this);
	pFilterLayout->addWidget(pFilterLabel, 0, 0);
	pFilterLayout->addWidget(pUserFilter,0,1);
	this->setLayout(pFilterLayout);
	this->Reset();

	connect( pUserFilter, SIGNAL(textEdited(const QString &)), pParent, SLOT(ApplyUserFilter(const QString &)) );
}