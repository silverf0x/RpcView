#include "RpcDecompileIt.h"

#include <string.h>  
#include <iostream>
#include <TlHelp32.h>

#include "RpcDecompiler/IdlInterface.h"

#if _WIN64
	#define RPC_CORE_IS_WOW64 false
#else
	#define RPC_CORE_IS_WOW64 true
#endif

bool
DecompileInit(
	_Inout_ PDECOMPILE_IT_CTXT Context
)
{
	WCHAR RefModuleName[MAX_PATH];
	HMODULE hMods[1024];
	DWORD 	ModulesSize;
	BOOL	bOwnProcessWow64 = false;
	BOOL	bRemoteProcessWow64 = false;

	HANDLE hProcess = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		Context -> TargetPID
	);

	if (hProcess==NULL)
	{
		printf("[x] Could not access the remote process : %d\n", GetLastError());
		return false;
	}
	Context->hTargetProcess = hProcess;

	IsWow64Process(GetCurrentProcess(), &bOwnProcessWow64);
	IsWow64Process(hProcess, &bRemoteProcessWow64);

	if (bOwnProcessWow64 != bRemoteProcessWow64)
	{
		#define	PROCESS_ARCH(bIsWow64) (bIsWow64) ? "Wow64" : "x64"

		printf("[x] Remote process does not have the same arch as own process : %s != %s\n",
			PROCESS_ARCH(bRemoteProcessWow64),
			PROCESS_ARCH(bOwnProcessWow64)
		);
		return false;
	}


	


	if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &ModulesSize))
    {
		return GetLastError();
    }

	// stop at the first if ModuleName is not set
	if (!Context->ModuleName)
	{
		Context->ModuleBaseAddress = (uintptr_t) hMods[0];
	}
	else
	{
		swprintf_s(RefModuleName, sizeof(RefModuleName) / sizeof(WCHAR), L"%hs", Context->ModuleName);

		for (auto ModuleIndex = 0; (unsigned int) ModuleIndex < (ModulesSize / sizeof(HMODULE)); ModuleIndex++ )
		{
        	WCHAR ModuleName[MAX_PATH];


        	if (GetModuleBaseNameW(
        		hProcess, 
        		hMods[ModuleIndex], 
				ModuleName,
        		sizeof(ModuleName) / sizeof(WCHAR)
        	))
            {
	        	if (!_wcsicmp(ModuleName, RefModuleName))
	        	{
	        		Context->ModuleBaseAddress = (uintptr_t) hMods[ModuleIndex];
					break;
	        	}
	        }
        }		
	}

	if (!Context->ModuleBaseAddress)
    {
		return false;
    }
		
	
	if (Context->bAbsoluteAddress)
	{
		Context->DescriptorAddress = Context->DescriptorArg;
		Context->DescriptorOffset = Context->DescriptorAddress - Context->ModuleBaseAddress;

		Context->FormatStrAddress = Context->FormatStrArg;
		Context->FormatStrOffset = Context->FormatStrAddress - Context->ModuleBaseAddress;
	}
	else 
	{
		Context->DescriptorOffset = Context->DescriptorArg;
		Context->DescriptorAddress = Context->DescriptorOffset + Context->ModuleBaseAddress;

		Context->FormatStrOffset = Context->FormatStrArg;
		Context->FormatStrAddress = Context->FormatStrOffset + Context->ModuleBaseAddress;
	}

	return true;
}



bool
ReadRpcInterface(
	_In_  RpcViewHelper_T *RpcViewHelper,
	_In_  RpcModuleInfo_T  ModuleInfo,
	_In_  MIDL_STUB_DESC   MidlDescription,
	_Out_ RPC_IF_ID		  *RpcInterface
)
{
	unsigned int			RpcInterfaceInformationStructSize = 0;
	RPC_CLIENT_INTERFACE    RpcClientInterface = {0};


	if (!RpcInterface || !RpcViewHelper)
	{
		return false;
	}

	// Read RPC_CLIENT_INTERFACE.size to know how many bytes we need to read
	if (!RpcViewHelper->RpcGetProcessData(
		&ModuleInfo,
		(RVA_T) ( (uintptr_t) MidlDescription.RpcInterfaceInformation - ModuleInfo.pModuleBase),
		&RpcInterfaceInformationStructSize,
		sizeof(RpcClientInterface.Length)
	))
	{
		return false;
	}


	switch (RpcInterfaceInformationStructSize)
	{
	// RPC_SERVER_INTERFACE and RPC_CLIENT_INTERFACE are pretty much the same structure
	//case sizeof(RPC_SERVER_INTERFACE):
	case sizeof(RPC_CLIENT_INTERFACE):

		// Read the full structure
		if (!RpcViewHelper->RpcGetProcessData(
			&ModuleInfo,
			(RVA_T)((uintptr_t)MidlDescription.RpcInterfaceInformation - ModuleInfo.pModuleBase),
			&RpcClientInterface,
			sizeof(RPC_CLIENT_INTERFACE)
		))
		{
			return false;
		}
		
		RpcInterface->Uuid = RpcClientInterface.InterfaceId.SyntaxGUID;
		RpcInterface->VersMajor = RpcClientInterface.InterfaceId.SyntaxVersion.MajorVersion;
		RpcInterface->VersMinor= RpcClientInterface.InterfaceId.SyntaxVersion.MinorVersion;

		break;

	default:
		return false;
	}
	
	return true;
}

bool 
DecompileIt(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	//uintptr_t 				DecompilerHelperAddr = NULL;
	//RpcDecompilerHelper_T* 	DecompilerHelper = NULL;
	MIDL_STUB_DESC			MidlStubDesc;
	size_t					StubDescBytesRead;
	RPC_IF_ID				RpcInterfaceId;
	RpcDecompilerInfo_T		RpcDecompilerInfoStub;
	RpcDecompilerCtxt_T 	DecompilerStubContext;
	RpcViewHelper_T 		RpcViewHelperStub;

	RpcModuleInfo_T 	ModuleInfoStub = {
		/*Pid = */			Context.TargetPID,
		/*pModuleBase = */	Context.ModuleBaseAddress
	};


	// init RpcView helper stubs in order to use RpcGetProcessData
	DecompileItInitRpcViewStub(&RpcViewHelperStub, (PVOID) &Context);

	// Read MIDL_STUB_DESC and RPC_INTERFACE structures from remote target
	if (!ReadProcessMemory(
		Context.hTargetProcess,
		(LPCVOID)Context.DescriptorAddress,
		&MidlStubDesc,
		sizeof(MidlStubDesc),
		(SIZE_T*) &StubDescBytesRead
	))
	{
		printf("[x] Could not read MIDL_STUB_DESC structure from desc-offset rva.\n");
		return false;
	}

	if (!ReadRpcInterface(
		&RpcViewHelperStub,
		ModuleInfoStub,
		MidlStubDesc,
		&RpcInterfaceId
	))
	{
		printf("[x] Could not retrieve a RPC_INTERFACE for the  MIDL_STUB_DESC structure.\n");
		return false;
	}

	
	// Init stubs for RpcDecompiler
	RpcDecompilerInfoStub.ppProcNameTable = new WCHAR*[Context.NumberOfProcedures];
	for (size_t i = 0; i < Context.NumberOfProcedures; i++)
	{
		RpcDecompilerInfoStub.ppProcNameTable[i] = NULL;
	}
	

	RpcDecompilerInfoStub.pFormatStringOffsetTable = Context.FormatStrOffsets;
	RpcDecompilerInfoStub.pProcFormatString = (RVA_T) Context.FormatStrOffset;
	RpcDecompilerInfoStub.pTypeFormatString = (RVA_T) ((uintptr_t)MidlStubDesc.pFormatTypes - Context.ModuleBaseAddress);

	DecompilerStubContext.pRpcDecompilerInfo = &RpcDecompilerInfoStub;
	DecompilerStubContext.pRpcModuleInfo = &ModuleInfoStub;
	DecompilerStubContext.pRpcViewHelper = &RpcViewHelperStub;


	// Decode function
	std::string IfaceName("DecompileItInterface");
	IdlInterface Interface(IfaceName, RpcInterfaceId, Context.NumberOfProcedures);
	if (DS_SUCCESS == Interface.decode((PVOID)&DecompilerStubContext))
	{
		std::cout << Interface;

	}
	
	return true;
}

bool
DecompileUninit(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	if (Context.hTargetProcess == INVALID_HANDLE_VALUE)
	{
		CloseHandle(Context.hTargetProcess);
	}

	return true;
}


int main(int argc, char* argv[])
{
	DECOMPILE_IT_CTXT Context = {0};
	char*  	EndPtr;
	bool 	status;

	if (argc < 7)
	{
		printf("Usage: %s --pid PID [--module MODULE] --descriptor DESC_OFFSET --format-str FORMAT_STRING_OFFSET [--absolute] [--format-str-offsets OFF1,OFF2,OFFn]\n", argv[0]);
		printf("  --pid : PID of the target process. %s must be able to open a handle to read the target process memory.\n", argv[0]);
		printf("  --module : module name to read memory from. If not set, %s read the target executable own module. Ignored if --absolute is set.\n", argv[0]);
		printf("  --descriptor : offset to the rpc header descriptor for the interface. If --absolute is set, --descriptor is interpreted as a virtual address.\n");
		printf("  --format-str : offset to the rpc format string for the chosen proc. If --absolute is set, --format-str is interpreted as a virtual address.\n");
		printf("  --absolute : treat descriptor and format-str as absolute virtual addresses instead of offsets.\n");
		printf("  --format-str-offsets : offsets within the format string for the various procedures (default:0).\n");

		return 0;
	}


	bool bFormatStrOffsetsProvided = false;
	for (auto ArgIndex = 0; ArgIndex < argc; ArgIndex++) 
	{
		char *CurrentArgument = argv[ArgIndex];

		if (!_stricmp(CurrentArgument, "--pid"))
		{
			Context.TargetPID = (DWORD) strtoumax(argv[ArgIndex + 1], &EndPtr, 10);
		}
		if (!_stricmp(CurrentArgument, "--module"))
		{
			Context.ModuleName = argv[ArgIndex + 1];
		}
		else if (!_stricmp(CurrentArgument, "--descriptor"))
		{
			Context.DescriptorArg = (size_t) strtoumax(argv[ArgIndex + 1], &EndPtr, 16);
		}
		else if (!_stricmp(CurrentArgument, "--format-str"))
		{
			Context.FormatStrArg = (size_t) strtoumax(argv[ArgIndex + 1], &EndPtr, 16);
		}
		else if (!_stricmp(CurrentArgument, "--absolute"))
		{
			Context.bAbsoluteAddress = true;
		}
		else if (!_stricmp(CurrentArgument, "--format-str-offsets"))
		{
			bFormatStrOffsetsProvided = true;

			// parsing offsets using STL since strtok sucks.
			std::string offsets_str(argv[ArgIndex + 1]);
			std::vector<std::string> offsets;

			size_t last_pos = 0;
			size_t pos = offsets_str.find(',');

			while (pos != std::string::npos) {
				offsets.push_back(offsets_str.substr(last_pos, pos - last_pos));
				last_pos = ++pos;
				pos = offsets_str.find(',', pos);

				if (pos == std::string::npos)
				{
					offsets.push_back(
						offsets_str.substr(last_pos, offsets_str.length())
					);
				}
					
			}

			Context.NumberOfProcedures = offsets.size();
			Context.FormatStrOffsets = new uint16_t[Context.NumberOfProcedures];

			for (size_t i = 0; i < offsets.size(); i++)
			{
				Context.FormatStrOffsets[i] = (uint16_t)strtoumax(offsets[i].c_str(), &EndPtr, 0);
			}

		}
	}

	if (!bFormatStrOffsetsProvided)
	{
		Context.NumberOfProcedures = 1;
		Context.FormatStrOffsets = new uint16_t[1];
		Context.FormatStrOffsets[0] = 0;
	}


	if (!DecompileInit(
		&Context
	))
	{
		printf("Could not init the DecompileIt context : %d.\n", GetLastError());
		return -1;
	}

	status = DecompileIt(Context);


	DecompileUninit(Context);
	return !status;
}