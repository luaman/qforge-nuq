/*
	vid_glx.c

	OpenGL GLX video driver

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2000       Marcus Sundberg [mackan@stacken.kth.se]

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
# include "config.h"
#endif

#include <string.h>

#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef HAVE_DGA
# include <X11/extensions/xf86dga.h>
#endif

#include "compat.h"
#include "console.h"
#include "context_x11.h"
#include "glquake.h"
#include "host.h"
#include "input.h"
#include "qargs.h"
#include "qendian.h"
#include "quakefs.h"
#include "sbar.h"
#include "va.h"

#define WARP_WIDTH		320
#define WARP_HEIGHT 	200

static qboolean		vid_initialized = false;

static GLXContext	ctx = NULL;

extern void GL_Init_Common (void);
extern void VID_Init8bitPalette (void);
/*-----------------------------------------------------------------------*/

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

void
VID_Shutdown (void)
{
	if (!vid_initialized)
		return;

	Con_Printf ("VID_Shutdown\n");

	x11_restore_vidmode ();
	x11_close_display ();
}

#if 0
static void
signal_handler (int sig)
{
	printf ("Received signal %d, exiting...\n", sig);
	Sys_Quit ();
	exit (sig);
}

static void
InitSig (void)
{
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);
	signal (SIGQUIT, signal_handler);
	signal (SIGILL, signal_handler);
	signal (SIGTRAP, signal_handler);
	signal (SIGIOT, signal_handler);
	signal (SIGBUS, signal_handler);
/*	signal (SIGFPE, signal_handler); */
	signal (SIGSEGV, signal_handler);
	signal (SIGTERM, signal_handler);
}
#endif

/*
===============
GL_Init
===============
*/
void
GL_Init (void)
{
	GL_Init_Common ();
}

void
GL_EndRendering (void)
{
	glFlush ();
	glXSwapBuffers (x_disp, x_win);
	Sbar_Changed ();
}

void
VID_Init (unsigned char *palette)
{
	int i;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};

	VID_GetWindowSize (640, 480);
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *) vid.colormap + 2048));

	/* Interpret command-line params
	 */

	/* Set vid parameters */

	if ((i = COM_CheckParm ("-conwidth")))
		vid.conwidth = atoi(com_argv[i+1]);
	else
		vid.conwidth = scr_width;

	vid.conwidth &= 0xfff8; // make it a multiple of eight
	vid.conwidth = max (vid.conwidth, 320);

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth * 3 / 4;

	if ((i = COM_CheckParm ("-conheight")))	// conheight no smaller than 200px
		vid.conheight = atoi (com_argv[i+1]);
	vid.conheight = max (vid.conheight, 200);

	x11_open_display ();

	x_visinfo = glXChooseVisual (x_disp, x_screen, attrib);
	if (!x_visinfo) {
		fprintf (stderr, "Error couldn't get an RGB, Double-buffered, Depth visual\n");
		exit (1);
	}
	x_vis = x_visinfo->visual;

	x11_set_vidmode (scr_width, scr_height);
	x11_create_window (scr_width, scr_height);
	/* Invisible cursor */
	x11_create_null_cursor ();

	x11_grab_keyboard ();

	XSync (x_disp, 0);

	ctx = glXCreateContext (x_disp, x_visinfo, NULL, True);

	glXMakeCurrent (x_disp, x_win, ctx);

	vid.height = vid.conheight = min (vid.conheight, scr_height);
	vid.width = vid.conwidth = min (vid.conwidth, scr_width);

	vid.aspect = ((float) vid.height / (float) vid.width) * (320.0 / 240.0);
	vid.numpages = 2;

	//InitSig (); // trap evil signals

	GL_Init ();

	//XXX not yet GL_CheckBrightness (palette);
	VID_SetPalette (palette);

	// Check for 8-bit extension and initialize if present
	VID_Init8bitPalette ();

	Con_Printf ("Video mode %dx%d initialized.\n", scr_width, scr_height);

	vid_initialized = true;

	vid.recalc_refdef = 1;		// force a surface cache flush
}

void
VID_Init_Cvars ()
{
	x11_Init_Cvars();
}

void
VID_SetCaption (char *text)
{
	if (text && *text) {
		char *temp = strdup (text);
		x11_set_caption (va ("%s %s: %s", PROGRAM, VERSION, temp));
		free (temp);
	} else {
		x11_set_caption (va ("%s %s", PROGRAM, VERSION));
	}
}

void VID_HandlePause (qboolean paused)
{
}
