//==================================================================
/// RI_MicroPolygonGrid.h
///
/// Created by Davide Pasca - 2009/2/8
/// See the file "license.txt" that comes with this project for
/// copyright info. 
//==================================================================

#ifndef RI_MICROPOLYGONGRID_H
#define RI_MICROPOLYGONGRID_H

#include "DSystem/include/DTypes.h"
#include "DSystem/include/DContainers.h"
#include "DMath/include/DMath.h"
#include "RI_Symbol.h"
#include "RI_SlRunContext.h"

//==================================================================
namespace RI
{

//==================================================================
class Attributes;
class SlRunContext;

/*
//==================================================================
template <class _T, size_t _MAX>
class DumbPool
{
	_T		mStorage[_MAX];
	size_t	mStorageN;

public:
	DumbPool() :
		mStorageN(0)
	{
	}

	~DumbPool()
	{
	}

	_T	*Alloc()
	{
		DASSERT( mStorageN < _MAX );
		return &mStorage[ mStorageN++ ];
	}
};
*/

//==================================================================
class MicroPolygonGrid
{
	SlColor			*mpDataCi;
	SlColor			*mpDataOi;
	SlColor			*mpDataCs;
	SlColor			*mpDataOs;
	SlRunContext	mSurfRunCtx;
	SlRunContext	mDispRunCtx;

public:
	u_int			mXDim;
	u_int			mXBlocks;
	u_int			mYDim;
	u_int			mPointsN;
	SlVec3			*mpPointsWS;
	float			mURange[2];
	float			mVRange[2];
	SymbolIList		mSymbolIs;

	Matrix44		mMtxLocalWorld;
	Matrix44		mMtxWorldCamera;
	Matrix44		mMtxLocalCamera;

	MicroPolygonGrid( const SymbolList &globalSymbols );
	~MicroPolygonGrid();

	void Setup(u_int xdim,
			   u_int ydim,
			   const float uRange[2],
			   const float vRange[2],
			   const Matrix44 &mtxLocalWorld,
			   const Matrix44 &mtxWorldCamera );
			   
	u_int GetPointsN() const { return mPointsN; }
	
	void Displace( const Attributes &attribs );
	void Shade( const Attributes &attribs );

private:
	void *addSymI( const SymbolList &globalSyms, const char *pName );
};

//==================================================================
}

#endif
