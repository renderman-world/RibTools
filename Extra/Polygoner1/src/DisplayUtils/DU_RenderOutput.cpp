/*
 *  RenderOutput.cpp
 *  Polygoner1
 *
 *  Created by Davide Pasca on 09/02/12.
 *  Copyright 2009 Davide Pasca. All rights reserved.
 *
 */

#include <GLUT/glut.h>
#include "DU_RenderOutput.h"

//==================================================================
namespace DU
{

//==================================================================
// RenderOutput
//==================================================================
RenderOutput::RenderOutput() :
	mpBuffer(NULL), mTexId(0), mWd(0), mHe(0)
{
	const char *pExtensionsStr = (const char *)glGetString( GL_EXTENSIONS );
	
	glGenTextures( 1, &mTexId );
	glBindTexture( GL_TEXTURE_2D, mTexId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

//==================================================================
RenderOutput::~RenderOutput()
{
	glDeleteTextures( 1, &mTexId );
	DSAFE_DELETE_ARRAY( mpBuffer );
}

//==================================================================
void RenderOutput::Update( u_int w, u_int h, const float *pSrcData )
{
	glBindTexture( GL_TEXTURE_2D, mTexId );

	//glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	alloc( w, h );
	convert( w, h, pSrcData );
	glTexSubImage2D(
					GL_TEXTURE_2D,
					0,
					0,
					0,
					w,
					h,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					mpBuffer );

	//glPixelStorei( GL_PACK_ALIGNMENT, 0 );
}

//==================================================================
void RenderOutput::Blit() const
{
	//glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0, (float)mWd, (float)mHe, 0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, mTexId );
	
	float w = mWd;
	float h = mHe;

	glColor3f( 1, 1, 1 );
	glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 ); glVertex2f( w*0, h*0 );
		glTexCoord2f( 1, 0 ); glVertex2f( w*1, h*0 );
		glTexCoord2f( 1, 1 ); glVertex2f( w*1, h*1 );
		glTexCoord2f( 0, 1 ); glVertex2f( w*0, h*1 );
	glEnd();
}

//==================================================================
void RenderOutput::alloc( u_int w, u_int h )
{
	if ( w == mWd && h == mHe )
		return;

	mWd = w;
	mHe = h;
	glTexImage2D( GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	mpBuffer = new u_char [ w * h * 4 ];
	
	glutReshapeWindow( w, h );
}

//==================================================================
void RenderOutput::convert( u_int w, u_int h, const float *pSrcData )
{
	u_char	*pDest = mpBuffer;

	for (u_int j=0; j < h; ++j)
		for (u_int i=0; i < w; ++i)
		{
			float	r = pSrcData[0] * 255.0f;
			float	g = pSrcData[1] * 255.0f;
			float	b = pSrcData[2] * 255.0f;
			pSrcData += 3;

			if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
			if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
			if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;

			pDest[0] = (u_char)r;
			pDest[1] = (u_char)g;
			pDest[2] = (u_char)b;
			pDest[3] = 255;
			pDest += 4;
		}
}

//==================================================================
}