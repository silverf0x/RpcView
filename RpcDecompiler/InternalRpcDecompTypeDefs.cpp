#include "internalRpcDecompTypeDefs.h"
#include "internalRpcDecompiler.h"
#include <sstream>

// ***********************************************
//
// class used to describe an idl function
//
// ***********************************************

IdlFunctionDesc::IdlFunctionDesc() :
	m_uNbParam (0),
	m_bHasRangeOnConformance (FALSE),
	m_bHasReturn(FALSE)
{
	
}


IdlFunctionDesc::IdlFunctionDesc(std::string strFunctionName) :
	m_uNbParam (0),
	m_bHasRangeOnConformance (FALSE),
	m_bHasReturn(FALSE),
	m_strFunctionName(strFunctionName)
{

}


UINT32	IdlFunctionDesc::getNbParam() const
{
	return m_uNbParam;
}


VOID	IdlFunctionDesc::setNbParam(UINT32 uNbParam)
{
	m_uNbParam = uNbParam;
}

VOID	IdlFunctionDesc::setHasReturn(BOOL hasReturn)
{
	m_bHasReturn = hasReturn;
}

VOID	IdlFunctionDesc::addParamToList(ParamDesc parameter)
{
	m_listParam.push_back(parameter);
}

std::list<ParamDesc>&	IdlFunctionDesc::getParamList()
{
	return m_listParam;
}

BOOL	IdlFunctionDesc::hasRangeOnConformance() const
{
	return m_bHasRangeOnConformance;
}
BOOL	IdlFunctionDesc::hasReturn() const
{
	return m_bHasReturn;
}

VOID IdlFunctionDesc::parseWin32ExtHeader(Win2kExt_Header_t* pWin32kExtHeader)
{
	m_bHasRangeOnConformance = pWin32kExtHeader->interpreter_opt_flag2.HasRangeOnConf;

}


// ***********************************************
//
// class TypeToDefine
//
// ***********************************************
/*TypeToDefine::TypeToDefine()
{

}*/

TypeToDefine::TypeToDefine(const RVA_T rva, const ParamDesc& paramDesc):
	m_rva(rva),
	//m_uOffset(paramDesc.m_uOffset),
	m_fcType(paramDesc.getFcType()),
	m_bHasRangeOnConf(paramDesc.hasRangeOnConformance())	
{
	
}

bool TypeToDefine::operator<( const TypeToDefine& right)
{
	return (this->m_rva < right.m_rva);
}
bool operator== ( const TypeToDefine& self, const TypeToDefine& right)
{
	return (self.m_rva == right.m_rva);
}



// ***********************************************
//
// class ParamDesc
//
// ***********************************************
ParamDesc::ParamDesc(): 
	m_bString(FALSE),
	m_bIn(FALSE),
	m_bUnique(FALSE),
	m_bOut(FALSE),
	m_bReturn(FALSE),
	m_uPtrLevel(0),
	m_hasRangeOnConformance(FALSE),
	m_arrayIsAttributedPointer(FALSE),
	m_uStructMemberNum(0),
	m_uMemorySize(0),
	m_fcType(FC_ZERO)
{

}

ParamDesc::ParamDesc(std::string strTypeName):
	m_strTypeName(strTypeName),
	m_bString(FALSE),
	m_bIn(FALSE),
	m_bUnique(FALSE),
	m_bOut(FALSE),
	m_bReturn(FALSE),
	m_arrayIsAttributedPointer(FALSE),
	m_uPtrLevel(0),
	m_hasRangeOnConformance(FALSE),
	m_uStructMemberNum(0),
	m_uMemorySize(0)
{

}

ParamDesc::ParamDesc(std::string strTypeName, UINT uStructMemberNum, std::vector<UINT> vectMemberOffset):
	m_strTypeName(strTypeName),
	m_bString(FALSE),
	m_bIn(FALSE),
	m_bUnique(FALSE),
	m_bOut(FALSE),
	m_bReturn(FALSE),
	m_arrayIsAttributedPointer(FALSE),
	m_uPtrLevel(0),
	m_hasRangeOnConformance(FALSE),
	m_uStructMemberNum(uStructMemberNum),
	m_uMemorySize(0),
	m_vectMembersOffset(vectMemberOffset)
{

}

//ParamDesc::ParamDesc(const TypeToDefine& UnionStructDesc):
//	m_strTypeName(),
//	m_bString(FALSE),
//	m_fcType(UnionStructDesc.getFcType()),
//	m_bIn(FALSE),
//	m_bUnique(FALSE),
//	m_bOut(FALSE),
//	m_uPtrLevel(0),
//	m_hasRangeOnConformance(UnionStructDesc.getHasRangeOnConformance()),
//	m_uStructMemberNum(0),
//	m_uMemorySize(0)
//{
//
//}


// used for structures/union in order to report properties such
// as hasRangeOnConformance on structure/union member
void	ParamDesc::inheritProperties(_In_ const TypeToDefine& parentTypeDesc)
{
	if(parentTypeDesc.getHasRangeOnConformance())
	{
		this->m_hasRangeOnConformance = TRUE;
	}
}


void	ParamDesc::gestDescr(_Inout_ std::string& strDesc)
{
	strDesc = "Todo";
}




	






void	ParamDesc::addConformanceDescr(_In_ ConformanceDescr_T conformanceDescr)
{
	m_listConfDescr.push_back(conformanceDescr);
}


LONG	ParamDesc::getRelativeOffsetFromFmtString(_In_ RVA_T pFormatString) const
{
	return (LONG)(m_rva - pFormatString);
}


void	ParamDesc::fillWithParamAttr(_In_ PARAM_ATTRIBUTES paramAttr)
{
	if(paramAttr.IsIn) m_bIn = TRUE;
	if(paramAttr.IsOut) m_bOut = TRUE;
	
	// TODO how to handle simple ref ?
	if(paramAttr.IsSimpleRef) m_uPtrLevel++;
	//....
}