#ifndef _FILTER_WIDGET_H_
#define _FILTER_WIDGET_H_

#include "..\Qt\Qt.h"

//-----------------------------------------------------------------------------
class FilterWidget_C : public QWidget
{
    Q_OBJECT

public:
    FilterWidget_C(QWidget* pParent);
	void Reset();
	const QString GetText();

private slots:
	void Hide();
	void Show();

private:
	QLineEdit*	pUserFilter;
	QLabel*		pMatchingItems;
};


#endif //_FILTER_WIDGET_H_