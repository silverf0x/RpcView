#ifndef _PROCESS_ENTRY_H_
#define _PROCESS_ENTRY_H_

#include <windows.h>

//------------------------------------------------------------------------------
class ProcessEntry_C
{
public:
	ProcessEntry_C(ULONG Ppid, ULONG Pid)
	{
		this->Ppid=Ppid;
		this->Pid = Pid;
	}
	ULONG	Ppid;
	ULONG	Pid;
};

#endif //_PROCESS_ENTRY_H_