#ifndef _PDB_H_
#define _PDB_H_

#include <windows.h>

EXTERN_C __checkReturn	void* WINAPI PdbInit(__in HANDLE hProcess, __in VOID* pModuleBase, __in UINT ModuleSize);
EXTERN_C __checkReturn	BOOL WINAPI PdbGetSymbolName(__in void* pPdbCtxt, __in VOID* pSymbol, __out WCHAR* pName, __in UINT NameLength);
EXTERN_C				void WINAPI PdbUninit(__in void* pPdbCtxt);

#endif // _PDB_H_