/*
	vid.h

	@description@

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

	$Id$
*/

#ifndef __vid_h
#define __vid_h

#include "qtypes.h"

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)

typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s	*pnext;
} vrect_t;

typedef struct
{
	pixel_t			*buffer;		// invisible buffer
	pixel_t			*colormap;		// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	int				fullbright;		// index of first fullbright color
	unsigned		rowbytes;	// may be > width if displayed in a window
	unsigned		width;		
	unsigned		height;
	float			aspect;		// width / height -- < 0 is taller than wide
	int				numpages;
	int				recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t			*conbuffer;
	int				conrowbytes;
	unsigned		conwidth;
	unsigned		conheight;
	int				maxwarpwidth;
	int				maxwarpheight;
	pixel_t			*direct;		// direct drawing to framebuffer, if not
									//  NULL
} viddef_t;

extern	viddef_t	vid;				// global video state
extern	unsigned short	d_8to16table[256];
extern	unsigned	d_8to24table[256];
extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);

qboolean VID_Is8bit(void);

void	VID_SetPalette (unsigned char *palette);
// called at startup and after any gamma correction

void	VID_ShiftPalette (unsigned char *palette);
// called for bonus and pain flashes, and for underwater color changes

void	VID_Init (unsigned char *palette);
void	VID_InitCvars (void);
// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void	VID_Shutdown (void);
// Called at shutdown

void	VID_Update (vrect_t *rects);
// flushes the given rectangles from the view buffer to the screen

int VID_SetMode (int modenum, unsigned char *palette);
// sets the mode; only used by the Quake engine for resetting to mode 0 (the
// base mode) on memory allocation failures

void VID_HandlePause (qboolean pause);
// called only on Win32, when pause happens, so the mouse can be released

void VID_SetCaption (char *text);

#if defined(_WIN32) && !defined(WINDED)
void	VID_LockBuffer (void);
void	VID_UnlockBuffer (void);
#else
#define	VID_LockBuffer()
#define	VID_UnlockBuffer()
#endif

#endif // __vid_h
