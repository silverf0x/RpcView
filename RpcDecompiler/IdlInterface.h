#ifndef _RPC_DECOMP_INTERFACE_
#define _RPC_DECOMP_INTERFACE_

#include "IdlFunction.h"
#include "IdlType.h"
#include "internalRpcDecompTypeDefs.h"
#include <vector>
#include <sstream>

class IdlFunction;
class IdlType;



// IdlInterface class definition

class IdlInterface
{
	friend class IdlType;
private:
	std::string						m_strIfName;			// Interface name
	RPC_IF_ID						m_RpcIfId;				// inteface id
	std::vector<IdlFunction*>		m_vectFunctions;		// vector holding pointer to every function defined in interface
															// this vector will be filled once functions are decompiled
	
	size_t							m_szNbFunctions;		// number of functions defined in interface
	
	std::list<TypeToDefine>			m_listProcTypes;		// list of complex types (structure / union) found while decompiling functions
															// once this list is filled it is parsed to produce declaration of these complex types
	
	std::ostringstream				m_ossComplexTypes;		// string stream that will contained decoded version of complex types 
															// (once they've all been found and decoded)
public:
	// ======================================================================
	// CONSTRUCTOR
	// ======================================================================
	IdlInterface(std::string strIfName, RPC_IF_ID& RpcIfId, size_t szNbFunctions);
	~IdlInterface();

	// ======================================================================
	// ACCESSOR
	// ======================================================================
	std::vector<IdlFunction*>	getIntefaceFunctions();
	std::list<TypeToDefine>		getListTypeToDefine();

	// ======================================================================
	// DECODIND METHODS
	// ======================================================================

	DECOMP_STATUS	decode(void* pCtx);
	DECOMP_STATUS	decodeFunction(UINT uFunctionIdx, void* pCtx);
	DECOMP_STATUS	decodeInterfaceComplexTypes(void* pCtx);
	std::ostream&	dump(std::ostream& o) const;
	std::ostream&	dumpIfInfo(std::ostream& o) const;
};


std::ostream&	operator<<(std::ostream& o, const IdlInterface& idlInterface);

#endif