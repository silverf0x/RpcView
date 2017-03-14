#ifndef _INTERNAL_TYPE_TOOLS_H_
#define _INTERNAL_TYPE_TOOLS_H_

#include <sstream>
#include <list>


//----------------------------------------------------------------------------
BOOL __fastcall isSimpleType(
	_In_	FC_TYPE	type);


//-----------------------------------------------------------------------------
BOOL __fastcall printType(
	_In_	VOID* pContext, 
	_In_	TypeToDefine& typeToDefine,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_	std::ostringstream& oss);




//------------------------------------------------------------------------------
void __fastcall printDefaultEnum(
	_Inout_	std::ostringstream& oss);


#endif//_INTERNAL_TYPE_TOOLS_H_