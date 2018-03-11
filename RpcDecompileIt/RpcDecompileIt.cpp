#include "RpcDecompileIt.h"

#include <iostream>
#include <TlHelp32.h>

#include "RpcDecompiler/IdlInterface.h"

#if _WIN64
	#define RPC_CORE_IS_WOW64 false
#else
	#define RPC_CORE_IS_WOW64 true
#endif

int
DecompileInit(
	_Inout_ PDECOMPILE_IT_CTXT Context
)
{
	WCHAR RefModuleName[MAX_PATH];
	HMODULE hMods[1024];
	DWORD 	ModulesSize;

	HANDLE hProcess = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		Context -> TargetPID
	);

	if (hProcess==NULL)
	{
		return GetLastError();
	}
	Context -> hTargetProcess = hProcess;


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
		return -1;
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

	return 0;
}



int 
DecompileIt(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	uintptr_t 				DecompilerHelperAddr = NULL;
	RpcDecompilerHelper_T* 	DecompilerHelper = NULL;
	//
	//RpcInterfaceInfo_T*		RpcInterfaceInfo = NULL;
	//void* 					RpcDecompilerContext;
	MIDL_STUB_DESC			MidlStubDesc;
	size_t					StubDescBytesRead;
	size_t					RpcInterfaceInformationStructSize;
	RPC_CLIENT_INTERFACE    RpcClientInterface;
	//RPC_SERVER_INTERFACE	RpcServerInterface;
	RPC_IF_ID				RpcInterfaceId;
	ASSIGN_RPC_VIEW_STUB(RpcViewHelperStub, &Context);


	// resolve RpcDecompilerHelper address
	HMODULE hRpcDecompiler = LoadLibrary("RpcDecompiler");
	if (!hRpcDecompiler)
	{
		return -1;
	}

	DecompilerHelperAddr = (uintptr_t) GetProcAddress(hRpcDecompiler, "RpcDecompilerHelper");
	if (!DecompilerHelperAddr)
	{
		return -1;
	}
	DecompilerHelper = (RpcDecompilerHelper_T*) DecompilerHelperAddr;

	if (!ReadProcessMemory(
		Context.hTargetProcess,
		(LPCVOID)Context.DescriptorAddress,
		&MidlStubDesc,
		sizeof(MidlStubDesc),
		(SIZE_T*) &StubDescBytesRead
	))
	{
		return -1;
	}

	if (!ReadProcessMemory(
		Context.hTargetProcess,
		MidlStubDesc.RpcInterfaceInformation,
		&RpcInterfaceInformationStructSize,
		sizeof(RpcInterfaceInformationStructSize),
		(SIZE_T*) &StubDescBytesRead
	))
	{
		return -1;
	}

	switch (RpcInterfaceInformationStructSize)
	{
	case sizeof(RPC_CLIENT_INTERFACE):

		if (!ReadProcessMemory(
			Context.hTargetProcess,
			MidlStubDesc.RpcInterfaceInformation,
			&RpcClientInterface,
			sizeof(RpcClientInterface),
			(SIZE_T*) &StubDescBytesRead
		))
		{
			return -1;
		}
		
		RpcInterfaceId.Uuid = RpcClientInterface.InterfaceId.SyntaxGUID;
		RpcInterfaceId.VersMajor = RpcClientInterface.InterfaceId.SyntaxVersion.MajorVersion;
		RpcInterfaceId.VersMinor= RpcClientInterface.InterfaceId.SyntaxVersion.MinorVersion;

		//RpcDecompilerInfoStub.pSyntaxId = &RpcClientInterface.InterfaceId;
		break;

	//case sizeof(RPC_SERVER_INTERFACE):
	default:
		return -1;
	}


	std::string IfaceName("DecompileItInterface");
	IdlInterface Interface(IfaceName, RpcInterfaceId, 1);

	
	// Init stubs for RpcDecompiler
	RpcDecompilerInfo_T		RpcDecompilerInfoStub;
	RpcDecompilerInfoStub.ppProcNameTable = new WCHAR*[1];
	RpcDecompilerInfoStub.ppProcNameTable[0] = NULL;

	RpcDecompilerInfoStub.pFormatStringOffsetTable = new USHORT[1];
	RpcDecompilerInfoStub.pFormatStringOffsetTable[0] = 0;
	RpcDecompilerInfoStub.pProcFormatString = (RVA_T) Context.FormatStrOffset;

	RpcDecompilerInfoStub.pTypeFormatString = (RVA_T) (MidlStubDesc.pFormatTypes - Context.ModuleBaseAddress);

	RpcModuleInfo_T ModuleInfoStub = {
		/*Pid = */Context.TargetPID,
		/*pModuleBase = */Context.ModuleBaseAddress
	};

	RpcDecompilerCtxt_T DecompilerStubContext;
	DecompilerStubContext.pRpcDecompilerInfo = &RpcDecompilerInfoStub;
	DecompilerStubContext.pRpcModuleInfo = &ModuleInfoStub;
	DecompilerStubContext.pRpcViewHelper = &RpcViewHelperStub;


	// Decode function
	if (DS_SUCCESS == Interface.decode((PVOID)&DecompilerStubContext))
	{
		std::cout << Interface;

	}
	
	return 0;
}

int
DecompileUninit(
	_In_ DECOMPILE_IT_CTXT Context
)
{
	if (Context.hTargetProcess == INVALID_HANDLE_VALUE)
	{
		CloseHandle(Context.hTargetProcess);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	DECOMPILE_IT_CTXT Context = {0};
	char*  	EndPtr;
	int 	status;

	if (argc < 7)
	{
		printf("Usage: %s --pid PID [--module MODULE] --descriptor DESC_OFFSET --format-str FORMAT_STRING_OFFSET [--absolute]\n", argv[0]);
		printf("  --pid : PID of the target process. %s must be able to open a handle to read the target process memory.\n", argv[0]);
		printf("  --module : module name to read memory from. If not set, %s read the target executable own module. Ignored if --absolute is set.\n", argv[0]);
		printf("  --descriptor : offset to the rpc header descriptor for the interface. If --absolute is set, --descriptor is interpreted as a virtual address.\n");
		printf("  --format-str : offset to the rpc format string for the chosen proc. If --absolute is set, --format-str is interpreted as a virtual address.\n");
		printf("  --absolute : treat descriptor and format-str as absolute virtual addresses instead of offsets.\n");

		return 0;
	}


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
	}


	status = DecompileInit(
		&Context
	);
	if (status)
	{
		printf("Could not init the DecompileIt context : %d.\n", GetLastError());
		return status;
	}

	status = DecompileIt(Context);


	DecompileUninit(Context);
	return status;
}