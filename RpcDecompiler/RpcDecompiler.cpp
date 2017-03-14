#include <windows.h>
#include <stdio.h>
#include <string.h>

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

#include <sstream>
#include	<list>
//DBG
#include <iostream>
//DBG

#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalRpcDecompiler.h"
#include "RpcDecompiler.h"
#include "internalRpcUtils.h"
#include "internalTypeTools.h"
#include "IdlFunction.h"
#include "IdlInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

	////////////////////////////////////////////////////////////////////////////////
	// Function declaration
	////////////////////////////////////////////////////////////////////////////////
	VOID*	__fastcall RpcDecompilerInit(RpcViewHelper_T* pRpcViewHelper, RpcDecompilerInfo_T* pDecompilerInfo);	//returns NULL in case of failure
	VOID	__fastcall RpcDecompilerUninit(VOID* pRpcDecompilerCtxt);


	BOOL	__fastcall RpcDecompilerPrintAllProcedures(VOID* pRpcDecompilerCtxt);
	BOOL	__fastcall RpcDecompilerPrintProcedure(VOID* pRpcDecompilerCtxt, UINT ProcIndex);
	BOOL	__fastcall RpcDecompilerPrintOneProcedure(VOID* pRpcDecompilerCtxt, UINT ProcIndex, std::list<TypeToDefine>& listProcType, std::ostringstream& ossProc);
	BOOL	__fastcall RpcDecompilerDecodeOneProcedureInlined(VOID*	pContext, UINT	ProcIndex, IdlFunctionDesc& IdlFunctionDesc, std::list<TypeToDefine>& listProcType);
	BOOL	__fastcall RpcDecompilerPrintOneProcedureInlined(VOID* pContext, UINT ProcOffset, IdlFunctionDesc& IdlFunctionDesc,	std::list<TypeToDefine>& listProcType, std::ostringstream& ossProc);
	BOOL	__fastcall RpcDecompilerPrintHiddenFUProcedure(VOID* pRpcDecompilerCtxt, UINT * procOffset, std::list<TypeToDefine>& listProcType, std::ostringstream& ossProc);

	BOOL	__fastcall RpcDecompilerPrintAllProceduresNew(VOID* pRpcDecompilerCtxt);



	VOID	__fastcall RpcDecompilerPrintFunctionDbgInfo(VOID* pContext, UINT procIndex, std::ostringstream& oss);

#pragma comment(lib,"Rpcrt4.lib")

	__declspec(dllexport) RpcDecompilerHelper_T RpcDecompilerHelper=
	{
		&RpcDecompilerInit,
		&RpcDecompilerUninit,
		&RpcDecompilerPrintProcedure,
		&RpcDecompilerPrintAllProceduresNew,
	};


	//------------------------------------------------------------------------------
	VOID* __fastcall RpcDecompilerInit(RpcViewHelper_T* pRpcViewHelper, RpcDecompilerInfo_T* pRpcDecompilerInfo)
	{
		RpcDecompilerCtxt_T *	pRpcDecompilerCtxt = NULL;

#ifdef _DEBUG
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#endif

		if (pRpcViewHelper == NULL) goto End;
		if (pRpcDecompilerInfo == NULL) goto End;

		//Alloc the private context
		pRpcDecompilerCtxt = (RpcDecompilerCtxt_T *) pRpcViewHelper->RpcAlloc( sizeof(RpcDecompilerCtxt_T) );
		if (pRpcDecompilerCtxt==NULL) goto End;
		//Init the private context
		pRpcDecompilerCtxt->pRpcViewHelper		= pRpcViewHelper;
		pRpcDecompilerCtxt->pRpcDecompilerInfo	= pRpcDecompilerInfo;
		pRpcDecompilerCtxt->pRpcModuleInfo      = (RpcModuleInfo_T *) pRpcViewHelper->RpcAlloc( sizeof(RpcModuleInfo_T) );
		if (pRpcDecompilerCtxt->pRpcModuleInfo == NULL)
		{
			goto End;
		}

		pRpcDecompilerCtxt->pRpcModuleInfo->Pid         = pRpcDecompilerCtxt->pRpcDecompilerInfo->Pid;
		pRpcDecompilerCtxt->pRpcModuleInfo->pModuleBase = pRpcDecompilerCtxt->pRpcDecompilerInfo->pModuleBase;




		// set global var

		// is it a 64 bits application ?
		is64B = pRpcDecompilerCtxt->pRpcDecompilerInfo->bIs64Bits;

		// robust flags case
		if(	pRpcDecompilerCtxt->pRpcDecompilerInfo->NDRVersion >= NDR_VERSION_5_2 )
		{
			robustFlagWasSet = TRUE;
		}
		else
		{
			robustFlagWasSet = FALSE;
		}

		RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerInit\n");
End:
		return (pRpcDecompilerCtxt);
	}


	//------------------------------------------------------------------------------
	VOID __fastcall RpcDecompilerUninit(VOID* pContext)
	{
		RpcDecompilerCtxt_T *	pRpcDecompilerCtxt = (RpcDecompilerCtxt_T*) pContext;

		if (pRpcDecompilerCtxt == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;

		//Free the private the context
		RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerUninit\n");
		RPC_FREE_FN(pRpcDecompilerCtxt->pRpcModuleInfo);
		RPC_FREE_FN( pRpcDecompilerCtxt );

	End:
		return;
	}



	BOOL __fastcall RpcDecompilerPrintProcedure(VOID* pContext, UINT ProcIndex)
	{
        UNREFERENCED_PARAMETER(ProcIndex);
        UNREFERENCED_PARAMETER(pContext);

		return FALSE;
	}


	BOOL __fastcall RpcDecompilerDecodeOneProcedureInlined(
		VOID*							pContext, 
		UINT							ProcIndex,
		IdlFunctionDesc&				IdlFunctionDesc,
		std::list<TypeToDefine>&		listProcType
		)
	{
		BOOL					bResult				= FALSE;
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;

		inlinedParam			param;
		BOOL					noMoreParameter		= FALSE;
		BOOL					noMoreParameterBecauseOfReturn = FALSE;
		UINT					nbOfParameters		= 0;

        UNREFERENCED_PARAMETER(listProcType);

		RVA_T pData = pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcFormatInlined[ProcIndex];

		// check if we have an entry defined in ppProcFormatInlined
		if(pData == NULL)
		{
			RPC_DEBUG_FN("no entry defined for this function in ppProcFormatInlined");
			bResult = FALSE;

			goto END;
		}

		// Loop over each parameter and add them to the list
		while(!noMoreParameter  && !noMoreParameterBecauseOfReturn)
		{
			ParamDesc	parameter;
			RPC_GET_PROCESS_DATA(pData, &param, sizeof(param));

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN("Failed to read proc format string in DecodeOneProcedureInlined");
			}

			// Set parameter flags (in, out)
			switch(param.baseParam.bFcTypeParamProperties)
			{
			case FC_IN_PARAM_BASETYPE:
				parameter.setFcType((FC_TYPE)param.baseParam.bFcType);
			case FC_IN_PARAM:
				parameter.setIsIn();
				break;
			case FC_IN_PARAM_NO_FREE_INST:
				// TODO
				break;
			case FC_IN_OUT_PARAM:
				parameter.setIsIn();
				parameter.setIsOut();
				break;
			case FC_OUT_PARAM:
				parameter.setIsOut();
				break;
			case FC_RETURN_PARAM_BASETYPE:
				parameter.setFcType((FC_TYPE)param.baseParam.bFcType);
			case FC_RETURN_PARAM:
				parameter.setIsReturn();
				IdlFunctionDesc.setHasReturn(TRUE);
				noMoreParameterBecauseOfReturn = TRUE;
				// TODO : check in which case this is true
				break;
				// End of parameters
			case FC_END:
			case FC_ZERO:
				noMoreParameter = TRUE;
				break;
			default:
				bResult = FALSE;
				break;
			}

			//No parameter left, dont add anything to the list
			if(!noMoreParameter)
			{
				// Jump to next parameter in the format string
				if(isSimpleType(parameter.getFcType()))
				{

					// Parameter type is right after the param description
					parameter.setRva(pData + FIELD_OFFSET(INLINED_PARAM_BASE_T, bFcType));
					pData += sizeof(INLINED_PARAM_BASE_T);
				}else{
					//FC_TYPE complexType;
					BYTE bfcType;

					// Type given in the type format string
					parameter.setRva(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString + param.complexParam.offset);


					RPC_GET_PROCESS_DATA(parameter.getRva(), &bfcType, sizeof(bfcType));
					parameter.setFcType((FC_TYPE)bfcType);

					pData += sizeof(INLINED_PARAM_COMPLEX_T);
				}	

				IdlFunctionDesc.addParamToList(parameter);
				nbOfParameters++;
			}
		}
		IdlFunctionDesc.setNbParam(nbOfParameters);

END:

		return bResult;
	}



	BOOL __fastcall RpcDecompilerPrintOneProcedureInlined(
		VOID*							pContext, 
		UINT							ProcOffset,
		IdlFunctionDesc&				IdlFunctionDesc,
		std::list<TypeToDefine>&		listProcType,
		std::ostringstream&				ossProc)
	{
		BOOL					bResult = FALSE;

        UNREFERENCED_PARAMETER(ProcOffset);

		ossProc << "\t";

		if(!IdlFunctionDesc.hasReturn())
		{
			ossProc << "void ";
		}else
		{
			for(auto iter = IdlFunctionDesc.getParamList().begin(); iter != IdlFunctionDesc.getParamList().end(); iter++)
			{
				if (iter->isReturn())
				{
					rpcDumpType(pContext, *iter, listProcType, ossProc);
				}
			}
		}

		ossProc << IdlFunctionDesc.getFunctionName() << " ";
		ossProc << "(";
		UINT32 i = 0;
		for(auto iter = IdlFunctionDesc.getParamList().begin(); iter != IdlFunctionDesc.getParamList().end(); iter++)
		{
			ossProc << std::endl << "\t\t";

			if (!(iter->isReturn()))
			{
				if(iter->isIn())
				{
					ossProc << "[in]";
				}
				if(iter->isOut())
				{
					ossProc << "[out]";
				}

				ossProc << " ";
				bResult = rpcDumpType(pContext, *iter, listProcType, ossProc);
				ossProc << " arg_" << i;
				// If this is not the last parameter (list contains all parameters including returned value)
				if(i != (IdlFunctionDesc.getNbParam() - 2))
				{
					ossProc  << ",";
				}// else nothing to do

				i++;
			}
		}
		ossProc << ");\n";
		return bResult;
	}


	//------------------------------------------------------------------------------
	BOOL __fastcall RpcDecompilerPrintOneProcedure(
		VOID*	pContext, 
		UINT	ProcIndex, 
		std::list<TypeToDefine>& listProcType,
		std::ostringstream& ossProc)
	{
		UINT					formatStringOffset	= 0;
		UINT					paramSizeInBytes	= RPC_DECOMPILER_INVALID_PARAM_SIZE;
		BOOL					bResult				= FALSE;
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;

		UINT					paramOffset			= 0;
		UINT					numParam			= 0;
		BOOL					isReturnParam		= FALSE;
		BOOL					nextIsReturnParam	= FALSE;
		UINT					dummy;

		IdlFunctionDesc			IdlFunctionDesc;


		if (pRpcDecompilerCtxt == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL) goto End;


		if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pFormatStringOffsetTable==NULL)
		{
			formatStringOffset = 0;
		}
		else
		{
			formatStringOffset = pRpcDecompilerCtxt->pRpcDecompilerInfo->pFormatStringOffsetTable[ProcIndex];
		}

		// carriage return before display function
		//ossProc << "\t/* Function 0x" << std::hex << ProcIndex<< " */"<< std::endl;
		ossProc << std::endl;
		ossProc << "[helpstring(\"RVA: 0x" << std::hex << pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcAddressTable[ProcIndex] << "\")]" << std::endl;
		RpcDecompilerPrintFunctionDbgInfo(pContext, ProcIndex, ossProc);


		//todo
		bResult = RpcDecompilerDecodeAndPrintPrototypeReturnType(
			/* in */ pRpcDecompilerCtxt, 
			/* in */ formatStringOffset, 
			/* out */ &paramOffset, 
			/* out */ &dummy,
			/* out */ IdlFunctionDesc,
			/* in/out */listProcType,
			/* in/out */ ossProc);

		if (bResult == FALSE) goto End;

		bResult = RpcDecompilerPrintPrototypeName(
			/* in */ pRpcDecompilerCtxt, 
			/* in */ ProcIndex,
			/* in/out */ ossProc);

		if (bResult == FALSE) goto End;

		RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: RpcDecompilerPrintPrototypeName returned nbParamToPrint = %d\n", IdlFunctionDesc.getNbParam());


		if(IdlFunctionDesc.getNbParam() == 0)
		{
			//No parameter to be printed
			ossProc << " void ";
		}


		//Print each parameter
		while( (numParam < IdlFunctionDesc.getNbParam()) )
		{
			RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: numParam = 0x%x on total to print = 0x%x\n", numParam, IdlFunctionDesc.getNbParam());

			bResult = RpcDecompilerGetReturnParamInfo(/* in */ pRpcDecompilerCtxt, /* in */ paramOffset, /* in */ paramDescrOif, /* out */ &isReturnParam);
			if (bResult == FALSE) goto End;

			if( ! isReturnParam)
			{
				//Print the parameter
				bResult = RpcDecompilerPrintParam(
					/* in */ pRpcDecompilerCtxt, 
					/* in */ paramOffset, 
					/* in */ paramDescrOif, 
					/* out */ &paramSizeInBytes, 
					/* in  */ IdlFunctionDesc,
					listProcType, 
					ossProc); //TODO : decompile paramDescrOi...

				if (bResult == FALSE || paramSizeInBytes == RPC_DECOMPILER_INVALID_PARAM_SIZE)
				{
					displayErrorMessage(ossProc, "RpcDecompilerPrintOneProcedure : unable to decode param");
					goto End;
				}
			}
			else
			{
				paramSizeInBytes = OIF_PARAM_SIZE; //TODO : traiter le cas où codage pas OIF
			}
			paramOffset += paramSizeInBytes;//paramSizeInBytes;
			numParam++;
			RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: paramOffset = %d, numParam = %d\n", paramOffset, numParam);

			//Is there one additionnal parameter to be printed ?
			if ( (! isReturnParam) && (numParam < IdlFunctionDesc.getNbParam()) )
			{
				//The last parameter has been printed because it was not a return parameter
				//There is still at least 1 parameter to be printed
				bResult = RpcDecompilerGetReturnParamInfo(/* in */ pRpcDecompilerCtxt, /* in */ paramOffset, /* in */ paramDescrOif, /* out */ &nextIsReturnParam);
				if (bResult == FALSE) goto End;

				if (! nextIsReturnParam)
				{
					//The next parameter will have to be printed because it is not a return parameter
					ossProc << ", ";
				}

			}
		}//while(numParam <= IdlFunctionDesc.getNbParam());


		// Print the end of the procedure prototype
		ossProc<<");"<<std::endl;

		bResult = TRUE;

End:
		return (bResult);
	}


	BOOL	__fastcall RpcDecompilerPrintHiddenFUProcedure(VOID* pContext, UINT * procOffset, std::list<TypeToDefine>& listProcType, std::ostringstream& ossProc)
	{
		UINT					paramSizeInBytes	= RPC_DECOMPILER_INVALID_PARAM_SIZE;
		BOOL					bResult				= FALSE;
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;

		UINT					paramOffset			= 0;
		UINT					numParam			= 0;
		BOOL					isReturnParam		= FALSE;
		BOOL					nextIsReturnParam	= FALSE;
		UINT					sizeOfProcDescr		= 0;

		IdlFunctionDesc			IdlFunctionDesc;


		if (pRpcDecompilerCtxt == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL) goto End;

		RVA_T pFunction = pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + *procOffset;
		// carriage return before display function
		//ossProc << "\t/* Function 0x" << std::hex << ProcIndex<< " */"<< std::endl;
		ossProc << std::endl;
		//RpcDecompilerPrintFunctionDbgInfo(pContext, *procOffset, ossProc);
		ossProc << "\t /* Function index : 0x" << std::hex << *procOffset;
		ossProc << "\t Module Base : 0x" << (unsigned long) pRpcDecompilerCtxt->pRpcDecompilerInfo->pModuleBase;
		ossProc << "\t RVA of proc in format string : 0x" << (unsigned long) ((UINT64)pFunction - pRpcDecompilerCtxt->pRpcDecompilerInfo->pModuleBase);
		ossProc << "  */"<<std::endl;

		//todo
		bResult = RpcDecompilerDecodeAndPrintPrototypeReturnType(
			/* in */ pRpcDecompilerCtxt, 
			/* in */ *procOffset, 
			/* out */ &paramOffset, 
			/* out */ &sizeOfProcDescr,
			/* out */ IdlFunctionDesc,
			/* in/out */listProcType,
			/* in/out */ ossProc);

		if (bResult == FALSE) goto End;

		ossProc << " _HiddenFunction_" << std::dec << *procOffset << "(";

		// Increment procOffset to read the next procedure description
		*procOffset += sizeOfProcDescr;

		if (bResult == FALSE) goto End;

		RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: RpcDecompilerPrintPrototypeName returned nbParamToPrint = %d\n", IdlFunctionDesc.getNbParam());


		if(IdlFunctionDesc.getNbParam() == 0)
		{
			//No parameter to be printed
			ossProc << " void ";
		}


		//Print each parameter
		while( (numParam < IdlFunctionDesc.getNbParam()) )
		{
			RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: numParam = 0x%x on total to print = 0x%x\n", numParam, IdlFunctionDesc.getNbParam());

			bResult = RpcDecompilerGetReturnParamInfo(/* in */ pRpcDecompilerCtxt, /* in */ paramOffset, /* in */ paramDescrOif, /* out */ &isReturnParam);
			if (bResult == FALSE) goto End;

			if( ! isReturnParam)
			{
				//Print the parameter
				bResult = RpcDecompilerPrintParam(
					/* in */ pRpcDecompilerCtxt, 
					/* in */ paramOffset, 
					/* in */ paramDescrOif, 
					/* out */ &paramSizeInBytes, 
					/* in  */ IdlFunctionDesc,
					listProcType, 
					ossProc); //TODO : décompiler paramDescrOi en plus de paramDescrOif

				if (bResult == FALSE || paramSizeInBytes == RPC_DECOMPILER_INVALID_PARAM_SIZE)
				{
					displayErrorMessage(ossProc, "RpcDecompilerPrintOneProcedure : unable to decode param");
					goto End;
				}
			}
			else
			{
				paramSizeInBytes = OIF_PARAM_SIZE; //TODO : traiter le cas où codage pas OIF
			}
			paramOffset += paramSizeInBytes;//paramSizeInBytes;
			numParam++;
			RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerPrintProcedure: paramOffset = %d, numParam = %d\n", paramOffset, numParam);

			//Is there one additionnal parameter to be printed ?
			if ( (! isReturnParam) && (numParam < IdlFunctionDesc.getNbParam()) )
			{
				//The last parameter has been printed because it was not a return parameter
				//There is still at least 1 parameter to be printed
				bResult = RpcDecompilerGetReturnParamInfo(/* in */ pRpcDecompilerCtxt, /* in */ paramOffset, /* in */ paramDescrOif, /* out */ &nextIsReturnParam);
				if (bResult == FALSE) goto End;

				if (! nextIsReturnParam)
				{
					//The next parameter will have to be printed because it is not a return parameter
					ossProc << ", ";
				}

			}
		}//while(numParam <= IdlFunctionDesc.getNbParam());


		// Print the end of the procedure prototype
		ossProc<<");"<<std::endl;

		bResult = TRUE;

End:
		return (bResult);

	}

	//------------------------------------------------------------------------------
	VOID __fastcall RpcDecompilerPrintFunctionDbgInfo(VOID* pContext, UINT procIndex, std::ostringstream& oss)
	{
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;
		UINT64 moduleBase = pRpcDecompilerCtxt->pRpcDecompilerInfo->pModuleBase;
		RVA_T rvaFunction		= pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcAddressTable[procIndex];

		oss << "\t /* Function index : 0x" << std::hex << procIndex;
		oss << "\t Module Base : 0x" << moduleBase;
		oss << "\t RVA : 0x" << rvaFunction;
		oss << "  */"<<std::endl;
	}


	//------------------------------------------------------------------------------
	VOID __fastcall RpcDecompilerPrintProlog(VOID* pContext, std::ostringstream& oss)
	{
		UCHAR*					pUuidString			= NULL;
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;

		if (pRpcDecompilerCtxt == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;


		if ( UuidToStringA(&pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId->Uuid,&pUuidString) != RPC_S_OK )
		{
			goto End;
		}
		oss<< "[\nuuid(" << pUuidString; 
		oss<<"),\nversion("<< pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId->VersMajor;
		oss<<"."<<pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId->VersMinor <<"),\n]\n"; 
/*
		RPC_DEBUG_FN((UCHAR*)"[\nuuid(%s),\nversion(%u.%u),\n]\n", 
			pUuidString, 
			pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId->SyntaxVersion.MajorVersion,
			pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId->SyntaxVersion.MinorVersion
			);
*/

End:
		if (pUuidString != NULL) RpcStringFreeA(&pUuidString);
		return;
	}

	//------------------------------------------------------------------------------
	VOID RpcPrintInformation(VOID* pContext, std::ostringstream& oss)
	{
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;

		if (pRpcDecompilerCtxt == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;


		//TODO : fonction de lecture de tous les headers de proc pour lister tous les handles et ecrire le fichier ACF
		oss << "\n/*****************************************************************";
		oss << "\n * Do not forget to write the ACF file using the following model:";
		oss << "\n   // ACF file header";
		oss << "\n   [";
		oss << "\n   \t// List here the handles and other specific options.\n";
		oss << "\n   ]\n";
		oss << "\n   // ACF file body";
		//oss << "\n   interface "<< narrow(std::wstring(DEFAULT_IF_NAME)) <<"\n   {\n   }";
		oss << "\n   interface "<< DEFAULT_IF_NAME <<"\n   {\n   }";
		oss << "\n\n *";
		oss << "\n *****************************************************************/\n\n";

End:;

	}

	BOOL __fastcall RpcDecompilerPrintAllProceduresInlined(VOID* pContext)
    {
        UNREFERENCED_PARAMETER(pContext);
		return FALSE;
		/* */
	}

	//#define DBG_BUF_SIZE	512

	//------------------------------------------------------------------------------
	BOOL __fastcall RpcDecompilerPrintAllProcedures(VOID* pContext)
	{

		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T*)pContext;
		UINT					uProcIdx;
		std::list<TypeToDefine>			listProcType;
		std::ostringstream		ossHeader;
		std::ostringstream		ossProc;
		std::ostringstream		ossType;
		//BYTE					dbgBuf[DBG_BUF_SIZE];

		if (pRpcDecompilerCtxt == NULL)
		{
			return FALSE;		
		}
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL)
		{
			return FALSE;
		}
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL)
		{
			return FALSE;
		}

		RPC_DEBUG_FN((UCHAR*)"RpcDecompilerPrintAllProcedures\n");
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL)
		{
			RPC_DEBUG_FN((UCHAR*)"Cannot decompile inlined stub!\n");
			//return FALSE;
		}


		//// <dbg>

		//dump proc index offset :

		//ossHeader << "proc index offset "<<std::endl;

		//for(int i=0; i< pRpcDecompilerCtxt->pRpcDecompilerInfo->NumberOfProcedures; i++)
		//{
		//	ossHeader<<"proc "<<i<<"offset : "<<pRpcDecompilerCtxt->pRpcDecompilerInfo->pFormatStringOffsetTable[i] << std::endl;
		//}
		//
		//// Dump proc type and proc format string
		//RPC_GET_PROCESS_DATA(
		//		pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString,
		//		dbgBuf, 
		//		DBG_BUF_SIZE
		//		);

		//ossHeader<<"ProcFormatString : "<<std::endl;
		//for(int i=0; i<DBG_BUF_SIZE; i++)
		//{

		//	ossHeader<<" 0x"<<std::hex<<(int)dbgBuf[i];
		//	if(i % 2 == 0 )
		//	{
		//		ossHeader<<"/* "<<i<<" */";
		//	}

		//	ossHeader<<std::endl;
		//}


		//RPC_GET_PROCESS_DATA(
		//	(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString ),
		//	dbgBuf, 
		//	DBG_BUF_SIZE
		//	);	

		//ossHeader<<"TypeFormatString : "<<std::endl;
		//for(int i=0; i<DBG_BUF_SIZE; i++)
		//{

		//	ossHeader<<" 0x"<<std::hex<<(int)dbgBuf[i];
		//	if(i % 2 == 0 )
		//	{
		//		ossHeader<<"/* "<<i<<" */";
		//	}

		//	ossHeader<<std::endl;
		//}
		//// </end dbg>


		RpcPrintInformation(pContext, ossHeader);

		RpcDecompilerPrintProlog(pContext, ossHeader);


		// display Interface name

		ossHeader << "\n\ninterface ";
		// is there a default if name defined ?
		if(wcslen(pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName) > 0)
		{
			size_t szConverted = 0;
			size_t sz = wcslen(pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName) + 1;
			char* pTmp = (char*)pRpcDecompilerCtxt->pRpcViewHelper->RpcAlloc(sz);

			if(pTmp != NULL)
			{
				ZeroMemory(pTmp, sz);

				wcstombs_s(&szConverted, pTmp, sz, pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName, sz);

				ossHeader << pTmp;
				pRpcDecompilerCtxt->pRpcViewHelper->RpcFree(pTmp);
			}

			//ossHeader << narrow(std::wstring(pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName)) <<" \n{\n";


			/*RPC_DEBUG_FN((UCHAR*) "[ERROR] an error has occured while decompilating proc : %d \n",uProcIdx);
			ossProc << "[ERROR] an error has occured while decompilating proc : "<<uProcIdx<<std::endl;
			RPC_PRINT_FN ((UCHAR*)ossHeader.str().c_str());
			RPC_PRINT_FN((UCHAR*)ossProc.str().c_str());
			return FALSE;*/
		}
		else
		{
			//ossHeader << narrow(std::wstring(DEFAULT_IF_NAME)) <<" \n{\n";
			ossHeader << DEFAULT_IF_NAME <<" \n{\n";
		}
		RPC_DEBUG_FN((UCHAR*)"\ninterface %ls\n{\n", DEFAULT_IF_NAME);
		//Decompile each type
		//if ( RpcDecompilerPrintAllTypes(pContext) == FALSE ) goto End;

		// pFormatStringOffsetTable can be (is always?) null for DCOM interfaces
		//		if(pRpcDecompilerCtxt->pRpcDecompilerInfo->pFormatStringOffsetTable != NULL)
		//		{

		//Decompile each proc
		for(uProcIdx = 0; uProcIdx < pRpcDecompilerCtxt->pRpcDecompilerInfo->NumberOfProcedures; uProcIdx++)
		{

			// select decompile method
			//if(pRpcDecompilerCtxt->pRpcDecompilerInfo->pbFunctionInterpreted[uProcIdx] == TRUE)
			//{
				// interpreted case
				if ( RpcDecompilerPrintOneProcedure(pContext, uProcIdx, listProcType, ossProc) == FALSE )
				{
					RPC_DEBUG_FN((UCHAR*) "[ERROR] an error has occured while decompiling proc : %d \n",uProcIdx);
					ossProc << "[ERROR] an error has occured while decompiling proc : "<<uProcIdx<<std::endl;
				}
		//	}
		//	else
		//	{
		//		std::stringstream   ss; 
		//		BOOL				bResult = FALSE;

		//		// TODO  : factorize fnname
		//		ss << "_Function_0x" << std::hex << uProcIdx << std::dec;
		//		IdlFunctionDesc		idlFnDesc(ss.str());


		//		// inlined case
		//		ossProc << std::endl << "\t/* Function index 0x" << std::hex << uProcIdx << " inlined : will be done " << std::endl;

		//		//bResult = RpcDecompilerDecodeOneProcedureInlined(pContext, uProcIdx, idlFnDesc, listProcType);
		//		//if(bResult == FALSE)
		//		//{
		//		//	ossProc << "[ERROR] an error has occured while decompiling inlined proc : 0x" << std::hex << uProcIdx << std::dec << std::endl;
		//		//}

		//		//bResult = RpcDecompilerPrintOneProcedureInlined(pContext, uProcIdx, idlFnDesc, listProcType, ossProc);

		//		// TO BE FILLED 
		//	}/*	endif																						*/


		}

		//		}

		ossProc << "\n\n}\n";



		// dump type list accorded to saved types offset 

		RpcDecompilerPrintAllTypesInList(pContext, listProcType, ossType);

		// and then dump content
		RPC_PRINT_FN(pRpcDecompilerCtxt->pRpcViewHelper->pContext,(const char*)ossHeader.str().c_str());
		RPC_PRINT_FN(pRpcDecompilerCtxt->pRpcViewHelper->pContext,(const char*)ossType.str().c_str());
		RPC_PRINT_FN(pRpcDecompilerCtxt->pRpcViewHelper->pContext,(const char*)ossProc.str().c_str());


		return TRUE;
	}//end RpcDecompilerPrintAllProcedures

	BOOL	__fastcall RpcDecompilerPrintAllProceduresNew(VOID* pContext)
	{
		RpcDecompilerCtxt_T*	pRpcDecompilerCtxt = (RpcDecompilerCtxt_T*)pContext;
		std::ostringstream		ossIf;
		DECOMP_STATUS			status;
		std::string				strIfname;

		if (pRpcDecompilerCtxt == NULL)
		{
			return FALSE;
		}
		if (pRpcDecompilerCtxt->pRpcViewHelper == NULL)
		{
			return FALSE;
		}
		if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL)
		{
			return FALSE;
		}

		if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL)
		{
			RPC_DEBUG_FN((UCHAR*)"Cannot decompile inlined stub!\n");
			//return FALSE;
		}
		

		// convert interface name to std::string
		if(wcslen(pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName) == 0)
		{
			strIfname = DEFAULT_IF_NAME;
		}
		else
		{
			strIfname = narrow(std::wstring(pRpcDecompilerCtxt->pRpcDecompilerInfo->InterfaceName));
		}
		
		
		// Create idlInterface used to decode interface
		IdlInterface		idlIf(strIfname, *(pRpcDecompilerCtxt->pRpcDecompilerInfo->pIfId), pRpcDecompilerCtxt->pRpcDecompilerInfo->NumberOfProcedures);
		
		status = idlIf.decode(pContext);

		if(status != DS_SUCCESS)
		{
			RPC_ERROR_FN("decompilation failed\n");
		}
		ossIf << idlIf;
		// and then dump content
		RPC_PRINT_FN(pRpcDecompilerCtxt->pRpcViewHelper->pContext,ossIf.str().c_str());
		
		return TRUE;
	}//end RpcDecompilerPrintAllProcedures


	//------------------------------------------------------------------------------
	BOOL WINAPI DllMain(HANDLE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
	{
        UNREFERENCED_PARAMETER(hInstDLL);
        UNREFERENCED_PARAMETER(dwReason);
        UNREFERENCED_PARAMETER(lpvReserved);
		//nothing to do here!
		return (TRUE);
	}

#ifdef __cplusplus
}
#endif
