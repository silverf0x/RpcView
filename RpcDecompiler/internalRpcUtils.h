#ifndef _RPC_UTILS_
#define _RPC_UTILS_

#include <string>
#include <locale>
#include <vector>
#include <tchar.h>
#include "RpcDecompiler.h"


//---------------------------------------------------------------------------
BOOL __fastcall isStandardCharacter(_In_ const WCHAR wc);


//--------------------------------------------------------------------------
std::string narrow(
	_In_	const std::wstring& ws);


//-------------------------------------------------------------------------
VOID displayPtrLevel(
	_In_	const UINT uPtrLevel, 
	_Inout_	std::ostringstream& oss);

//--------------------------------------------------------------------------
VOID displayErrorMessage(
	_Inout_ std::ostringstream& oss, 
	_In_	PCHAR message);

#endif