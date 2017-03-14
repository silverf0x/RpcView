#ifndef _RPC_DECOMP_TYPE_
#define _RPC_DECOMP_TYPE_

#include "IdlFunction.h"
#include "internalRpcDecompTypeDefs.h"
#include <sstream>


class IdlFunction;

class IdlType
{
private:
	std::string					m_name;					// Param name
	IdlFunction*				m_pFunction;			// pointer to related function
	UINT						m_uOffsetInProcFmt;		// offset where param is described in proc format string
	ProcFormatStringParam_U		m_paramDescription;		// Parameter description in proc format string
	

public:
	

	// ======================================================================
	// CONSTRUCTOR
	// ======================================================================
	IdlType(IdlFunction* pFunction, const UINT m_uOffsetInProcFmt); 


	// ======================================================================
	// ACCESSOR
	// ======================================================================
	unsigned short getStackOffset() const;


	// ======================================================================
	// OTHER METHODS
	// ======================================================================
	DECOMP_STATUS	decode(const void* pCtx);
	bool			isReturnArg() const;
	std::ostream&	dump(std::ostream& o) const;

};


std::ostream&	operator<<(std::ostream& o, const IdlType& idlType);

#endif