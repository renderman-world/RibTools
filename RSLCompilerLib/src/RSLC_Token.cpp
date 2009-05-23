//==================================================================
/// RSLC_Token.cpp
///
/// Created by Davide Pasca - 2009/5/23
/// See the file "license.txt" that comes with this project for
/// copyright info. 
//==================================================================

#include <stdlib.h>
#include "RSLC_Token.h"

//==================================================================
namespace RSLC
{

//==================================================================
struct TokenDef
{
	const char	*pStr;
	TokenIDType	idType;
	TokenID		id;
};

//==================================================================
#define OP_DEF(_STR_,_X_)	_STR_, T_TYPE_OPERATOR,	T_OP_##_X_
#define DT_DEF(_X_)			#_X_, T_TYPE_DATATYPE,	T_DT_##_X_
#define KW_DEF(_X_)			#_X_, T_TYPE_KEYWORD,	T_KW_##_X_
#define SF_DEF(_X_)			#_X_, T_TYPE_STDFUNC,	T_SF_##_X_

//==================================================================
static TokenDef _sTokenDefs[TOKEN_N] =
{
	NULL	,	T_TYPE_UNKNOWN	,	T_UNKNOWN	,

	NULL	,	T_TYPE_VALUE	,	T_VL_NUMBER	,
	NULL	,	T_TYPE_VALUE	,	T_VL_STRING	,

	OP_DEF(	"<="	,	LSEQ		)	,
	OP_DEF(	">="	,	GEEQ		)	,
	OP_DEF(	"<"		,	LSTH		)	,
	OP_DEF(	">"		,	GRTH		)	,
	OP_DEF(	"&&"	,	LOGIC_AND	)	,
	OP_DEF(	"||"	,	LOGIC_OR	)	,
	OP_DEF(	"=="	,	EQ			)	,
	OP_DEF(	"!="	,	NEQ			)	,
	OP_DEF(	"+="	,	PLUSEQ		)	,
	OP_DEF(	"-="	,	MINUSEQ		)	,
	OP_DEF(	"*="	,	MULEQ		)	,
	OP_DEF(	"/="	,	DIVEQ		)	,
	OP_DEF(	"+"		,	PLUS		)	,
	OP_DEF(	"-"		,	MINUS		)	,
	OP_DEF(	"*"		,	MUL			)	,
	OP_DEF(	"/"		,	DIV			)	,
	OP_DEF(	"="		,	ASSIGN		)	,
	OP_DEF(	"^"		,	POW			)	,
	OP_DEF(	"."		,	DOT			)	,
	OP_DEF(	":"		,	COLON		)	,
	OP_DEF(	","		,	COMMA		)	,
	OP_DEF(	";"		,	SEMICOL		)	,
	OP_DEF(	"("		,	LBRARND		)	,
	OP_DEF(	")"		,	RBRARND		)	,
	OP_DEF(	"["		,	LBRASQA		)	,
	OP_DEF(	"]"		,	RBRASQA		)	,
	OP_DEF(	"{"		,	LBRACRL		)	,
	OP_DEF(	"}"		,	RBRACRL		)	,

	DT_DEF( float			)	,
	DT_DEF( vector			)	,
	DT_DEF( point			)	,
	DT_DEF( normal			)	,
	DT_DEF( color			)	,
	DT_DEF( string			)	,

	KW_DEF( varying			)	,
	KW_DEF( uniform			)	,

	KW_DEF( if				)	,
	KW_DEF( for				)	,
	KW_DEF( while			)	,
	KW_DEF( solar			)	,
	KW_DEF( illuminate		)	,
	KW_DEF( illuminance		)	,
	KW_DEF( break			)	,
	KW_DEF( continue		)	,
	KW_DEF( return			)	,
		
		
	KW_DEF( texture			)	,	// texture_type
	KW_DEF( environment		)	,
	KW_DEF( bump			)	,
	KW_DEF( shadow			)	,
	KW_DEF( light			)	,	// shader types
	KW_DEF( surface			)	,
	KW_DEF( volume			)	,
	KW_DEF( displacement	)	,
	KW_DEF( transformation	)	,
	KW_DEF( imager			)	,

	SF_DEF( abs				)	,
	SF_DEF( ambient			)	,
	SF_DEF( calculatenormal	)	,
	SF_DEF( cellnoise		)	,
	SF_DEF( clamp			)	,
	SF_DEF( comp			)	,
	SF_DEF( concat			)	,
	SF_DEF( ctransform		)	,
	SF_DEF( determinant		)	,
	SF_DEF( diffuse			)	,
	SF_DEF( environment		)	,
	SF_DEF( faceforward		)	,
	SF_DEF( filterstep		)	,
	SF_DEF( format			)	,
	SF_DEF( inversesqrt		)	,
	SF_DEF( match			)	,
	SF_DEF( max				)	,
	SF_DEF( min				)	,
	SF_DEF( mix				)	,
	SF_DEF( noise			)	,
	SF_DEF( normalize		)	,
	SF_DEF( ntransform		)	,
	SF_DEF( pnoise			)	,
	SF_DEF( ptlined			)	,
	SF_DEF( random			)	,
	SF_DEF( reflect			)	,
	SF_DEF( rotate			)	,
	SF_DEF( scale			)	,
	SF_DEF( setcomp			)	,
	SF_DEF( shadow			)	,
	SF_DEF( specular		)	,
	SF_DEF( specularbrdf	)	,
	SF_DEF( spline			)	,
	SF_DEF( sqrt			)	,
	SF_DEF( texture			)	,
	SF_DEF( transform		)	,
	SF_DEF( translate		)	,
	SF_DEF( vtransform		)	,
};

//==================================================================
static size_t _sTokenDefsIdxInvSortLen[TOKEN_N];

//==================================================================
#undef OP_DEF
#undef DT_DEF
#undef KW_DEF
#undef SF_DEF

//==================================================================
static bool matches( const char *pStr, size_t i, size_t strSize, const char *pFindStr )
{
	for (; i < strSize && *pFindStr; ++i)
		if ( pStr[i] != *pFindStr++ )
			return false;

	return true;
}

//==================================================================
static bool findSkipWhites( const char *pStr, size_t &i, size_t strSize )
{
	char	ch = pStr[i];

	if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' )
	{
		do {
			ch = pStr[ ++i ];
		} while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' );

		return true;
	}

	return false;
}

//==================================================================
static void newToken( DVec<Token> &tokens )
{
	if ( tokens.back().str.size() )
		tokens.grow();
}

//==================================================================
static bool matchTokenDef( DVec<Token> &tokens, const char *pSource, size_t &i, size_t sourceSize, bool wasPrecededByWS )
{
	for (size_t j=1; j < TOKEN_N; ++j)
	{
		const TokenDef &tokDef = _sTokenDefs[ _sTokenDefsIdxInvSortLen[ j ] ];

		if NOT( tokDef.pStr )
			continue;

		if ( matches( pSource, i, sourceSize, tokDef.pStr ) )
		{
			newToken( tokens );
			tokens.back().str		= tokDef.pStr;
			tokens.back().idType	= tokDef.idType;
			tokens.back().id		= tokDef.id;
			i += strlen( tokDef.pStr );
			if ( wasPrecededByWS )
			{
				tokens.back().isPrecededByWS = wasPrecededByWS;
			}
			newToken( tokens );
			return true;
		}
	}

	return false;
}

//==================================================================
static bool handleComment(
					const char	*pSource,
					size_t		&i,
					size_t		sourceSize,
					bool		&isInComment )
{
	if ( isInComment )
	{
		if ( matches( pSource, i, sourceSize, "*/" ) )
		{
			isInComment = false;
			i += 2;
		}
		else
		{
			i += 1;
		}

		return true;
	}
	else
	if ( matches( pSource, i, sourceSize, "/*" ) )
	{
		isInComment = true;
		i += 2;
		return true;
	}

	return false;
}

//==================================================================
static bool isDigit( char ch )
{
	return ch >= '0' && ch <= '9';
}

//==================================================================
static bool handleNumber(
					const char	*pSource,
					size_t		&i,
					size_t		sourceSize,
					DVec<Token> &tokens )
{
	char ch		= pSource[i];

	// not the beginning of a number.. ?
	if ( (ch < '0' || ch > '9') && ch != '.' )
		return false;

	char nextCh = (i+1) < sourceSize ? pSource[i+1] : 0;

	// starts with a dot ?
	if ( ch == '.' )	// next must be a digit
		if NOT( isDigit( nextCh ) )
			return false;

	newToken( tokens );

	tokens.back().idType = T_TYPE_VALUE;
	tokens.back().id	 = T_VL_NUMBER;

	char	prevCh = 0;

	bool	gotDot = false;

	for (; i < sourceSize;)
	{
		ch		= pSource[i];
		nextCh	= (i+1) < sourceSize ? pSource[i+1] : 0;

		if ( isDigit( ch ) )
		{
			tokens.back().str += ch;
			++i;
		}
		else
		if ( ch == '.' )
		{
			if ( gotDot )
			{
				// 2+ decimal dots means bad number !
				tokens.back().isBadNumber = true;
				newToken( tokens );
				return true;
			}

			gotDot = true;
			tokens.back().str += ch;
			++i;
		}
		else
		if ( ch == 'f' || ch == 'F' || ch == 'd' || ch == 'D' )	// end of a float or double ?
		{
			tokens.back().str += ch;
			++i;
			newToken( tokens );
			return true;
		}
		else
		if ( ch == 'e' || ch == 'E' )
		{
			tokens.back().str += ch;
			++i;	// take it

			if ( nextCh != 0 )
			{
				tokens.back().str += nextCh;
				++i;	// take the next character if there is any
			}

			// must be +, - or a digit
			if NOT( nextCh == '+' || nextCh == '-' || isDigit( nextCh ) )
			{
				// if not, then it's a bad number !
				tokens.back().isBadNumber = true;
				newToken( tokens );
				return true;
			}
		}
		else
		{
			break;
		}

		prevCh = ch;
	}

	newToken( tokens );
	return true;
}

//==================================================================
const char *GetTokenTypeStr( TokenIDType tokidtype )
{
	switch ( tokidtype )
	{
	case T_TYPE_UNKNOWN:	return "Unknown";
	case T_TYPE_VALUE:		return "Value";
	case T_TYPE_OPERATOR:	return "Operator";
	case T_TYPE_DATATYPE:	return "Data Type";
	case T_TYPE_KEYWORD:	return "Keyword";
	case T_TYPE_STDFUNC:	return "Std Func";

	default:
		return "SYMBOL NOT FOUND ?!!";
	}
}

//==================================================================
const char *GetTokenIDStr( TokenID tokid )
{
	for (size_t i=0; i < TOKEN_N; ++i)
	{
		if ( _sTokenDefs[i].id == tokid )
			return _sTokenDefs[i].pStr ? _sTokenDefs[i].pStr : "N/A";
	}

	return "SYMBOL NOT FOUND ?!!";
}

//==================================================================
struct SortItem
{
	size_t	strLen;
	size_t	idx;
};

//==================================================================
static int tokNamesCmpFunc( const void *a, const void *b )
{
	const SortItem	*pA = (const SortItem *)a;
	const SortItem	*pB = (const SortItem *)b;

	return (int)pB->strLen - (int)pA->strLen;
}

//==================================================================
static void initSortedTable()
{
	if ( _sTokenDefsIdxInvSortLen[0] != 0 )
		return;

	SortItem	sortItems[ TOKEN_N ];

	for (size_t i=0; i < TOKEN_N; ++i)
	{
		sortItems[i].strLen = _sTokenDefs[i].pStr ? strlen( _sTokenDefs[i].pStr ) : 0;
		sortItems[i].idx	= i;
	}

	qsort( sortItems, TOKEN_N, sizeof(SortItem), tokNamesCmpFunc );

	for (size_t i=0; i < TOKEN_N; ++i)
	{
		_sTokenDefsIdxInvSortLen[i] = sortItems[i].idx;
	}
}

//==================================================================
void Tokenizer( DVec<Token> &tokens, const char *pSource, size_t sourceSize )
{
	initSortedTable();

	bool	isEscape	= false;
	bool	isInString	= false;
	bool	isInComment	= false;

	tokens.grow();

	TokenDef	*pCurTDef = NULL;

	bool wasPrecededByWS = false;

	for (size_t i=0; i < sourceSize; )
	{
		if ( handleComment( pSource, i, sourceSize, isInComment ) )
			continue;

		if ( handleNumber( pSource, i, sourceSize, tokens ) )
			continue;

		char	ch = pSource[i];

		if ( ch == '"' )
		{
			wasPrecededByWS = false;

			if ( isInString )
			{
				if ( isEscape )
				{
					tokens.back().str += ch;
					++i;
				}
				else
				{
					isInString = false;
					tokens.back().str += ch;
					++i;
					newToken( tokens );
				}
			}
			else
			{
				isInString = true;
				newToken( tokens );
				tokens.back().str += ch;
				tokens.back().idType = T_TYPE_VALUE;
				tokens.back().id	 = T_VL_STRING;
				++i;
			}
		}
		else
		{
			if ( findSkipWhites( pSource, i, sourceSize ) )
			{
				wasPrecededByWS = true;
				newToken( tokens );
			}
			else
			{
				if NOT( matchTokenDef( tokens, pSource, i, sourceSize, wasPrecededByWS ) )
				{
					tokens.back().str += ch;
					++i;
				}

				wasPrecededByWS = false;
			}
		}
	}
}


//==================================================================
}
