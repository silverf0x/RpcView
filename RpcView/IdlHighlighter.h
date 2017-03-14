#ifndef _IDL_HIGHLIGHTER_H_
#define _IDL_HIGHLIGHTER_H_

#include "../Qt/Qt.h"

 class IdlHighlighter_C : public QSyntaxHighlighter
 {
     Q_OBJECT

 public:
     IdlHighlighter_C(QTextDocument *parent = 0);

 protected:
     void highlightBlock(const QString &text);

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

     QRegExp commentStartExpression;
     QRegExp commentEndExpression;

     QTextCharFormat keywordFormat;
     QTextCharFormat classFormat;
     QTextCharFormat singleLineCommentFormat;
     QTextCharFormat multiLineCommentFormat;
     QTextCharFormat quotationFormat;
     QTextCharFormat functionFormat;
 };

#endif //_IDL_HIGHLIGHTER_H_