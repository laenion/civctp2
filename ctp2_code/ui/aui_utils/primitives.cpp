//----------------------------------------------------------------------------
//
// Project      : Call To Power 2
// File type    : C++ source
// Description  : The primitive rectabgles for painting
// Id           : $Id$
//
//----------------------------------------------------------------------------
//
// Disclaimer
//
// THIS FILE IS NOT GENERATED OR SUPPORTED BY ACTIVISION.
//
// This material has been developed at apolyton.net by the Apolyton CtP2
// Source Code Project. Contact the authors at ctp2source@apolyton.net.
//
//----------------------------------------------------------------------------
//
// Compiler flags
//
// - None
//
//----------------------------------------------------------------------------
//
// Modifications from the original Activision code:
//
// - Initialized local variables. (Sep 9th 2005 Martin Gühmann)
// - Standardized code (May 21st 2006 Martin Gühmann)
// - Added primitives_GetScreenAdjustedRectCopy function. (3-Mar-2007 Martin Gühmann)
//
//----------------------------------------------------------------------------

#include "c3.h"
#include "primitives.h"

#include "Globals.h"

#include "aui.h"
#include "aui_surface.h"
#include "aui_image.h"
#include "pattern.h"
#include "aui_blitter.h"
#include "aui_tabgroup.h"

#include "c3ui.h"

#include "pixelutils.h"
#include "textutils.h"
#include "tileset.h"
#include "colorset.h"               // g_colorSet

extern C3UI			*g_c3ui;

PRIMITIVES_ERRCODE primitives_SetRect(
	RECT *rect,
	sint32 left,
	sint32 top,
	sint32 right,
	sint32 bottom
	)
{
	Assert(rect);
	if (rect==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_FrameRect16(
	aui_Surface *pSurface,
	RECT *pRect,
	Pixel16 color
	)
{
	BOOL			wasUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	if (pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		wasUnlocked = TRUE;
	} else {
		pSurfBase = (uint8 *)pSurface->Buffer();
	}

	sint32      surfPitch = pSurface->Pitch();

	sint32      width   = right - left;
	sint32      height  = bottom - top;
	uint16 *    pDest   = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32      inc1    = (surfPitch >> 1) - width;
	sint32      inc2    = width - 1;

	sint32 i;
	for (i=width;i;i--)
		*pDest++ = color;

	sint32 tempHeight = height - 2;
	for (sint32 j=tempHeight;j;j--)
	{
		pDest += inc1;
		*pDest = color;
		pDest += inc2;
		*pDest++ = color;
	}

	pDest += inc1;

	for (sint32 k = width; k > 0; --k)
		*pDest++ = color;

	if (wasUnlocked) {
		errcode = pSurface->Unlock((LPVOID)pSurfBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	return PRIMITIVES_ERRCODE_OK;
}





#define RED555(c) ((c)&0x1f)
#define GREEN555(c) (((c)>>5)&0x1f)
#define BLUE555(c) (((c)>>10)&0x1f)
#define RED565(c) ((c)&0x1f)
#define GREEN565(c) (((c)>>5)&0x3f)
#define BLUE565(c) (((c)>>11)&0x1f)

PRIMITIVES_ERRCODE primitives_Scale16(
	aui_Surface *pSrc,
	aui_Surface *pDst,
	const fRect &sRect,
	const fRect &dRect,
	bool bFilter
	)
{
	BOOL			srcUnlocked = FALSE, dstUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	if ((pSrc == NULL) || (pDst == NULL))
	{
		Assert(0);
		return PRIMITIVES_ERRCODE_INVALIDPARAM;
	}

	Assert(dRect.left <= dRect.right);
	Assert(dRect.top <= dRect.bottom);
	if ((dRect.left >= dRect.right) || (dRect.top >= dRect.bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 width = static_cast<sint32>(ceil(dRect.right) - ceil(dRect.left));
	sint32 height = static_cast<sint32>(ceil(dRect.bottom) - ceil(dRect.top));
	if ((width == 0) || (height == 0))
		return PRIMITIVES_ERRCODE_OK;

	uint16 *pSrcBase;

	if (pSrc->Buffer() == NULL) {
		errcode = pSrc->Lock(NULL,(LPVOID *)&pSrcBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		srcUnlocked = TRUE;
	} else {
		pSrcBase = (uint16 *)pSrc->Buffer();
	}

	uint16 *pDstBase;

	if (pDst->Buffer() == NULL) {
		errcode = pDst->Lock(NULL,(LPVOID *)&pDstBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		dstUnlocked = TRUE;
	} else {
		pDstBase = (uint16 *)pDst->Buffer();
	}

	sint32 srow = pSrc->Pitch()>>1;
	sint32 drow = pDst->Pitch()>>1;


	sint32 dst_y0 = static_cast<sint32>(ceil(dRect.top));
	sint32 dst_x0 = static_cast<sint32>(ceil(dRect.left));
	uint16 *pDstPixel = pDstBase + dst_y0 * drow + dst_x0;

	double src_dy = (sRect.bottom - sRect.top)/(dRect.bottom - dRect.top);
	double src_dx = (sRect.right - sRect.left)/(dRect.right - dRect.left);
	double src_y = sRect.top + (dst_y0 - dRect.top) * src_dy;
	if (bFilter) src_y -= 0.5;
	for (sint32 j=0;j<height;j++)
	{
		double src_x = sRect.left + (dst_x0 - dRect.left) * src_dx;
		if (bFilter) src_x -= 0.5;
		sint32 tmp_y = srow * ((uint32 )floor(src_y));
		if(tmp_y<0) tmp_y = 0;
		uint16 *pSrcPixel1 = &pSrcBase[tmp_y];
		tmp_y = srow * ((uint32 )floor(src_y+(bFilter?1:0)));
		if(tmp_y<0) tmp_y = 0;
		uint16 *pSrcPixel2 = &pSrcBase[tmp_y];
		double fy = src_y - floor(src_y);
		for (sint32 i=0;i<width;i++)
		{
			sint32 x = sint32(floor(src_x));
			uint32 x0 = (x<0)?0:x;
			uint32 x1 = (x+1>=srow)?(srow-1):(x+1);
			if (!bFilter)
			{
				pDstPixel[i] = pSrcPixel1[x0];
			}
			else
			{
				uint16 c0 = pSrcPixel1[x0], c1 = pSrcPixel1[x1];
				uint16 c2 = pSrcPixel2[x0], c3 = pSrcPixel2[x1];
				double f0, f1, f2, f3;
				double fx = src_x - floor(src_x);
				f1 = 1.0 - fy;
				f2 = 1.0 - fx;
				f3 = fx * fy;
				f0 = f1 * f2;
				f1 *= fx;
				f2 *= fy;
				if (g_is565Format)
				{
					double red = f0 * RED565(c0) + f1 * RED565(c1) + f2 * RED565(c2) + f3 * RED565(c3);
					double green = f0 * GREEN565(c0) + f1 * GREEN565(c1) + f2 * GREEN565(c2) + f3 * GREEN565(c3);
					double blue = f0 * BLUE565(c0) + f1 * BLUE565(c1) + f2 * BLUE565(c2) + f3 * BLUE565(c3);
					pDstPixel[i] = ((uint16)red) + (((uint16)green)<<5) + (((uint16)blue)<<11);
				}
				else
				{
					double red = f0 * RED555(c0) + f1 * RED555(c1) + f2 * RED555(c2) + f3 * RED555(c3);
					double green = f0 * GREEN555(c0) + f1 * GREEN555(c1) + f2 * GREEN555(c2) + f3 * GREEN555(c3);
					double blue = f0 * BLUE555(c0) + f1 * BLUE555(c1) + f2 * BLUE555(c2) + f3 * BLUE555(c3);
					pDstPixel[i] = ((uint16)red) + (((uint16)green)<<5) + (((uint16)blue)<<10);
				}
			}
			src_x += src_dx;
		}
		src_y += src_dy;
		pDstPixel += drow;
	}

	if (srcUnlocked) {

		errcode = pSrc->Unlock((LPVOID)pSrcBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	if (dstUnlocked) {

		errcode = pDst->Unlock((LPVOID)pDstBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	return PRIMITIVES_ERRCODE_OK;
}








PRIMITIVES_ERRCODE primitives_PaintRect16(
	aui_Surface *pSurface,
	RECT *pRect,
	Pixel16 color
	)
{
	BOOL			wasUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	if (pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		wasUnlocked = TRUE;
	} else {
		pSurfBase = pSurface->Buffer();
	}

	sint32 surfPitch = pSurface->Pitch();

	sint32 width = right - left;
	sint32 height = bottom - top;

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	for (sint32 j=height;j;j--)
	{
		for (sint32 i=width;i;i--)
			*pDestPixel++ = color;
		pDestPixel += inc;
	}

	if (wasUnlocked) {

		errcode = pSurface->Unlock((LPVOID)pSurfBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelRect16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	bool wasUnlocked = false;

	sint32 errcode;
	if(pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		wasUnlocked = true;
	} else {
		pSurfBase = (uint8 *)pSurface->Buffer();
	}

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	if (flag)
	{
		sint32 j;
		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = inc;
		tempWidth = width-1;
		pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

		for (sint32 k = level; k; --k)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 temp = top+1;
		sint32 i;
		for (i = 0;i < level;i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (temp+i) * surfPitch + ((left+i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		temp = right - 1;
		for (i = 0; i < level;i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((temp-i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}
	else
	{
		sint32 j;
		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = inc;
		tempWidth = width-1;
		pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 temp = top+1;
		sint32 i;
		for (i = 0;i < level;i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (temp+i) * surfPitch + ((left+i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		temp = right - 1;
		for (i = 0;i < level;i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((temp-i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}

	if(wasUnlocked) {
		errcode = pSurface->Unlock((LPVOID)pSurfBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelRect16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	if ( left < 0 || top < 0 || right > pSurface->Width() || bottom > pSurface->Height() )
		return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	bool wasUnlocked = false;

	sint32 errcode;

	if(pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;
		wasUnlocked = true;
	} else {
		pSurfBase = (uint8 *)pSurface->Buffer();
	}


	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	if (flag)
	{
		sint32 j;
		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = inc;
		tempWidth = width-1;
		pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 temp = top+1;
		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (temp+i) * surfPitch + ((left+i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		temp = right - 1;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((temp-i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}
	else
	{
		sint32 j;
		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = inc;
		tempWidth = width-1;
		pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 temp = top+1;
		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (temp+i) * surfPitch + ((left+i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		temp = right - 1;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((temp-i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}

	if(wasUnlocked) {

		errcode = pSurface->Unlock((LPVOID)pSurfBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
	}

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_FrameThickRect16(
	aui_Surface *pSurface,
	RECT *pRect,
	Pixel16 color,
	sint32 level
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	{
		sint32 j;
		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{


				*pDestPixel++ = color;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = inc;
		tempWidth = width-1;
		pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

		for (j = level;j;j--)
		{
			for (sint32 i=tempWidth;i;i--)
			{


				*pDestPixel-- = color;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 temp = top+1;
		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (temp+i) * surfPitch + ((left+i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				*pDestPixel++ = color;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		temp = right - 1;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((temp-i) << 1));

			for (sint32 y=tempHeight;y;y--)
			{

				*pDestPixel++ = color;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}

constexpr sint32 sgn(sint32 x) {
	return x >= 0 ? 1 : -1;
}

PRIMITIVES_ERRCODE primitives_DrawLine16(
	aui_Surface *pSurface,
	sint32 x1,
	sint32 y1,
	sint32 x2,
	sint32 y2,
	Pixel16 color
	)
{
	Assert(pSurface);
	if(pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 dx = x2-x1;
	sint32 dy = y2-y1;
	sint32 sdx = sgn(dx);
	sint32 sdy = sgn(dy);
	sint32 absdx = abs(dx);
	sint32 absdy = abs(dy);
	sint32 num = absdy >> 1;
	sint32 den = absdx >> 1;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32      surfPitch   = pSurface->Pitch();
	uint16 *    pDest       = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
	*pDest = color;

	if (absdx >= absdy)
	{
		for (sint32 i=absdx;i;i--)
		{
			num += absdy;
			if (num > absdx)
			{
				num -= absdx;
				y1 += sdy;
			}
			x1 += sdx;

			pDest = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
			*pDest = color;
		}
	}
	else
	{
		for (sint32 i=absdy;i;i--)
		{
			den += absdx;
			if (den > absdy)
			{
				den -= absdy;
				x1 += sdx;
			}
			y1 += sdy;

			pDest = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
			*pDest = color;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}

PRIMITIVES_ERRCODE primitives_DrawDashedLine16(
	aui_Surface *pSurface,
	sint32 x1,
	sint32 y1,
	sint32 x2,
	sint32 y2,
	Pixel16 color,
	sint32 length
	)
{
	Assert(pSurface);
	if(pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 dx = x2-x1;
	sint32 dy = y2-y1;
	sint32 sdx = sgn(dx);
	sint32 sdy = sgn(dy);
	sint32 absdx = abs(dx);
	sint32 absdy = abs(dy);
	sint32 num = absdy >> 1;
	sint32 den = absdx >> 1;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32      surfPitch   = pSurface->Pitch();
	uint16 *    pDest       = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
	*pDest = color;

	sint32 draw = length / 2; // start with only half the dash length for more evenly distributed dashes
	sint32 skip = 0;

	if (absdx >= absdy)
	{
		for (sint32 i=absdx;i;i--)
		{
			num += absdy;
			if (num > absdx)
			{
				num -= absdx;
				y1 += sdy;
			}
			x1 += sdx;

			if ( draw  ) {
			        pDest = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
			        *pDest = color;
				draw--;
				if ( !draw ) {
					skip = length;
				}
			}
			else {
				skip--;
				if ( !skip ) {
					draw = length;
				}
			}
		}
	}
	else
	{
		for (sint32 i=absdy;i;i--)
		{
			den += absdx;
			if (den > absdy)
			{
				den -= absdy;
				x1 += sdx;
			}
			y1 += sdy;

			if ( draw  ) {
			        pDest = (uint16 *)(pSurfBase + y1 * surfPitch + (x1 << 1));
			        *pDest = color;
				draw--;
				if ( !draw ) {
					skip = length;
				}
			}
			else {
				skip--;
				if ( !skip ) {
					draw = length;
				}
			}
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}

#define k_MAX_BLEND_VALUES	8
#define k_BLEND_COLOR		0x0000
#define k_BLEND_VALUE		10

PRIMITIVES_ERRCODE primitives_DrawText(
	aui_Surface *pDirectSurface,
	sint32 x,
	sint32 y,
	const MBCHAR *pString,
	COLORREF color,
	bool bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	sint32 oldMode = 0;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, color);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	TextOut(hdc,x,y,pString,strlen(pString));

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSRELEASEDCFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();
	    
	if(font){
	    RECT rect = {x, y - 0.5 * font->GetLineSkip(), pDirectSurface->Width(), pDirectSurface->Height()}; // ony x, y matter; width and hight get clipped in DrawString
	    RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, rect);
	    font->DrawString(pDirectSurface, &rect, &clipRect, pString, 0, color, 0); // no bg correspondence
	    }
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}




PRIMITIVES_ERRCODE primitives_DrawBoundedText(
		aui_Surface *pDirectSurface,
		RECT *bound,
		const MBCHAR *pString,
		COLORREF color,
		BOOL bg
		)
{
#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	sint32 oldMode = 0;
#endif // __AUI_USE_DIRECTX__

	Assert(pDirectSurface);
	if (pDirectSurface==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pString);
	if (pString==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, g_colorSet->GetColorRef(COLOR_BUTTON_TEXT_DROP));

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	DrawText(hdc,pString,-1,bound,DT_WORDBREAK);

	oldColor = SetTextColor(hdc, color);

	RECT bound2 = *bound;
	OffsetRect(&bound2, -1, -1);
	DrawText(hdc,pString,-1,&bound2,DT_WORDBREAK);

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSRELEASEDCFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();
	    
	if(font){
	    RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, *bound);
	    font->DrawString(pDirectSurface, bound, &clipRect, pString, 0, color, 0); // no bg correspondence
	    }
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_DrawTextBatch(
	aui_Surface *pDirectSurface,
	sint32 x,
	sint32 y,
	const MBCHAR **pString,
	sint32 numStrings,
	COLORREF color,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	TEXTMETRIC tm;
	sint32 oldMode = 0;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	GetTextMetrics(hdc,&tm);

	oldColor = SetTextColor(hdc, color);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	for (sint32 i=0;i < numStrings;i++)
	{
		TextOut(hdc,x,y,pString[i],strlen(pString[i]));
		y += tm.tmHeight + tm.tmExternalLeading;
	}

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSRELEASEDCFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();
	    
	if(font){
	    for (sint32 i=0;i < numStrings;i++)
		{
		RECT rect = {x, y, pDirectSurface->Width(), pDirectSurface->Height()}; // ony x, y matter; width and hight get clipped in DrawString
		RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, rect);
		font->DrawString(pDirectSurface, &rect, &clipRect, pString[i], 0, color, 0); // no bg correspondence
		y += font->GetLineSkip();
		
		}
	    }
#endif

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_DropText(
	aui_Surface *pDirectSurface,
	sint32 x,
	sint32 y,
	const MBCHAR *pString,
	COLORREF color,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	sint32 oldMode = 0;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, 0);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	COLORREF	dropTextColor = g_colorSet->GetColorRef(COLOR_BUTTON_TEXT_DROP);

	SetTextColor(hdc, dropTextColor);
	TextOut(hdc,x+1,y+1,pString,strlen(pString));

	SetTextColor(hdc, color);
	TextOut(hdc,x,y,pString,strlen(pString));

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
#else
	primitives_DrawText(pDirectSurface, x, y, pString, color, bg);
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_ColoredDropText(
	aui_Surface *pDirectSurface,
	sint32 x,
	sint32 y,
	const MBCHAR *pString,
	COLORREF textColor,
	COLORREF dropColor,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	sint32 oldMode = 0;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, 0);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);





	SetTextColor(hdc, dropColor);
	TextOut(hdc,x+1,y+1,pString,strlen(pString));

	SetTextColor(hdc, textColor);
	TextOut(hdc,x,y,pString,strlen(pString));

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
#else
	primitives_DrawText(pDirectSurface, x, y, pString, textColor, bg); // not handling dropColor
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}

PRIMITIVES_ERRCODE primitives_DropTextCentered(
	aui_Surface *pDirectSurface,
	RECT *destRect,
	const MBCHAR *pString,
	COLORREF color,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC			hdc;
	HRESULT		hr;
	COLORREF	oldColor;
	sint32		oldMode = 0;
	SIZE		size;
	sint32		x,y;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, 0);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	COLORREF	dropTextColor = g_colorSet->GetColorRef(COLOR_BUTTON_TEXT_DROP);

	GetTextExtentPoint32(hdc, pString, strlen(pString),  &size);

	x = destRect->left + (destRect->right-destRect->left)/2 - size.cx/2;
	y = destRect->top + (destRect->bottom-destRect->top)/2 - size.cy/2;

	SetTextColor(hdc, dropTextColor);
	TextOut(hdc,x+1,y+1,pString,strlen(pString));

	SetTextColor(hdc, color);
	TextOut(hdc,x,y,pString,strlen(pString));

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();
	    
	if(font){
	    RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, *destRect);
	    font->DrawString(pDirectSurface, destRect, &clipRect, pString, k_AUI_BITMAPFONT_DRAWFLAG_VERTCENTER, color, 0); // no bg correspondence
	    }
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}

PRIMITIVES_ERRCODE primitives_ColoredDropTextCentered(
	aui_Surface *pDirectSurface,
	RECT *destRect,
	const MBCHAR *pString,
	COLORREF textColor,
	COLORREF dropColor,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC			hdc;
	HRESULT		hr;
	COLORREF	oldColor;
	sint32		oldMode = 0;
	SIZE		size;
	sint32		x,y;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	oldColor = SetTextColor(hdc, 0);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);





	GetTextExtentPoint32(hdc, pString, strlen(pString),  &size);

	x = destRect->left + (destRect->right-destRect->left)/2 - size.cx/2;
	y = destRect->top + (destRect->bottom-destRect->top)/2 - size.cy/2;

	SetTextColor(hdc, dropColor);
	TextOut(hdc,x+1,y+1,pString,strlen(pString));

	SetTextColor(hdc, textColor);
	TextOut(hdc,x,y,pString,strlen(pString));

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();
	    
	if(font){
	    RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, *destRect);
	    font->DrawString(pDirectSurface, destRect, &clipRect, pString, k_AUI_BITMAPFONT_DRAWFLAG_VERTCENTER, textColor, 0); // no bg correspondence, not handling dropColor
	    }
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}








PRIMITIVES_ERRCODE primitives_DropTextBatch(
	aui_Surface *pDirectSurface,
	sint32 x,
	sint32 y,
	const MBCHAR **pString,
	sint32 numStrings,
	COLORREF color,
	BOOL bg
	)
{
	Assert(pDirectSurface);
	if (pDirectSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pString);
	if (pString == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

#ifdef __AUI_USE_DIRECTX__
	HDC hdc;
	HRESULT hr;
	COLORREF oldColor;
	TEXTMETRIC tm;
	sint32 oldMode = 0;
	sint32 saveY = y;

	hr = pDirectSurface->GetDC(&hdc);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSGETDCFAILED;

	if (bg)
		oldMode = SetBkMode(hdc,TRANSPARENT);

	GetTextMetrics(hdc,&tm);

	oldColor = SetTextColor(hdc, 0);

	HFONT hOldFont = NULL;
	if (g_hFont)
		hOldFont = (HFONT)SelectObject(hdc,g_hFont);

	COLORREF	dropTextColor = g_colorSet->GetColorRef(COLOR_BUTTON_TEXT_DROP);

	SetTextColor(hdc, dropTextColor);

	sint32 i;
	for (i = 0; i < numStrings; i++)
	{

		TextOut(hdc,x+1,y+1,pString[i],strlen(pString[i]));
		y += tm.tmHeight + tm.tmExternalLeading;
	}

	SetTextColor(hdc, color);
	y = saveY;

	for (i = 0; i < numStrings; i++)
	{
		TextOut(hdc,x,y,pString[i],strlen(pString[i]));
		y += tm.tmHeight + tm.tmExternalLeading;
	}

	if (g_hFont)
		SelectObject(hdc,hOldFont);

	SetTextColor(hdc, oldColor);

	if (bg)
		SetBkMode(hdc,oldMode);

	hr = pDirectSurface->ReleaseDC(hdc);
	Assert(hr == AUI_ERRCODE_OK);
	if (hr != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_DSRELEASEDCFAILED;
#else	
	aui_BitmapFont *font= getBitmapFont();

	if(font){
	    for (sint32 i=0;i < numStrings;i++)
		{
		RECT rect = {x, y, pDirectSurface->Width(), pDirectSurface->Height()}; // ony x, y matter; width and hight get clipped in DrawString
		RECT clipRect = primitives_GetScreenAdjustedRectCopy(pDirectSurface, rect);
		font->DrawString(pDirectSurface, &rect, &clipRect, pString[i], 0, color, 0); // no bg correspondence
		y += font->GetLineSkip();
		
		}
	    }
#endif // __AUI_USE_DIRECTX__

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelPane16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

	if (flag)
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((right-1-i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}
	else
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((right-1-i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelTabSelected16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

    if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;

	sint32 tempHeight = height;
	sint32 tempInc = inc;

	if (flag)
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}
	}
	else
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelTabDeselected16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -=1;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -= 1;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -=1;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{
				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -= 1;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelLeftPiece16(
	aui_Surface *pSurface,
	sint32 xStart,
	sint32 xEnd,
	sint32 y,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;


	Assert(xStart <= xEnd);
	if (xStart >= xEnd) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = xEnd-xStart;

	uint16 * pDestPixel = (uint16 *)(pSurfBase + y * surfPitch + (xStart << 1));
	sint32 inc = (surfPitch >> 1) - (width-1);

	sint32 tempWidth = width;
	sint32 tempInc = inc;

	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_OldBevelRightPiece16(
	aui_Surface *pSurface,
	sint32 xStart,
	sint32 xEnd,
	sint32 y,
	sint32 level,
	BOOL flag
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;


	Assert(xStart <= xEnd);
	if (xStart >= xEnd) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32      width       = xEnd-xStart;
	uint16 *    pDestPixel  = (uint16 *)(pSurfBase + y * surfPitch + (xStart << 1));
	sint32      inc         = (surfPitch >> 1) - (width);
	sint32      tempWidth   = width-1;
	sint32      tempInc     = inc;

	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Shadow(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_Lightening(*pDestPixel);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelPane16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark,
	AUI_TABGROUP_ALIGNMENT a
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

    sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	pDestPixel = (uint16 *)(pSurfBase + (bottom-1) * surfPitch + ((right-1) << 1));

	if ( a == AUI_TABGROUP_ALIGNMENT_TOP )
	{
	if (flag)
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((right-1-i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}
	else
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel-- = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel -= tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i) * surfPitch + ((right-1-i) << 1));

			for (sint32 y = tempHeight;y;y--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}

			tempHeight -= 2;
		}
	}
	}
	else if ( a == AUI_TABGROUP_ALIGNMENT_LEFT )
	{
	}
	else
	{

		Assert( FALSE );
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelTabSelected16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark,
	AUI_TABGROUP_ALIGNMENT a
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;

	sint32 tempHeight = height;
	sint32 tempInc = inc;

	if ( a == AUI_TABGROUP_ALIGNMENT_TOP )
	{
	if (flag)
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}
	}
	else
	{

		for (sint32 j = level;j;j--)
		{
			for (sint32 i = tempWidth;i;i--)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
		}
	}
	}
	else if ( a == AUI_TABGROUP_ALIGNMENT_LEFT )
	{
	}
	else
	{

		Assert( FALSE );
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelTabDeselected16(
	aui_Surface *pSurface,
	RECT *pRect,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark,
	AUI_TABGROUP_ALIGNMENT a
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 left = pRect->left, top = pRect->top, right = pRect->right, bottom = pRect->bottom;

	Assert(left <= right);
	Assert(top <= bottom);
	if ((left >= right) || (top >= bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;

	sint32 width = right - left;
	sint32 height = bottom - top;

	if (width > height)
	{
		if (level > (height >> 1))
			level = height >> 1;
	}
	else
	{
		if (level > (width >> 1))
			level = width >> 1;
	}

	uint16 * pDestPixel = (uint16 *)(pSurfBase + top * surfPitch + (left << 1));
	sint32 inc = (surfPitch >> 1) - width;

	sint32 tempWidth = width-1;
	sint32 tempHeight = height-1;
	sint32 tempInc = inc;

	if ( a == AUI_TABGROUP_ALIGNMENT_TOP )
	{
	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -=1;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -= 1;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
			}
			tempWidth-=2;
			tempInc += 2;
			pDestPixel += tempInc;
		}

		tempInc = (surfPitch >> 1) - 1;

		sint32 i;
		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((left+i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -=1;
		}

		tempHeight = height-1;

		for (i = 0; i < level; i++)
		{
			pDestPixel = (uint16 *)(pSurfBase + (top+i+1) * surfPitch + ((right-1-i) << 1));

			for (sint32 y=0;y < tempHeight;y++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
				pDestPixel += tempInc;
			}
			tempHeight -= 1;
		}
	}
	}
	else if ( a == AUI_TABGROUP_ALIGNMENT_LEFT )
	{
	}
	else
	{

		Assert( FALSE );
	}


	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelLeftPiece16(
	aui_Surface *pSurface,
	RECT *tab,
	RECT *pane,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark,
	AUI_TABGROUP_ALIGNMENT a
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 xStart = 0;
	sint32 xEnd = 0;
	sint32 y = 0;

	switch ( a )
	{
	case AUI_TABGROUP_ALIGNMENT_TOP:
		xStart = pane->left;
		xEnd = tab->left;
		y = pane->top;
		break;

	case AUI_TABGROUP_ALIGNMENT_LEFT:
		xStart = tab->bottom;
		xEnd = pane->bottom;
		y = pane->left;
		break;

	default:

		Assert( FALSE );
	}


	Assert(xStart <= xEnd);
	if (xStart >= xEnd) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32      surfPitch   = pSurface->Pitch();
	sint32      width       = xEnd-xStart;
	uint16 *    pDestPixel  = (uint16 *)(pSurfBase + y * surfPitch + (xStart << 1));
	sint32      inc         = (surfPitch >> 1) - (width-1);
	sint32      tempWidth   = width;
	sint32      tempInc     = inc;

	Pixel16 srcPixel;

	if ( a == AUI_TABGROUP_ALIGNMENT_TOP )
	{
	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	}
	else if ( a == AUI_TABGROUP_ALIGNMENT_LEFT )
	{
	}
	else
	{

		Assert( FALSE );
	}


	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_BevelRightPiece16(
	aui_Surface *pSurface,
	RECT *tab,
	RECT *pane,
	sint32 level,
	BOOL flag,
	sint32 blendLight,
	sint32 blendDark,
	AUI_TABGROUP_ALIGNMENT a
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32 xStart = 0;
	sint32 xEnd = 0;
	sint32 y = 0;

	switch ( a )
	{
	case AUI_TABGROUP_ALIGNMENT_TOP:
		xStart = tab->right;
		xEnd = pane->right;
		y = pane->top;
		break;

	case AUI_TABGROUP_ALIGNMENT_LEFT:
		xStart = pane->top;
		xEnd = tab->top;
		y = pane->left;
		break;

	default:

		Assert( FALSE );
	}


	Assert(xStart <= xEnd);
	if (xStart >= xEnd) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(level >= 0);
	if (level <= 0) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	uint8 *pSurfBase;
	sint32 errcode = pSurface->Lock(NULL,(LPVOID *)&pSurfBase,0);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACELOCKFAILED;

	sint32      surfPitch   = pSurface->Pitch();
	sint32      width       = xEnd-xStart;
	uint16 *    pDestPixel  = (uint16 *)(pSurfBase + y * surfPitch + (xStart << 1));
	sint32      inc         = (surfPitch >> 1) - (width);
	sint32      tempWidth   = width-1;
	sint32      tempInc     = inc;

	Pixel16 srcPixel;
	if ( a == AUI_TABGROUP_ALIGNMENT_TOP )
	{
	if (flag)
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0x0000,blendDark);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	else
	{

		for (sint32 j = 0;j < level;j++)
		{
			for (sint32 i=0;i < tempWidth;i++)
			{

				srcPixel = pixelutils_BlendFast(*pDestPixel,0xffff,blendLight);
				*pDestPixel++ = srcPixel;
			}
			pDestPixel += tempInc;
		}
	}
	}
	else if ( a == AUI_TABGROUP_ALIGNMENT_LEFT )
	{
	}
	else
	{

		Assert( FALSE );
	}




	errcode = pSurface->Unlock((LPVOID)pSurfBase);
	Assert(errcode == AUI_ERRCODE_OK);
	if (errcode != AUI_ERRCODE_OK) return PRIMITIVES_ERRCODE_SURFACEUNLOCKFAILED;

	return PRIMITIVES_ERRCODE_OK;
}







PRIMITIVES_ERRCODE primitives_DrawFrame16(
	aui_Surface *pSurface,
	aui_Image *pImageUpperLeft,
	aui_Image *pImageUpperRight,
	aui_Image *pImageLowerLeft,
	aui_Image *pImageLowerRight,
	Pattern *pPatternLeft,
	Pattern *pPatternTop,
	Pattern *pPatternRight,
	Pattern *pPatternBottom,
	RECT *pRect
	)
{
	Assert(pSurface);
	if (pSurface == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pImageUpperLeft);
	if (pImageUpperLeft==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pImageUpperRight);
	if (pImageUpperRight==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pImageLowerLeft);
	if (pImageLowerLeft==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pImageLowerRight);
	if (pImageLowerRight==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pPatternLeft);
	if (pPatternLeft==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pPatternTop);
	if (pPatternTop==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pPatternRight);
	if (pPatternRight==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;
	Assert(pPatternBottom);
	if (pPatternBottom==NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect);
	if (pRect == NULL) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	Assert(pRect->left < pRect->right);
	Assert(pRect->top < pRect->bottom);
	if ((pRect->left >= pRect->right) || (pRect->top >= pRect->bottom)) return PRIMITIVES_ERRCODE_INVALIDPARAM;

	sint32	imageWidth = pImageUpperLeft->TheSurface()->Width();
	sint32	imageHeight = pImageUpperLeft->TheSurface()->Height();
	RECT	srcRect = { 0, 0, imageWidth, imageHeight };

	sint32	x = pRect->left, y = pRect->top;

	g_c3ui->TheBlitter()->Blt(pSurface, x, y, pImageUpperLeft->TheSurface(), &srcRect, k_AUI_BLITTER_FLAG_COPY);

	x = pRect->right - imageWidth;

	g_c3ui->TheBlitter()->Blt(pSurface, x, y, pImageUpperRight->TheSurface(), &srcRect, k_AUI_BLITTER_FLAG_COPY);

	x = pRect->left;
	y = pRect->bottom - imageHeight;

	g_c3ui->TheBlitter()->Blt(pSurface, x, y, pImageLowerLeft->TheSurface(), &srcRect, k_AUI_BLITTER_FLAG_COPY);

	x = pRect->right - imageWidth;

	g_c3ui->TheBlitter()->Blt(pSurface, x, y, pImageLowerRight->TheSurface(), &srcRect, k_AUI_BLITTER_FLAG_COPY);

	RECT rect;
	rect.left = pRect->left + imageWidth;
	rect.top = pRect->top;
	rect.right = pRect->right - imageWidth;
	rect.bottom = pRect->top + imageHeight;

	pPatternTop->Draw(pSurface, &rect);

	rect.top = pRect->bottom - imageHeight;
	rect.bottom = pRect->bottom;

	pPatternBottom->Draw(pSurface, &rect);

	rect.left = pRect->left;
	rect.top = pRect->top + imageHeight;
	rect.right = pRect->left + imageWidth;
	rect.bottom = pRect->bottom - imageHeight;

	pPatternLeft->Draw(pSurface, &rect);

	rect.left = pRect->right - imageWidth;
	rect.right = pRect->right;

	pPatternRight->Draw(pSurface, &rect);

	return PRIMITIVES_ERRCODE_OK;
}

#include "tileutils.h"

void primitives_HackTileDraw(aui_Surface *pSurface)
{
	uint16		len;
	sint32		x, y;
	sint32		startX, endX;
	uint32		accumTable[k_TILE_PIXEL_HEIGHT][3];
	FILE *      file = fopen("gtfb000.bin", "rb");
	fread((void *)&accumTable, 1, sizeof(uint32)*3*k_TILE_PIXEL_HEIGHT, file);
	fread((void *)&len, 1, sizeof(uint16), file);

	Pixel16 *   data = new Pixel16[len/2];
	fread((void *)data, 1, len, file);
	fclose(file);

	Pixel16 *   dataPtr = data;

	uint8 *pSurfBase;

	sint32 errcode = pSurface->Lock(NULL, (LPVOID *)&pSurfBase, 0);
	if ( errcode != AUI_ERRCODE_OK ) return;

	sint32 surfPitch = pSurface->Pitch();

	Pixel16 srcPixel;
	uint16 *pDestPixel;

	sint32 yoffset = 300;

	{

		for (y=0; y<k_TILE_PIXEL_HEIGHT; y++) {
			if (y<=23) {
				startX = (23-y)*2;
				endX = k_TILE_PIXEL_WIDTH - startX;
			} else {
				startX = (y-24)*2;
				endX = k_TILE_PIXEL_WIDTH - startX;
			}

			for (x = startX; x<endX; x++) {
				srcPixel = *dataPtr++;
				switch (srcPixel) {
				case 0x0000 : srcPixel = 0xF800; break;
				case 0x0001 : srcPixel = 0x07E0; break;
				case 0x0002 : srcPixel = 0x001F; break;
				case 0x0003 : srcPixel = 0xF81F; break;
				}
				pDestPixel = (Pixel16 *)(pSurfBase + ((y+yoffset) * surfPitch) + (x << 1));
				*pDestPixel = srcPixel;
			}
		}
	}

	errcode = pSurface->Unlock((LPVOID)pSurfBase);

}


#include "tiledmap.h"
#include "tileset.h"

extern TiledMap *g_tiledMap;

void primitives_HackStencilDraw(aui_Surface *pSurface)
{
	extern uint32 g_bitsTable[];
	extern Pixel16 g_offsets[];





	if (g_tiledMap == NULL) return;

	static uint16 from=4, to=14;

	tileutils_LoadStencil();


	Pixel16 *image = NULL;

	TileSet *tileSet = g_tiledMap->GetTileSet();

	if (tileSet)
		image = tileSet->GetTransitionData(from, to, 0);

	if (image != NULL) {

		Assert(pSurface);
		if (pSurface==NULL) return;

		uint8 *pSurfBase;

		sint32 errcode = pSurface->Lock(NULL, (LPVOID *)&pSurfBase, 0);
		if ( errcode != AUI_ERRCODE_OK ) return;

		sint32 surfPitch = pSurface->Pitch();

		Pixel16 srcPixel;
		uint16 *pDestPixel;

		{
			uint32		accum;
			sint32		i,j;
			sint32		nudge;
			sint32		x, y;
			Pixel16		*srcPixelPtr;

			x = 10; y = 400;

			srcPixelPtr = image;
			for (i=0; i<48; i++) {
				accum = g_bitsTable[i];

				if (i<=23) nudge = (23-i)*2;
				else nudge = (i-24)*2;

				x=nudge;

				for (j=0; j<32; j++) {
					if (accum & 1) {

						srcPixel = *srcPixelPtr;
						srcPixelPtr++;
						pDestPixel = (Pixel16 *)(pSurfBase + (y * surfPitch) + (x << 1));
						*pDestPixel = srcPixel;
					} else {

					}
					accum>>=1;
					x++;
				}
				y++;
			}
		}

		errcode = pSurface->Unlock((LPVOID)pSurfBase);

	}

	uint16 tmp = from;
	from = to;
	to = tmp;











}


#define SWAPVARS(a, b)		{ sint32 temp; temp = a; a = b; b = temp; }

static int adj_pixinc_x[4] =	{	1, 0, 1, 0		};
static int adj_pixinc_y[4] =	{	0, 1, 0, -1		};
static int diag_pixinc_x[4] =	{	1, 1, 1, 1,		};
static int diag_pixinc_y[4] =	{	1, 1, -1, -1	};
static int orth_pixinc_x[4] =	{	0, 1, 0, 1		};
static int orth_pixinc_y[4] =	{	1, 0, -1, 0		};

void primitives_BlendSurfaces( aui_Surface *pOldSurface, aui_Surface *pNewSurface, aui_Surface *pDstSurface, RECT *pDstRect, sint32 blend )
{
	BOOL			oldWasUnlocked = FALSE;
	BOOL			newWasUnlocked = FALSE;
	BOOL			dstWasUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	uint8 *pOldBase;
	uint8 *pNewBase;
	uint8 *pDstBase;

	if (pOldSurface->Buffer() == NULL) {
		errcode = pOldSurface->Lock(NULL,(LPVOID *)&pOldBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
		oldWasUnlocked = TRUE;
	} else {
		pOldBase = pOldSurface->Buffer();
	}

	if (pNewSurface->Buffer() == NULL) {
		errcode = pNewSurface->Lock(NULL,(LPVOID *)&pNewBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
		newWasUnlocked = TRUE;
	} else {
		pNewBase = pNewSurface->Buffer();
	}

	if (pDstSurface->Buffer() == NULL) {
		errcode = pDstSurface->Lock(NULL,(LPVOID *)&pDstBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
		dstWasUnlocked = TRUE;
	} else {
		pDstBase = pDstSurface->Buffer();
	}

	sint32 oldSurfPitch = pOldSurface->Pitch();

	sint32 newSurfPitch = pNewSurface->Pitch();

	sint32 dstSurfPitch = pDstSurface->Pitch();

	uint16 *pOldSrc = (uint16 *)pOldBase;
	uint16 *pNewSrc = (uint16 *)pNewBase;

	sint32 width = pDstRect->right - pDstRect->left;
	sint32 height = pDstRect->bottom - pDstRect->top;

	uint16 * pDestPixel = (uint16 *)(pDstBase + pDstRect->top * dstSurfPitch + (pDstRect->left << 1));
	sint32 dstInc = (dstSurfPitch >> 1) - width;

	sint32 oldInc = (oldSurfPitch >> 1) - width;
	sint32 newInc = (newSurfPitch >> 1) - width;

	Pixel16 color;

	for (sint32 j=height;j;j--) {
		for (sint32 i=width;i;i--) {
			color = pixelutils_BlendFast( *pOldSrc++, *pNewSrc++, blend );
			*pDestPixel++ = color;
		}
		pDestPixel += dstInc;
		pOldSrc += oldInc;
		pNewSrc += newInc;
	}

	if ( oldWasUnlocked ) {

		errcode = pOldSurface->Unlock((LPVOID)pOldBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
	}

	if ( newWasUnlocked ) {

		errcode = pNewSurface->Unlock((LPVOID)pNewBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
	}

	if ( dstWasUnlocked ) {

		errcode = pDstSurface->Unlock((LPVOID)pDstBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
	}

	return;
}

void primitives_LightenSurface( aui_Surface *pSurface, sint32 percentLighten )
{
	BOOL			wasUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	uint8 *pBase;

	if (pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
		wasUnlocked = TRUE;
	} else {
		pBase = pSurface->Buffer();
	}

	sint32 surfWidth = pSurface->Width();
	sint32 surfHeight = pSurface->Height();
	sint32 surfPitch = pSurface->Pitch();

	sint32 width = surfWidth;
	sint32 height = surfHeight;

	uint16 * pDestPixel = (uint16 *)(pBase);
	sint32 inc = (surfPitch >> 1) - width;

	Pixel16 color;

	for (sint32 j=height;j;j--) {
		for (sint32 i=width;i;i--) {
			if ( *pDestPixel ) {
				color = pixelutils_PercentLighten( *pDestPixel, percentLighten );
				*pDestPixel++ = color;
			}
			else {
				pDestPixel++;
			}
		}
		pDestPixel += inc;
	}

	if ( wasUnlocked ) {

		errcode = pSurface->Unlock((LPVOID)pBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
	}

	return;
}

void primitives_LightenRect(aui_Surface *pSurface, RECT &rect, sint32 percentLighten)
{
	BOOL			wasUnlocked = FALSE;
	AUI_ERRCODE		errcode;

	uint8 *pBase;

	if (pSurface->Buffer() == NULL) {
		errcode = pSurface->Lock(NULL,(LPVOID *)&pBase,0);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
		wasUnlocked = TRUE;
	} else {
		pBase = pSurface->Buffer();
	}

	sint32 surfPitch = pSurface->Pitch();

	uint16 *pDestPixel;

	sint32 width = rect.right - rect.left;
	sint32 height = rect.bottom - rect.top;

	Pixel16 color;

	for (sint32 j=0; j<height; j++) {

		pDestPixel = (uint16 *)(pBase + (rect.top + j) * surfPitch + rect.left * 2);
		for (sint32 i=width;i;i--) {
			color = pixelutils_Desaturate( *pDestPixel );
			*pDestPixel++ = color;
		}
	}

	if ( wasUnlocked ) {

		errcode = pSurface->Unlock((LPVOID)pBase);
		Assert(errcode == AUI_ERRCODE_OK);
		if (errcode != AUI_ERRCODE_OK) return;
	}

	return;
}

inline uint32 ReverseLinePattern(uint32 fullPattern, uint32 patternLength)
{
	uint32 reversePattern = 0;
	for (uint32 i = 0; i < patternLength; i++) {
		reversePattern <<= 1;
		reversePattern |= fullPattern & 1;
		fullPattern >>= 1;
	}
	return reversePattern | (1 << patternLength); // add roll-over bit
}

inline uint32 UpdateLinePattern(uint32 fullPattern, uint32 currentPattern)
{
	currentPattern >>= 1;
	if (currentPattern == 1) {
		currentPattern = fullPattern;
	}
	return currentPattern;
}

inline bool IsLinePatternActive(uint32 currentPattern)
{
	return currentPattern & 1;
}

inline bool RectIntersectSurface(const aui_Surface & surf, const RECT & rect)
{
	return rect.left < surf.Width() && rect.right >= 0 && rect.top < surf.Height() && rect.bottom >= 0;
}

inline RECT ClipRect(const aui_Surface & surf, const RECT & rect)
{
	Assert(rect.left <= rect.right);
	Assert(rect.top <= rect.bottom);

	if (!RectIntersectSurface(surf, rect)) {
		return RECT { 0, 0, 0, 0 };
	}

	return RECT {
		rect.left >= 0 ? rect.left : 0,
		rect.top >= 0 ? rect.top : 0,
		rect.right < surf.Width() ? rect.right : surf.Width() - 1,
		rect.bottom < surf.Height() ? rect.bottom : surf.Height() - 1
	};
}

bool ClipLine(const RECT & rect, sint32 & x1, sint32 & y1, sint32 & x2, sint32 & y2)
{
	Assert(y1 <= y2);

	sint32 left   = rect.left;
	sint32 right  = rect.right;
	sint32 top    = rect.top;
	sint32 bottom = rect.bottom;

	bool mirror = false;

	if ( y1 > bottom || y2 < top) {
		return false;
	}
	if (x1 > x2)
	{
		if (x1 < left || x2 > right) {
			return false;
		}
		mirror = true;
		x1 = -x1;
		x2 = -x2;

		sint32 temp = left;
		left  = -right;
		right = -temp;
	} else {
		if (x1 > right || x2 < left) {
			return false;
		}
	}
	sint32 deltaX = x2 - x1;
	sint32 deltaY = y2 - y1;
	if (x1 < left)
	{
		y1 += ((left - x1) * deltaY) / deltaX;
		if (y1 > bottom) {
			return false;
		}
		x1 = left;
	}
	if (y1 < top)
	{
		x1 += ((top - y1) * deltaX) / deltaY;
 		if (x1 > right) {
 			return false;
 		}
		y1 = top;
	}
	if (x2 > right)
	{
		y2 = y1 + ((right - x1) * deltaY) / deltaX;
		x2 = right;
	}
	if (y2 > bottom)
	{
		x2 = x1 + ((bottom - y1) * deltaX) / deltaY;
		y2 = bottom;
	}

	if (mirror)
	{
		x1 = -x1;
		x2 = -x2;
	}
	return true;
}


inline Pixel16 * GetBasePixel16(const aui_Surface & surf, sint32 x, sint32 y)
{
	uint8 * base = surf.Buffer();
	Assert(base);
	return (Pixel16 *) (base) + y * (surf.Pitch() >> 1) + x;
}

inline void DrawLine16(Pixel16 * pixel, sint32 length, sint32 pitch, Pixel16 color)
{
	Pixel16 * endPixel = pixel + length * pitch;
	while (pixel < endPixel) {
		*pixel = color;
		pixel += pitch;
	}
}

inline void DrawPatternLine16(Pixel16 * pixel, sint32 length, sint32 pitch, Pixel16 color,
		uint32 fullPattern, uint32 currentPattern)
{
	Pixel16 * endPixel = pixel + length * pitch;
	while (pixel < endPixel) {
		if (IsLinePatternActive(currentPattern)) {
			*pixel = color;
		}
		pixel += pitch;
		currentPattern = UpdateLinePattern(fullPattern, currentPattern);
	}
}

inline void BlendLine16(Pixel16 * pixel, sint32 length, sint32 pitch, Pixel16 color, uint8 alpha, int blendRGBMask)
{
	Pixel16 * endPixel = pixel + length * pitch;
	while (pixel < endPixel) {
		*pixel = pixelutils_Blend16(*pixel, color, alpha, blendRGBMask);
		pixel += pitch;
	}
}

inline void DrawShadowLine16(Pixel16 * pixel, sint32 length, sint32 pitch, int shadowRGBMask)
{
	Pixel16 * endPixel = pixel + length * pitch;
	while (pixel < endPixel) {
		*pixel = pixelutils_Shadow16(*pixel, shadowRGBMask);
		pixel += pitch;
	}
}

inline void DrawRect(Pixel16 * base, sint32 width, sint32 height, sint32 pitch, Pixel16 color)
{
	Pixel16 * pixel = base;
	Pixel16 * endPixel = pixel + height * pitch;
	while (pixel < endPixel)
	{
		DrawLine16(pixel, width, 1, color);
		pixel += pitch;
	}
}

inline void BlendRect(Pixel16 * base, sint32 width, sint32 height, sint32 pitch, Pixel16 color, uint8 alpha)
{
	const int RGB_MASK = pixelutils_GetBlend16RGBMask();

	Pixel16 * pixel = base;
	Pixel16 * endPixel = pixel + height * pitch;
	while (pixel < endPixel)
	{
		BlendLine16(pixel, width, 1, color, alpha, RGB_MASK);
		pixel += pitch;
	}
}

void DrawAntiAliasedLine16(Pixel16 * pixel, sint32 majorLength, sint32 minorLength, sint32 majorPitch,
		sint32 minorPitch, Pixel16 color)
{
	const uint32 RGB_MASK = pixelutils_GetBlend16RGBMask();

	Pixel16 * endPixel = pixel + majorLength * majorPitch + minorLength * minorPitch;

	// first and last pixel are full-pixel
	*pixel    = color;
	*endPixel = color;

	// calculate 16-bit fixed-point fractional part of a
	// pixel that minor advances each time major advances 1 pixel, truncating the
	// result so that we won't overrun the endpoint along the minor axis
	const uint16 errorFraction = uint16 ((minorLength << 16) / (uint32) majorLength);
	// Initialize the line error accumulator to 0
	uint16 errorAccumulator = 0;

	while (pixel < endPixel)
	{
		const uint16 error = errorAccumulator; // remember current accumulated error
		errorAccumulator += errorFraction;     // calculate error for next pixel

		if (errorAccumulator <= error)
		{
			// Error accumulator turned over, so advance the minor
			pixel += minorPitch;
		}
		pixel += majorPitch; // always advance major

		Pixel16 * pairedPixel = pixel + minorPitch;
		if (pairedPixel <= endPixel + 2) // + 2 to allow lines with negative delta-x to connect to end-pixel
		{
			// Most significant bits of exrrorAccumulator determine the weight of this pixel
			uint8 weight = errorAccumulator >> 8;
			*pixel = pixelutils_Blend16(*pixel, color, weight ^ 255, RGB_MASK);
			*pairedPixel = pixelutils_Blend16(*pairedPixel, color, weight, RGB_MASK);
		}
	}
}

void DrawAntiAliasedPatternLine16(Pixel16 * pixel, sint32 majorLength, sint32 minorLength, sint32 majorPitch,
		sint32 minorPitch, Pixel16 color, uint32 fullPattern, uint32 currentPattern)
{
	const uint32 RGB_MASK = pixelutils_GetBlend16RGBMask();

	Pixel16 * endPixel = pixel + majorLength * majorPitch + minorLength * minorPitch;

	// first pixel is full-pixel
	if (IsLinePatternActive(currentPattern)) {
		*pixel = color;
	}
	currentPattern = UpdateLinePattern(fullPattern, currentPattern);

	// calculate 16-bit fixed-point fractional part of a
	// pixel that minor advances each time major advances 1 pixel, truncating the
	// result so that we won't overrun the endpoint along the minor axis
	const uint16 errorFraction = uint16 ((minorLength << 16) / (uint32) majorLength);
	// Initialize the line error accumulator to 0
	uint16 errorAccumulator = 0;

	while (pixel < endPixel)
	{
		const uint16 error = errorAccumulator; // remember current accumulated error
		errorAccumulator += errorFraction;     // calculate error for next pixel

		if (errorAccumulator <= error)
		{
			// Error accumulator turned over, so advance the minor
			pixel += minorPitch;
		}
		pixel += majorPitch; // always advance major

		Pixel16 * pairedPixel = pixel + minorPitch;
		// + 2 to allow lines with negative delta-x to connect to end-pixel
		if ((pairedPixel <= endPixel + 2) && IsLinePatternActive(currentPattern))
		{
			// Most significant bits of exrrorAccumulator determine the weight of this pixel
			uint8 weight = errorAccumulator >> 8;
			*pixel = pixelutils_Blend16(*pixel, color, weight ^ 255, RGB_MASK);
			*pairedPixel = pixelutils_Blend16(*pairedPixel, color, weight, RGB_MASK);
		}
		currentPattern = UpdateLinePattern(fullPattern, currentPattern);
	}
	// last pixel is full-pixel
	if (IsLinePatternActive(currentPattern)) {
		*endPixel = color;
	}
}

void primitives_ClippedPaintRect16(aui_Surface & surf, const RECT & rect, Pixel16 color, uint8 alpha)
{
	RECT clippedRect = ClipRect(surf, rect);
	sint32 width  = clippedRect.right - clippedRect.left;
	sint32 height = clippedRect.bottom - clippedRect.top;

	sint32 surfPitchPixels = surf.Pitch() >> 1;

	Pixel16 * base = GetBasePixel16(surf, clippedRect.left, clippedRect.top);
	if (alpha == pixelutils_OPAQUE) {
		DrawRect(base, width, height, surfPitchPixels, color);
	} else {
		BlendRect(base, width, height, surfPitchPixels, color, alpha);
	}
}

void primitives_ClippedFrameRect16(aui_Surface & surf, const RECT & rect, Pixel16 color, uint8 alpha)
{
	const int blendRGBMask = pixelutils_GetBlend16RGBMask();

	RECT clippedRect = ClipRect(surf, rect);
	sint32 width  = clippedRect.right - clippedRect.left;
	sint32 height = clippedRect.bottom - clippedRect.top;

	sint32 surfPitchPixels = surf.Pitch() >> 1;
	Pixel16 * basePixel = GetBasePixel16(surf, clippedRect.left, clippedRect.top);

	// top line
	if (rect.top >= 0)
	{
		BlendLine16(basePixel, width, 1, color, alpha, blendRGBMask);
	}
	// left line
	if (rect.left >= 0)
	{
		BlendLine16(basePixel + surfPitchPixels, height - 2, surfPitchPixels, color, alpha, blendRGBMask);
	}
	// right line
	if (rect.right < surf.Width())
	{
		BlendLine16(basePixel + surfPitchPixels + width - 1, height - 2, surfPitchPixels, color, alpha, blendRGBMask);
	}
	// bottom line
	if (rect.bottom < surf.Height())
	{
		BlendLine16(basePixel + (height - 1) * surfPitchPixels, width, 1, color, alpha, blendRGBMask);
	}
}

void primitives_ClippedShadowRect16(aui_Surface & surf, const RECT & rect)
{
	const int shadowRGBMask = pixelutils_GetShadow16RGBMask();

	RECT clippedRect = ClipRect(surf, rect);
	sint32 width  = clippedRect.right - clippedRect.left;
	sint32 height = clippedRect.bottom - clippedRect.top;

	sint32 surfPitchPixels = surf.Pitch() >> 1;

	Pixel16 * pixel = GetBasePixel16(surf, clippedRect.left, clippedRect.top);
	Pixel16 * endPixel = pixel + height * surfPitchPixels;
	while (pixel < endPixel)
	{
		DrawShadowLine16(pixel, width, 1, shadowRGBMask);
		pixel += surfPitchPixels;
	}
}

void primitives_ClippedAntiAliasedLine16(aui_Surface & surf, sint32 x1, sint32 y1, sint32 x2, sint32 y2, Pixel16 color)
{
	if (y2 < y1)
	{
		SWAPVARS(x1, x2)
		SWAPVARS(y1, y2);
	}

	RECT clipRect = { 0, 0, surf.Width() - 1, surf.Height() - 1 };
	if (!ClipLine(clipRect, x1, y1, x2, y2)) {
		return;
	}

	sint32 deltaY     = y2 - y1;
	sint32 deltaX     = x2 - x1;
	sint32 incrementX = deltaX < 0 ? -1 : 1;
	if (incrementX < 0) {
		deltaX = -deltaX;
	}

	Pixel16 * base = GetBasePixel16(surf, x1, y1);
	sint32 surfPitchPixels = surf.Pitch() >> 1;
	if (deltaX == 0) { // vertical
		DrawLine16(base, deltaY + 1, surfPitchPixels, color);
	} else if (deltaY == 0) { // horizontal
		if (incrementX < 0) {
			base = base - deltaX;
		}
		DrawLine16(base, deltaX + 1, 1, color);
	} else if (deltaX == deltaY) { // diagonal
		DrawLine16(base, deltaX + 1, surfPitchPixels + incrementX, color);
	} else if (deltaX < deltaY) {
		DrawAntiAliasedLine16(base, deltaY, deltaX, surfPitchPixels, incrementX, color);
	} else {
		Assert (deltaX > deltaY);
		DrawAntiAliasedLine16(base, deltaX, deltaY, incrementX, surfPitchPixels, color);
	}
}

void primitives_ClippedAntiAliasedPatternLine16(aui_Surface & surf, sint32 x1, sint32 y1, sint32 x2, sint32 y2,
		Pixel16 color, uint32 fullPattern, uint32 patternLength)
{
	if (y2 < y1)
	{
		SWAPVARS(x1, x2)
		SWAPVARS(y1, y2);
	}

	RECT clipRect = { 0, 0, surf.Width() - 1, surf.Height() - 1 };
	if (!ClipLine(clipRect, x1, y1, x2, y2)) {
		return;
	}

	sint32 deltaY     = y2 - y1;
	sint32 deltaX     = x2 - x1;
	sint32 incrementX = deltaX < 0 ? -1 : 1;
	if (incrementX < 0) {
		deltaX = -deltaX;
	}

	Pixel16 * base = GetBasePixel16(surf, x1, y1);
	sint32 surfPitchPixels = surf.Pitch() >> 1;
	if (deltaX == 0) { // vertical
		uint32 startPattern = fullPattern >> (y1 % patternLength);
		DrawPatternLine16(base, deltaY + 1, surfPitchPixels, color, fullPattern, startPattern);
	} else if (deltaY == 0) { // horizontal
		uint32 startPattern = fullPattern >> ((incrementX < 0 ? (x1 - deltaX) : x1) % patternLength);
		if (incrementX < 0) {
			base = base - deltaX;
		}
		DrawPatternLine16(base, deltaX + 1, 1, color, fullPattern, startPattern);
	} else if (deltaX == deltaY) { // diagonal
		uint32 startPattern = fullPattern >> (y1 % patternLength);
		DrawPatternLine16(base, deltaX + 1, surfPitchPixels + incrementX, color, fullPattern, startPattern);
	} else if (deltaX < deltaY) {
		uint32 startPattern = fullPattern >> (y1 % patternLength);
		DrawAntiAliasedPatternLine16(base, deltaY, deltaX, surfPitchPixels, incrementX, color, fullPattern,
		                             startPattern);
	} else {
		Assert (deltaX > deltaY);
		uint32 startPattern;
		if (incrementX < 0) {
			fullPattern = ReverseLinePattern(fullPattern, patternLength);
			startPattern = fullPattern >> (patternLength - 1 - (x1 % patternLength));
		} else {
			startPattern = fullPattern >> (x1 % patternLength);
		}
		DrawAntiAliasedPatternLine16(base, deltaX, deltaY, incrementX, surfPitchPixels, color, fullPattern,
		                             startPattern);
	}
}

RECT primitives_GetScreenAdjustedRectCopy(aui_Surface *surf, RECT &theRect)
{

	RECT clipRect = theRect;

	// Adjust clipRect to fit on the screen
	if (clipRect.left   <  0             ) clipRect.left   = 0;
	if (clipRect.top    <  0             ) clipRect.top    = 0;
	if (clipRect.right  >= surf->Width() ) clipRect.right  = surf->Width()  - 1;
	if (clipRect.bottom >= surf->Height()) clipRect.bottom = surf->Height() - 1;

	return clipRect;
}
