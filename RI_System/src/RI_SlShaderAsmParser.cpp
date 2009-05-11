//==================================================================
/// RI_SlShaderAsmParser.cpp
///
/// Created by Davide Pasca - 2009/3/4
/// See the file "license.txt" that comes with this project for
/// copyright info. 
//==================================================================

#include "stdafx.h"
#include <stdarg.h>
#include "DUtils.h"
#include "RI_SlRunContext.h"
#include "RI_SlShaderAsmParser.h"

//==================================================================
namespace RI
{

//==================================================================
struct OpCodeDef
{
	const char	*pName;
	u_int		OperCnt;
};

//==================================================================
static OpCodeDef	gsOpCodeDefs[] =
{
	"movss",		2,
	"movvs",		2,
	"movvv",		2,
	"addss",		3,
	"addvs",		3,
	"addvv",		3,
	"subss",		3,
	"subvs",		3,
	"subvv",		3,
	"mulss",		3,
	"mulvs",		3,
	"mulvv",		3,
	"divss",		3,
	"divvs",		3,
	"divvv",		3,
	"normalize",	2,
	"faceforward",	3,
	"diffuse",		2,
	"ambient",		1,
	NULL
};

//==================================================================
/// ShaderAsmParser
//==================================================================
ShaderAsmParser::ShaderAsmParser( DUT::MemFile &file, SlShader *pShader, const char *pName ) :
	mpFile(&file),
	mpShader(pShader),
	mpName(pName)
{
	doParse( file );
}

//==================================================================
static void stripComments( char *pTxt )
{
	while ( pTxt[0] )
	{
		if ( pTxt[0] == ';' )
		{
			pTxt[0] = 0;
			return;
		}

		if ( pTxt[0] == '/' )
			if ( pTxt[1] == '/' )
			{
				pTxt[0] = 0;
				return;
			}
			
		++pTxt;
	}
}

//==================================================================
void ShaderAsmParser::doParse( DUT::MemFile &file )
{
	char		lineBuff[1024];
	Section		curSection = CODE;
	int			lineCnt = 0;

	
	while ( file.ReadTextLine( lineBuff, sizeof(lineBuff) ) )
	{
		char	lineWork[1024];

		strcpy_s( lineWork, lineBuff );

		stripComments( lineWork );
	
		DUT::StrStripBeginEndWhite( lineWork );

		if NOT( lineWork[0] )
		{
			++lineCnt;
			continue;
		}

		try 
		{
			if ( mpShader->mType == SlShader::TYPE_UNKNOWN )
			{
				if ( 0 == strcasecmp( lineWork, "surface" ) )
					mpShader->mType = SlShader::TYPE_SURFACE;
				else
				if ( 0 == strcasecmp( lineWork, "light" ) )
					mpShader->mType = SlShader::TYPE_LIGHT;
				else
				if ( 0 == strcasecmp( lineWork, "displacement" ) )
					mpShader->mType = SlShader::TYPE_DISPLACEMENT;
				else
				{
					onError( "Shader type undefined !" );
				}
			}
			else
			{
				if ( 0 == strcasecmp( lineWork, ".data" ) )
					curSection = DATA;
				else
				if ( 0 == strcasecmp( lineWork, ".code" ) )
					curSection = CODE;
				else
				if ( curSection == DATA )
				{
					parseDataLine( lineWork, lineCnt );
				}
				else
				{
					parseCodeLine( lineWork, lineCnt );
				}
			}
		}
		catch ( ... )
		{
			printf( "For shader '%s' at line %i\n%i) %s", mpName, lineCnt+1, lineCnt+1, lineBuff );
			throw;
		}

		++lineCnt;
	}
}

//==================================================================
void ShaderAsmParser::getVector( const char *pStr, float out_val[], int n )
{
	int	gotN;
	
	if ( n == 1 ) gotN = sscanf( pStr, "%f"			, out_val+0, out_val+1, out_val+2 ); else
	if ( n == 2 ) gotN = sscanf( pStr, "%f %f"		, out_val+0, out_val+1, out_val+2 ); else
	if ( n == 3 ) gotN = sscanf( pStr, "%f %f %f"	, out_val+0, out_val+1, out_val+2 ); else
	if ( n == 4 ) gotN = sscanf( pStr, "%f %f %f %f", out_val+0, out_val+1, out_val+2 ); else
	{
		onError( "getVector() can't handle so many dimensions (^^;) !" );
	}

	if ( gotN != n && gotN != 1 )
		onError( "Got %i values, but %i (or 1) are required !", gotN, n );

	if ( gotN == 1 )
		for (int i=1; i < n; ++i)
			out_val[i] = out_val[0];
}

//==================================================================
void ShaderAsmParser::parseDataLine( char lineBuff[], int lineCnt )
{
	//printf( "DATA: %s\n", lineBuff );
	
	const char *pLineEnd = lineBuff + strlen( lineBuff );
	
	char *pTokCtx;
	char *pTok;
	
	SlSymbol	symbol;		
	symbol.Reset();

	if NOT( pTok = strtok_r( lineBuff, " \t", &pTokCtx ) )
		return;

	symbol.mName = pTok;

	if NOT( pTok = strtok_r(NULL, " \t", &pTokCtx) )
	{
		onError( "ERROR: Expecting storage definition" );
	}
		
	if ( 0 == strcmp( pTok, "constant"  ) ) symbol.mStorage = SlSymbol::CONSTANT ; else
	if ( 0 == strcmp( pTok, "parameter" ) ) symbol.mStorage = SlSymbol::PARAMETER; else
	if ( 0 == strcmp( pTok, "temporary" ) ) symbol.mStorage = SlSymbol::TEMPORARY; else
	if ( 0 == strcmp( pTok, "global"	) ) symbol.mStorage = SlSymbol::GLOBAL   ; else
	{
		onError( "ERROR: Invalid storage definition: '%s'", pTok );
	}

	if NOT( pTok = strtok_r(NULL, " \t", &pTokCtx) )
	{
		onError( "ERROR: Expecting variability definition" );
	}

	if ( 0 == strcmp( pTok, "varying"	) ) symbol.mIsVarying = true ; else
	if ( 0 == strcmp( pTok, "uniform"	) ) symbol.mIsVarying = false; else
	{
		onError( "ERROR: Invalid variability definition: '%s'", pTok );
	}

	if NOT( symbol.mIsVarying )
		if ( symbol.mStorage == SlSymbol::TEMPORARY )
			onError( "Use 'temporary' only for varying values !" );


	if NOT( pTok = strtok_r(NULL, " \t", &pTokCtx) )
	{
		onError( "ERROR: Expecting type definition" );
	}

	if ( 0 == strcmp( pTok, "float"  ) ) symbol.mType = SlSymbol::FLOAT ; else
	if ( 0 == strcmp( pTok, "point"  ) ) symbol.mType = SlSymbol::POINT ; else
	if ( 0 == strcmp( pTok, "color"  ) ) symbol.mType = SlSymbol::COLOR ; else
	if ( 0 == strcmp( pTok, "string" ) ) symbol.mType = SlSymbol::STRING; else
	if ( 0 == strcmp( pTok, "vector" ) ) symbol.mType = SlSymbol::VECTOR; else
	if ( 0 == strcmp( pTok, "normal" ) ) symbol.mType = SlSymbol::NORMAL; else
	if ( 0 == strcmp( pTok, "matrix" ) ) symbol.mType = SlSymbol::MATRIX; else
	{
		onError( "ERROR: Invalid type definition: '%s'", pTok );
	}

	int	defParamCnt = 0;

	const char *pDefaultValueStr = pTok + strlen(pTok) + 1;
	if ( pDefaultValueStr < pLineEnd )
	{
		if ( symbol.mIsVarying )
			onError( "No default values possible for varying variables !" );

		symbol.mArraySize = 1;
		symbol.AllocData();

		float	tmp[64];

		switch ( symbol.mType )
		{
		case SlSymbol::FLOAT :
						getVector( pDefaultValueStr, tmp, 1 );
						((SlScalar *)symbol.mpDefaultVal)[0] = tmp[0];
						break;

		case SlSymbol::POINT :
						getVector( pDefaultValueStr, tmp, 3 );
						((SlVec3 *)symbol.mpDefaultVal)[0][0] = tmp[0];
						((SlVec3 *)symbol.mpDefaultVal)[0][1] = tmp[1];
						((SlVec3 *)symbol.mpDefaultVal)[0][2] = tmp[2];
						break;

		case SlSymbol::COLOR :
						getVector( pDefaultValueStr, tmp, 3 );
						((SlColor *)symbol.mpDefaultVal)[0][0] = tmp[0];
						((SlColor *)symbol.mpDefaultVal)[0][1] = tmp[1];
						((SlColor *)symbol.mpDefaultVal)[0][2] = tmp[2];
						break;
		
		case SlSymbol::VECTOR:
		case SlSymbol::NORMAL:
						getVector( pDefaultValueStr, tmp, 3 );
						((SlVec3 *)symbol.mpDefaultVal)[0][0] = tmp[0];
						((SlVec3 *)symbol.mpDefaultVal)[0][1] = tmp[1];
						((SlVec3 *)symbol.mpDefaultVal)[0][2] = tmp[2];
						break;
		
		case SlSymbol::STRING:
		case SlSymbol::MATRIX:
			{
				onError( "Currently unsupported default value (^^;> '%s'", pTok );
			}
		}
	
		printf( "Default value '%s'\n", pDefaultValueStr );
	}
		
	mpShader->mSymbols.push_back( symbol );
}

//==================================================================
const OpCodeDef	*ShaderAsmParser::findOpDef( const char *pOpName, u_int &opCodeIdx )
{
	for (size_t i=0; gsOpCodeDefs[i].pName != NULL; ++i)
		if ( 0 == strcasecmp( pOpName, gsOpCodeDefs[i].pName ) )
		{
			opCodeIdx = i;
			return gsOpCodeDefs + i;
		}
		
	return NULL;
}

//==================================================================
int ShaderAsmParser::findSymbol( const char *pName, bool ignoreCase ) const
{
	for (size_t i=0; i < mpShader->mSymbols.size(); ++i)
	{
		const char *pShaderSymName = mpShader->mSymbols[i].mName.c_str();
		
		if ( ignoreCase )
		{
			if ( 0 == strcasecmp( pName, pShaderSymName ) )
				return i;
		}
		else
		{
			if ( 0 == strcmp( pName, pShaderSymName ) )
				return i;
		}
	}

	return -1;
}

//==================================================================
int ShaderAsmParser::findOrAddTempSymbol( const char *pName )
{
	int	idx = findSymbol( pName, true );
	if ( idx != -1 )
		return idx;
		
	size_t	len = strlen( pName );
	if ( len < 3 )
		return -1;
		
	SlSymbol	symbol;		
	symbol.Reset();
	symbol.mName	= pName;
	symbol.mStorage	= SlSymbol::TEMPORARY;
	symbol.mIsVarying	= true;
	symbol.mArraySize	= 0;
	symbol.mpDefaultVal	= NULL;

	int	retIdx = (int)mpShader->mSymbols.size();

	if ( pName[1] == 's' || pName[1] == 'S' )
	{
		// scalar base
		symbol.mType	= SlSymbol::FLOAT;
		mpShader->mSymbols.push_back( symbol );
		return retIdx;
	}
	else
	if ( pName[1] == 'v' || pName[1] == 'V' )
	{
		// vector base
		symbol.mType	= SlSymbol::VECTOR;
		mpShader->mSymbols.push_back( symbol );
		return retIdx;
	}
	else
		return -1;
}


//==================================================================
static OperTypeID getOperTypeFromSlSymbolType( SlSymbol::Type slSymType, bool &out_success )
{
	out_success = true;

	switch ( slSymType )
	{
	case SlSymbol::FLOAT:	return OPRTYPE_F1 ;
	//case SlSymbol:::		return OPRTYPE_F2 ;
	case SlSymbol::POINT:
	case SlSymbol::COLOR:
	case SlSymbol::VECTOR:
	case SlSymbol::NORMAL:
							return OPRTYPE_F3 ;
	//case SlSymbol:::		return OPRTYPE_F4 ;
	case SlSymbol::MATRIX:	return OPRTYPE_M44;
	
	default:
		out_success = false;
		return OPRTYPE_F1;
	}
}

//==================================================================
static bool isTempSymbol( const char *pTok )
{
	return pTok[0] == '$';
}

//==================================================================
void ShaderAsmParser::parseCodeLine( char lineBuff[], int lineCnt )
{
	//printf( "CODE: %s\n", lineBuff );
	
	char *pTokCtx;
	char *pTok;

	if NOT( pTok = strtok_r( lineBuff, " \t", &pTokCtx ) )
		return;
		
	DUT::StrStripBeginEndWhite( pTok );
	
	u_int	opCodeIdx;
	const OpCodeDef	*pOpDef = findOpDef( pTok, opCodeIdx );
	
	if NOT( pOpDef )
	{
		onError( "ERROR: Unknown opcode '%s' !\n", pTok );
	}

	SlCPUWord		instruction;
	instruction.mOpCode.mTableOffset	= opCodeIdx;
	instruction.mOpCode.mOperandCount	= pOpDef->OperCnt;
	instruction.mOpCode.mDestOpType		= 0;
	size_t	instrIdx = mpShader->mCode.size();
	mpShader->mCode.push_back( instruction );

	for (u_int i=0; i < pOpDef->OperCnt; ++i)
	{
		SlCPUWord	word;
		
		if NOT( pTok = strtok_r(NULL, " \t", &pTokCtx) )
		{
			onError( "ERROR: missing operand #%i, "
					"expecting %i operands\n",
					i+1,
					pOpDef->OperCnt );
		}

		DUT::StrStripBeginEndWhite( pTok );

		int	symbolIdx;
		if ( isTempSymbol( pTok ) )
		{
			symbolIdx = findOrAddTempSymbol( pTok );
		}
		else
		{
			symbolIdx = findSymbol( pTok, false );
		}

		if ( symbolIdx == -1 )
		{
			onError( "ERROR: Symbol '%s' unknown !\n", pTok );
		}

		word.mSymbol.mTableOffset = (u_int)symbolIdx;
		word.mSymbol.mIsVarying	  = mpShader->mSymbols[symbolIdx].mIsVarying;

		mpShader->mCode.push_back( word );
		
		// copy the type of the first operator into the opcode
		if ( i == 0 )
		{
			bool success;
			instruction.mOpCode.mDestOpType =
				getOperTypeFromSlSymbolType( mpShader->mSymbols[symbolIdx].mType, success );
				
			if NOT( success )
			{
				onError( "ERROR: Invalid operand type for '%s' !\n", pTok );
			}
		}
	}

	// write again
	mpShader->mCode[instrIdx] = instruction;
}

//==================================================================
void ShaderAsmParser::onError( const char *pFmt, ... )
{
	va_list	vl;
	va_start( vl, pFmt );

	char	buff[1024];
	vsnprintf( buff, _countof(buff)-1, pFmt, vl );

	va_end( vl );

	puts( buff );

	throw std::runtime_error( buff );
}

//==================================================================
}

