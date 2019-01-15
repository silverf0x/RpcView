#include "InternalComplexTypesMisc.h"
#include "internalRpcDecompiler.h"
#include "internalRpcUtils.h"
#include <string>


BOOL __fastcall processBindContext(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
    UNREFERENCED_PARAMETER(listProcTypes);
    UNREFERENCED_PARAMETER(pType);
    UNREFERENCED_PARAMETER(pContext);

	oss << "[context_handle] void*";
	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss <<" "<<ParamDesc.getStrTypeName();
	return TRUE;
}


BOOL __fastcall process_FC_BLKHOLE(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	short					wOffsetRange = 0;
	DWORD					dwRangeBegin = 0;
	DWORD					dwRangeEnd = 0;
	RVA_T					pNewType = NULL;
	BYTE					bRead = 0;


	pType += sizeof(BYTE) + sizeof(BYTE); // skip FC_TYPE and first BYTE

	//read offset to complex type
	RPC_GET_PROCESS_DATA(pType,&wOffsetRange,sizeof(wOffsetRange));
	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] process_FC_BLKHOLE : unable to read process data\n");
		return FALSE;
	}



	pNewType = pType + wOffsetRange;
	pType += sizeof(wOffsetRange);

	// read FC_TYPE at given offset
	RPC_GET_PROCESS_DATA(pNewType,&bRead,sizeof(bRead));
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] process_FC_BLKHOLE : unable to read process data\n");
		return FALSE;
	}


	// if type is a FC_BIND_CONTEXT range doesn't make sense
	// todo : check why this happen
	if((FC_TYPE) bRead != FC_BIND_CONTEXT) 
	{
		//read range begin
		RPC_GET_PROCESS_DATA(pType,&dwRangeBegin,sizeof(dwRangeBegin));
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] process_FC_BLKHOLE : unable to read process data\n");
			return FALSE;
		}

		pType += sizeof(dwRangeBegin);

		//read range end
		RPC_GET_PROCESS_DATA(pType,&dwRangeEnd,sizeof(dwRangeEnd));
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] process_FC_BLKHOLE : unable to read process data\n");
			return FALSE;
		}




		oss<<"[range("<<dwRangeBegin<<","<<(UINT)dwRangeEnd<<")] ";
	}


	ParamDesc.setRva(pNewType);

	bResult = rpcDumpType(
		pContext,
		ParamDesc,
		listProcTypes,
		oss);


	return bResult;
}


BOOL __fastcall getFC_BLKHOLEMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance)
{


	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	short					offsetRange = 0;
	RVA_T					pNewType = NULL;

	pType += sizeof(BYTE) + sizeof(BYTE); // skip FC_TYPE and first BYTE

	//read offset to complex type
	RPC_GET_PROCESS_DATA(pType,&offsetRange,sizeof(offsetRange));
	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getFC_BLKHOLEMemorySize : unable to read process data\n");
		return FALSE;
	}



	pNewType = pType + offsetRange;

	bResult = getTypeMemorySize(pContext, pNewType, pszMemorySize, bHasRangeOnConformance);


	return bResult;

}


BOOL __fastcall processRange(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	std::ostringstream		ossTmp;

	Range_t					range;

    UNREFERENCED_PARAMETER(listProcTypes);

	RPC_GET_PROCESS_DATA(pType, &range, sizeof(range));

	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processRange : unable to read process data\n");
		return FALSE;
	}

	processSimpleType(
		pContext,
		(FC_TYPE)(range.flags_type & 0xF), //type is on lower byte
		ParamDesc,
		ossTmp);


	if(ParamDesc.getuPtrLevel() != 0)
	{
		oss << "/*";
	}
	oss << "[range(" << std::dec << range.lowValue << "," << range.highValue<< ")] ";
	if(ParamDesc.getuPtrLevel() != 0)
	{
		oss << "*/";
	}


	oss<<ossTmp.str();


	return TRUE;
}


BOOL __fastcall getRangeMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	Range_t					range;


	RPC_GET_PROCESS_DATA(pType, &range, sizeof(range));

	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getRangeMemorySize : unable to read process data\n");
		return FALSE;
	}


	(*pszMemorySize) =  getSimpleTypeMemorySize((FC_TYPE) (range.flags_type & 0xF));

	return TRUE;
}


BOOL __fastcall ProcessArrayRange(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& temp)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;

	Range_t					range;
    UNREFERENCED_PARAMETER(ParamDesc);

	RPC_GET_PROCESS_DATA(pType, &range, sizeof(range));
	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] ProcessArrayRange : unable to read process data\n");
		return FALSE;
	}
	temp << "/*[range(" << std::dec << range.lowValue << "," << range.highValue<< ")]*/ ";

	return bResult;
}


UINT getElementByOffset(std::vector <UINT> v, UINT offset)
{
	for(UINT i=0; i<v.size(); i++)
	{
		if(v[i] == offset)
		{
			return i;
		}
	}
	return (UINT)-1;
}


BOOL __fastcall processUserMarshal(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{

	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	UserMarshal_t			userMarshal;
	RVA_T					pUserMarshalTarget = NULL;
	std::string				strParamNameTmp;

	RPC_GET_PROCESS_DATA(pType, &userMarshal, sizeof(userMarshal));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processUserMarshal : unable to read process data\n");
		return FALSE;
	}

	oss << "/* WARNING : user_marshall should be defined in ACF file */ [user_marshall(";

	// compute user marshal target offset
	pUserMarshalTarget = pType +  FIELD_OFFSET(UserMarshal_t, offset_to_the_transmitted_type) + userMarshal.offset_to_the_transmitted_type;
	ParamDesc.setRva(pUserMarshalTarget);

	strParamNameTmp = ParamDesc.getStrTypeName();
	ParamDesc.setParamName("");

	if (rpcDumpType(pContext, ParamDesc, listProcTypes, oss) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processUserMarshal : unable to dump type\n");
		return FALSE;
	}


	oss << ")] " << strParamNameTmp;

	// set user_marshal size ??


	return TRUE;
}


BOOL __fastcall getUserMarshallMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	UserMarshal_t			userMarshal;


	RPC_GET_PROCESS_DATA(pType, &userMarshal, sizeof(userMarshal));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processUserMarshal : unable to read process data\n");
		return FALSE;
	}

	*pszMemorySize = userMarshal.user_type_memory_size;
	
	return TRUE;
}


BOOL __fastcall processPipe(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	Pipe_t					pipe;
	RVA_T					pPipeTarget = NULL;
	std::string				strParamNameTmp;

	RPC_GET_PROCESS_DATA(pType, &pipe, sizeof(pipe));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processPipe : unable to read process data\n");
		return FALSE;
	}



	// compute pipe target offset
	pPipeTarget = pType +  FIELD_OFFSET(Pipe_t, offsetToType ) + pipe.offsetToType;
	ParamDesc.setRva(pPipeTarget);


	oss << "pipe ";

	if (rpcDumpType(pContext, ParamDesc, listProcTypes, oss) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processPipe : unable to read process data\n");
		return FALSE;
	}



	// set pipe size ??
	
	return TRUE;
}


BOOL __fastcall getPipeMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	Pipe_t					pipe;
	RVA_T					pPipeTarget = NULL;
	

	RPC_GET_PROCESS_DATA(pType, &pipe, sizeof(pipe));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getPipeMemorySize : unable to read process data\n");
		return FALSE;
	}



	// compute pipe target offset
	pPipeTarget = pType +  FIELD_OFFSET(Pipe_t, offsetToType ) + pipe.offsetToType;
	
	bResult = getTypeMemorySize(pContext, pPipeTarget, pszMemorySize, bHasRangeOnConformance);

	return bResult;
}


BOOL __fastcall processCorrelationDescriptor(
	_In_	VOID* pContext,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss,
	_Inout_	ParamDesc& paramDesc)
{

	BOOL	bResult = FALSE;

	oss<<"[";

	// display conformance type
	switch(confDesc.confType)
	{
	case size_is:
		oss<<"size_is(";
		break;
	case switch_is:
		oss<<"switch_is(";
		break;
	case length_is:
		oss<<"length_is(";
		break;
	case byte_count:
		oss<<"byte_count(";
		break;
	default:
		oss<<"unknow_corr_type(";
	}

	// Check on which pointer level conformance should be applied
	{
		UINT uPtrLevel = paramDesc.getuPtrLevel();

		if(uPtrLevel > 1)
		{
			while(--uPtrLevel > 1)
			{
				oss << ",";
			}
		}
	}


	bResult = processCorrelationDescriptorNaked(pContext, confDesc, oss, paramDesc);

	oss<<")]";

	return bResult;
}


BOOL __fastcall processCorrelationDescriptorNaked(
	_In_	VOID* pContext,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss,
	_Inout_	ParamDesc& paramDesc)
{

	BOOL					bResult;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	std::string		strCorrelationItem;		// string describing X in size_is(X)


	// special cases : OperationType == FC_EXPR || OperationType === FC_CALLBACK
	if(confDesc.corrDesc.correlation_operator == FC_EXPR )
	{
		USHORT		offset = 0;

		// read offset
		RPC_GET_PROCESS_DATA(
			((RVA_T)pRpcDecompilerCtxt->pRpcDecompilerInfo->pExprOffset + confDesc.corrDesc.offset*sizeof(USHORT)),  
			&offset, 
			sizeof(USHORT));

		if(bResult == FALSE)
		{
			oss << "/* ERROR unable to read ExprOffset table */";
			return FALSE;
		}

		oss << "/* FC_EXPR */";

		bResult = processFcExpr(
			pContext, 
			pRpcDecompilerCtxt->pRpcDecompilerInfo->pExprFormatString + offset,
			paramDesc,
			confDesc,
			oss);

		return bResult;
	}
	if(confDesc.corrDesc.correlation_operator == FC_CALLBACK )
	{
		oss << "/*FC_CALLBACK not implemented */";
		return TRUE;
	}

	// FIRST STEP : according to correlation type correlation item

	// case of a FC_CONSTANT_CONFORMANCE
	if(confDesc.corrDesc.correlation_type & FC_CONSTANT_CONFORMANCE)
	{
		INT32 iSize = ((confDesc.corrDesc.correlation_operator << 16) | confDesc.corrDesc.offset) & 0xFFFFFF;

		oss<<std::dec<<iSize;
		return TRUE;
	}


	// case of a FC_TOP_LEVEL_CONFORMANCE
	// only used in function definition ?
	if(confDesc.corrDesc.correlation_type & FC_TOP_LEVEL_CONFORMANCE)
	{
		UINT uArgNbr = 0;
		std::stringstream ss;
#ifdef DBG_DECOMP
		oss <<"/*FC_TOP_LEVEL_CONFORMANCE*/";
#endif
		uArgNbr = confDesc.corrDesc.offset / (VIRTUAL_STACK_OFFSET_GRANULARITY);

		ss<<"arg_"<<std::dec<<uArgNbr; 
		strCorrelationItem = ss.str();
	}
	else if(confDesc.corrDesc.correlation_type & FC_POINTER_CONFORMANCE) // case of FC_POINTER_CONFORMANCE
	{
		std::stringstream ss;
		UINT	uCorrDescMember;
#ifdef DBG_DECOMP
		oss<<"/*FC_POINTER_CONFORMANCE(" << std::dec << confDesc.corrDesc.offset << ") */" ;
#endif
		// check if vectMemberOffset is correctly filled
		if(paramDesc.getVectMembersOffset().size() == 0)
		{
			oss << " /* ERROR : FC_POINTER_CONFORMANCE but an empty vectMemberOffset has been provided  (offset = " <<confDesc.corrDesc.offset<< ") */";
			return FALSE;
		}

		// get corr desc  member number according to its offset
		uCorrDescMember = getElementByOffset(paramDesc.getVectMembersOffset(), confDesc.corrDesc.offset);

		if(uCorrDescMember == -1)
		{
			oss << "/* ERROR invalid corrDescMember */";
			return FALSE;
		}

		ss << "StructMember"<<std::dec<<uCorrDescMember;
		strCorrelationItem = ss.str();
	}
	else if(confDesc.corrDesc.correlation_type & FC_TOP_LEVEL_MULTID_CONFORMANCE) // case of FC_TOP_LEVEL_MULTID_CONFORMANCE
	{
		// currently not implemented
		oss<<"/*FC_TOP_LEVEL_MULTID_CONFORMANCE not implemented */)]";
		return FALSE;
	}
	else	
	{
		// last remaining case is  FC_NORMAL_CONFORMANCE
		// Onlys used in structure definition ?
		UINT	uCorrDescMemberOffset;
		UINT	uCorrDescMember;
		std::stringstream ss;
#ifdef DBG_DECOMP
		oss<<"/*FC_NORMAL_CONFORMANCE ("<< std::dec << confDesc.corrDesc.offset<<")*/ ";
#endif
		// check if vectMemberOffset is correctly filled
		if(paramDesc.getVectMembersOffset().size() == 0)
		{
			oss << " /* ERROR : FC_NORMAL_CONFORMANCE but an empty vectMemberOffset has been provided */";
			return FALSE;
		}

		// compute correlation member offset
		if(paramDesc.getVectMembersOffset().size() <= paramDesc.getuStructMemberNum())
		{
			oss << "/* ERROR invalid structMemberNum */";
			return FALSE;
		}

		// get member number according to its offset
		uCorrDescMemberOffset = paramDesc.getVectMembersOffset()[paramDesc.getuStructMemberNum()] + confDesc.corrDesc.offset;
		uCorrDescMember = getElementByOffset(paramDesc.getVectMembersOffset(), uCorrDescMemberOffset);

		if(uCorrDescMember == -1)
		{
			oss << "/* ERROR invalid corrDescMember || corr desc offset =  */" << std::dec << confDesc.corrDesc.offset;
			return TRUE;
			//return FALSE
		}

		ss << "StructMember"<<std::dec<<uCorrDescMember;
		strCorrelationItem = ss.str();

	}

	// SECOND STEP : display correlation item according to correlation operator
	switch(confDesc.corrDesc.correlation_operator)
	{

	case FC_DEREFERENCE:

		// the correlation item describing the size of the current item
		// can derive from an out parameter. In that case, we can only set it
		// has a max range and not the exact size since it's not known before the call.
		// Ex :
		//	HRESULT	Proc0 (
		//		[in] int arg1,
		//		[out] int *arg2,
		//		[out][size_is( , *arg2)] int **arg2
		//	);
		if (paramDesc.isOut()) 
		{
			oss << ", *" << strCorrelationItem;
		}
		else
		{
			oss << "*" << strCorrelationItem;
		}
		
		break;

	case FC_ADD_1:
		oss << strCorrelationItem << "+1";
		break;

	case FC_SUB_1:
		oss << strCorrelationItem << "-1";
		break;

	case FC_DIV_2:
		oss << strCorrelationItem << "/2";
		break;

	case FC_MULT_2:
		oss << strCorrelationItem << "*2";
		break;

	case FC_EXPR:
		oss << "/* FC_EXPR not implemented */";
		break;

	case FC_CALLBACK:
		oss << "/* FC_CALLBACK not implemented */";

	default:
		oss << strCorrelationItem;

	}
	return TRUE;
}


UINT __fastcall processFcExpr(
	_In_	VOID* pContext,
	_Out_	RVA_T pExpr,
	_In_	ParamDesc& paramDesc,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss)
{
	EXPR_OPERATOR	operation;
	EXPR_CONST32	constante32;
	//EXPR_CONST64	constante64; Not used as of yet. Maybe with NDR64?
	EXPR_VAR		var;
	UINT32			varNb;
	BOOL			bResult;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	UINT			sizeToAdvance = 0;
	UINT			sizeOfExpr = 0;

	RPC_GET_PROCESS_DATA(
		pExpr,
		&operation,
		sizeof(EXPR_OPERATOR)
		);

	switch(operation.ExprType)
	{
	case FC_EXPR_OPER:
		if(operation.Operator >= OP_UNARY_PLUS && operation.Operator <= OP_UNARY_AND) // Unary operator
		{
			oss << getOperatorType(operation);

			pExpr += sizeof(EXPR_OPERATOR);
			sizeToAdvance += sizeof(EXPR_OPERATOR);

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc, confDesc, oss);
			sizeToAdvance += sizeOfExpr;
		}else if(operation.Operator >= OP_PLUS && operation.Operator <= OP_LOGICAL_OR) // binary operator
		{
			std::string tmpOper = getOperatorType(operation);
			oss << "(";
			pExpr += sizeof(EXPR_OPERATOR);
			sizeToAdvance += sizeof(EXPR_OPERATOR);

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc, confDesc, oss);

			pExpr += sizeOfExpr;
			sizeToAdvance += sizeOfExpr;

			oss <<" "<< tmpOper.c_str()<<" ";

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc, confDesc, oss);

			sizeToAdvance += sizeOfExpr;

			oss << ")";
		}else{ // Ternary operator => (expr ? expr1 : expr2)
			std::ostringstream tmpIfYes;
			std::ostringstream tmpIfNo;

			pExpr += sizeof(EXPR_OPERATOR);
			sizeToAdvance += sizeof(EXPR_OPERATOR);

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc, confDesc, tmpIfYes);
			sizeToAdvance += sizeOfExpr;
			pExpr += sizeOfExpr;

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc,confDesc, tmpIfNo);
			sizeToAdvance += sizeOfExpr;
			pExpr += sizeOfExpr;

			oss << "(";

			sizeOfExpr = processFcExpr(pContext, pExpr, paramDesc, confDesc, oss);
			sizeToAdvance += sizeOfExpr;

			oss << "?";
			oss << tmpIfYes.str();
			oss << ":";
			oss << tmpIfNo.str();
			oss << ")";
		}
		break;
	case FC_EXPR_VAR:
		RPC_GET_PROCESS_DATA(pExpr, &var, sizeof(EXPR_VAR));

		if(confDesc.corrDesc.correlation_type & FC_TOP_LEVEL_CONFORMANCE)
		{
			varNb = var.Offset / VIRTUAL_STACK_OFFSET_GRANULARITY;
			oss << "arg_" << std::dec << varNb;
		}else{
			INT	uCorrDescMemberOffset;
			INT	uCorrDescMember;
			std::stringstream ss;
			// get member number according to its offset
			if(paramDesc.getArrayIsAttributedPointer())
			{
				uCorrDescMemberOffset = var.Offset;	
			}else{
				uCorrDescMemberOffset = paramDesc.getVectMembersOffset()[paramDesc.getuStructMemberNum()] + var.Offset;
			}
			uCorrDescMember = getElementByOffset(paramDesc.getVectMembersOffset(), uCorrDescMemberOffset);

			if(uCorrDescMember == -1)
			{
				oss << "/* [ERROR] invalid corrDescMember || var offset =  " << std::dec << var.Offset << "*/";
				return FALSE;
			}

			oss << "StructMember"<<std::dec<<uCorrDescMember;
		}


		sizeToAdvance += sizeof(EXPR_VAR);
		break;
	case FC_EXPR_CONST32:

		RPC_GET_PROCESS_DATA(pExpr, &constante32, sizeof(EXPR_CONST32));

		oss << std::dec << constante32.ConstValue;

		sizeToAdvance += sizeof(EXPR_CONST32);
		break;
	case FC_EXPR_CONST64:
		// TODO
		break;
	default:
		oss << "/*[ERROR] unknown expression type */";
		break;
	}

	return sizeToAdvance;
}


std::string __fastcall getOperatorType(
	_In_ EXPR_OPERATOR oper)
{
	std::string		result;
	switch(oper.Operator)
	{

	case OP_UNARY_PLUS:
		result = "+";
		break;
	case OP_UNARY_MINUS:
		result = "-"; 
		break;
	case OP_UNARY_NOT:
		result = "!"; 
		break;
	case OP_UNARY_COMPLEMENT:
		result = "~";
		break;
	case OP_UNARY_CAST:
		switch(oper.CastType)
		{
		case FC_SMALL:
			result = "(byte)" ;
			break;
		case FC_USMALL:
			result = "(char)";
			break;
		case FC_USHORT:
			result = "(unsigned short)";
			break;
		case FC_LONG:
			result = "(long)";
			break;
		case FC_ULONG:
			result = "(unsigned long)";
			break;
		default:
			result = "/* unknown cast type */";
			break;
		}
		break;
	case OP_UNARY_INDIRECTION:
		result = "*";
		break;
	case OP_UNARY_AND:
		result = "&";
		break;
	case OP_PLUS:
		result = "+";
		break;
	case OP_MINUS:
		result = "-";
		break;
	case OP_STAR:
		result = "*";
		break;
	case OP_SLASH:
		result = "/";
		break;
	case OP_MOD:
		result = "%";
		break;
	case OP_LEFT_SHIFT:
		result = "<<";
		break;
	case OP_RIGHT_SHIFT:
		result = ">>";
		break;
	case OP_LESS:
		result = "<";
		break;
	case OP_LESS_EQUAL:
		result = "<=";
		break;
	case OP_GREATER_EQUAL:
		result = ">=";
		break;
	case OP_GREATER:
		result = ">";
		break;
	case OP_EQUAL:
		result = "==";
		break;
	case OP_NOT_EQUAL:
		result = "!=";
		break;
	case OP_AND:
		result = "&";
		break;
	case OP_OR:
		result = "|";
		break;
	case OP_XOR:
		result = "^";
		break;
	case OP_LOGICAL_AND:
		result = "&&";
		break;
	case OP_LOGICAL_OR:
		result = "||";
		break;
	case OP_EXPRESSION:
		break;
	default:
		result = "/*[ERROR] unknown operation */" ;
		break;
	}
	return result;
}


BOOL __fastcall processTransmitRepresentAs(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss)
{
	BOOL			bResult;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	TransmitRepresentAs_t	transmit;

	RPC_GET_PROCESS_DATA(pType, &transmit, sizeof(TransmitRepresentAs_t));

	RVA_T pTransmittedType = pType + sizeof(TransmitRepresentAs_t) + transmit.transmitted_type_offset - sizeof(transmit.transmitted_type_offset);

	oss << "/* data transmitted as */";
	ParamDesc.setRva(pTransmittedType);
	if( rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss) == FALSE)
	{
		RPC_DEBUG_FN("[ERROR] processTransmitRepresentAs : dump type failed \n");
		return FALSE;
	}

	return TRUE;
}


BOOL __fastcall getTransmitAsRepresentAsMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize)
{
	BOOL			bResult;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	TransmitRepresentAs_t	transmit;

	RPC_GET_PROCESS_DATA(pType, &transmit, sizeof(TransmitRepresentAs_t));

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN("[ERROR] getTransmitAsRepresentAsMemorySize : unable to get process data\n");
		return FALSE;
	}
	
	// TODO : check if this is this size and not target size that is used for conformance
	*pszMemorySize = transmit.presented_type_memory_size; 
	
	return TRUE;
}


UINT getCorrelationDescriptorSize(
	_In_	BOOL bRobustFlagWasSet, 
	_In_	BOOL bHasRangeOnConformance)
{

	UINT uSize = 0;

    UNREFERENCED_PARAMETER(bRobustFlagWasSet);

	if(robustFlagWasSet)
	{
		uSize = sizeof(CorrelationDescriptorRobust_t);
	}
	else
	{
		uSize = sizeof(CorrelationDescriptorNonRobust_t);
	}

	if(bHasRangeOnConformance)
	{
		uSize += sizeof(Range_t);	
	}

	return uSize;
}