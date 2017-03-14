#include "IdlType.h"
#include "internalRpcDecompiler.h"
#include <sstream>


IdlType::IdlType(IdlFunction* pFunction, const UINT m_uOffsetInProcFmt):
	m_pFunction(pFunction),
	m_uOffsetInProcFmt(m_uOffsetInProcFmt)
{

}


// ======================================================================
// ACCESSOR
// ======================================================================
unsigned short IdlType::getStackOffset() const
{
	return m_paramDescription.oif_Format.stack_offset;
}


DECOMP_STATUS		IdlType::decode(const void* pCtx)
{
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *)pCtx;
	BOOL						bResult;
	UINT32						argNbr = 0;
	ParamDesc					ParamDesc;
	std::ostringstream			ossTmp;
	std::ostringstream			oss;

	//========================================================
	// Check arg
	//========================================================
	if(pRpcDecompilerCtxt == NULL)
	{
		return DS_ERR_INVALID_PARAM;
	}



	//========================================================
	// Read param description
	//========================================================
	bResult = RPC_GET_PROCESS_DATA2(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + m_uOffsetInProcFmt),
			&m_paramDescription, 
			sizeof(m_paramDescription)
			);	



	if(is64B)	argNbr = m_paramDescription.oif_Format.stack_offset / VIRTUAL_STACK_OFFSET_GRANULARITY_64B;
	else		argNbr = m_paramDescription.oif_Format.stack_offset / VIRTUAL_STACK_OFFSET_GRANULARITY_32B;


	// add param IdlFunctionDesc properties
	if(m_pFunction->hasRangeOnConformance())
	{
		ParamDesc.setHasRangeOnConformance();
	}

	// fill param desc with ParamAttr
	ParamDesc.fillWithParamAttr(m_paramDescription.oif_Format.paramAttributes);

	
	
	// TODO : ParamDesc was used in old mode, remove it
	if(m_paramDescription.oif_Format.paramAttributes.IsReturn == 0)
	{
		ossTmp << "arg_" << argNbr;
		ParamDesc.setParamName(ossTmp.str());
	}
	

				
	
	// param is base type
	if(m_paramDescription.oif_Format.paramAttributes.IsBasetype)
	{
		bResult = processSimpleType(
			pRpcDecompilerCtxt,  
			(FC_TYPE)m_paramDescription.oif_Format.paramType.base_type_format_char.type_format_char, 
			ParamDesc, 
			oss);

		if(FALSE == bResult)
		{
			RPC_ERROR_FN("processSimpleType failed\n");
			return DS_ERR_IN_SIMPLE_TYPE;
		}


	} //if(paramToPrint.oif_Format.paramAttributes.IsBasetype)
	else
	{
		ParamDesc.setRva(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString + m_paramDescription.oif_Format.paramType.other_type_offset);

		bResult = rpcDumpType(
			pRpcDecompilerCtxt,
			ParamDesc,
			m_pFunction->getpIdlInterface()->m_listProcTypes,
			oss);

		if(bResult == FALSE)
		{
			RPC_ERROR_FN("rpcDumpType failed\n");
			return DS_ERR_IN_SIMPLE_TYPE;
		}		
	}// !(paramToPrint.oif_Format.paramAttributes.IsBasetype)
	
	
	m_name = oss.str();

	return DS_SUCCESS;
}

bool			IdlType::isReturnArg() const
{
	return (m_paramDescription.oif_Format.paramAttributes.IsReturn != 0);
}



std::ostream&	IdlType::dump(std::ostream& o) const
{

	if( (m_paramDescription.oif_Format.paramAttributes.IsReturn) == 0)
	{

		if(m_paramDescription.oif_Format.paramAttributes.IsIn)
		{
			o << "[in]";
		}

		if(m_paramDescription.oif_Format.paramAttributes.IsOut)
		{
			o << "[out]";
		}

	#ifdef _DEBUG
		if(m_paramDescription.oif_Format.paramAttributes.IsSimpleRef)
		{
			o << "/* simple_ref */";
		}
	#endif
        //
        // pipe appears to be an invalid attribute for MIDL
        // see EfsRpc (c681d488-d850-11d0-8c52-00c04fd90f7e) EfsRpcReadFileRaw_Downlevel
        //
        /*
		if(m_paramDescription.oif_Format.paramAttributes.IsPipe)
		{
			o << "[pipe]";
		}
        */
	}

	o << m_name;

	return o;
}


std::ostream&	operator<<(std::ostream& o, const IdlType& idlType)
{
	return idlType.dump(o);
}