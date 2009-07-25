//==================================================================
/// RSLCompiler.cpp
///
/// Created by Davide Pasca - 2009/5/21
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <time.h>
#include "DSystem/include/DUtils.h"
#include "DSystem/include/DContainers.h"
#include "RSLC_Token.h"
#include "RSLC_Tree.h"
#include "RSLC_Variables.h"
#include "RSLC_Functions.h"
#include "RSLC_Operators.h"
#include "RSLC_Registers.h"
#include "RSLC_Builtins.h"
#include "RSLCompiler.h"

//==================================================================
using namespace	RSLC;

//==================================================================
const char	*RSLCompiler::mpsVersionString = "0.1a";

//==================================================================
static void grabfile( const char *pFName, DVec<char> &out_data )
{
	out_data.clear();

	FILE	*pFile;

	if ( fopen_s( &pFile, pFName, "rb" ) )
	{
		std::string	msg = DUT::SSPrintFS( "Failed to open %s", pFName );

		throw Exception( msg );
	}

	fseek( pFile, 0, SEEK_END );
	size_t	len = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	out_data.resize( len );

	fread( &out_data[0], 1, len, pFile );

	fclose( pFile );
}

//==================================================================
RSLCompiler::RSLCompiler(
		const char *pSource,
		size_t sourceSize,
		const char *pBaseInclude,
		const Params &params )
{
	DVec<char>	sourceInc;

	grabfile( pBaseInclude, sourceInc );

	sourceInc.append_array( pSource, sourceSize );

	Tokenizer( mTokens, &sourceInc[0], sourceInc.size() );

#if 0	// useful to debug the tokenizer
	for (size_t i=0; i < mTokens.size(); ++i)
	{
		printf( "%3i) %i - %-12s - %s\n",
				i,
				mTokens[i].isPrecededByWS,
				GetTokenTypeStr( mTokens[i].idType ),
				mTokens[i].str.c_str() );
	}
#endif

	mpRoot = DNEW TokNode( (Token *)NULL );

	// make the basic tree with nested blocks based on brackets
	MakeTree( mpRoot, mTokens );

	//
	AddStandardVariables( mpRoot );

	// discover variables declarations
	DiscoverVariablesDeclarations( mpRoot );

	// discover functions declarations and usage
	DiscoverFunctions( mpRoot );

	// remove closing brackets as they serve no additional purpose
	// ..since the tree has already been defined at this point
	RemoveClosingBrackets( mpRoot );

	// remove semicolons as they serve no additional purpose
	RemoveSemicolons( mpRoot );

	// develop the tree based on operators with the proper precedence
	ReparentOperators( mpRoot );

	// discover variables usage
	DiscoverVariablesUsage( mpRoot );

	ResolveFunctionCalls( mpRoot );

	RealizeConstants( mpRoot );

	AssignRegisters( mpRoot );

	mUsedStdVars.clear();
	CollectUsedStdVars( mpRoot, mUsedStdVars );

	// produce some debug info in the output file
	if ( params.mDbgOutputTree )
		TraverseTree( mpRoot, 0 );
}

//==================================================================
RSLCompiler::~RSLCompiler()
{
	DSAFE_DELETE( mpRoot );
}

//==================================================================
void RSLCompiler::SaveASM( const char *pFName, const char *pRefSourceName )
{
	FILE	*pFile;

	if ( fopen_s( &pFile, pFName, "wb" ) )
	{
		DASSTHROW( 0, ("Failed to save %s", pFName) );
	}

	char dateStr[256];
	char timeStr[256];
	numstrdate( dateStr);
	_strtime_s( timeStr );

	fprintf_s( pFile, ";==========================================================\n" );
	fprintf_s( pFile, ";= %s\n", pFName );
	fprintf_s( pFile, ";= Source file %s\n", pRefSourceName );
	fprintf_s( pFile, ";= Creation date %s - %s\n", dateStr, timeStr );
	fprintf_s( pFile, ";= File automatically generated by RSLCompilerCmd %s\n", mpsVersionString );
	fprintf_s( pFile, ";=========================================================\n\n" );

	fprintf_s( pFile, "\n.data\n" );

	WriteVariables( pFile, mpRoot, mUsedStdVars );

	fprintf_s( pFile, "\n.code\n" );

	WriteFunctions( pFile, mpRoot );

	fclose( pFile );
}

