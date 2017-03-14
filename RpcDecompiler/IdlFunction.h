#ifndef _RPC_DECOMP_FUNCTION_
#define _RPC_DECOMP_FUNCTION_

#include <list>

#include <ostream>
#include <sstream>

#include "IdlType.h"
#include "IdlInterface.h"
#include "internalRpcDecompTypeDefs.h"

class IdlType;
class IdlInterface;

//#define _ADD_IMPLICIT_HANDLE
#define DEFAULT_FN_PREFIX  "_Function_"

class IdlFunction
{
private:
	std::string				m_strFunctionName;			// function name
	IdlInterface*			m_pIdlInterface;				// interface owning function
	std::list<IdlType>		m_listArg;					// list of function argument
	unsigned int			m_uProcIdx;					// function index in proc interface proc format string
	BOOL					m_bInlined;					// TRUE if function is inlined
	BOOL					m_bHasRangeOnConformance;	// TRUE if function has range on conformance
	PROC_HEADER_T			m_ProcHeader;				
	UINT					m_uOffsetFirstArg;			// Offset of first argument in proc format string
	BOOL					m_bDecoded;					// BOOL indicating if function has been properly decoded
public:
	

	// ======================================================================
	// CONSTRUCTOR
	// ======================================================================
	IdlFunction(unsigned int uProcIdx, IdlInterface* pIdlInterface);


	// ======================================================================
	// ACCESSOR
	// ======================================================================
	IdlInterface*	getpIdlInterface()		const;
	size_t			getNbArguments()		const;
	bool			hasRangeOnConformance() const;


	const IdlType*	getReturnArg()				const;


	// ======================================================================
	// OTHER METHODS
	// ======================================================================
	DECOMP_STATUS	decode(void* pCtx);
	void			setName(void* pCtx);
	DECOMP_STATUS	decodeProcHeader(void* pCtx);
	DECOMP_STATUS	decodeArguments(void* pCtx);
	std::ostream&	dump(std::ostream& o) const;


};


std::ostream&	operator<<(std::ostream& o, const IdlFunction& idlFunction);

#endif