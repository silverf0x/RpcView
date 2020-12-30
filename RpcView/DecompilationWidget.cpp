#include "DecompilationWidget.h"

#define TAB_AS_CHARS	4

static const char WidgetName[] = "Decompilation";


//------------------------------------------------------------------------------
void DecompilationWidget_C::InsertText(const char* Txt)
{
	/*
	pTextEdit->moveCursor(QTextCursor::End);
	pTextEdit->append( QString::fromLatin1(Txt) );
	pTextEdit->ensureCursorVisible();
	*/
	pTextEdit->setText( QString::fromLatin1(Txt) );
	pTextEdit->ensureCursorVisible();
}

//------------------------------------------------------------------------------
DecompilationWidget_C::DecompilationWidget_C(QWidget *parent) : QDockWidget(WidgetName)
{
	QFont font;

    UNREFERENCED_PARAMETER(parent);
	setObjectName(WidgetName);

	font.setFamily("Courier");
	font.setFixedPitch(true);

	pTextEdit = new QTextEdit(this);
	pTextEdit->setLineWrapMode(QTextEdit::NoWrap);
	pTextEdit->setFont(font);
	pTextEdit->setTabStopDistance(font.pointSize()*TAB_AS_CHARS);

	pIdlHighlighter = new IdlHighlighter_C(pTextEdit->document());
	setWidget(pTextEdit);
}