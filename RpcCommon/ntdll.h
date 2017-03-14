#ifndef _NTDLL_H_
#define _NTDLL_H_

#include <windows.h>

typedef LONG KPRIORITY; 

typedef struct _UNICODE_STRING{
	USHORT	Length;
	USHORT	MaximumLength;
	PWSTR	Buffer;
}UNICODE_STRING,*PUNICODE_STRING;


typedef VOID (NTAPI* RtlInitUnicodeStringFn_T)(
	__out PUNICODE_STRING DestinationString,
	__in_z_opt __drv_aliasesMem PCWSTR SourceString
	);

VOID NTAPI RtlInitUnicodeString(
    __out PUNICODE_STRING DestinationString,
    __in_z_opt __drv_aliasesMem PCWSTR SourceString
    );


typedef struct _PROCESS_PARAMETERS {
	ULONG			AllocationSize;
	ULONG			Size;
	ULONG			Flags;
	ULONG			Reserved;
	ULONG_PTR		Console;
	ULONG			ProcessGroup;
	HANDLE			hStdInput;
	HANDLE			hStdOutput;
	HANDLE			hStdError;
	UNICODE_STRING	CurrentDirectoryName;
	HANDLE			CurrentDirectoryHandle;
	UNICODE_STRING	DllPath;
	UNICODE_STRING	ImageFile;
	UNICODE_STRING	CommandLine;
	PWSTR			Environment;
	ULONG			dwX;
	ULONG			dwY;
	ULONG			dwXSize;
	ULONG			dwYSize;
	ULONG			dwXCountChars;
	ULONG			dwYCountChars;
	ULONG			dwFillAttribute;
	ULONG			dwFlags;
	ULONG			wShowWindow;
	UNICODE_STRING	WindowTitle;
	UNICODE_STRING	Desktop;
	UNICODE_STRING	Reserved1;
	UNICODE_STRING	Reserved2;
} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

#pragma warning(push)
#pragma warning(disable:4214)
typedef struct _PEB{
     UCHAR 					InheritedAddressSpace;
     UCHAR 					ReadImageFileExecOptions;
     UCHAR 					BeingDebugged;
     UCHAR 					ImageUsesLargePages			: 1;
     UCHAR 					IsProtectedProcess			: 1;
     UCHAR 					IsLegacyProcess				: 1;
     UCHAR 					IsImageDynamicallyRelocated	: 1;
     UCHAR 					SpareBits					: 4;
     VOID* 					Mutant;
     VOID* 					ImageBaseAddress;
     VOID* 					Ldr;
     PROCESS_PARAMETERS*	ProcessParameters;
}PEB;
#pragma warning(pop)

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation,			// 0
	ProcessQuotaLimits,					// 1
	ProcessIoCounters,					// 2
	ProcessVmCounters,					// 3
	ProcessTimes,						// 4
	ProcessBasePriority,				// 5
	ProcessRaisePriority,				// 6
	ProcessDebugPort,					// 7
	ProcessExceptionPort,				// 8
	ProcessAccessToken,					// 9
	ProcessLdtInformation,				// 10
	ProcessLdtSize,						// 11
	ProcessDefaultHardErrorMode,		// 12
	ProcessIoPortHandlers,				// 13
	ProcessPooledUsageAndLimits,		// 14
	ProcessWorkingSetWatch,				// 15
	ProcessUserModeIOPL,				// 16
	ProcessEnableAlignmentFaultFixup,	// 17
	ProcessPriorityClass,				// 18
	ProcessWx86Information,				// 19
	ProcessHandleCount,					// 20
	ProcessAffinityMask,				// 21
	ProcessPriorityBoost,				// 22
	ProcessDeviceMap,					// 23
	ProcessSessionInformation,			// 24
	ProcessForegroundInformation,		// 25
	ProcessWow64Information				// 26
} PROCESSINFOCLASS;


typedef NTSTATUS (NTAPI* NtQueryInformationProcessFn_T)(
	IN	HANDLE				ProcessHandle,
	IN	PROCESSINFOCLASS	ProcessInformationClass,
	OUT	PVOID				ProcessInformation,
	IN	ULONG				ProcessInformationLength,
	OUT	PULONG				ReturnLength OPTIONAL
	);

NTSTATUS NTAPI NtQueryInformationProcess(
	IN	HANDLE				ProcessHandle,
	IN	PROCESSINFOCLASS	ProcessInformationClass,
	OUT	PVOID				ProcessInformation,
	IN	ULONG				ProcessInformationLength,
	OUT	PULONG				ReturnLength OPTIONAL
);


#define STATUS_SUCCESS	0

//
// Valid values for the Attributes field
//

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L


typedef struct _PROCESS_BASIC_INFORMATION { // Information Class 0
	ULONG_PTR	ExitStatus;
	PEB*		PebBaseAddress;
	ULONG_PTR	AffinityMask;
	ULONG_PTR	BasePriority;
	ULONG_PTR	UniqueProcessId;
	ULONG_PTR	InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;


typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

//++
//
// VOID
// InitializeObjectAttributes(
//     __out POBJECT_ATTRIBUTES p,
//     __in PUNICODE_STRING n,
//     __in ULONG a,
//     __in HANDLE r,
//     __in PSECURITY_DESCRIPTOR s
//     )
//
//--

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

typedef NTSTATUS (NTAPI* NtOpenSectionFn_T)(
	__out PHANDLE SectionHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS NTAPI NtOpenSection(
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

//
// Section Information Structures.
//

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

typedef NTSTATUS (NTAPI* NtCreateSectionFn_T)(
	_Out_     PHANDLE SectionHandle,
	_In_      ACCESS_MASK DesiredAccess,
	_In_opt_  POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_  PLARGE_INTEGER MaximumSize,
	_In_      ULONG SectionPageProtection,
	_In_      ULONG AllocationAttributes,
	_In_opt_  HANDLE FileHandle
	);

NTSTATUS NTAPI NtCreateSection(
  _Out_     PHANDLE SectionHandle,
  _In_      ACCESS_MASK DesiredAccess,
  _In_opt_  POBJECT_ATTRIBUTES ObjectAttributes,
  _In_opt_  PLARGE_INTEGER MaximumSize,
  _In_      ULONG SectionPageProtection,
  _In_      ULONG AllocationAttributes,
  _In_opt_  HANDLE FileHandle
);


typedef NTSTATUS (NTAPI* NtMapViewOfSectionFn_T)(
	__in HANDLE SectionHandle,
	__in HANDLE ProcessHandle,
	__inout PVOID *BaseAddress,
	__in ULONG_PTR ZeroBits,
	__in SIZE_T CommitSize,
	__inout_opt PLARGE_INTEGER SectionOffset,
	__inout PSIZE_T ViewSize,
	__in SECTION_INHERIT InheritDisposition,
	__in ULONG AllocationType,
	__in ULONG Win32Protect
	);


NTSTATUS NTAPI NtMapViewOfSection(
    __in HANDLE SectionHandle,
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __in SIZE_T CommitSize,
    __inout_opt PLARGE_INTEGER SectionOffset,
    __inout PSIZE_T ViewSize,
    __in SECTION_INHERIT InheritDisposition,
    __in ULONG AllocationType,
    __in ULONG Win32Protect
    );


typedef enum THREAD_INFORMATION_CLASS {
	ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger
} THREAD_INFORMATION_CLASS, *PTHREAD_INFORMATION_CLASS;

typedef struct {
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
} CLIENT_ID;

typedef struct _THREAD_BASIC_INFORMATION {
  NTSTATUS                ExitStatus;
  PVOID                   TebBaseAddress;
  CLIENT_ID               ClientId;
  KAFFINITY               AffinityMask;
  KPRIORITY               Priority;
  KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef NTSTATUS (NTAPI* NtQueryInformationThreadFn_T)(
	__in HANDLE						ThreadHandle,
	__in THREAD_INFORMATION_CLASS	ThreadInformationClass,
	__out PVOID						ThreadInformation,
	__in ULONG						ThreadInformationLength,
	__out PULONG					ReturnLength				OPTIONAL
	);


NTSTATUS NTAPI NtQueryInformationThread(
	__in HANDLE						ThreadHandle,
	__in THREAD_INFORMATION_CLASS	ThreadInformationClass,
	__out PVOID						ThreadInformation,
	__in ULONG						ThreadInformationLength,
	__out PULONG					ReturnLength				OPTIONAL 
);


typedef NTSTATUS (NTAPI* NtUnmapViewOfSectionFn_T)(
	__in HANDLE ProcessHandle,
	__in_opt PVOID BaseAddress
	);

NTSTATUS NTAPI NtUnmapViewOfSection(
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress
    );

#pragma warning(push)
#pragma warning(disable:4201)
typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};

	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
#pragma warning(pop)

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

typedef NTSTATUS (NTAPI* NtOpenFileFn_T)(
	__out PHANDLE FileHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__out PIO_STATUS_BLOCK IoStatusBlock,
	__in ULONG ShareAccess,
	__in ULONG OpenOptions
);

NTSTATUS NTAPI NtOpenFile(
	__out PHANDLE FileHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__out PIO_STATUS_BLOCK IoStatusBlock,
	__in ULONG ShareAccess,
	__in ULONG OpenOptions
);
#endif //_NTDLL_H_