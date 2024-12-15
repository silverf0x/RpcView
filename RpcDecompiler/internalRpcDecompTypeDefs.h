#ifndef _INTERNAL_RPC_DECOMP_TYPE_DEFS_H_
#define _INTERNAL_RPC_DECOMP_TYPE_DEFS_H_


#include <string>
#include <list>
#include <vector>


#include "..\RpcCommon\RpcView.h"
#include "RpcDecompiler.h"

extern boolean robustFlagWasSet;
extern BOOL is64B;





////////////////////////////////////////////////////////////////////////////////
// Private types declaration
////////////////////////////////////////////////////////////////////////////////
typedef struct _RpcDecompilerCtxt_T{
	RpcViewHelper_T*		pRpcViewHelper;
	RpcDecompilerInfo_T*	pRpcDecompilerInfo;
	RpcModuleInfo_T*		pRpcModuleInfo;
	//...
}RpcDecompilerCtxt_T;

#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#define DEBUG_EXIT(x)	ExitProcess(x)
#else
#define DEBUG_BREAK()
#define DEBUG_EXIT(x)
#endif

#define DEFAULT_IF_NAME "DefaultIfName"
#define RPC_DECOMPILER_INVALID_TYPE_SIZE	((UINT)-1)
#define RPC_DECOMPILER_INVALID_PARAM_SIZE	((UINT)-1)
//#define RPC_DEBUG_FN	pRpcDecompilerCtxt->pRpcViewHelper->RpcDebug
#define RPC_ERROR_FN(x)		printf("[%s] error %s",__FUNCTION__,x);
#define RPC_DEBUG_FN(...)    printf((const char*)__VA_ARGS__)
#define RPC_PRINT_FN	pRpcDecompilerCtxt->pRpcViewHelper->RpcPrint
#define RPC_ALLOC_FN	pRpcDecompilerCtxt->pRpcViewHelper->RpcAlloc
#define RPC_FREE_FN		pRpcDecompilerCtxt->pRpcViewHelper->RpcFree
#define RPC_GET_PROCESS_DATA( pAddress, pBuffer, BufferLength)												\
{																											\
	/*BOOL	bResult;*/																						\
	\
	bResult = pRpcDecompilerCtxt->pRpcViewHelper->RpcGetProcessData(pRpcDecompilerCtxt->pRpcModuleInfo, pAddress, pBuffer, BufferLength);	\
	if(bResult == FALSE)																					\
{																										\
	RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s ligne = %d - ERROR RpcGetProcessData", __FILE__, __LINE__);					\
	*((UCHAR*)NULL)=0;																				\
}																										\
}
#define RPC_GET_PROCESS_DATA2( pAddress, pBuffer, BufferLength)	pRpcDecompilerCtxt->pRpcViewHelper->RpcGetProcessData(pRpcDecompilerCtxt->pRpcModuleInfo, pAddress, pBuffer, BufferLength)											\

#define ERROR_ZERO_TYPEOFFSET_MSG "\nERROR: TypeOffset == 0 whereas Type is not simple Type\n"


#define NB_DIGIT_MAX_INT_32BITS_BASE_10		11	//2^32 = 4294967296 ==> 10 caractères ou word
//2^31 = 2147483648 + signe ==> 11 caractères ou word
#define BASE_10								10

#define EMPTY_PARAM_ATTR					0x0000

#define VIRTUAL_STACK_OFFSET_GRANULARITY_32B	4
#define VIRTUAL_STACK_OFFSET_GRANULARITY_64B	8
#define VIRTUAL_STACK_OFFSET_GRANULARITY			(is64B?VIRTUAL_STACK_OFFSET_GRANULARITY_64B:VIRTUAL_STACK_OFFSET_GRANULARITY_32B)



////////////////////////////////////////////////////////////////////////////////
// Codes retour de fonctions
////////////////////////////////////////////////////////////////////////////////
typedef enum RpcDecompilerErrorCodes_E
{
	RPC_DECOMP_SUCCESS	= 0,
	RPC_DECOMP_ERROR	= 1,
	RPC_DECOMP_NULL_PTR	= 2,
	RPC_DECOMP_NEED_MORE_SOURCE_BYTE = 3
}RpcDecompilerErrorCodes_E;

////////////////////////////////////////////////////////////////////////////////
// _MIDL_PROC_FORMAT_STRING types
////////////////////////////////////////////////////////////////////////////////

// types de headers possibles
typedef enum HeaderID_E
{
	oi,				// voir typedef struct Oi_Header_t
	oif,			// voir typedef struct Oif_Header_t
	win2kext,		// voir typedef struct Win2kExt_Header_t

	undefHeaderType
}HeaderID_E;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// HANDLE types
//------------------------------------------------------------------------------
// handle_type values
//#define FC_BIND_CONTEXT		0x30
//#define FC_BIND_GENERIC		0x31
//#define FC_BIND_PRIMITIVE	0x32
//#define FC_AUTO_HANDLE		0x33
//#define FC_CALLBACK_HANDLE	0x34
#define FC_EXPLICIT_HANDLE	0x00

#pragma pack(push, 1) // Align all structure on 1 byte

typedef struct ExplicitHandlePrimitive_t
{
	unsigned char	flag;
	unsigned short	offset;
}ExplicitHandlePrimitive_t;

typedef struct ExplicitHandleGeneric_t
{
	unsigned char	flagAndSize;
	unsigned short	offset;
	unsigned char	binding_routine_pair_index;
	unsigned char	pad;
}ExplicitHandleGeneric_t;

typedef struct ExplicitHandleContext_t
{
	unsigned char	flags;
	unsigned short	offset;
	unsigned char	context_rundown_routine_index;
	unsigned char	param_num;
}ExplicitHandleContext_t;

typedef union ExplicitHandle_U
{
	ExplicitHandlePrimitive_t	explicitHandlePrimitive;
	ExplicitHandleGeneric_t		explicitHandleGeneric;
	ExplicitHandleContext_t		explicitHandleContext;
}ExplicitHandle_U;

#define EXPLICIT_HANDLE_MIN_SIZE		4		// EXPLICIT_HANDLE_PRIMITIVE_SIZE
#define EXPLICIT_HANDLE_PRIMITIVE_SIZE	4		// sizeof(ExplicitHandle_t.htype) + sizeof(ExplicitHandlePrimitive_t)
#define EXPLICIT_HANDLE_GENERIC_SIZE	6		// sizeof(ExplicitHandle_t.htype) + sizeof(ExplicitHandleGeneric_t)
#define EXPLICIT_HANDLE_CONTEXT_SIZE	6		// sizeof(ExplicitHandle_t.htype) + sizeof(ExplicitHandleContext_t)
typedef struct ExplicitHandle_t
{
	unsigned char		htype;		// voir handle_type values
	ExplicitHandle_U	hContent;
}ExplicitHandle_t;



//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// OI header types
//------------------------------------------------------------------------------
//oi_flags values
#define Oi_FULL_PTR_USED			0x01	//Uses the full pointer package
#define Oi_RPCSS_ALLOC_USED			0x02	//Uses the RpcSs memory package
#define Oi_OBJECT_PROC				0x04	//A procedure in an object interface
#define Oi_HAS_RPCFLAGS				0x08	//The procedure has Rpc flags
#define Oi_star10					0x10	//Overloaded
#define Oi_star20					0x20	//Overloaded
#define Oi_USE_NEW_INIT_ROUTINES	0x40	//Uses Windows NT3.5 Beta2+ init routines
#define Oi_Unused					0x80	//Unused

//oi_flags values : the following flags are overloaded
#define ENCODE_IS_USED				0x10	//Used only in pickling
#define DECODE_IS_USED				0x20	//Used only in pickling
#define Oi_IGNORE_OBJECT_EXCEPTION_HANDLING	0x10	//Not used any more (old OLE)
#define	Oi_HAS_COMM_OR_FAULT		0x20	//In raw RPC only, [comm _, fault_status]
#define Oi_OBJ_USE_V2_INTERPRETER	0x20	//In DCOM only, use -Oif interpreter






typedef struct Oi_Header_1stPart_t
{
	unsigned char		handle_type;	// voir handle_type values
	unsigned char		oi_flags;		// voir oi_flags values
}Oi_Header_1stPart_t;


typedef struct Oi_Header_RpcFlags_t
{
	unsigned char rpc_flags[4];
}Oi_Header_RpcFlags_t;

typedef struct Oi_Header_3rdPart_t
{
	unsigned short		procNum;		// provides the procedure's procedure number
	unsigned short		stack_size;		/* provides the total size of all parameters on the stack, 
										including any this pointer and/or return value */
}Oi_Header_3rdPart_t;

#define OI_EXT_HEADER_BASE_SIZE	6
#define OI_EXT_HEADER_OIFLAGS_SIZE 4 // si oi_flags contient flag Oi_HAS_RPCFLAGS
typedef struct Oi_Header_t
{
	unsigned char			handle_type;	// voir handle_type values
	unsigned char			oi_flags;		// voir oi_flags values
	Oi_Header_RpcFlags_t *	pRpcFlags;		// != NULL si oi_flags contient Oi_HAS_RPCFLAGS sinon NULL
	unsigned short			procNum;		// provides the procedure's procedure number
	unsigned short			stack_size;		/* provides the total size of all parameters on the stack, 
											including any this pointer and/or return value */
	ExplicitHandle_t *	explicitHandleDescr; /* NULL si handle_type != FC_EXPLICIT_HANDLE */
}Oi_Header_t;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// OIF header types
//------------------------------------------------------------------------------
//interpreter_opt_flag values
typedef struct INTERPRETER_OPT_FLAGS
{
	unsigned char   ServerMustSize      : 1;    // 0x01 set if the server needs to perform a buffer sizing pass
	unsigned char   ClientMustSize      : 1;    // 0x02 set if the client needs to perform a buffer sizing pass
	unsigned char   HasReturn           : 1;    // 0x04 set if the procedure has a return value
	unsigned char   HasPipes            : 1;    // 0x08 set if the pipe package needs to be used to support a pipe argument
	unsigned char   Unused              : 1;
	unsigned char   HasAsyncUuid        : 1;    // 0x20 set if the procedure is a, asynchronous DCOM procedure
	unsigned char   HasExtensions       : 1;    // 0x40 indicates that Windows 2000 and later extensions are used, voir Win2kExt_Header_t
	unsigned char   HasAsyncHandle      : 1;    /* 0x80 indicates an asynchronous RPC procedure 
												The HasAsyncHandle bit has been initially used for a different DCOM implementation 
												of async support, and hence could not be used for the current style async support 
												in DCOM. The HasAsyncUuid bit currently indicates this. */
} INTERPRETER_OPT_FLAGS, *PINTERPRETER_OPT_FLAGS;

#define OIF_EXT_HEADER_SIZE	6
typedef struct Oif_Header_t
{
	//	Oi_Header_t				oiHeader;					// Voir Oi_Header_t
	unsigned short			constant_client_buffer_size;/* Provides the size of the marshaling buffer that could have
														been precomputed by the compiler. This may be only a partial size,
														as the CLIENT_MUST_SIZE flag triggers the sizing. */
	unsigned short			constant_server_buffer_size;/* Provides the size of the marshaling buffer that could have
														been precomputed by the compiler. This may be only a partial size,
														as the SERVER_MUST_SIZE flag triggers the sizing. */
	INTERPRETER_OPT_FLAGS	interpreter_opt_flag;		// Voir interpreter_opt_flag values
	unsigned char			number_of_param;			// Nombre de paramètres décrits de la procédure, return compris
}Oif_Header_t, OIF_HEADER_T;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// WIN2KEXT header types
//------------------------------------------------------------------------------
//interpreter_opt_flag2 values
typedef struct INTERPRETER_OPT_FLAGS2
{
	unsigned char   HasNewCorrDesc      : 1;    /* 0x01 The HasNewCorrDesc member indicates whether new correlation descriptors 
												are used in the format strings generated by the compiler. The new correlation
												descriptor is related to the denial-of-attack functionality. The ClientCorrCheck
												and ServerCorrCheck members are set when the routine needs the correlation check
												on the indicated side. */
	unsigned char   ClientCorrCheck     : 1;    /* 0x02 The ClientCorrCheck member is a cache size hint on the client side and
												ServerCorrCheck is a hint on the server side. When the size comes out as zero,
												a default size should be used. */
	unsigned char   ServerCorrCheck     : 1;    /* 0x04 The ClientCorrCheck member is a cache size hint on the client side and
												ServerCorrCheck is a hint on the server side. When the size comes out as zero,
												a default size should be used. */
	unsigned char   HasNotify           : 1;    /* 0x08 The HasNotify and HasNotify2 flags indicate that the routine uses the notify 
												feature as defined by the [notify] and [notify_flag] attributes, respectively. */
	unsigned char   HasNotify2          : 1;    /* 0x10 The HasNotify and HasNotify2 flags indicate that the routine uses the notify 
												feature as defined by the [notify] and [notify_flag] attributes, respectively. */
	unsigned char   HasComplexReturn	: 1;	/* 0x20 Undocumented flag. Unknown purpose */
	unsigned char   HasRangeOnConf		: 1;	/* 0x40 Undocumented flag. If set, check for range information within NDR structures (array...) */
	unsigned char	HasBigByValueParam	: 1;    /* 0x80 Undocumented flag. Purpose unknown so far. */

} INTERPRETER_OPT_FLAGS2, *PINTERPRETER_OPT_FLAGS2;

typedef UINT16 FloatDoubleMask_t;	/* The FloatDoubleMask element addresses the issue of a floating point argument for 64-bit Windows.
									This field is generated only for 64-bit stubs. The mask is needed for the assembly routines that
									download/upload registers from/to the virtual stack to handle floating-point arguments and registers
									properly. The mask consist of 2 bits per argument, or rather per floating-point register.
									The coding is as follows: The least significant bits correspond to the first FP register, the next 2
									bits correspond to the second register, and so on. */

#define WIN2K_EXT_HEADER_32B_SIZE	sizeof(Win2kExt_Header_t) - sizeof(FloatDoubleMask_t)
#define WIN2K_EXT_HEADER_64B_SIZE	sizeof(Win2kExt_Header_t) /* TODO : verifier avec test car valeur 12 extraite de la doc MSDN mais doute sur valeur 
									car somme taille des champs = 10 pas 12... */
typedef struct Win2kExt_Header_t
{
	//	Oif_Header_t			oifHeader;				// Voir Oif_Header_t
	unsigned char			extension_version;		/* Provides the size of the Win2kExt section in bytes. Doing so
													make it possible for the current NDR engine to step over the extension
													section correctly even if the section were to come from a mater compiler
													version with more fields than the current engine understands.
													*/
	INTERPRETER_OPT_FLAGS2	interpreter_opt_flag2;	// Voir interpreter_opt_flag2 values
	unsigned short			clientCorrHint;			/* The ClientCorrCheck member is a cache size hint on the client side and
													ServerCorrCheck is a hint on the server side. When the size comes out as zero,
													a default size should be used. */ 
	unsigned short			serverCorrHint;
	unsigned short			notifyIndex;			// TODO ? : The NotifyIndex element is an index to a notify routine, if one is used.
	FloatDoubleMask_t 		floatDoubleMask;		/* Si extension_version == WIN2K_EXT_HEADER_32B_SIZE floatDoubleMask est non présent,
													sinon si extension_version == WIN2K_EXT_HEADER_64B_SIZE floatDoubleMask est présent */
}Win2kExt_Header_t;



//
// Oi Header
typedef struct _OI_HEADER_BEGINNING_T
{
	BYTE				bHandle_type;
	BYTE				bOi_flags;
}OI_HEADER_BEGINNING_T;

typedef struct _OI_HEADER_END_T
{
	WORD				wProc_num;
	WORD				wStack_size;
	//	[explicit_handle_description<>]
}OI_HEADER_END_T;

typedef struct _OI_HEADER_T
{
	OI_HEADER_BEGINNING_T	beginning;
	DWORD					dwRpc_flags;
	OI_HEADER_END_T			end;
}OI_HEADER_T;

//
// Procedure header
typedef struct  _PROC_HEADER_T
{
	OI_HEADER_T				oiHeader;
	ExplicitHandle_t		explicitHandle;
	OIF_HEADER_T			oifheader;
	Win2kExt_Header_t		win2KextHeader;
}PROC_HEADER_T;



//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// UNION HEADER type (OI / OIF / WIN2KEXT)
//------------------------------------------------------------------------------
typedef union ProcFormatStringHeader_U
{
	Oi_Header_t			oiHeader;
	Oif_Header_t		oifHeader;
	Win2kExt_Header_t	win2kHeader;
}ProcFormatStringHeader_U;

typedef struct ProcFormatStringHeader_t
{
	HeaderID_E					headerTypeID; // Voir HeaderID_E
	unsigned int				headerEffectiveSize; /* OI_EXT_HEADER_BASE_SIZE si headerTypeID == oi 
													 ou OI_EXT_HEADER_OIFLAGS_SIZE  si headerTypeID == oi et Oi_Header_t.oi_flags contient Oi_HAS_RPCFLAGS

													 + OIF_EXT_HEADER_SIZE si headerTypeID == oif ou headerTypeID == win2kext
													 + Win2kExt_Header_t.extension_version si headerTypeID == win2kext */

	ProcFormatStringHeader_U	headerContent; /* Voir Oi_Header_t si HeaderID_E == oi
											   Voir Oif_Header_t si HeaderID_E == oif
											   Voir Win2kExt_Header_t si HeaderID_E == win2kext */
}ProcFormatStringHeader_t;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// PARAM types
//------------------------------------------------------------------------------
// types de param descriptor possibles
typedef enum ParamID_E
{
	paramDescrOi,	// voir typedef struct ParamDescriptor_oi_t, Possible uniquement en environnement 32bits
	paramDescrOif,	// voir typedef struct ParamDescriptor_oif_t
}ParamID_E;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// OI PARAM descriptor types - The -Oi mode is not supported on 64-bit platforms
//------------------------------------------------------------------------------
typedef struct OiParamComplexType_t
{
	unsigned char	stack_size;		/* The stack_size<1> is the size of the parameter on the stack, expressed 
									as the number of integers the parameter occupies on the stack. */
	unsigned short	type_offset;	/* The type_offset<2> field is the offset in the type format string table, 
									indicating the type descriptor for the argument. */

}OiParamComplexType_t;

typedef unsigned char OiParamSimpleType_t;

typedef union OiParamType_U
{
	OiParamSimpleType_t		simple_type;
	OiParamComplexType_t	complex_type;
}OiParamType_U;

// param_direction values
//#define FC_IN_PARAM_BASETYPE		0x4e
//#define FC_RETURN_PARAM_BASETYPE	0x53
//#define FC_IN_PARAM					0x4d
//#define FC_IN_OUT_PARAM				0x50
//#define FC_OUT_PARAM				0x51
//#define FC_RETURN_PARAM				0x52 //A procedure return value
//#define FC_IN_PARAM_NO_FREE_INST	0x4f //An in xmit/rep as a parameter for which no freeing is made.

typedef struct ParamDescriptor_oi_t // The -Oi mode is not supported on 64-bit platforms
{
	unsigned short			param_direction; // Voir param_direction values
	OiParamType_U			paramType; /* Si param_direction == FC_IN_PARAM_BASETYPE ou FC_RETURN_PARAM_BASETYPE
									   Alors paramType est de type OiParamSimpleType_t
									   Sinon paramType est de type OiParamComplexType_t */
}ParamDescriptor_oi_t;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// OIF PARAM descriptor types
//------------------------------------------------------------------------------
//OIF paramAttributes values
typedef struct PARAM_ATTRIBUTES
{
	unsigned short  MustSize            : 1;    // 0x0001 set only if the parameter must be sized
	unsigned short  MustFree            : 1;    // 0x0002 set if the server must call the parameter's NdrFree* routine
	unsigned short  IsPipe              : 1;    // 0x0004
	unsigned short  IsIn                : 1;    // 0x0008
	unsigned short  IsOut               : 1;    // 0x0010
	unsigned short  IsReturn            : 1;    // 0x0020
	unsigned short  IsBasetype          : 1;    /* 0x0040 set for simple types that are being marshaled by the main Oif 
												interpreter loop. In particular, a simple type with a range 
												attribute on it is not flagged as a base type in order to force 
												the range routine marshaling through dispatching using an FC_RANGE 
												token.
												*/
	unsigned short  IsByValue           : 1;    /* 0x0080 set for compound types being sent by value, but is not set for simple 
												types, regardless of whether the argument is a pointer. The compound 
												types for which it is set are structures, unions, transmit_as, 
												represent_as, wire_marshal and SAFEARRAY. In general, the bit was 
												introduced for the benefit of the main interpreter loop in the Oicf
												interpreter, to ensure the nonsimple arguments (refe		rred to as compound 
												type arguments) are properly dereferenced. This bit was never used in 
												previous versions of the interpreter.
												*/
	unsigned short  IsSimpleRef         : 1;    /* 0x0100 set for a parameter that is a reference pointer to anything 
												other than another pointer, and which has no allocate attributes.
												For such a type, the parameter description's type_offset<> field, 
												except for a reference pointer to a base type, provides the offset 
												to the referent's type; the reference pointer is simply skipped.
												*/
	unsigned short  IsDontCallFreeInst  : 1;    // 0x0200 set for certain represent_as parameters whose free instance routines should not be called.
	unsigned short  SaveForAsyncFinish  : 1;    // 0x0400
	unsigned short  IsPartialIgnore     : 1;	/* 0x0800 Undocumented flag. Unknown purpose */
	unsigned short	IsForceAllocate		: 1;	/* 0x1000 Undocumented flag. Unknown purpose */
	unsigned short  ServerAllocSize     : 3;    /* 0xe000 bits are nonzero if the parameter is [ out], [ in], or [ in,out] 
												pointer to pointer, or pointer to enum16, and will be initialized 
												on the server interpreter's stack, rather than using a call to 
												I_RpcAllocate. If nonzero, this value is multiplied by 8 to get 
												the number of bytes for the parameter. Note that doing so means at 
												least 8 bytes are always allocated for a pointer. */

} PARAM_ATTRIBUTES, *PPARAM_ATTRIBUTES;


typedef struct ParamDescrOifBaseType_t
{
	unsigned char type_format_char;		/* For base types, the argument type is given directly by the format character 
										corresponding to the type.  */
	unsigned char unused;
}ParamDescrOifBaseType_t;

typedef unsigned short ParamDescrOifOtherType_t;

typedef union OifParamType_U
{
	ParamDescrOifBaseType_t		base_type_format_char; // Voir ParamDescrOifBaseType_t
	ParamDescrOifOtherType_t	other_type_offset;	/* For other types, the type_offset<2> field gives the offset in
													the type format string table where the type descriptor for the
													argument is located. */
}OifParamType_U;

typedef struct ParamDescriptor_oif_t
{
	PARAM_ATTRIBUTES		paramAttributes;	// Voir paramAttributes values
	unsigned short			stack_offset;		/* Indicates the offset on the virtual argument stack, in bytes. 
												Pour Base types voir ParamDescrOifBaseType_t
												Pour Other types voir OifParamType_U.other_type_offset 
												*/
	OifParamType_U			paramType;			/* Si paramAttributes contient le flag IsBaseType
												Alors paramType est de type ParamDescrOifBaseType_t
												Sinon de type ParamDescrOifOtherType_t */
}ParamDescriptor_oif_t;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// UNION PARAM type (OI / OIF format)
//-----------------------------------------------------------------------------
typedef union ProcFormatStringParam_U
{
	ParamDescriptor_oi_t	oi_Format;
	ParamDescriptor_oif_t	oif_Format;
}ProcFormatStringParam_U;

#define OI_PARAM_BASETYPE_SIZE	2
#define OI_PARAM_OTHERTYPE_SIZE	4
#define OIF_PARAM_SIZE			6
typedef struct ProcFormatStringParam_t
{
	ParamID_E				paramDescrType;			// Voir ParamID_E
	unsigned int			fieldEffectiveSize;		/* Si paramDescrType == paramDescrOi et 
													paramContent.oiFormat.param_direction == FC_IN_PARAM_BASETYPE ou
													paramContent.oiFormat.param_direction == FC_RETURN_PARAM_BASETYPE
													Alors fieldEffectiveSize = OI_PARAM_BASETYPE_SIZE
													Sinon Si paramDescrType == paramDescrOi
													Alors fieldEffectiveSize = OI_PARAM_OTHERTYPE_SIZE

													Si paramDescrType == paramDescrOi
													Alors fieldEffectiveSize = OIF_PARAM_SIZE */
	ProcFormatStringParam_U	paramContent;			/* Voir ParamDescriptor_oi_t si paramDescrType == paramDescrOi
													Voir ParamDescriptor_oif_t si paramDescrType == paramDescrOif */
	struct ProcFormatStringParam_t * nextParam;
}ProcFormatStringParam_t;

//------------------------------------------------------------------------------
// _MIDL_PROC_FORMAT_STRING types :
// MIDL_PROC_FORMAT_STRING
//-----------------------------------------------------------------------------
typedef struct Fabrique_MIDL_PROC_FORMAT_STRING
{
	short			pad;
	unsigned int	cumulatedEffectiveSize;
	ProcFormatStringHeader_t	header;
	ProcFormatStringParam_t	*	param;
}Fabrique_MIDL_PROC_FORMAT_STRING;



//////////////////////////////////////////////////////////////////////////////////
//// _MIDL_TYPE_FORMAT_STRING types
//////////////////////////////////////////////////////////////////////////////////
enum FC_TYPE
{
	FC_ZERO = 0x0,
	FC_BYTE = 0x1,
	FC_CHAR = 0x2,
	FC_SMALL = 0x3,
	FC_USMALL = 0x4,
	FC_WCHAR = 0x5,
	FC_SHORT = 0x6,
	FC_USHORT = 0x7,
	FC_LONG = 0x8,
	FC_ULONG = 0x9,
	FC_FLOAT = 0xA,
	FC_HYPER = 0xB,
	FC_DOUBLE = 0xC,
	FC_ENUM16 = 0xD,
	FC_ENUM32 = 0xE,
	FC_IGNORE = 0xF,
	FC_ERROR_STATUS_T = 0x10,
	FC_RP = 0x11,
	FC_UP = 0x12,
	FC_OP = 0x13,
	FC_FP = 0x14,
	FC_STRUCT = 0x15,
	FC_PSTRUCT = 0x16,
	FC_CSTRUCT = 0x17,
	FC_CPSTRUCT = 0x18,
	FC_CVSTRUCT = 0x19,
	FC_BOGUS_STRUCT = 0x1A,
	FC_CARRAY = 0x1B,
	FC_CVARRAY = 0x1C,
	FC_SMFARRAY = 0x1D,
	FC_LGFARRAY = 0x1E,
	FC_SMVARRAY = 0x1F,
	FC_LGVARRAY = 0x20,
	FC_BOGUS_ARRAY = 0x21,
	FC_C_CSTRING = 0x22,
	FC_C_BSTRING = 0x23,
	FC_C_SSTRING = 0x24,
	FC_C_WSTRING = 0x25,
	FC_CSTRING = 0x26,
	FC_BSTRING = 0x27,
	FC_SSTRING = 0x28,
	FC_WSTRING = 0x29,
	FC_ENCAPSULATED_UNION = 0x2A,
	FC_NON_ENCAPSULATED_UNION = 0x2B,
	FC_BYTE_COUNT_POINTER = 0x2C,
	FC_TRANSMIT_AS = 0x2D,
	FC_REPRESENT_AS = 0x2E,
	FC_IP = 0x2F,
	FC_BIND_CONTEXT = 0x30,
	FC_BIND_GENERIC = 0x31,
	FC_BIND_PRIMITIVE = 0x32,
	FC_AUTO_HANDLE = 0x33,
	FC_CALLBACK_HANDLE = 0x34,
	FC_UNUSED1 = 0x35,
	FC_POINTER = 0x36,
	FC_ALIGNM2 = 0x37,
	FC_ALIGNM4 = 0x38,
	FC_ALIGNM8 = 0x39,
	FC_UNUSED2 = 0x3A,
	FC_UNUSED3 = 0x3B,
	FC_SYSTEM_HANDLE = 0x3C,
	FC_STRUCTPAD1 = 0x3D,
	FC_STRUCTPAD2 = 0x3E,
	FC_STRUCTPAD3 = 0x3F,
	FC_STRUCTPAD4 = 0x40,
	FC_STRUCTPAD5 = 0x41,
	FC_STRUCTPAD6 = 0x42,
	FC_STRUCTPAD7 = 0x43,
	FC_STRING_SIZED = 0x44,
	FC_UNUSED5 = 0x45,
	FC_NO_REPEAT = 0x46,
	FC_FIXED_REPEAT = 0x47,
	FC_VARIABLE_REPEAT = 0x48,
	FC_FIXED_OFFSET = 0x49,
	FC_VARIABLE_OFFSET = 0x4A,
	FC_PP = 0x4B,
	FC_EMBEDDED_COMPLEX = 0x4C,
	FC_IN_PARAM = 0x4D,
	FC_IN_PARAM_BASETYPE = 0x4E,
	FC_IN_PARAM_NO_FREE_INST = 0x4F,
	FC_IN_OUT_PARAM = 0x50,
	FC_OUT_PARAM = 0x51,
	FC_RETURN_PARAM = 0x52,
	FC_RETURN_PARAM_BASETYPE = 0x53,
	FC_DEREFERENCE = 0x54,
	FC_DIV_2 = 0x55,
	FC_MULT_2 = 0x56,
	FC_ADD_1 = 0x57,
	FC_SUB_1 = 0x58,
	FC_CALLBACK = 0x59,
	FC_CONSTANT_IID = 0x5A,
	FC_END = 0x5B,
	FC_PAD = 0x5C,
	FC_EXPR = 0x5D,
	FC_SPLIT_DEREFERENCE = 0x74,
	FC_SPLIT_DIV_2 = 0x75,
	FC_SPLIT_MULT_2 = 0x76,
	FC_SPLIT_ADD_1 = 0x77,
	FC_SPLIT_SUB_1 = 0x78,
	FC_SPLIT_CALLBACK = 0x79,
	FC_HARD_STRUCT = 0xB1,
	FC_TRANSMIT_AS_PTR = 0xB2,
	FC_REPRESENT_AS_PTR = 0xB3,
	FC_USER_MARSHAL = 0xB4,
	FC_PIPE = 0xB5,
	FC_BLKHOLE = 0xB6,
	FC_RANGE = 0xB7,
	FC_INT3264 = 0xB8,
	FC_UINT3264 = 0xB9,
	FC_END_OF_UNIVERSE = 0xBA,
};







//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// correlation descriptors
//------------------------------------------------------------------------------
/* A correlation descriptor is a format string that describes an expression based 
on one argument related to another argument. A correlation descriptor is needed 
for handling semantics related to attributes like [ size_is()], [ length_is()], 
[ switch_is()] and [ iid_is()]. Correlation descriptors are used with arrays, 
sized pointers, unions, and interface pointers. The eventual expression value can 
be a size, a length, a union discriminant, or a pointer to an IID, respectively. 
In terms of format strings, correlation descriptors are used with arrays, unions, 
and interface pointers. A sized pointer is described in format strings as a 
pointer to an array.

Correlation descriptors have been designed to support only very limited expressions.
For complicated situations, the compiler generates an expression evaluation routine 
to be called by the engine when needed.
*/

// correlation types
#define FC_NORMAL_CONFORMANCE			0x00	/* A normal case of conformance, such as that 
described in a field of a structure. */
#define FC_POINTER_CONFORMANCE			0x10	/* For attributed pointers ( size_is(), length_is())
which are fields in a structure. This affects the 
way the base memory pointer is set. */
#define FC_TOP_LEVEL_CONFORMANCE		0x20	/* For top-level conformance described by another parameter. */
#define FC_CONSTANT_CONFORMANCE			0x40	/* For a constant value. The compiler precalculates the value 
from a constant expression supplied by the user. When this 
is the case, the subsequent 3 bytes in the conformance 
description contain the lower 3 bytes of a long describing 
the conformance size. No further computation is required. */
#define FC_TOP_LEVEL_MULTID_CONFORMANCE	0x80	/* For top-level conformance of a multidimensional array described
by another parameter.
Note  Multidimensional sized arrays and pointers trigger a 
switch to Oicf. */
#define CORR_TYPE_4_LOWER_NIBBLE_MASK	0x0F
#define CORR_TYPE_4_UPPER_NIBBLE_MASK	0xF0

typedef  struct  _NDR_CORRELATION_FLAGS
{
	unsigned short   Early		: 1;	/* indicates early versus late correlation. An early correlation is when the expression
										argument precedes the described argument, for example a size argument is before a sized 
										pointer argument. A late correlation is when the expression argument comes after the related
										argument. The engine performs validation of early correlation values right away, the late
										correlation values are stored for checking after unmarshaling is done. */
	unsigned short   Split		: 1;	/* indicates an async split among [in] and [out] arguments. For example, a size argument may be
										[in] while the sized pointer may be [out]. In the DCOM async context, these arguments happen
										to be on different stacks, so the engine must be aware of this. */
	unsigned short   IsIidIs	: 1;	/* indicates an iid_is() correlation. The NdrComputeConformance routine is tricked to obtain a 
										pointer to IID as an expression value, but the validation routine cannot compare such values 
										(they would be pointers) and so the flag indicates that actual IIDs need to be compared. */
	unsigned short   DontCheck	: 1;
	unsigned short	 Ranged		: 1; /* Undocumented flag. Something about range on conformance */
	unsigned short   Unused		: 11;
} NDR_CORRELATION_FLAGS;

// correlation_operator values
//#define FC_DEREFERENCE	0x54 // The FC_DEREFERENCE constant is used for correlation being a pointee, like for [size_is(*pL)].
//#define FC_DIV_2		0x55 // Arithmetic operators just use the indicated constant. 
//#define FC_MULT_2		0x56 // Arithmetic operators just use the indicated constant. 
//#define FC_ADD_1		0x57 // Arithmetic operators just use the indicated constant. 
//#define FC_SUB_1		0x58 // Arithmetic operators just use the indicated constant. 
//#define FC_CALLBACK		0x59 // The FC_CALLBACK constant indicates that an expression evaluation routine needs to be called.
//#define FC_EXPR			0x5D




typedef struct CorrelationDescriptorNonRobust_t
{
	unsigned char	correlation_type;	/* Voir correlation types pour les 4 bits de poids fort
										FC_LONG | FC_ULONG | FC_SHORT | FC_USHORT | FC_SMALL 
										| FC_USMALL | FC_HYPER pour les 4 bits de poids faible */
	/* Note  64-bit expressions are not supported. FC_HYPER is used only for iid_is() on 64-bit platforms to extract the pointer value for IID*. */
	/* The compiler sets the type nibble to zero for the following cases: constant expression mentioned above and when evaluation expression routine
	needs to be called, for example, when FC_CONSTANT_CONFORMANCE and FC_CALLBACK are used. */

	unsigned char	correlation_operator; // voir correlation_operator values
	INT16			offset;
	/* The offset<2> field is typically a relative memory offset to the expression argument variable. It can also be an expression evaluationroutine
	index. As mentioned previously in this document, for constant expressions it is a part of actual, final expression value.

	The interpretation of the offset<2> field as memory offset depends on the complexity of the expression, the location of the expression variable, 
	and in the case of an array, whether the array is actually an attributed pointer.

	If the array is an attributed pointer and the conformance variable is a field in a structure, the offset field contains the offset from the beginning 
	of the structure to the conformance-describing field. If the array is not an attributed pointer and the conformance variable is a field in a structure,
	the offset field contains the offset from the end of the nonconformant part of the structure to the conformance-describing field. Typically, the 
	conformant array is at the end of the structure.

	For top-level conformance, the offset field contains the offset from the stub's first parameter's location on the stack to the parameter that describes
	the conformance. This is not used in Os mode. There are other exceptions to the interpretation of the offset field; such exceptions are described in 
	the description of those types.

	When offset<2> is used with FC_CALLBACK, it contains an index in the expression evaluation routine table generated by the compiler. The stub message 
	is passed to the evaluation routine, which then computes the conformance value and assigns it to the MaxCount field of the stub message.
	*/
}CorrelationDescriptorNonRobust_t;

typedef struct CorrelationDescriptorRobust_t
{
	unsigned char			correlation_type;	/* Voir correlation types pour les 4 bits de poids fort
												FC_LONG | FC_ULONG | FC_SHORT | FC_USHORT | FC_SMALL 
												| FC_USMALL | FC_HYPER pour les 4 bits de poids faible */
	/* Note  64-bit expressions are not supported. FC_HYPER is used only for iid_is() on 64-bit platforms to extract the pointer value for IID*. */
	/* The compiler sets the type nibble to zero for the following cases: constant expression mentioned above and when evaluation expression routine
	needs to be called, for example, when FC_CONSTANT_CONFORMANCE and FC_CALLBACK are used. */

	unsigned char			correlation_operator; // voir correlation_operator values
	INT16					offset;
	/* The offset<2> field is typically a relative memory offset to the expression argument variable. It can also be an expression evaluationroutine
	index. As mentioned previously in this document, for constant expressions it is a part of actual, final expression value.

	The interpretation of the offset<2> field as memory offset depends on the complexity of the expression, the location of the expression variable, 
	and in the case of an array, whether the array is actually an attributed pointer.

	If the array is an attributed pointer and the conformance variable is a field in a structure, the offset field contains the offset from the beginning 
	of the structure to the conformance-describing field. If the array is not an attributed pointer and the conformance variable is a field in a structure,
	the offset field contains the offset from the end of the nonconformant part of the structure to the conformance-describing field. Typically, the 
	conformant array is at the end of the structure.

	For top-level conformance, the offset field contains the offset from the stub's first parameter's location on the stack to the parameter that describes
	the conformance. This is not used in Os mode. There are other exceptions to the interpretation of the offset field; such exceptions are described in 
	the description of those types.

	When offset<2> is used with FC_CALLBACK, it contains an index in the expression evaluation routine table generated by the compiler. The stub message 
	is passed to the evaluation routine, which then computes the conformance value and assigns it to the MaxCount field of the stub message.
	*/
	NDR_CORRELATION_FLAGS	robust_flags;
}CorrelationDescriptorRobust_t;


#define ROBUST_DELTA  (sizeof(CorrelationDescriptorRobust_t) - sizeof(CorrelationDescriptorNonRobust_t))

typedef union CorrelationDescriptor_U
{
	CorrelationDescriptorNonRobust_t	corrDescNonRobust;
	CorrelationDescriptorRobust_t		corrDescRobust;
}CorrelationDescriptor_U;


//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types pointeurs
//------------------------------------------------------------------------------

// pointer_attributes values
enum FC_POINTER_ATTRIBUTE
{
	FC_ALLOCATE_ALL_NODES=0x1,	// The pointer is a part of an allocate(all_nodes) allocation scheme.
	FC_DONT_FREE=0x2,			// An allocate(dont_free) pointer
	FC_ALLOCED_ON_STACK=0x4,	// A pointer whose referent is allocated on the stub's stack.
	FC_SIMPLE_POINTER=0x8,		/* A pointer to a simple type or nonsized conformant string. This 
								//flag being set indicates layout of the pointer description as the 
								//simple pointer layout described above, otherwise the descriptor 
								//format with the offset is indicated. */
								FC_POINTER_DEREF=0x10,		// A pointer that must be dereferenced before handling the pointer's referent.
								/* Pointers that have size_is(), max_is(), length_is(), last_is(), and/or first_is() applied to them have 
								format string descriptions identical to a pointer to an array of the appropriate type (for example, a 
								conformant array if size_is() is applied, a conformant varying array if size_is() and length_is are applied).
								*/
								FC_MAYBENULL_SIZEIS=0x20	// Undocumented flag.
};


typedef struct CommonPtrSimple_t
{
	unsigned char pointerType;			// Voir pointer_type values : FC_RP ou FC_UP ou FC_FP ou FC_OP
	unsigned char pointer_attributes;	// Voir pointer_attributes values
	unsigned char simple_type;			// Voir simple_types values
	unsigned char pad;					// FC_PAD
}CommonPtrSimple_t;
#define SIZE_COMMON_PTR_SIMPLE	sizeof(CommonPtrSimple_t)

typedef struct CommonPtrComplex_t
{
	unsigned char	pointerType;		// Voir pointer_type values : FC_RP ou FC_UP ou FC_FP ou FC_OP
	unsigned char	pointer_attributes; // Voir pointer_attributes values
	INT16	offset_to_complex_description; // Offset vers la description du type dans MIDL_TYPE_FORMAT_STRING
}CommonPtrComplex_t;


typedef struct InterfacePtrConstantIID_t
{
	unsigned char	pointerType;			// Voir pointer_type values : FC_IP
	unsigned char	fc_constant_IID_code;	// FC_CONSTANT_IID
	GUID			iid;					/* The iid<16> part is the actual IID for the interface pointer. The iid 
											is written to the format string in a format identical to the GUID data 
											structure: long, short, short, char [8]. */
}InterfacePtrConstantIID_t;

typedef struct InterfacePtrNonConstantIID_t
{
	unsigned char	pointerType;				// Voir pointer_type values : FC_IP
	unsigned char	pad;						// FC_PAD
	CorrelationDescriptor_U	iid_description;	/* CorrelationDescriptorRobust_t si /robust
												Sinon CorrelationDescriptorNonRobust_t */
}InterfacePtrNonConstantIID_t;


typedef struct ByteCountHeader_T
{
	unsigned char			pointerType;			// FC_BYTE_COUNT_POINTER
	unsigned char			simpleTypeOrPad;		// simple type or FC_PAD
}ByteCountHeader_T;

typedef struct ByteCountPtrSimple_t
{
	ByteCountHeader_T		byteCountHeader;
	CorrelationDescriptor_U	byte_count_description; /* CorrelationDescriptorRobust_t si /robust
													Sinon CorrelationDescriptorNonRobust_t */
}ByteCountPtrSimple_t;

typedef struct ByteCountPtrComplex_t
{
	ByteCountHeader_T		byteCountHeader;
	CorrelationDescriptor_U	byte_count_description; /* CorrelationDescriptorRobust_t si /robust
													Sinon CorrelationDescriptorNonRobust_t */
	unsigned char *			pointee_description;	//TODO : taille de ce parametre ?
}ByteCountPtrComplex_t;

typedef union PointerTypeContent_U
{
	CommonPtrSimple_t				commonPtrSimple;
	CommonPtrComplex_t				commonPtrComplex;
	InterfacePtrConstantIID_t		interfPtrConstantIID;
	InterfacePtrNonConstantIID_t	interfPtrNonConstantIID;
	ByteCountPtrSimple_t			byteCountPtrSimple;
	ByteCountPtrComplex_t			byteCountPtrComplex;
}PointerTypeContent_U;


//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types pointer layout
//------------------------------------------------------------------------------
// Pointer layout describes pointers of a structure or an array.


/* A pointer layout has a variable size and is composed of 
- a PointerLayoutHeader_t
- one or several pointer instance layout which can be 3 different structures with different size :
- SingleInstancePtrToSimpleType_t wich contains one PointerInstance_t
- FixedRepeatPtr_t + one or several additional PointerInstance_t
- VariableRepeatPtr_t  + one or several additional PointerInstance_t
- a PointerLayoutEnd_t
*/
#define POINTER_LAYOUT_MIN_SIZE		(sizeof(PointerLayoutHeader_t) + sizeof(SingleInstancePtrToSimpleType_t) + sizeof(PointerLayoutEnd_t))
typedef struct PointerLayoutHeader_t
{
	UINT8			pointerType;				// FC_PP
	UINT8			pad;						// FC_PAD
}PointerLayoutHeader_t;

// one or several pointer instance layout, see SingleInstancePtrToSimpleType_t, FixedRepeatPtrHeader_t and VariableRepeatPtrHeader_t below

typedef UINT8 PointerLayoutEnd_t;				// FC_END


// a pointer instance layout contains at least one PointerInstance_t
typedef union PointerDescription_U
{
	CommonPtrSimple_t	commonPtrSimple;
	CommonPtrComplex_t	commonPtrComplex;
}PointerDescription_U;

typedef struct PointerInstance_t
{
	INT16					offsetToPtrInMemory;	/* The signed offset to the pointer's location in memory. For a pointer residing
													in a structure, this offset is a negative offset from the end of the structure
													(the end of the nonconformant portion of conformant structures); for arrays, 
													the offset is from the beginning of the array.
													*/
	INT16					offsetToPtrInBuffer;	/* The signed offset to the pointer's location in the buffer. For a pointer residing
													in a structure, this offset is a negative offset from the end of the structure 
													(the end of the nonconformant portion of conformant structures): for arrays, the 
													offset is from the beginning of the array.
													*/
	PointerDescription_U	pointerDescription;		/* d'après la doc MSDN partie Pointer Layout, 
													pointerDescription est de taille 4 ce qui correspond à la taille des types
													commonPtrSimple et commonPtrComplex. 
													TODO : valider cette hypothèse au cours des tests
													*/
}PointerInstance_t;


typedef struct SingleInstancePtrToSimpleType_t
{
	UINT8					repeatType;			// FC_NO_REPEAT
	UINT8					pad;				// FC_PAD
	PointerInstance_t		pointerInstance;	// voir PointerInstance_t
}SingleInstancePtrToSimpleType_t;


#define OFFSET_SINGLE_INSTANCE_TO_PTR_DESC FIELD_OFFSET(SingleInstancePtrToSimpleType_t , pointerInstance) + FIELD_OFFSET(PointerInstance_t,pointerDescription)


//For fixed repeat and variable repeat pointer instances there is a set of offset and pointer descriptions for each pointer in the repeat instance.
typedef struct FixedRepeatPtr_t
{
	UINT8					repeatType;			// FC_FIXED_REPEAT
	UINT8					pad;				// FC_PAD
	UINT16					iterations;			// Total number of pointers that have the same layout<> described.
	UINT16					increment;			// Increment between successive pointers during a REPEAT.
	INT16					offsetToArray;		/* Offset from an enclosing structure to the embedded array whose
												pointers are being handled. For top-level arrays, this field will always be zero. */
	UINT16					numberOfPointers;	// Number of different pointers in a repeat instance.
	//PointerInstance_t		pointerInstance;	/* Several pointerInstance can follow according to iterations, increment and numberOfPointers.
												//	PointerInstance_t		pointerInstance;	   See PointerInstance_t */
}FixedRepeatPtr_t;

//For fixed repeat and variable repeat pointer instances there is a set of offset and pointer descriptions for each pointer in the repeat instance.
typedef struct VariableRepeatPtr_t
{
	UINT8					repeatType0;		// FC_VARIABLE_REPEAT
	UINT8					repeatType1;		// FC_FIXED_OFFSET or FC_VARIABLE_OFFSET
	UINT16					increment;			// Increment between successive pointers during a REPEAT.
	INT16					offsetToArray;		/* Offset from an enclosing structure to the embedded array whose
												pointers are being handled. For top-level arrays, this field will always be zero. */
	UINT16					numberOfPointers;	// Number of different pointers in a repeat instance.
	//PointerInstance_t		pointerInstance;	/* Several pointerInstance can follow according to iterations, increment and numberOfPointers.
												//	PointerInstance_t		pointerInstance;	   See PointerInstance_t */
}VariableRepeatPtr_t;


typedef union PointerInstanceLayout_U
{
	SingleInstancePtrToSimpleType_t	singleInstPtrToSimpleType;	// See ptrLayoutSingleInstancePtrToSimpleTypeDescr
	FixedRepeatPtr_t				fixedRepeatPtr;				// See ptrLayoutFixedRepeatPtrDescr
	VariableRepeatPtr_t				variableReapeatPtr;			// See ptrLayoutVariableRepeatPtrDescr
}PointerInstanceLayout_U;



//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types arrays
//------------------------------------------------------------------------------
// array types values
//#define FC_CARRAY		0x1b	// TODO a confirmer - conformant array
//#define FC_CVARRAY		0x1c	// TODO a confirmer - conformant varying array
//#define FC_SMFARRAY		0x1d	// TODO a confirmer - small (<64K) fixed array
//#define FC_LGFARRAY		0x1e	// TODO a confirmer - large (>= 64K) fixed array
//#define FC_SMVARRAY		0x1f	// TODO a confirmer - small (<64K) varying array
//#define FC_LGVARRAY		0x20	// TODO a confirmer - large (>= 64K) varying array
//#define FC_BOGUS_ARRAY	0x21	// TODO a confirmer - complex array


/* An array descriptor has a variable size and is composed of 
- an ArrayDescrHeader_U according to the type of array (See SMFixedSizedArrayHeader_t, LGFixedSizedArrayHeader_t, 
ConformantArrayHeader_t, ConformantVaryingArrayHeader_t, SMVaryingArrayHeader_t, LGVaryingArrayHeader_t, ComplexArrayHeader_t)
- an optional pointer layout (See PointerInstanceLayout_U)
- an ArrayDescrEnd_t
*/



/* A fixed-sized array format string is generated for arrays that have a known size, 
and therefore may be block-copied to the marshaling buffer. */
typedef struct SMFixedSizedArrayHeader_t
{
	UINT8			arrayDescrType;		// FC_SMFARRAY
	UINT8			alignment;			
	UINT16			totalSize;			/* Total size of the array in memory, in bytes. This is the same as wire size after alignment.
										The total size is calculated for categories for which the padding issue does not exist and 
										the size is actual array size. */
}SMFixedSizedArrayHeader_t;

/* A fixed-sized array format string is generated for arrays that have a known size, 
and therefore may be block-copied to the marshaling buffer. */
#define LGFIXED_ARRAY_HEADER_SIZE	6
typedef struct LGFixedSizedArrayHeader_t
{
	UINT8			arrayDescrType;		// FC_LGFARRAY
	UINT8			alignment;			
	UINT32 			totalSize;			/* Total size of the array in memory, in bytes. This is the same as wire size after alignment.
										The total size is calculated for categories for which the padding issue does not exist and 
										the size is actual array size. */
}LGFixedSizedArrayHeader_t;

// The conformance_description<> is a correlation descriptor and has 4 or 6 bytes depending on whether /robust is used.
#define CONFORMANT_ARRAY_HEADER_SIZE_NONROBUST	(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(CorrelationDescriptorNonRobust_t))
#define CONFORMANT_ARRAY_HEADER_SIZE_ROBUST		(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(CorrelationDescriptorRobust_t))
typedef struct ConformantArrayHeader_t
{
	UINT8					arrayDescrType;		// FC_CARRAY
	UINT8					alignment;		
	UINT16					elementSize;		// Element size in memory of a single element of the array, including padding (this may happen for complex arrays only).
	//CorrelationDescriptor_U	conformanceDescription; // See CorrelationDescriptor_U
}ConformantArrayHeader_t;

// The conformance_description<> is a correlation descriptor and has 4 or 6 bytes depending on whether /robust is used.
#define CONFORMANT_VAR_ARRAY_HEADER_SIZE_NONROBUST	(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(CorrelationDescriptorNonRobust_t) + sizeof(CorrelationDescriptorNonRobust_t))
#define CONFORMANT_VAR_ARRAY_HEADER_SIZE_ROBUST		(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(CorrelationDescriptorRobust_t) + sizeof(CorrelationDescriptorRobust_t))
typedef struct ConformantVaryingArrayHeader_t
{
	UINT8					arrayDescrType;		// FC_CVARRAY
	UINT8					alignment;		
	UINT16					elementSize;		// Element size in memory of a single element of the array, including padding (this may happen for complex arrays only)
	//	CorrelationDescriptor_U	conformanceDescription;	// See CorrelationDescriptor_U
	//	CorrelationDescriptor_U	varianceDescription;	// See CorrelationDescriptor_U
}ConformantVaryingArrayHeader_t;

// The conformance_description<> is a correlation descriptor and has 4 or 6 bytes depending on whether /robust is used.
#define CONFORMANT_SMVAR_ARRAY_HEADER_SIZE_NONROBUST	(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(UINT16) + sizeof(UINT16) + sizeof(CorrelationDescriptorNonRobust_t))
#define CONFORMANT_SMVAR_ARRAY_HEADER_SIZE_ROBUST		(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT16) + sizeof(UINT16) + sizeof(UINT16) + sizeof(CorrelationDescriptorRobust_t))
typedef struct SMVaryingArrayHeader_t
{
	UINT8					arrayDescrType;			// FC_SMVARRAY
	UINT8					alignment;		
	UINT16					totalSize;				/* Total size of the array in memory, in bytes. This is the same as wire size after alignment.
													The total size is calculated for categories for which the padding issue does not exist and 
													the size is actual array size. */
	UINT16					numberOfElements;	
	UINT16					elementSize;			// Element size in memory of a single element of the array, including padding (this may happen for complex arrays only)
	//	CorrelationDescriptor_U	varianceDescription;	// See CorrelationDescriptor_U
	/* For varying arrays embedded inside of a structure, the offset<2> field of the variance_description<> is a relative offset from the varying array's position in the
	structure to the variance describing field. The offset is typically relative to the beginning of the structure. */
}SMVaryingArrayHeader_t;

// The conformance_description<> is a correlation descriptor and has 4 or 6 bytes depending on whether /robust is used.
#define CONFORMANT_LGVAR_ARRAY_HEADER_SIZE_NONROBUST	(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT32) + sizeof(UINT32) + sizeof(UINT16) + sizeof(CorrelationDescriptorNonRobust_t))
#define CONFORMANT_LGVAR_ARRAY_HEADER_SIZE_ROBUST		(sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT32) + sizeof(UINT32) + sizeof(UINT16) + sizeof(CorrelationDescriptorRobust_t))
typedef struct LGVaryingArrayHeader_t
{
	UINT8					arrayDescrType;			// FC_LGVARRAY
	UINT8					alignment;		
	UINT32					totalSize;				/* Total size of the array in memory, in bytes. This is the same as wire size after alignment.
													The total size is calculated for categories for which the padding issue does not exist and 
													the size is actual array size. */
	UINT32					numberOfElements;	
	UINT16					elementSize;			// Element size in memory of a single element of the array, including padding (this may happen for complex arrays only)
	//	CorrelationDescriptor_U	varianceDescription;	// See CorrelationDescriptor_U
	/* For varying arrays embedded inside of a structure, the offset<2> field of the variance_description<> is a relative offset from the varying array's position in the
	structure to the variance describing field. The offset is typically relative to the beginning of the structure. */
}LGVaryingArrayHeader_t;




/* A complex array is any array with an element that prevents it from being block-copied, and as such, additional action needs to be taken. These elements make an array complex:

simple types: ENUM16, __INT3264 (on 64-bit platforms only), an integral with [ range] 
reference and interface pointers (all pointers on 64-bit platforms) 
unions 
complex structures (see the Complex Structure Description topic for a full list of reasons for a structure to be complex) 
elements defined with [ transmit_as], [ user_marshal] 
All multidimensional arrays with at least one conformant and/or varying dimension are complex regardless of the underlying element type.
*/
typedef struct ComplexArrayHeader_t
{
	UINT8					arrayDescrType;			// FC_BOGUS_ARRAY
	UINT8					alignment;		
	UINT16					numberOfElements;		// The number_of_elements<2> field is zero if the array is conformant.
	//	CorrelationDescriptor_U	conformanceDescription;	// See CorrelationDescriptor_U
	//	CorrelationDescriptor_U	varianceDescription;	// See CorrelationDescriptor_U
	/* If the array has conformance and/or variance then the conformance_description<> and/or variance_description<> field(s) 
	have valid descriptions, otherwise the first 4 bytes of the correlation descriptor are set to 0xFFFFFFFF. The flags, when present, are set to zero. */
}ComplexArrayHeader_t;

#define ARRAY_DESCR_HEADER_MIN_SIZE		sizeof(SMFixedSizedArrayHeader_t)
typedef union ArrayDescrHeader_U
{
	SMFixedSizedArrayHeader_t		smFixedSizedArrayHdr;		// See arraySMFixedSizedDescr
	LGFixedSizedArrayHeader_t		lgFixedSizedArrayHdr;		// See arrayLGFixedSizedDescr
	ConformantArrayHeader_t			conformantArrayHdr;			// See arrayConformantDescr
	ConformantVaryingArrayHeader_t	conformantVaryingArrayHdr;	// See arrayConformantVaryingDescr
	SMVaryingArrayHeader_t			smVaryingArrayHdr;			// See arraySMVaryingDescr
	LGVaryingArrayHeader_t			lgVaryingArrayHdr;			// See arrayLGVaryingDescr
	ComplexArrayHeader_t			complexArray;				// See arrayComplexDescr
}ArrayDescrHeader_U;


typedef struct ArrayDescrEnd_t
{
	unsigned char *			elementDescription;	// TODO : taille de ce parametre ?
	UINT8					end;				// FC_END
}ArrayDescrEnd_t;



//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types string
//------------------------------------------------------------------------------


typedef struct NonConformantStr_t
{
	UINT8			stringType;		// FC_CSTRING | FC_WSTRING
	UINT8			pad;			// FC_PAD
	UINT16			string_size;
}NonConformantStr_t; //An example of nonconformant string is a [string] on a fixed-size array.


// a conformant string describes common strings, like a [string] char * argument. 
/* Si stringType = FC_C_CSTRING | FC_C_WSTRING et StringTypeContent_U.confSizedStr.fc_string_sized_code != FC_STRING_SIZED */
typedef struct ConformantStr_t
{
	UINT8			stringType;		// FC_C_CSTRING | FC_C_WSTRING
	UINT8			pad;			// FC_PAD
}ConformantStr_t;

// a sized conformant string
#define SIZE_OF_CONFORMANT_SIZED_STR (robustFlagWasSet)?sizeof(ConformantSizedStr_t):(sizeof(ConformantSizedStr_t)-ROBUST_DELTA)

typedef struct ConformantSizedStr_t
{
	UINT8					stringType;					// FC_C_CSTRING | FC_C_WSTRING
	UINT8					fc_string_sized_code;		//FC_STRING_SIZED
	CorrelationDescriptor_U	conformance_description;	/* CorrelationDescriptorRobust_t si /robust
														Sinon CorrelationDescriptorNonRobust_t */
}ConformantSizedStr_t;



typedef struct NonConfStructStr_t
{
	UINT8			stringType;				// FC_SSTRING
	UINT8			element_size;
	UINT16			number_of_elements;
}NonConfStructStr_t; //nonconformant string-able structure



//Conformant string-able structure:
/* Si stringType = FC_C_SSTRING et StringTypeContent_U.confSizedStructStr.fc_string_sized_code != FC_STRING_SIZED */
typedef struct ConfStructStr_t
{
	UINT8			stringType;							// FC_C_SSTRING
	UINT8			element_size;
}ConfStructStr_t;

//Conformant sized string-able structure.
typedef struct ConfSizedStructStr_t
{
	UINT8			stringType;							// FC_C_SSTRING
	UINT8			element_size;
	UINT8			fc_string_sized_code;				// FC_STRING_SIZED
	UINT8			pad;								// FC_PAD
	CorrelationDescriptor_U	conformance_description;	/* CorrelationDescriptorRobust_t si /robust
														Sinon CorrelationDescriptorNonRobust_t */
}ConfSizedStructStr_t;


typedef union StringTypeContent_U
{
	ConformantStr_t			confStr;				// Voir strConfStrDescr
	ConformantSizedStr_t	confSizedStr;			// Voir strConfSizedStrDescr
	NonConformantStr_t		nonConfStr;				// Voir strNonConfStrDescr
	NonConfStructStr_t		nonConfStructStr;		// Voir strNonConfStructStrDescr
	ConfStructStr_t			confStructStr;			// Voir strConfStructStrDescr
	ConfSizedStructStr_t	confSizedStructStr;		// Voir strConfSizedStructStrDescr
}StringTypeContent_U;



//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types structures
//------------------------------------------------------------------------------
// struct types values
//#define FC_STRUCT		0x15	// TODO a confirmer - simple structure
//#define FC_PSTRUCT		0x16	// TODO a confirmer - simple structure avec pointeur
//#define FC_CSTRUCT		0x17	// TODO a confirmer - conformant structure
//#define FC_CPSTRUCT		0x18	// TODO a confirmer - conformant structure avec pointeur
//#define FC_CVSTRUCT		0x19	// TODO a confirmer - conformant varying structure
//#define FC_BOGUS_STRUCT 0x1a

/* A structure descriptor has a variable size and is composed of 
- a StructDescrHeader_U according to the type of struct (See SimpleStructHeader_t, SimpleWithPtrsStructHeader_t, 
ConfStructHeader_t, ConfWithPtrsStructHeader_t, ConfVaryingStructHeader_t, HardStructHeader_t, ComplexStructHeader_t)
- a pointer layout only for SimpleWithPtrsStructHeader_t, ConfWithPtrsStructHeader_t, ConfVaryingStructHeader_t (optional)
and ComplexStructHeader_t (optional) See "types pointer layout" described above : variable size.
- a member layout which has variable length (See StructMemberLayout_U)
- a StructDescrEnd_t
*/

/* A simple structure contains only base types, fixed arrays, and other simple structures. The main feature of the simple
structure is that it can be block-copied as a whole. */
typedef struct SimpleStructHeader
{
	UINT8			structType;			// FC_STRUCT
	UINT8			alignment;			/* The necessary alignment of the buffer before unmarshaling the structure.
										Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;		/* Size of the structure in memory in bytes; for conformant structures this 
										size does not include the array's size. */
}SimpleStructHeader_t;

/* A simple structure with pointers contains only base types, pointers, fixed arrays, simple structures, and other simple
structures with pointers. Because the layout<> will only have to be visited when doing an endianess conversion, it is placed 
at the end of the description. */
typedef struct SimpleWithPtrsStructHeader_t
{
	UINT8			structType;			// FC_PSTRUCT
	UINT8			alignment;			/* The necessary alignment of the buffer before unmarshaling the structure.
										Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;		/* Size of the structure in memory in bytes; for conformant structures this 
										size does not include the array's size. */
	// type_pointer_layout	pointer_layout;	// See "types pointer layout" described above : variable size.
	// StructMemberLayout_U	member_layout;	// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;				// FC_END
}SimpleWithPtrsStructHeader_t;

/* A conformant structure contains only base types, fixed arrays, and simple structures, and must contain either a conformant
string or a conformant array. This array could actually be contained in another conformant structure or conformant structure
with pointers which is embedded in this structure. */
typedef struct ConfStructHeader_t
{
	UINT8			structType;					// FC_CSTRUCT
	UINT8			alignment;					/* The necessary alignment of the buffer before unmarshaling the structure.
												Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;				/* Size of the structure in memory in bytes; for conformant structures this 
												size does not include the array's size. */
	INT16			offsetToArrayDescription;	/* Offset from the current format string pointer to the description of the 
												conformant array contained in a structure. */
	// StructMemberLayout_U	member_layout;	// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;				// FC_END	
}ConfStructHeader_t;

/* A conformant structure with pointers contains only base types, pointers, fixed arrays, simple structures, and simple 
structures with pointers; a conformant structure must contain a conformant array. This array could actually be contained 
in another conformant structure or conformant structure with pointers that is embedded in this structure. */
typedef struct ConfWithPtrsStructHeader_t
{
	UINT8			structType;					// FC_CSTRUCT
	UINT8			alignment;					/* The necessary alignment of the buffer before unmarshaling the structure.
												Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;				/* Size of the structure in memory in bytes; for conformant structures this 
												size does not include the array's size. */
	INT16			offsetToArrayDescription;	/* Offset from the current format string pointer to the description of the 
												conformant array contained in a structure. */
	// type_pointer_layout	pointer_layout;			// See "types pointer layout" described above : variable size.
	// StructMemberLayout_U	member_layout;			// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;						// FC_END	
}ConfWithPtrsStructHeader_t;

/* A conformant varying structure contains only simple types, pointers, fixed arrays, simple structures, and simple structures 
with pointers; a conformant varying structure must contain either a conformant string or a conformant-varying array. 
The conformant string or array can actually be contained in another conformant structure or conformant structure with pointers 
that is embedded in this structure. */
typedef struct ConfVaryingStructHeader_t
{
	UINT8			structType;					// FC_CVSTRUCT
	UINT8			alignment;					/* The necessary alignment of the buffer before unmarshaling the structure.
												Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;				/* Size of the structure in memory in bytes; for conformant structures this 
												size does not include the array's size. */
	INT16			offsetToArrayDescription;	/* Offset from the current format string pointer to the description of the 
												conformant array contained in a structure. */
	// type_pointer_layout	pointer_layout;			// See "types pointer layout" described above : variable size. OPTIONAL
	// StructMemberLayout_U	member_layout;			// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;						// FC_END	
}ConfVaryingStructHeader_t;

/* The hard structure was a concept aimed at eliminating steep penalties related to processing complex structures. It is derived
from an observation that a complex structure typically has only one or two conditions that prevent block-copying, and therefore
spoil its performance compared to a simple structure. The culprits are usually unions or enumeration fields.

A hard structure is a structure that has an enum16, end-padding in memory, or a union as the last member. These three elements 
prevent the structure from falling into one of the previous structure categories, which enjoy small interpretation overhead and 
maximum optimization potential, but do not force it into the very costly complex structure category.

The enum16 must not cause the memory and wire sizes of the structure to differ. The structure cannot have any conformant array,
nor any pointers (unless part of the union); the only other members allowed are base types, fixed arrays, and simple structures. */
typedef struct HardStructHeader_t
{
	UINT8			structType;					// FC_HARD_STRUCTURE
	UINT8			alignment;					/* The necessary alignment of the buffer before unmarshaling the structure.
												Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;				/* Size of the structure in memory in bytes; for conformant structures this 
												size does not include the array's size. */
	UINT32			reserved;
	INT16			enumOffset;					/* The enum_offset<2> field provides the offset from the beginning of the
												structure in memory to an enum16 if it contains one; otherwise the 
												enum_offset<2> field is 1. */
	UINT16			copySize;					/* The copy_size<2> field provides the total number of bytes in the structure,
												which may be block-copied into/from the buffer. This total does not include
												any trailing union nor any end-padding in memory. This value is also the amount
												the buffer pointer should be incremented following the copy. */
	UINT16			memCopyIncr;				/* The mem_copy_incr<2> field is the number of bytes the memory pointer should be
												incremented following the block-copy before handling any trailing union. Incrementing
												by this amount (not by copy_size<2> bytes) yields a proper memory pointer to any 
												trailing union. */
	INT16			unionDescriptionOffset;



	// StructMemberLayout_U	member_layout;			// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;						// FC_END
}HardStructHeader_t;

#define NULL_OFFSET	0x00
typedef struct ComplexStructHeader_t
{
	UINT8			structType;					// FC_BOGUS_STRUCT
	UINT8			alignment;					/* The necessary alignment of the buffer before unmarshaling the structure.
												Valid values are 0, 1, 3, and 7 (the actual alignment minus 1). */
	UINT16			memory_size;				/* Size of the structure in memory in bytes; for conformant structures this 
												size does not include the array's size. */
	INT16			offsetToArrayDescription;	/* If the structure contains a conformant array, the offset_to_conformant_array_description<2>
												field provides the offset to the conformant array's description, otherwise it is zero. */
	INT16			offsetToPointerLayout;		/* If the structure has pointers, the offset_to_pointer_layout<2> field provides the offset 
												past the structure's layout to the pointer layout, otherwise this field is zero. */
	// StructMemberLayout_U	member_layout;			// See StructMemberLayout_U : variable size.
	// StructDescrEnd_t	end;						// FC_END	
	// type_pointer_layout	pointer_layout;			// See "types pointer layout" described above : variable size. OPTIONAL
	/* The pointer_layout<> field of a complex structure is handled somewhat differently than for other 
	structures. The pointer_layout<> field of a complex structure only contains descriptions of actual
	pointer fields in the structure itself. Any pointers contained within any embedded arrays, unions,
	or structures are not described in the complex structure's pointer_layout<> field.

	Note  This is in contrast to other structures, which duplicate the description of any pointers 
	contained in embedded arrays or structures in their own pointer _layout<> field as well.

	The format of a complex structure's pointer layout is also radically different. Because it only 
	contains descriptions of actual pointer members and because a complex structure is marshaled and 
	unmarshaled one field at a time, the pointer_layout<> field simply contains the pointer description
	of all pointer members. There is no beginning FC_PP, and none of the usual pointer_layout<> information. */
}ComplexStructHeader_t;

#define STRUCT_DESCR_HEADER_MIN_SIZE		sizeof(SimpleStructHeader_t)
typedef union StructDescrHeader_U
{
	SimpleStructHeader_t			simpleStruct;			// Voir structSimpleDescr
	SimpleWithPtrsStructHeader_t	simpleWithPtrsStruct;	// Voir structSimpleWithPtrsDescr
	ConfStructHeader_t				confStruct;				// Voir structConformantDescr
	ConfWithPtrsStructHeader_t		confWithPtrsStruct;		// Voir structConformantWithPtrsDescr
	ConfVaryingStructHeader_t		confVaryingStruct;		// Voir structConformantVaryingDescr
	HardStructHeader_t				hardStruct;				// Voir structHardDescr
	ComplexStructHeader_t			complexStruct;			// Voir structComplexDescr
}StructDescrHeader_U;

/* Then a struct descriptor contains a member layout which has a variable size :
A structure's layout description contains one or more of the following format characters:
- Any of the base type characters, like FC_CHAR, and so on 
- Alignment directives. There are three format characters specifying alignment of the memory pointer: FC_ALIGNM2, 
FC_ALIGNM4, and FC_ALIGNM8. Note  There are also buffer alignment tokens, FC_ALIGNB2 through FC_ALIGNM8; these are not used.
- Memory padding. These occur only at the end of a structure's description and denote the number of bytes of padding in
memory before the conformant array in the structure: FC_STRUCTPADn, where n is the number of bytes of padding. 
- Any embedded nonbase type (note, however, that a conformant array never occurs in the structure layout). This has a 4-byte description: */ 

typedef UINT8 StructBaseTypeMemberLayout_t;	// base type like FC_CHAR and so on, or structure alignement values, or structure memory padding

// structure alignement values
//#define FC_ALIGNM2			0x37
//#define	FC_ALIGNM4			0x38
//#define FC_ALIGNM8			0x39

// structure memory padding
//#define FC_STRUCTPAD1		0x3d
//#define FC_STRUCTPAD2		0x3e
//#define FC_STRUCTPAD3		0x3f
//#define FC_STRUCTPAD4		0x40
//#define FC_STRUCTPAD5		0x41
//#define FC_STRUCTPAD6		0x42
//#define FC_STRUCTPAD7		0x43

//#define FC_EMBEDDED_COMPLEX	0x4c

typedef struct StructNonBaseTypeMemberLayout_t
{
	UINT8		memberType;				// FC_EMBEDDED_COMPLEX
	UINT8		memoryPad;				// memory_pad<1> is a padding needed in memory before the complex field.
	INT16		offsetToDescription;	/* offset_to_description<2> is a relative type offset to the embedded type.
										where the offset is not guaranteed to be 2-byte aligned. */
}StructNonBaseTypeMemberLayout_t;


typedef union StructMemberLayout_U
{
	StructBaseTypeMemberLayout_t	baseTypeMemberLayout;		// See StructBaseTypeMemberLayout_t
	StructNonBaseTypeMemberLayout_t	nonBaseTypeMemberLayout;	// See StructNonBaseTypeMemberLayout_t
}StructMemberLayout_U;


/* There may also be an FC_PAD before the terminating FC_END if needed to ensure that the format string will be aligned at a 2-byte boundary following the FC_END. */
typedef UINT8 StructDescrEnd_t;			// FC_END

//Following type created in order to manipulate common header members of differents structures types descriptors
typedef struct AllStructDescrCommonHeader_MemberType_Align_MemorySize_t
{
	UINT8			structType;
	UINT8			alignment;
	UINT16			memory_size;
}AllStructDescrCommonHeader_MemberType_Align_MemorySize_t;


//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types union
//------------------------------------------------------------------------------

// UnionArmSelector_t : Both encapsulated and nonencapsulated unions share a common union_arm_selector<> format:
/* An UnionArmSelector_t is composed of	
- a UnionArm_t
- one to several ArmXCase_t
- one DefaultArmDescr_t
*/
#define UNION_ARM_SELTOR_ALIGN_MASK				0xF000
#define UNION_ARM_SELTOR_NB_UNION_MEMBER_MASK	0x0FFF
typedef UINT16 UnionArmSelector_t;			/* The union_arms<2> field consists of two parts. If the union is a MIDL 1.0 style union, 
											the upper 4 bits contain the alignment of the union arm (alignment of greatest aligned arm).
											Otherwise the upper 4 bits are zero. The lower 12 bits contain the number of arms in the union. 
											In other words:
											alignment<highest nibble> arm_counter<three lower nibbles> */

#define ARM_X_CASE_SIZE		(sizeof(UINT32) + sizeof(INT16))
typedef struct ArmXCase_t
{
	UINT32		armCaseValue;
	INT16		offsetToArmDescription;		/* The offset_to_arm_description<2> fields contain a relative signed offset to the arm's type 
											description. However, the field is overloaded with optimization for simple types. For these,
											the upper byte of this offset field is FC_MAGIC_UNION_BYTE (0x80) and the lower byte of the short 											
											is the actual format character type of the arm. As such, there are two ranges for the offset values:
											"80 xx" means that xx is a type format string; and everything else within range (80 FF .. 7f FF)
											means an actual offset. This makes offsets from range <80 00 .. 80 FF > unavailable as offsets.
											The compiler checks that as of MIDL version 5.1.164. */
}ArmXCase_t;

typedef UINT16 DefaultArmDescr_t;			/* The default_arm_description<2> field indicates the type of union arm for the default arm, if any.
											If there is no default arm specified for the union then the default_arm_description<2> field is 0xFFFF
											and an exception is raised if the switch_is value does not match any of the arm case values. If the
											default arm is specified but empty, then the default_arm_description<2> field is zero. Otherwise the 
											default_arm_description<2> field has the same semantics as the offset_to_arm_description<2> fields. */
/* The following is a summary:
0 - empty default 
FFFF - no default 
80xx - simple type 
other - relative offset */

// union types values
//#define FC_ENCAPSULATED_UNION			0x2a
//#define FC_NON_ENCAPSULATED_UNION		0x2b

#define FC_MAGIC_UNION_BYTE				0x8000
#define MIN_UNION_SIMPLE_TYPE_ENCODE	0x8000
#define MAX_UNION_SIMPLE_TYPE_ENCODE	0x80FF
#define FC_MAGIC_UNION_GET_SIMPLE_TYPE	0x00FF
#define NO_DEFAULT_ARM_DESCRIPTION		0xFFFF
#define EMPTY_DEFAULT_ARM_DESCRIPTION	0x0000

typedef struct SizeAndArmDescription_t
{
	UINT16				memorySize;			/* The memory_size<2> field is the size of the union only, and is identical to 
											nonencapsulated unions. To obtain the total size of the structure that contains the union,
											add memory_size<2> to memory increment to step over, that is to the upper nibble of the 
											switch_type<1> field, then align by the alignment corresponding to the increment. */
	// UnionArmSelector_t	union_arm_selector	// See UnionArmSelector_t (composed of UnionArm_t, ArmXCase_t, DefaultArmDescr_t) : Variable size and content
}SizeAndArmDescription_t;

/* An encapsulated union comes from a special union syntax in IDL. Effectively, an encapsulated union is a 
bundle structure with a discriminant field at the beginning of the structure and the union as the only other member. */
#define	ENCAP_UNION_SWITCH_TYPE_TYPE_MASK	0x0F
#define ENCAP_UNION_SWITCH_MEMORY_INCREMENT_MASK 0xF0
typedef struct EncapUnion_t
{
	UINT8					unionType;			// FC_ENCAPSULATED_UNION
	UINT8					switchType;			/* An encapsulated union's switch_type<1> field has two parts. The lower nibble 
												provides the actual switch type, and the upper nibble provides the memory increment
												to step over that is an amount that the memory pointer must be incremented to skip
												past the switch_is field, which includes any padding between the switch_is() field
												of the stub-constructed structure and the actual union field. */
	SizeAndArmDescription_t	sizeAndArmDescr;	// See sizeAndArmDescription_t
}EncapUnion_t;

/* A nonencapsulated union is a typical situation where a union is one argument or field and the switch is another argument or field, respectively. */
#define NON_ENCAP_UNION_NON_ROBUST_HEADER_SIZE	(sizeof(NonEncapUnionHeader_t) + sizeof(CorrelationDescriptorNonRobust_t) + sizeof(nonEncapUnion_offsetToSizeAndArmDescription_t))
#define NON_ENCAP_UNION_ROBUST_HEADER_SIZE		(sizeof(NonEncapUnionHeader_t) + sizeof(CorrelationDescriptorRobust_t) + sizeof(nonEncapUnion_offsetToSizeAndArmDescription_t))

#define NON_ENCAP_UNION_OFFSET_TO_SIZE_AND_ARM_JUST_FOLLOW_UNION_DESCR	0x02
typedef INT16 nonEncapUnion_offsetToSizeAndArmDescription_t;

typedef struct NonEncapUnionHeader_t
{
	UINT8					unionType;						// FC_NON_ENCAPSULATED_UNION
	UINT8					switchType;						// The switch_type<1> field is a format character for the discriminant.
	/*	CorrelationDescriptor_U	switchIsDescription;			/* The switch_is_descriptor<> field is a correlation descriptor and has 4 or 6 bytes 
	depending on whether /robust is used. However, for the switch_is_description<>, if
	the union is embedded in a structure, the offset field of the switch_is_description<>
	is the offset to the switch_is field from the union's position in the structure 
	(not from the beginning of the structure). */
	/*	nonEncapUnion_offsetToSizeAndArmDescription_t offsetToSizeAndArmDescription;	/* The offset_to_size_and_arm_description<2> field gives the offset to the union's size 
	and arm description, which is identical to that for encapsulated unions and is shared 
	by all nonencapsulated unions of the same type. See sizeAndArmDescription_t. */

}NonEncapUnionHeader_t;



/*typedef union UnionDescr_U
{
EncapUnion_t	encapUnion;			// See unionEncapsulatedDescr
NonEncapUnion_t nonEncapUnion;		// See unionNonEncapsulatedDescr
}UnionDescr_U;*/


//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// type Range
//------------------------------------------------------------------------------
typedef struct Range_t
{
	unsigned char			range_type;			// FC_RANGE
	unsigned char			flags_type;
	long					lowValue;
	long					highValue;
}Range_t;


#pragma pack(pop)

//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types Transmit_as and Represent_as
//------------------------------------------------------------------------------
// TODO

typedef struct TransmitRepresentAs_t
{
	unsigned char			type;
	unsigned char			flag;
	short					quintuple_index;
	unsigned short			presented_type_memory_size;
	unsigned short			transmitted_type_buffer_size;
	short					transmitted_type_offset;
}TransmitRepresentAs_t;

//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types User-marshal
//------------------------------------------------------------------------------
typedef struct _UserMarshal_t
{
	unsigned char			userMarshalType;			// FC_USER_MARSHAL
	unsigned char			flags;
	unsigned short			quadruple_index;
	unsigned short			user_type_memory_size;
	unsigned short			transmitted_type_buffer_size;
	short					offset_to_the_transmitted_type;
}UserMarshal_t;


//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// types Pipe
//------------------------------------------------------------------------------
typedef struct _Pipe_t
{
	unsigned char			pipeType;			// FC_USER_MARSHAL
	unsigned char			unknowChar;
	short					offsetToType;
	unsigned short			size0;
	unsigned short			size1;
}Pipe_t;


// TODO : delete following types
/*
//------------------------------------------------------------------------------
// _MIDL_TYPE_FORMAT_STRING types :
// type end
//-----------------------------------------------------------------------------
typedef unsigned char EndTypeContent_t;

typedef struct EndDescriptor_t
{
unsigned int fieldEffectiveSize; // 1 octet
EndTypeContent_t end;			// FC_NULL
}EndDescriptor_t;



typedef union TypeFormat_U
{
StringDescriptor_t	stringDescriptor;
PointerDescriptor_t	pointerDescriptor;
EndDescriptor_t		endDescriptor;
}TypeFormat_U;

typedef struct TypeFormat_t
{
FieldTypeID_E	TypeDescrID;
TypeFormat_U	descriptor;
struct TypeFormat_t * nextType;
} TypeFormat_t;

typedef TypeFormat_t * PtrTypeFormat_t;

#define FIRST_PAD_VALUE	0x0000
typedef unsigned short typeFormat1stPad_t;*/

#define START_VALUE		0x0000
typedef unsigned short typeFormatStart_t;


/*typedef struct Fabrique_MIDL_TYPE_FORMAT_STRING
{
unsigned int	cumulatedEffectiveSize;
unsigned short	pad;
unsigned short	start; //TODO : confirmer le role de ce champ
TypeFormat_t * firstType;
}Fabrique_MIDL_TYPE_FORMAT_STRING;*/




#define FC_NULL	0x00	//TODO : a confirmer


// TODO : a confirmer - transmit as / represent as values
//#define FC_TRANSMIT_AS	0x2d
//#define FC_REPRESENT_AS	0x2e

// TODO : voir utilité
//#define FC_POINTER		0x36
//
//
//#define FC_STRING_SIZED	0x44

//#define FC_END			0x5b
//#define FC_PAD			0x5c

//#define FC_USER_MARSHAL	0xb4
//#define FC_RANGE		0xb7

// user marshal flags
#define USER_MARSHAL_UNIQUE		0x80
#define USER_MARSHAL_REF		0x40
#define USER_MARSHAL_POINTER	0xc0
#define USER_MARSHAL_IID		0x20

// context handle flags
#define NDR_CONTEXT_HANDLE_CANNOT_BE_NULL	0x01
#define NDR_CONTEXT_HANDLE_SERIALIZE		0x02
#define NDR_CONTEXT_HANDLE_NO_SERIALIZE		0x04
#define NDR_STRICT_CONTEXT_HANDLE			0x08

/* AddElement(Container_T* pContainer, void* pElement, UINT Id); */







// NEW TYPE

class ParamDesc;

typedef struct _INLINED_PARAM_BASE_T
{
	BYTE	bFcTypeParamProperties;
	BYTE	bFcType;
}INLINED_PARAM_BASE_T;

typedef struct _INLINED_PARAM_COMPLEX_T
{
	BYTE	bFcTypeParamProperties;
	BYTE	stackSize;
	USHORT	offset;
}INLINED_PARAM_COMPLEX_T;

typedef union inlinedParam
{
	INLINED_PARAM_BASE_T	baseParam;
	INLINED_PARAM_COMPLEX_T	complexParam;
}inlinedParam;

// class used to describe an idl function
class IdlFunctionDesc
{
private:
	std::string					m_strFunctionName;
	UINT32						m_uNbParam;
	BOOL						m_bHasRangeOnConformance;
	BOOL						m_bHasReturn;
	std::list<ParamDesc>		m_listParam;

public:
	IdlFunctionDesc();
	IdlFunctionDesc(std::string strFunctionName);

	UINT32	getNbParam() const;
	std::string	getFunctionName()	const	{return m_strFunctionName;}
	VOID	setNbParam(UINT32 uNbParam);
	VOID	setHasReturn(BOOL hasReturn);
	VOID	addParamToList(ParamDesc parameter);
	std::list<ParamDesc>&	getParamList();

	BOOL	hasRangeOnConformance() const;
	BOOL	hasReturn() const;

	VOID	parseWin32ExtHeader(Win2kExt_Header_t* pWin32kExtHeader);
};


enum FC_CONFORMANCE_TYPE_T
{
	switch_is,
	size_is,
	length_is,
	byte_count,
};

typedef struct ConformanceDescr_T_
{
	RVA_T								pType;
	FC_CONFORMANCE_TYPE_T				confType;
	CorrelationDescriptorNonRobust_t	corrDesc; // non robust, we'll see if robust part is needed
}ConformanceDescr_T;

// structure used to store information about structMebmer
typedef struct StructureMember_T_
{
	FC_TYPE		fcType;
	UINT		uSize;
}StructureMember_T;


// Enum used to define FC_EXPR calculus for size_is/length_is as extracted by Brandon Falk from Windows 8 pdb
// brandonfa.lk/win8/win8_devrel_head_x86/combase.h

enum EXPR_TOKEN
{
	FC_EXPR_START = 0,
	FC_EXPR_ILLEGAL = 0,
	FC_EXPR_CONST32 = 1,
	FC_EXPR_CONST64 = 2,
	FC_EXPR_VAR = 3,
	FC_EXPR_OPER = 4,
	FC_EXPR_NOOP = 5,
	FC_EXPR_END = 6
};

enum OP_TOKEN
{
	OP_UNARY_PLUS = 1,
	OP_UNARY_MINUS = 2,
	OP_UNARY_NOT = 3,
	OP_UNARY_COMPLEMENT = 4,
	OP_UNARY_INDIRECTION = 5,
	OP_UNARY_CAST = 6,
	OP_UNARY_AND = 7,
	// Gap of unknown values

	OP_PLUS = 0xE,
	OP_MINUS = 0xF,
	OP_STAR = 0x10,
	OP_SLASH = 0x11,
	OP_MOD = 0x12,
	OP_LEFT_SHIFT = 0x13,
	OP_RIGHT_SHIFT = 0x14,
	OP_LESS = 0x15,
	OP_LESS_EQUAL = 0x16,
	OP_GREATER_EQUAL = 0x17,
	OP_GREATER = 0x18,
	OP_EQUAL = 0x19,
	OP_NOT_EQUAL = 0x1A,
	OP_AND = 0x1B,
	OP_OR = 0x1C,
	OP_XOR = 0x1D,
	OP_LOGICAL_AND = 0x1E,
	OP_LOGICAL_OR = 0x1F,
	OP_EXPRESSION = 0x20

};

enum OP_OPERATOR_TYPE
{
	OP_UNARY,
	OP_BINARY,
	OP_TERNARY
};

typedef struct _EXPR_OPERATOR
{
	UINT8   ExprType;
	UINT8   Operator;
	UINT8   CastType;
	UINT8   Reserved;
} EXPR_OPERATOR;

typedef struct _EXPR_CONST32
{
	UINT8   ExprType;
	UINT8   Reserved;
	UINT16  Reserved1;
	UINT32  ConstValue;
} EXPR_CONST32;

typedef struct _EXPR_CONST64
{
	UINT8   ExprType;
	UINT8   Reserved;
	UINT16  Reserved1;
	INT64   ConstValue;
} EXPR_CONST64;

typedef struct __EXPR_VAR
{
	UINT8   ExprType;
	UINT8   VarType;
	INT16  Offset;
//	UINT32  Offset;
} EXPR_VAR;

typedef struct _EXPR_NOOP
{
	UINT8	ExprType;
	UINT8   Size;
	UINT16  Reserved;
} EXPR_NOOP;

class TypeToDefine;

// class used to store idl param description
class ParamDesc
{
private:
	BOOL	m_bString;									// param is [string]
	BOOL	m_bIn;										// param is	[in]
	BOOL	m_bUnique;									// param is [unique]
	BOOL	m_bOut;										// param is [out]
	BOOL	m_bReturn;									// param is return
	UINT	m_uPtrLevel;								// pointer level of the type (ie : number of '*')
	BOOL	m_hasRangeOnConformance;	
	BOOL	m_arrayIsAttributedPointer;					// Needed for correlation descriptor offset field
	FC_TYPE	m_fcType;
	RVA_T	m_rva;										// rva in type /proc format string									
	std::string m_strTypeName;							// param Name
	//UINT	m_uOffset;									// param offset in type format string
	std::list<ConformanceDescr_T>	m_listConfDescr;
	UINT	m_uStructMemberNum;							// when param is in a structure definition, indicate struct member number
	UINT	m_uMemorySize;								// size of the param in memory
	std::vector<UINT> m_vectMembersOffset;				// this field is only used when the param is a struct member 
	// it's a vector containing member offset of other struct member (used for conformance)

public:

	// constructors
	ParamDesc();
	ParamDesc(std::string strTypeName);
	ParamDesc(std::string strTypeName, UINT uStructMemberNum, std::vector<UINT> vectMemberOffset);
	//ParamDesc(const TypeToDefine& UnionStructDesc);




	// getter
	UINT		getuPtrLevel()			const	{return m_uPtrLevel;}
	/*UINT		getMemorySize()			const	{return m_uMemorySize;}*/
	BOOL		hasRangeOnConformance() const	{return m_hasRangeOnConformance;}
	BOOL		isReturn()				const	{return m_bReturn;}
	BOOL		isIn()					const	{return m_bIn;}
	BOOL		isOut()					const	{return m_bOut;}
	FC_TYPE		getFcType()				const	{return m_fcType;}
	RVA_T		getRva()				const	{return m_rva;}
	std::string	getStrTypeName()		const	{return m_strTypeName;}
	//UINT		getuOffset()			const	{return m_uOffset;}
	UINT		getuStructMemberNum()	const	{return m_uStructMemberNum;}
	BOOL		getArrayIsAttributedPointer() const	{return m_arrayIsAttributedPointer;}
	std::vector<UINT>& getVectMembersOffset()	{return m_vectMembersOffset;}


	//setter
	void	setString()									{m_bString = TRUE;}
	void	setUnique()									{m_bUnique = TRUE;}
	void	setHasRangeOnConformance()					{m_hasRangeOnConformance = TRUE;}
	void	setFcType(_In_ FC_TYPE fcType)				{m_fcType = fcType;}
	void	setRva(_In_ RVA_T	rva)					{m_rva = rva;}
	void	setParamName(_In_ std::string strParamName)	{m_strTypeName = strParamName;}
	//void	setuOffset(UINT uOffset)					{m_uOffset = uOffset;}
	void	setuStructMemberNum(_In_ UINT uStructMemberNum) {m_uStructMemberNum = uStructMemberNum;}
	/*void	setMemorySize(_In_ UINT uMemorySize)			{m_uMemorySize = uMemorySize;}*/
	void	incPtrLevel()								{m_uPtrLevel++;}
	void	setArrayIsAttributedPointer()				{m_arrayIsAttributedPointer = TRUE;}
	void	setIsIn()									{m_bIn = TRUE;}
	void	setIsOut()									{m_bOut = TRUE;}
	void	setIsReturn()								{m_bReturn = TRUE;}


	LONG	getRelativeOffsetFromFmtString(_In_ RVA_T pFormatString) const;
	void	fillWithParamAttr(_In_ PARAM_ATTRIBUTES paramAttr);
	void	inheritProperties(_In_ const TypeToDefine& parentTypeDesc);
	void	gestDescr(_Inout_ std::string& strDesc);

	void	addStructureMember(_In_ StructureMember_T strucMember);
	void	addConformanceDescr(_In_ ConformanceDescr_T conformanceDescr);




};




// class used to store Union or Structure that will be processed lately
class TypeToDefine
{
private :
	RVA_T	m_rva;
	FC_TYPE	m_fcType;
	UINT	m_uOffset;
	BOOL	m_bHasRangeOnConf;
	RVA_T	m_pNonEncapsulatedUnionHeader;		// only used with FC_NON_ENCAPSULATED_UNION


public:

	//TypeToDefine();
	TypeToDefine(const RVA_T pType, const ParamDesc& paramDesc);

	//getter
	RVA_T		getRva()							const	{return m_rva;}
	//PBYTE		getpType()							const	{return m_pType;}
	FC_TYPE		getFcType()							const	{return m_fcType;}
	//UINT		getuOffset()						const	{return m_uOffset;}
	BOOL		getHasRangeOnConformance()			const	{return m_bHasRangeOnConf;}
	RVA_T		getpNonEncapuslatedUnionHeader()	const	{return m_pNonEncapsulatedUnionHeader;}


	//setter
	void		setRva(RVA_T rva)								{m_rva = rva;}
	//void		setpType(PBYTE pType)							{m_pType = pType;}
	///void		setuOffset(UINT uOffset)						{m_uOffset = uOffset;}
	void		setpNonEncapsulatedUnionHeader(RVA_T pNonEncap) {m_pNonEncapsulatedUnionHeader = pNonEncap;}

	// operator
	bool operator<( const TypeToDefine& right);
	friend bool operator==( const TypeToDefine& self, const TypeToDefine& right);
};


////////////////////////////////////////////////////////////////////////////////
// NDR_VERSION definitions
////////////////////////////////////////////////////////////////////////////////
#define	NDR_VERSION_5_2 0x50002
#define NDR_VERSION_5_4 0x50004
#define NDR_VERSION_6_0 0x60000
#define NDR_VERSION_6_1 0x60001
#define NDR_VERSION_6_2 0x60002

#endif//_INTERNAL_RPC_DECOMP_TYPE_DEFS_H_