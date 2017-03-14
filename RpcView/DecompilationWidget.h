#ifndef _DECOMPILATION_WIDGET_H_
#define _DECOMPILATION_WIDGET_H_

#include "../Qt/Qt.h"
#include "IdlHighlighter.h"

class DecompilationWidget_C : public QDockWidget
 {
     Q_OBJECT

 public:
     DecompilationWidget_C(QWidget* pParent);
	 void InsertText(const char* Txt);

 private:
	 IdlHighlighter_C*	pIdlHighlighter;
	 QTextEdit*			pTextEdit;
 };

#endif //_DECOMPILATION_WIDGET_H_