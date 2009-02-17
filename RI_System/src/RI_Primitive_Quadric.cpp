/*
 *  RI_Primitive_Quadric.cpp
 *  RibTools
 *
 *  Created by Davide Pasca on 09/01/18.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <GLUT/glut.h>
#include "DMath.h"
#include "RI_Options.h"
#include "RI_Attributes.h"
#include "RI_Transform.h"
#include "RI_Primitive.h"
#include "RI_Primitive_Quadric.h"

//==================================================================
namespace RI
{

//==================================================================
Point3 &Cylinder::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	float	theta = u * mThetamaxRad;

	out_pt.x = mRadius * cosf( theta );
	out_pt.y = mRadius * sinf( theta );
	out_pt.z = mZMin + v * (mZMax - mZMin);
	
	return out_pt;
}

//==================================================================
Point3 &Cone::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	float	theta = u * mThetamaxRad;

	out_pt.x = mRadius * (1 - v) * cosf( theta );
	out_pt.y = mRadius * (1 - v) * sinf( theta );
	out_pt.z = v * mHeight;
	
	return out_pt;
}

//==================================================================
Point3 &Sphere::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	// $$$ following 2 are "uniform"
	float	alphamin	= asinf( mZMin / mRadius );
	float	alphadelta	= asinf( mZMax / mRadius ) - alphamin;

	float	theta = u * mThetamaxRad;
	float	alpha = alphamin + v * alphadelta;

	out_pt.x = mRadius * cosf( alpha ) * cosf( theta );
	out_pt.y = mRadius * cosf( alpha ) * sinf( theta );
	out_pt.z = mRadius * sinf( alpha );
	
	return out_pt;
}

//==================================================================
Point3 &Hyperboloid::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	float	theta = u * mThetamaxRad;

	float	x = mP1.x + (mP2.x - mP1.x) * v;
	float	y = mP1.y + (mP2.y - mP1.y) * v;
	float	z = mP1.z + (mP2.z - mP1.z) * v;

	out_pt.x = x * cosf( theta ) - y * sinf( theta );
	out_pt.y = x * sinf( theta ) + y * cosf( theta );
	out_pt.z = z;
	
	return out_pt;
}

//==================================================================
Point3 &Paraboloid::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	float	theta = u * mThetamaxRad;

	float	z = (mZmax - mZmin) * v;
	float	r = mRmax * sqrtf( z / mZmax );

	out_pt.x = r * cosf( theta );
	out_pt.y = r * sinf( theta );
	out_pt.z = z;

	return out_pt;
}

//==================================================================
Point3 &Torus::EvalP(
			float uGrid,
			float vGrid,
			Point3 &out_pt ) const
{
	float	u = mURange[0] + (mURange[1] - mURange[0]) * uGrid;
	float	v = mVRange[0] + (mVRange[1] - mVRange[0]) * vGrid;

	float	theta = u * mThetamaxRad;

	float	phi = mPhiminRad + (mPhimaxRad - mPhiminRad) * v;
	float	r = mMinRadius * cosf( phi );

	out_pt.x = (mMaxRadius + r) * cosf( theta );
	out_pt.y = (mMaxRadius + r) * sinf( theta );
	out_pt.z = mMinRadius * sinf( phi );
	
	return out_pt;
}

//==================================================================
inline Point3 polar( float radius, float theta )
{
	return Point3( cosf(theta)*radius, sinf(theta)*radius, 0 );
}

//==================================================================
inline void bounds2DSweepL(
					Bound &out_bound,
					float rmin,
					float rmax,
					float tmin,
					float tmax )
{
	out_bound.Expand( polar(rmin,tmin) );
	out_bound.Expand( polar(rmax,tmin) );
	out_bound.Expand( polar(rmin,tmax) );
	out_bound.Expand( polar(rmax,tmax) );
	
	if ( tmin < (float)M_PI_2 && tmax > (float)M_PI_2 )
		out_bound.Expand( polar( rmax, M_PI_2 ) );

	if ( tmin < (float)M_PI && tmax > (float)M_PI )
		out_bound.Expand( polar( rmax, M_PI ) );

	if ( tmin < (float)(M_PI+M_PI_2) && tmax > (float)(M_PI+M_PI_2) )
		out_bound.Expand( polar( rmax, (float)(M_PI+M_PI_2) ) );
}

//==================================================================
inline void bounds2DSweepR(
					Bound &out_bound,
					float r,
					float phiMin,
					float phiMax )
{
	out_bound.Expand( polar(r, phiMin) );
	out_bound.Expand( polar(r, phiMax) );

	for (int i=-3; i < 4; ++i)
	{
		float	phi = i * (float)M_PI_2;
		if ( phiMin < phi && phiMax > phi )
			out_bound.Expand( polar( r, phi ) );
	}
}

//==================================================================
inline void bounds2DSweepP(
					Bound &out_bound,
					const Point3 &p,
					float theMin,
					float theMax )
{
	float	r = sqrtf( p.x * p.x + p.y * p.y );
	float	delta = atan2f( p.y, p.x );
	
	theMin += delta;
	theMax += delta;
	
	out_bound.Expand( polar(r, theMin) );
	out_bound.Expand( polar(r, theMax) );

	for (int i=-1; i < 6; ++i)
	{
		float	the = i * (float)M_PI_2;
		if ( theMin < the && theMax > the )
			out_bound.Expand( polar( r, the ) );
	}
}

//==================================================================
void Cylinder::MakeBound( Bound &out_bound )
{
	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];
	out_bound.Reset();
	bounds2DSweepL( out_bound, mRadius, mRadius, tuMin, tuMax );
	out_bound.mBox[0].z = mZMin + mVRange[0]*(mZMax - mZMin);
	out_bound.mBox[1].z = mZMin + mVRange[1]*(mZMax - mZMin);
}

void Cone::MakeBound( Bound &out_bound )
{
	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];

	float	rMin = mRadius * (1 - mVRange[1]);
	float	rMax = mRadius * (1 - mVRange[0]);
	out_bound.Reset();
	bounds2DSweepL( out_bound, rMin, rMax, tuMin, tuMax );
	out_bound.mBox[0].z = mVRange[0] * mHeight;
	out_bound.mBox[1].z = mVRange[1] * mHeight;
}

void Sphere::MakeBound( Bound &out_bound )
{
	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];

	float	alphaMin = asinf( mZMin / mRadius );
	float	alphaMax = asinf( mZMax / mRadius );

	float	aVMin = DMix( alphaMin, alphaMax, mVRange[0] );
	float	aVMax = DMix( alphaMin, alphaMax, mVRange[1] );

	float	rVMin = cosf( aVMin ) * mRadius;
	float	rVMax = cosf( aVMax ) * mRadius;
	float	rMin = DMIN( rVMin, rVMax );
	
	float	rMax;
	
	if ( aVMin < 0 && aVMax > 0 )
		rMax = mRadius;
	else
		rMax = DMAX( rVMin, rVMax );
	
	out_bound.Reset();
	bounds2DSweepL( out_bound, rMin, rMax, tuMin, tuMax );

	out_bound.mBox[0].z = sinf( aVMin ) * mRadius;
	out_bound.mBox[1].z = sinf( aVMax ) * mRadius;
}

void Hyperboloid::MakeBound( Bound &out_bound )
{
	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];
	Point3	pMin; EvalP( 0, mVRange[0], pMin );
	Point3	pMax; EvalP( 0, mVRange[1], pMax );

	out_bound.Reset();
	bounds2DSweepP( out_bound, pMin, tuMin, tuMax );
	bounds2DSweepP( out_bound, pMax, tuMin, tuMax );
	
	out_bound.mBox[0].z = DMIN( pMin.z, pMax.z );
	out_bound.mBox[1].z = DMAX( pMin.z, pMax.z );
}

void Paraboloid::MakeBound( Bound &out_bound )
{
	float	scale = mRmax / sqrtf( mZmax );

	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];
	float	zVMin = mZmin + mVRange[0] * (mZmax-mZmin);
	float	zVMax = mZmin + mVRange[1] * (mZmax-mZmin);
	float	rMin = sqrtf( zVMin ) * scale;
	float	rMax = sqrtf( zVMax ) * scale;
	out_bound.Reset();
	bounds2DSweepL( out_bound, rMin, rMax, tuMin, tuMax );
	out_bound.mBox[0].z = zVMin;
	out_bound.mBox[1].z = zVMax;
}

void Torus::MakeBound( Bound &out_bound )
{
	float	tuMin = mThetamaxRad * mURange[0];
	float	tuMax = mThetamaxRad * mURange[1];

	float	phiVMin = DMix( mPhiminRad, mPhimaxRad, mVRange[0] );
	float	phiVMax = DMix( mPhiminRad, mPhimaxRad, mVRange[1] );

	Bound	a;
	a.Reset();
	bounds2DSweepR( a, mMinRadius, phiVMin, phiVMax );
	float	rMin = a.mBox[0].x + mMaxRadius;
	float	rMax = a.mBox[1].x + mMaxRadius;

	out_bound.Reset();
	bounds2DSweepL( out_bound, rMin, rMax, tuMin, tuMax );
	out_bound.mBox[0].z = a.mBox[0].y;
	out_bound.mBox[1].z = a.mBox[1].y;
}

/*
void Disk::MakeBound( Bound &out_bound )
{
}
*/

/*
//==================================================================
void Cylinder::Render( GState &gstate )
{
	PUTPRIMNAME( "* Cylinder" );
	
	glBegin( GL_TRIANGLE_STRIP );

	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float	theta = u * mThetamaxRad;
		
		for (float v=0; v <= 1.0f; v += 1.0f)
		{
			GVert	vert;

			vert.x = mRadius * cosf( theta );
			vert.y = mRadius * sinf( theta );
			vert.z = mZMin + v * (mZMax - mZMin);
			vert.u = u;
			vert.v = v;

			gstate.AddVertex( vert );
		}
	}
	
	glEnd();
}

//==================================================================
void Cone::Render( GState &gstate )
{
	PUTPRIMNAME( "* Cone" );

	glBegin( GL_TRIANGLE_STRIP );

	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float	theta = u * mThetamaxRad;

		for (float v=0; v <= 1.0f; v += 1.0f)
		{
			GVert	vert;

			vert.x = mRadius * (1 - v) * cosf( theta );
			vert.y = mRadius * (1 - v) * sinf( theta );
			vert.z = v * mHeight;
			vert.u = u;
			vert.v = v;

			gstate.AddVertex( vert );
		}
	}
	
	glEnd();
}

//==================================================================
void Sphere::Render( GState &gstate )
{
	PUTPRIMNAME( "* Sphere" );
	
	glBegin( GL_TRIANGLE_STRIP );

	float	alphamin	= asinf( mZMin / mRadius );
	float	alphadelta	= asinf( mZMax / mRadius ) - alphamin;

	GVert	buffer[NSUBDIVS+1];

	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float	theta = u * mThetamaxRad;

		for (int vI=0; vI <= NSUBDIVS; ++vI)
		{
			float	v = vI / (float)NSUBDIVS;

			float	alpha = alphamin + v * alphadelta;

			GVert	vert;
			vert.x = mRadius * cosf( alpha ) * cosf( theta );
			vert.y = mRadius * cosf( alpha ) * sinf( theta );
			vert.z = mRadius * sinf( alpha );
			vert.u = u;
			vert.v = v;

			if ( uI > 0 )
			{
				gstate.AddVertex( buffer[vI] );
				gstate.AddVertex( vert );
			}

			buffer[vI] = vert;
		}
	}
	
	glEnd();
}

//==================================================================
void Hyperboloid::Render( GState &gstate )
{
	PUTPRIMNAME( "* Hyperboloid" );
	
	glBegin( GL_TRIANGLE_STRIP );

	GVert	buffer[NSUBDIVS+1];

	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float theta = u * mThetamaxRad;

		for (int vI=0; vI <= NSUBDIVS; ++vI)
		{
			float	v = vI / (float)NSUBDIVS;

			float	x = mP1.x + (mP2.x - mP1.x) * v;
			float	y = mP1.y + (mP2.y - mP1.y) * v;
			float	z = mP1.z + (mP2.z - mP1.z) * v;

			GVert	vert;
			vert.x = x * cosf( theta ) - y * sinf( theta );
			vert.y = x * sinf( theta ) + y * cosf( theta );
			vert.z = z;
			vert.u = u;
			vert.v = v;

			if ( uI > 0 )
			{
				gstate.AddVertex( buffer[vI] );
				gstate.AddVertex( vert );
			}

			buffer[vI] = vert;
		}
	}

	glEnd();
}

//==================================================================
void Paraboloid::Render( GState &gstate )
{
	PUTPRIMNAME( "* Paraboloid" );

	glBegin( GL_TRIANGLE_STRIP );

	GVert	buffer[NSUBDIVS+1];

	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float theta = u * mThetamaxRad;

		for (int vI=0; vI <= NSUBDIVS; ++vI)
		{
			float	v = vI / (float)NSUBDIVS;

			float	z = (mZmax - mZmin) * v;

			float	r = mRmax * sqrtf( z / mZmax );

			GVert	vert;
			vert.x = r * cosf( theta );
			vert.y = r * sinf( theta );
			vert.z = z;
			vert.u = u;
			vert.v = v;

			if ( uI > 0 )
			{
				gstate.AddVertex( buffer[vI] );
				gstate.AddVertex( vert );
			}

			buffer[vI] = vert;
		}
	}

	glEnd();
}

//==================================================================
void Torus::Render( GState &gstate )
{
	PUTPRIMNAME( "* Torus" );

	GVert	buffer[NSUBDIVS+1];

	glBegin( GL_TRIANGLE_STRIP );
	for (int uI=0; uI <= NSUBDIVS; ++uI)
	{
		float	u = uI / (float)NSUBDIVS;

		float theta = u * mThetamaxRad;

		//glBegin( GL_TRIANGLE_STRIP );
		for (int vI=0; vI <= NSUBDIVS; ++vI)
		{
			float	v = vI / (float)NSUBDIVS;

			float	phi = mPhiminRad + (mPhimaxRad - mPhiminRad) * v;
			
			GVert	vert;

			float	r = mMinRadius * cosf( phi );
			vert.z	  = mMinRadius * sinf( phi );
			
			vert.x	= (mMaxRadius + r) * cosf( theta );
			vert.y	= (mMaxRadius + r) * sinf( theta );

			vert.u	= u;
			vert.v	= v;

			if ( uI > 0 )
			{
				gstate.AddVertex( buffer[vI] );
				gstate.AddVertex( vert );
			}

			buffer[vI] = vert;
		}
		//glEnd();
	}
	glEnd();
}
*/

//==================================================================
}