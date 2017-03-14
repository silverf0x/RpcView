#include "internalRpcUtils.h"
#include <list>
#include <sstream>


//--------------------------------------------------------------------------
BOOL __fastcall isStandardCharacter(_In_ const WCHAR wc)
{
	BOOL	bResult	= FALSE;

	if( ((wc >= L'a') && 
		(wc <= L'z'))

		||

		((wc >= L'A') &&
		(wc <= L'Z'))

		||

		((wc >= L'0') &&
		(wc <= L'9')) )
	{
		bResult = TRUE;
	}

	return (bResult);
}


//-------------------------------------------------------------------------
std::string narrow(
	_In_	const std::wstring& ws)
{
	//std::vector<char> buffer(ws.size());
	////std::locale loc("english");
	//std::locale loc;
	//std::use_facet< std::ctype<wchar_t> > (loc).narrow(ws.data(), ws.data() + ws.size(), '?', &buffer[0]);

	//return std::string(&buffer[0], buffer.size());

	return std::string(ws.begin(), ws.end());
}


//-------------------------------------------------------------------------
VOID displayPtrLevel(
	_In_	const UINT uPtrLevel, 
	_Inout_	std::ostringstream& oss)
{
	for(UINT i=0; i<uPtrLevel; i++)
	{
		oss<<"*";
	}
}


//----------------------------------------------------------------------------
VOID displayErrorMessage(
	_Inout_ std::ostringstream& oss, 
	_In_	PCHAR message)
{
	oss<< "/* [ERROR] : "<< message << "*/";
}

