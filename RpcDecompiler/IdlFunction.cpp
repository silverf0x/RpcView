#include "IdlFunction.h"
#include <sstream>
#include  "internalRpcDecompTypeDefsNew.h"
#include <algorithm>


// ======================================================================
// CONSTRUCTOR
// ======================================================================
IdlFunction::IdlFunction(unsigned int uProcIdx, IdlInterface* pIdlInterface) :
	m_uProcIdx(uProcIdx),
	m_pIdlInterface(pIdlInterface),
	m_bHasRangeOnConformance(FALSE),
	m_uOffsetFirstArg(0),
	m_bDecoded(FALSE)
{
	memset(&m_ProcHeader, 0, sizeof(m_ProcHeader));
}




// ======================================================================
// ACCESSOR
// ======================================================================
IdlInterface*	IdlFunction::getpIdlInterface()		const
{
	return m_pIdlInterface;
}

size_t			IdlFunction::getNbArguments() const
{
	return (size_t)m_ProcHeader.oifheader.bNumber_of_params;
}

bool			IdlFunction::hasRangeOnConformance() const
{
	return m_ProcHeader.win2KextHeader.interpreter_opt_flag2.HasRangeOnConf;
}

const IdlType*		IdlFunction::getReturnArg()	const	
{
		
	for(auto iter= m_listArg.begin(); iter != m_listArg.end(); iter++)
	{
		if(iter->isReturnArg() == TRUE)
		{
			return &*iter;
		}
	}

	return NULL;

}
// ======================================================================
// OTHER METHODS
// ======================================================================
DECOMP_STATUS IdlFunction::decode(void* pCtx)
{
	DECOMP_STATUS		status = DS_ERR;

	// set function name
	this->setName(pCtx);

	status = this->decodeProcHeader(pCtx);

	if(status != DS_SUCCESS)
	{
		RPC_ERROR_FN("decodeProcHeader failed\n");
		return DS_ERR_IN_DECODE_PROC_HEADER;
	}

	status = this->decodeArguments(pCtx);

	if(status != DS_SUCCESS)
	{
		RPC_ERROR_FN("decodeArguments failed\n");
		return DS_ERR_IN_DECODE_PROC_PARAMS;
	}

	m_bDecoded = TRUE;

	return DS_SUCCESS;
}


void IdlFunction::setName(void* pCtx)
{
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt = (RpcDecompilerCtxt_T *)pCtx;
    std::stringstream           ss;

	if (pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable == NULL)
	{
		return;
	}

    ss << "Proc" << std::dec << this->m_uProcIdx;

	if (pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[this->m_uProcIdx] == NULL)
	{
		m_strFunctionName = ss.str();
	}
	else
	{
		size_t sz = wcslen(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[this->m_uProcIdx]) + 1;
		size_t szConverted = 0;

		char* pTmp = (char*)pRpcDecompilerCtxt->pRpcViewHelper->RpcAlloc((UINT)sz);

		if (pTmp != NULL)
		{
			ZeroMemory(pTmp, sz);
			wcstombs_s(&szConverted, pTmp, sz, pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[this->m_uProcIdx], sz);
            
            m_strFunctionName = ss.str() + std::string("_") + std::string(pTmp);
            //
            // We have to replace all ':' by '_' as MIDL doesn't support it
            // example: NThreadingLibrary::TWorkItem::NotifyCancel
            //
            std::replace(m_strFunctionName.begin(), m_strFunctionName.end(), ':', '_');

			pRpcDecompilerCtxt->pRpcViewHelper->RpcFree(pTmp);
		}
	}


	return;
}



DECOMP_STATUS IdlFunction::decodeProcHeader(void* pCtx)
{
	BOOL						bResult				= FALSE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *)pCtx;
	UINT						uOffsetInProcFmtString;
	

	if (pRpcDecompilerCtxt == NULL)
	{
		return DS_ERR_INVALID_PARAM;
	}

	// get proc offset in type format string
	uOffsetInProcFmtString = pRpcDecompilerCtxt->pRpcDecompilerInfo->pFormatStringOffsetTable[this->m_uProcIdx];

	// Init ProcHeader
	memset(&this->m_ProcHeader, 0, sizeof(this->m_ProcHeader));

	//========================================================
	// Read Oi Header part of header
	//========================================================


	bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString), 
					&(this->m_ProcHeader.oiHeader.beginning),
					sizeof(this->m_ProcHeader.oiHeader.beginning));

	if (bResult == FALSE)
	{ 	
		return DS_ERR_UNABLE_TO_READ_MEMORY; 
	}

	uOffsetInProcFmtString += sizeof(this->m_ProcHeader.oiHeader.beginning);

	// should we read rpc_flags ?
	if ((this->m_ProcHeader.oiHeader.beginning.bOi_flags & Oi_HAS_RPCFLAGS) == Oi_HAS_RPCFLAGS)
	{
		bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString), 
					&(this->m_ProcHeader.oiHeader.dwRpc_flags),
					sizeof(this->m_ProcHeader.oiHeader.dwRpc_flags));

		if (bResult == FALSE){ 
			return DS_ERR_UNABLE_TO_READ_MEMORY; 
		}

		uOffsetInProcFmtString += sizeof(this->m_ProcHeader.oiHeader.dwRpc_flags);
	}

	// read OiHeader end
	bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString), 
					&(this->m_ProcHeader.oiHeader.end),
					sizeof(this->m_ProcHeader.oiHeader.end));

	if (bResult == FALSE){ 
		return DS_ERR_UNABLE_TO_READ_MEMORY; ; 
	}

	uOffsetInProcFmtString += sizeof(this->m_ProcHeader.oiHeader.dwRpc_flags);


	//========================================================
	// If necessary read exeplicit_handle_description
	//========================================================
	if(this->m_ProcHeader.oiHeader.beginning.bHandle_type == FC_EXPLICIT_HANDLE)
	{
		bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString), 
					&(this->m_ProcHeader.explicitHandle),
					EXPLICIT_HANDLE_MIN_SIZE);

		if(bResult == FALSE)
		{
			return DS_ERR_UNABLE_TO_READ_MEMORY;
		}

		switch(this->m_ProcHeader.explicitHandle.htype)
		{
		case FC_BIND_PRIMITIVE:
			uOffsetInProcFmtString+= EXPLICIT_HANDLE_PRIMITIVE_SIZE;
			break;

		case FC_BIND_GENERIC:
			bResult = RPC_GET_PROCESS_DATA2(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
				&(this->m_ProcHeader.explicitHandle),
				EXPLICIT_HANDLE_GENERIC_SIZE);

			if(bResult == FALSE)
			{
				return DS_ERR_UNABLE_TO_READ_MEMORY;
			}

			uOffsetInProcFmtString+= EXPLICIT_HANDLE_GENERIC_SIZE;
			break;

		case FC_BIND_CONTEXT:
			bResult = RPC_GET_PROCESS_DATA2(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
				&(this->m_ProcHeader.explicitHandle),
				EXPLICIT_HANDLE_CONTEXT_SIZE);

			if(bResult == FALSE)
			{
				RPC_ERROR_FN("can not read explicit bind handle");
				return DS_ERR_UNABLE_TO_READ_MEMORY;
			}

			uOffsetInProcFmtString+= EXPLICIT_HANDLE_CONTEXT_SIZE;
			break;

		default:
			RPC_ERROR_FN("invalid explicit handle type\n");
			return DS_ERR_INVALID_DATA;
		}
	}


	//========================================================
	// Read Oif header part of header
	//========================================================

	bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
					&(this->m_ProcHeader.oifheader),
					sizeof(this->m_ProcHeader.oifheader));

	if (bResult == FALSE){ 
		return DS_ERR_UNABLE_TO_READ_MEMORY; ; 
	}

	uOffsetInProcFmtString += sizeof(this->m_ProcHeader.oifheader);

	//========================================================
	// Read Win32Ext header part of header
	//========================================================

	// TODO : check under which condition win32Kext header is present
	bResult = RPC_GET_PROCESS_DATA2(
					(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
					&(this->m_ProcHeader.win2KextHeader.extension_version),
					sizeof(this->m_ProcHeader.win2KextHeader.extension_version));
	
	if (bResult == FALSE){ 
		return DS_ERR_UNABLE_TO_READ_MEMORY; ; 
	}


	switch (this->m_ProcHeader.win2KextHeader.extension_version)
	{
	case WIN2K_EXT_HEADER_32B_SIZE:
		bResult = RPC_GET_PROCESS_DATA2(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
			&(this->m_ProcHeader.win2KextHeader.extension_version),
			WIN2K_EXT_HEADER_32B_SIZE
			);

		uOffsetInProcFmtString += WIN2K_EXT_HEADER_32B_SIZE;

		break;
	case WIN2K_EXT_HEADER_64B_SIZE:
		bResult = RPC_GET_PROCESS_DATA2(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + uOffsetInProcFmtString),
			&(this->m_ProcHeader.win2KextHeader.extension_version),
			WIN2K_EXT_HEADER_64B_SIZE
			);

		uOffsetInProcFmtString += WIN2K_EXT_HEADER_64B_SIZE;
		break;

	default:
		return DS_ERR_INVALID_DATA;
	}

	m_uOffsetFirstArg =   uOffsetInProcFmtString;


	return DS_SUCCESS;
}



DECOMP_STATUS		IdlFunction::decodeArguments(void* pCtx)
{
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *)pCtx;
	UINT						uOffsetArg			= 0;

	//========================================================
	// Check arg
	//========================================================
	if(pRpcDecompilerCtxt == NULL)
	{
		return DS_ERR_INVALID_PARAM;
	}

	// ensure that offset to first arg is properly set
	if(m_uOffsetFirstArg == 0)
	{
		return DS_ERR_FUNCTION_NOT_PROPERLY_INITIALIZED;
	}


	uOffsetArg = m_uOffsetFirstArg;

	for(unsigned int i=0; i<this->getNbArguments(); i++)
	{
		DECOMP_STATUS	status;
		IdlType			idlArg(this, uOffsetArg);

		status = idlArg.decode(pRpcDecompilerCtxt);

		if(status != DS_SUCCESS)
		{
			RPC_ERROR_FN("decode failed\n");
			return status;
		}

		m_listArg.push_back(idlArg);
		
		// TODO : check if this size is constant
		uOffsetArg += OIF_PARAM_SIZE;
	}




	return DS_SUCCESS;

}


std::ostream&		IdlFunction::dump(std::ostream& o) const
{
	const IdlType*			pReturnArg			= this->getReturnArg();
	bool					bFirst				= true;


	// =============================================
	// ensure that function has been correctly decoded
	// =============================================
	if(m_bDecoded == FALSE)
	{
		o << DEFAULT_FN_PREFIX << m_uProcIdx << "_NotDecoded();" << std::endl;
		return o;
	}

	// =============================================
	// display return argument
	// =============================================
	if(pReturnArg == NULL)
	{
		o << "void ";
	}
	else
	{
		o << *pReturnArg;
	}

	// =============================================
	// display function name
	// =============================================
	o << m_strFunctionName << "(" << std::endl;
	
#ifdef _ADD_IMPLICIT_HANDLE
	// =============================================
	// display every arguments
	// =============================================
	// manage special handle case
	// sometimes a handle should be add at the beginning 
	// let's check if the first argument start at offset 0
	for(auto it=m_listArg.begin(); it!=m_listArg.end(); it++)
	{
		if(it->getStackOffset() == 0)
		{
			bNeedFirstHandle = false;
			break;
		}
	}

	if(bNeedFirstHandle == true)
	{
		o << "\t/*add default handle*/[in] handle_t arg_0";
		bFirst = false;
	}
#endif

	// display every arguments in argList
	for(auto it=m_listArg.begin(); it!=m_listArg.end(); it++)
	{
		if(it->isReturnArg() == false)
		{
			if(bFirst)
			{
				bFirst = false;
			}
			else
			{
				o <<", " << std::endl;
			}

		
			o << "\t" << *it;
		}
	}

	o << ");";

	return o;
}

std::ostream&	operator<<(std::ostream& o, const IdlFunction& idlFunction)
{
	return idlFunction.dump(o);
}