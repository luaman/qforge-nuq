/*
        vid_sdl.c

	Video driver for Sam Lantinga's Simple DirectMedia Layer

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SDL.h"

#include "host.h"
#include "menu.h"
#include "vid.h"
#include "sys.h"
#include "mathlib.h"    // needed by: protocol.h, render.h, client.h,
			//  modelgen.h, glmodel.h
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "net.h"        // needed by: client.h
#include "protocol.h"   // needed by: client.h
#include "cmd.h"
#include "keys.h"
#include "sbar.h"
#include "sound.h"
#include "render.h"     // needed by: client.h, gl_model.h, glquake.h
#include "client.h"     // need cls in this file
#include "console.h"
#include "qendian.h"
#include "qargs.h"
#include "compat.h"
#include "d_local.h"
#include "input.h"

cvar_t	*_windowed_mouse;

int old_windowed_mouse;

// static float oldin_grab = 0;

extern viddef_t    vid;                // global video state
unsigned short  d_8to16table[256];

#ifdef WIN32
// fixme: this is evil...
#include <windows.h>
HWND 		mainwindow;
#endif

int modestate; // fixme: just to avoid cross-comp. errors - remove later
                                                        
// The original defaults
//#define    BASEWIDTH    320
//#define    BASEHEIGHT   200
// Much better for high resolution displays
#define    BASEWIDTH    (320*2)
#define    BASEHEIGHT   (200*2)

int    VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes = 0;
byte    *VGA_pagebase;

static SDL_Surface *screen = NULL;

static qboolean mouse_avail;
static float   mouse_x, mouse_y;
static int mouse_oldbuttonstate = 0;

void    VID_SetPalette (unsigned char *palette)
{
	int i;
	SDL_Color colors[256];

	for ( i=0; i<256; ++i )
	{
		colors[i].r = *palette++;
		colors[i].g = *palette++;
		colors[i].b = *palette++;
	}
	SDL_SetColors(screen, colors, 0, 256);
}

void    VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

void    VID_Init (unsigned char *palette)
{
	int pnum, chunk;
	byte *cache;
	int cachesize;
	//Uint8 video_bpp;
	//Uint16 video_w, video_h;
	Uint32 flags;

	// Load the SDL library
	if (SDL_Init(SDL_INIT_VIDEO)<0) //|SDL_INIT_AUDIO|SDL_INIT_CDROM) < 0)
		Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());

	// Set up display mode (width and height)
	vid.width = BASEWIDTH;
	vid.height = BASEHEIGHT;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	if ((pnum=COM_CheckParm("-winsize")))
	{
		if (pnum >= com_argc-2)
			Sys_Error("VID: -winsize <width> <height>\n");
		vid.width = atoi(com_argv[pnum+1]);
		vid.height = atoi(com_argv[pnum+2]);
		if (!vid.width || !vid.height)
			Sys_Error("VID: Bad window width/height\n");
	}

	// Set video width, height and flags
	flags = (SDL_SWSURFACE|SDL_HWPALETTE);
	if ( COM_CheckParm ("-fullscreen") )
	flags |= SDL_FULLSCREEN;

	// Initialize display
	if (!(screen = SDL_SetVideoMode(vid.width, vid.height, 8, flags)))
		Sys_Error("VID: Couldn't set video mode: %s\n", SDL_GetError());
	VID_SetPalette(palette);
	VID_SetCaption("sdlquakeworld");

	// now know everything we need to know about the buffer
	VGA_width = vid.conwidth = vid.width;
	VGA_height = vid.conheight = vid.height;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	VGA_pagebase = vid.buffer = screen->pixels;
	VGA_rowbytes = vid.rowbytes = screen->pitch;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.direct = 0;

	// allocate z buffer and surface cache
	chunk = vid.width * vid.height * sizeof (*d_pzbuffer);
	cachesize = D_SurfaceCacheForRes (vid.width, vid.height);
	chunk += cachesize;
	d_pzbuffer = Hunk_HighAllocName(chunk, "video");
	if (d_pzbuffer == NULL)
		Sys_Error ("Not enough memory for video mode\n");

	// initialize the cache memory
	cache = (byte *) d_pzbuffer + vid.width * vid.height *
	       	sizeof (*d_pzbuffer);
	D_InitCaches (cache, cachesize);

	// initialize the mouse
	SDL_ShowCursor(0);

#ifdef WIN32
	// fixme: EVIL thing - but needed for win32 until we get
	// SDL_sound ready - without this DirectSound fails.
	// could replace this with SDL_SysWMInfo
	mainwindow=GetActiveWindow();
#endif

}

void    VID_Shutdown (void)
{
	SDL_Quit();
}

void    VID_Update (vrect_t *rects)
{
	// I feel this is an improvement on Sam's code, as it does not
	// calloc every frame. - DDOI
	while (rects) {
		SDL_UpdateRect (screen, rects->x, rects->y, rects->width,
				rects->height);
		rects = rects->pnext;
	}
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	Uint8 *offset;

	if (!screen) return;
	if ( x < 0 ) x = screen->w+x-1;
	offset = (Uint8 *)screen->pixels + y*screen->pitch + x;
	while ( height-- )
	{
		memcpy(offset, pbitmap, width);
		offset += screen->pitch;
		pbitmap += width;
	}
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	if (!screen)
		return;
	if (x < 0)
		x = screen->w+x-1;
	SDL_UpdateRect(screen, x, y, width, height);
}


/*
================
IN_SendKeyEvents
================
*/

void IN_SendKeyEvents (void)
{
	SDL_Event event;
	int sym, state, but;
	int modstate;

	while (SDL_PollEvent(&event))
	{
		switch (event.type) {

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				sym = event.key.keysym.sym;
				state = event.key.state;
				modstate = SDL_GetModState();
				switch(sym)
				{
					case SDLK_DELETE: sym = K_DEL; break;
					case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
					case SDLK_F1: sym = K_F1; break;
					case SDLK_F2: sym = K_F2; break;
					case SDLK_F3: sym = K_F3; break;
					case SDLK_F4: sym = K_F4; break;
					case SDLK_F5: sym = K_F5; break;
					case SDLK_F6: sym = K_F6; break;
					case SDLK_F7: sym = K_F7; break;
					case SDLK_F8: sym = K_F8; break;
					case SDLK_F9: sym = K_F9; break;
					case SDLK_F10: sym = K_F10; break;
					case SDLK_F11: sym = K_F11; break;
					case SDLK_F12: sym = K_F12; break;
					case SDLK_BREAK:
					case SDLK_PAUSE: sym = K_PAUSE; break;
					case SDLK_UP: sym = K_UPARROW; break;
					case SDLK_DOWN: sym = K_DOWNARROW; break;
					case SDLK_RIGHT: sym = K_RIGHTARROW; break;
					case SDLK_LEFT: sym = K_LEFTARROW; break;
					case SDLK_INSERT: sym = K_INS; break;
					case SDLK_HOME: sym = K_HOME; break;
					case SDLK_END: sym = K_END; break;
					case SDLK_PAGEUP: sym = K_PGUP; break;
					case SDLK_PAGEDOWN: sym = K_PGDN; break;
					case SDLK_RSHIFT:
					case SDLK_LSHIFT: sym = K_SHIFT; break;
					case SDLK_RCTRL:
					case SDLK_LCTRL: sym = K_CTRL; break;
					case SDLK_RALT:
					case SDLK_LALT: sym = K_ALT; break;
					case SDLK_CAPSLOCK: sym = K_CAPSLOCK; break;
					case SDLK_KP0:
						if(modstate & KMOD_NUM)
							sym = K_INS;
						else
							sym = SDLK_0;
						break;
					case SDLK_KP1:
						if(modstate & KMOD_NUM)
							sym = K_END;
						else
							sym = SDLK_1;
						break;
					case SDLK_KP2:
						if(modstate & KMOD_NUM)
							sym = K_DOWNARROW;
						else
							sym = SDLK_2;
						break;
					case SDLK_KP3:
						if(modstate & KMOD_NUM)
							sym = K_PGDN;
						else
							sym = SDLK_3;
						break;
					case SDLK_KP4:
						if(modstate & KMOD_NUM)
							sym = K_LEFTARROW;
						else
							sym = SDLK_4;
						break;
					case SDLK_KP5: sym = SDLK_5; break;
					case SDLK_KP6:
						if(modstate & KMOD_NUM)
							sym = K_RIGHTARROW;
						else
							sym = SDLK_6;
						break;
					case SDLK_KP7:
						if(modstate & KMOD_NUM)
							sym = K_HOME;
						else
							sym = SDLK_7;
						break;
					case SDLK_KP8:
						if(modstate & KMOD_NUM)
							sym = K_UPARROW;
						else
							sym = SDLK_8;
						break;
					case SDLK_KP9:
						if(modstate & KMOD_NUM)
							sym = K_PGUP;
						else
							sym = SDLK_9;
						break;
					case SDLK_KP_PERIOD:
						if(modstate & KMOD_NUM)
							sym = K_DEL;
						else
							sym = SDLK_PERIOD;
						break;
					case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
					case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
					case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
					case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
					case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
					case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;
				}
				// If we're not directly handled and still above
				// 255 just force it to 0
				if(sym > 255) sym = 0;
				Key_Event(sym, state);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				but = event.button.button;
				if (but == 2)
					but = 3;
				else if (but == 3)
					but = 2;

				switch (but)
				{
					case 1:
					case 2:
					case 3:
						Key_Event(K_MOUSE1 + but - 1, event.type == SDL_MOUSEBUTTONDOWN);
						break;
					case 4:
						Key_Event(K_MWHEELUP, 1);
						Key_Event(K_MWHEELUP, 0);
						break;
					case 5:
						Key_Event(K_MWHEELDOWN, 1);
						Key_Event(K_MWHEELDOWN, 0);
						break;
				}
				break;

			case SDL_MOUSEMOTION:
				if (_windowed_mouse->int_val)
				{
					if ((event.motion.x != (vid.width/2))
						|| (event.motion.y != (vid.height/2)) )
					{
						mouse_x = event.motion.xrel*10;
						mouse_y = event.motion.yrel*10;
						if ((event.motion.x < ((vid.width/2)-(vid.width/4))) || (event.motion.x > ((vid.width/2)+(vid.width/4))) || (event.motion.y < ((vid.height/2)-(vid.height/4))) || (event.motion.y > ((vid.height/2)+(vid.height/4))) )
							SDL_WarpMouse(vid.width/2, vid.height/2);
					}
				}
				else
				{
					mouse_x = event.motion.xrel*10;
					mouse_y = event.motion.yrel*10;
				}
				break;

			case SDL_QUIT:
				CL_Disconnect ();
				Sys_Quit ();
				break;
			default:
				break;
		}
	}
}

void IN_Commands (void)
{
	if (old_windowed_mouse != _windowed_mouse->int_val)
	{
		old_windowed_mouse = _windowed_mouse->int_val;
		if (!_windowed_mouse->int_val)
			SDL_WM_GrabInput (SDL_GRAB_OFF);
		else
			SDL_WM_GrabInput (SDL_GRAB_ON);
	}
}

void IN_Init (void)
{
	_windowed_mouse = Cvar_Get ("_windowed_mouse","0",CVAR_ARCHIVE,"None");

	if ( COM_CheckParm("-nomouse") && !_windowed_mouse->int_val)
		return;

	mouse_x = mouse_y = 0.0;
	mouse_avail = 1;
}

void IN_Shutdown (void)
{
	mouse_avail = 0;
}

void IN_Frame(void)
{
	int i;
	int mouse_buttonstate;

	if (!mouse_avail) return;

		i = SDL_GetMouseState(NULL, NULL);
		/* Quake swaps the second and third buttons */
		mouse_buttonstate = (i & ~0x06) | ((i & 0x02)<<1) |
			((i & 0x04)>>1);
	for (i=0 ; i<3 ; i++) {
		if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
			Key_Event (K_MOUSE1 + i, true);

		if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
			Key_Event (K_MOUSE1 + i, false);
	}
	mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_Move (usercmd_t *cmd)
{
	if (!mouse_avail)
		return;

	mouse_x *= sensitivity->value;
	mouse_y *= sensitivity->value;

	if ( (in_strafe.state & 1) || (lookstrafe->int_val && (in_mlook.state & 1) ))
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;
	if (in_mlook.state & 1)
		V_StopPitchDrift ();

	if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	} else {
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward->value * mouse_y;
		else
			cmd->forwardmove -= m_forward->value * mouse_y;
	}
	mouse_x = mouse_y = 0.0;
}

void VID_InitCvars ()
{
	// It may not look like it, but this is important
}

void VID_SetCaption (char *text)
{
	SDL_WM_SetCaption(text, NULL);
}

void VID_HandlePause (qboolean pause)
{
}

void IN_HandlePause (qboolean pause)
{
}
