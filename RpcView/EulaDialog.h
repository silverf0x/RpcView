#ifndef _EULA_DIALOG_H_
#define _EULA_DIALOG_H_

#include "..\Qt\Qt.h"

//------------------------------------------------------------------------------
class EulaDialog_C : public QDialog
{
    Q_OBJECT

public:
    EulaDialog_C();

private:
	QGridLayout*	pGridLayout;
	QTextEdit*		pTextEdit;
	QPushButton*	pAccept;
	QPushButton*	pDecline;
};

#endif// _EULA_DIALOG_H_