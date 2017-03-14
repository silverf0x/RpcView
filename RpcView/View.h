#ifndef _VIEW_H_
#define _VIEW_H_

#include "..\Qt\Qt.h"
#include "ViewVisitor.h"

//
// View Interface
//
class View_I
{
public:
    virtual void AcceptVisitor(ViewVisitor_C* pVisitor)=0;
};

#endif // _VIEW_H_