#ifndef _VIEW_FACTORY_H_
#define _VIEW_FACTORY_H_

#include "View.h"

using namespace std;

class ViewFactory_C
{
public:
	ViewFactory_C();
	void CreateViews(&vector<View_T*>);
	ViewVisitor_C* CreateVisitor(ViewAction_T ViewAction);
};

#endif //_VIEW_FACTORY_H_