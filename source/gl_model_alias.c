/*
	gl_model_alias.c

	model loading and caching

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

// models are the only shared resource between a client and server running
// on the same machine.

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "r_local.h"
#include "sys.h"
#include "console.h"
#include "qendian.h"
#include "checksum.h"
#include "glquake.h"

extern char loadname[];
extern model_t *loadmodel;

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

extern	aliashdr_t	*pheader;

extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];

// a pose is a single set of vertexes.  a frame may be
// an animating sequence of poses
extern	trivertx_t	*poseverts[MAXALIASFRAMES];
extern	int			posenum;

extern	byte		player_8bit_texels[320*200];

/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes - Ed
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

extern unsigned d_8to24table[];

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void Mod_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
}

void *Mod_LoadSkin (byte *skin, int skinsize, int snum, int gnum, qboolean group)
{
	int j;
	char name[32];

	Mod_FloodFillSkin( skin, pheader->mdl.skinwidth, pheader->mdl.skinheight );

	// This block is GL fullbright support for objects...
	{
		int		pixels;
		byte	*ptexel;

		// Check for fullbright pixels..
		pixels = pheader->mdl.skinwidth * pheader->mdl.skinheight;
		ptexel = (byte *)(skin + 1);

		for (j=0 ; j<pixels ; j++) {
			if (ptexel[j] >= 256-32) {
				loadmodel->hasfullbrights = true;
				break;
			}
		}

		if (loadmodel->hasfullbrights) {
			byte	*ptexels;

			//ptexels = Hunk_Alloc(s);
			ptexels = malloc(pixels);

			if (group) {
				snprintf(name, sizeof(name), "fb_%s_%i_%i", loadmodel->name,snum,gnum);
			} else {
				snprintf(name, sizeof(name), "fb_%s_%i", loadmodel->name,snum);
			}
			Con_DPrintf("FB Model ID: '%s'\n", name);
			for (j=0 ; j<pixels ; j++) {
				if (ptexel[j] >= 256-32) {
					ptexels[j] = ptexel[j];
				} else {
					ptexels[j] = 255;
				}
			}
			pheader->gl_fb_texturenum[snum][gnum] =
				GL_LoadTexture (name, pheader->mdl.skinwidth,
						pheader->mdl.skinheight, ptexels, true, true, 1);

			free(ptexels);
		}
	}

	if (group) {
		snprintf(name, sizeof(name), "%s_%i_%i", loadmodel->name,snum,gnum);
	} else {
		snprintf(name, sizeof(name), "%s_%i", loadmodel->name,snum);
	}
	pheader->gl_texturenum[snum][gnum] =
		GL_LoadTexture (name, pheader->mdl.skinwidth, 
						pheader->mdl.skinheight, skin, true, false, 1);
						// alpha param was true for non group skins
	return skin + skinsize;
}

/*
===============
Mod_LoadAllSkins
===============
*/
void *Mod_LoadAllSkins (int numskins, daliasskintype_t *pskintype, int *pskinindex)
{
	int		i, j, k;
	int		skinsize;
	byte	*skin;
	daliasskingroup_t		*pinskingroup;
	int		groupskins;
	daliasskininterval_t	*pinskinintervals;

	skin = (byte *)pskintype;

	if (numskins < 1 || numskins > MAX_SKINS)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	skinsize = pheader->mdl.skinwidth * pheader->mdl.skinheight;

	for (i=0 ; i<numskins ; i++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE) {
			skin+=4;
			skin = Mod_LoadSkin (skin, skinsize, i, 0, false);

			// save 8 bit texels for the player model to remap
			if (!strcmp(loadmodel->name,"progs/player.mdl"))
			{
				if (skinsize > sizeof(player_8bit_texels))
					Sys_Error ("Player skin too large");
				memcpy (player_8bit_texels, skin, skinsize);
			}

			for (j=1; j < 4; j++) {
				pheader->gl_texturenum[i][j] = 
						pheader->gl_texturenum[i][j - 1]; 
				pheader->gl_fb_texturenum[i][j] = 
						pheader->gl_fb_texturenum[i][j - 1]; 
			}
		} else {
			// animating skin group.  yuck.
			Con_Printf("Animating Skin Group, if you get this message please notify warp@debian.org\n");
			pskintype++;
			pinskingroup = (daliasskingroup_t *)pskintype;
			groupskins = LittleLong (pinskingroup->numskins);
			pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);

			pskintype = (void *)(pinskinintervals + groupskins);
			skin = (byte *)pskintype;

			for (j=0 ; j<groupskins ; j++)
			{
				skin+=4;
				skin = Mod_LoadSkin (skin, skinsize, i, j, true);
			}
			k = j;
			for (/* */; j < 4; j++) {
				pheader->gl_texturenum[i][j] = 
						pheader->gl_texturenum[i][j - k]; 
				pheader->gl_fb_texturenum[i][j] = 
						pheader->gl_fb_texturenum[i][j - k]; 
			}
		}
	}

	return (void *)skin;
}
