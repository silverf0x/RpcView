#include "IdlInterface.h"
#include "internalRpcDecompiler.h"

// ======================================================================
// CONSTRUCTOR
// ======================================================================
IdlInterface::IdlInterface(std::string strIfName, RPC_IF_ID& RpcIfId, size_t szNbFunctions):
	m_strIfName(strIfName),
	m_szNbFunctions(szNbFunctions),
	m_vectFunctions(szNbFunctions),
	m_RpcIfId(RpcIfId)
{
	
}

IdlInterface::~IdlInterface()
{
	for(size_t i=0; i<m_vectFunctions.size(); i++)
	{
		if(m_vectFunctions[i] != NULL)
		{
			delete m_vectFunctions[i];
		}
	}
}


// ======================================================================
// ACCESSOR
// ======================================================================
std::vector<IdlFunction*>	IdlInterface::getIntefaceFunctions()
{
	return m_vectFunctions;
}

// ======================================================================
// DECODING METHODS
// ======================================================================
DECOMP_STATUS	IdlInterface::decode(void* pCtx)
{
	DECOMP_STATUS dStatus		= DS_ERR;
	DECOMP_STATUS dStatusRet	= DS_SUCCESS; 
	
	// check param
	if(pCtx == NULL)
	{
		RPC_ERROR_FN("pCtx NULL pointer\n");
		return DS_ERR_INVALID_PARAM;
	}


	// decode every functions
	for(UINT i=0; i<m_szNbFunctions; i++)
	{
		dStatus = this->decodeFunction(i, pCtx);

		if(dStatus != DS_SUCCESS)
		{
			RPC_ERROR_FN("decodeFunction failed\n");
			printf("Idx %u\n", i);
			std::ostringstream		ossIf;
			this->dumpIfInfo(ossIf);
			printf("%s", ossIf.str().c_str());
			//DEBUG_BREAK();
			// don't stop at first failure in order to decompile every possible functions
			dStatusRet = dStatus;
		}
	}

	if(dStatusRet == DS_SUCCESS)
	{
		// once every functions is decoded 
		dStatusRet = decodeInterfaceComplexTypes(pCtx);
	}


	return dStatusRet;
}





DECOMP_STATUS				IdlInterface::decodeFunction(UINT uFunctionIdx, void* pCtx)
{
	// check param
	if(pCtx == NULL)
	{
		return DS_ERR_INVALID_PARAM;
	}
	if(uFunctionIdx > m_szNbFunctions)
	{
		return DS_ERR_INVALID_INDEX;
	}

	// has function already been decoded
	if(m_vectFunctions[uFunctionIdx] == NULL)
	{
		m_vectFunctions[uFunctionIdx] = new IdlFunction(uFunctionIdx, this);

		return (m_vectFunctions[uFunctionIdx])->decode(pCtx);
	}
	else
	{
		return DS_SUCCESS;
	}
}





DECOMP_STATUS		IdlInterface::decodeInterfaceComplexTypes(void* pCtx)
{
	std::list<TypeToDefine> listAllTypesSorted;
	std::ostringstream ossUseless;


	// check param
	if(pCtx == NULL)
	{
		return DS_ERR_INVALID_PARAM;
	}

	// clear string stream content
	m_ossComplexTypes.flush();


	// recursively explore all list in order to get a complete list of types
	if (getAllTypesSortedInAList(pCtx, m_listProcTypes, listAllTypesSorted, ossUseless) == FALSE)
	{
		m_ossComplexTypes<<"[ERROR] unable to get list of all types sorted" << std::endl;
		return DS_ERR_UNABLE_TO_DECODE_COMPLEX_TYPE;
	}

	if( dumpTypeList(pCtx, listAllTypesSorted, m_ossComplexTypes) == FALSE)
	{
		m_ossComplexTypes<<"[ERROR] unable to dump type list" << std::endl;
		return DS_ERR_UNABLE_TO_DECODE_COMPLEX_TYPE;
	}
	

	return DS_SUCCESS;

}


std::ostream&	IdlInterface::dump(std::ostream& o) const
{
	dumpIfInfo(o);

	o << std::endl;
	o << "interface "<< m_strIfName << std::endl;
	o << "{" << std::endl;

	// display complex type

	o << m_ossComplexTypes.str();

	// display functions
	for(size_t i=0; i< m_szNbFunctions; i++)
	{
		o << std::endl;

		if(m_vectFunctions[i] == NULL)
		{
			o << "void Opnum"<<i<<"NotDecoded(void)";
		}
		else
		{
			o << *(m_vectFunctions[i]);
		}

		o << std::endl;
	}

	o << "} ";

	return o;
}



#define MAX_UUID_STR 40
std::ostream&	IdlInterface::dumpIfInfo(std::ostream& o) const
{
	char chUuidStr[MAX_UUID_STR] = {0};
	int  res;

	res = sprintf_s(
		chUuidStr, 
		sizeof(chUuidStr), 
		"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		m_RpcIfId.Uuid.Data1,
		m_RpcIfId.Uuid.Data2,
		m_RpcIfId.Uuid.Data3,
		m_RpcIfId.Uuid.Data4[0],
		m_RpcIfId.Uuid.Data4[1],
		m_RpcIfId.Uuid.Data4[2],
		m_RpcIfId.Uuid.Data4[3],
		m_RpcIfId.Uuid.Data4[4],
		m_RpcIfId.Uuid.Data4[5],
		m_RpcIfId.Uuid.Data4[6],
		m_RpcIfId.Uuid.Data4[7]);

	o << "[" << std::endl;
	o << "uuid(";

	if(res != -1)
	{
		o << std::string(chUuidStr);
	}
	else
	{
		o << "/* ERROR IN UUID DECODING */";
	}

	o << ")," << std::endl;
	o << "version("<< m_RpcIfId.VersMajor <<"."<< m_RpcIfId.VersMinor <<")," << std::endl;
	o <<"]"; 

	return o;
}


std::ostream&	operator<<(std::ostream& o, const IdlInterface& idlInterface)
{
	return idlInterface.dump(o);
}