/*
	sw_model_alias.c

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

void *Mod_LoadSkin (byte *skin, int skinsize, int *pskinindex, int snum, int gnum)
{
	byte	*pskin;
	ushort	*pusskin;
	int		i;

	pskin = Hunk_AllocName (skinsize * r_pixbytes, loadname);
	*pskinindex = (byte *)pskin - (byte *)pheader;

	switch (r_pixbytes) {
	case 1:
		memcpy (pskin, skin, skinsize);
		break;
	case 2:
		pusskin = (ushort*)skin;
		for (i=0; i<skinsize; i++)
			pusskin[i] = d_8to16table[skin[i]];
		break;
	default:
		Sys_Error ("Mod_LoadAliasSkin: driver set invalid r_pixbytes: %d\n",
					r_pixbytes);
		break;
	}
	return skin + skinsize;
}

/*
===============
Mod_LoadAllSkins
===============
*/
void *Mod_LoadAllSkins (int numskins, daliasskintype_t *pskintype, int *pskinindex)
{
	int		i, j;
	int		skinsize;
	byte	*skin;
	int		groupskins;
	daliasskingroup_t		*pinskingroup;
	daliasskininterval_t	*pinskinintervals;
	maliasskindesc_t		*pskindesc;
	maliasskingroup_t		*paliasskingroup;
	float					*poutskinintervals;

	skin = (byte *)pskintype;

	if (numskins < 1 || numskins > MAX_SKINS)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	skinsize = pheader->mdl.skinwidth * pheader->mdl.skinheight;
	pskindesc = Hunk_AllocName (numskins * sizeof (maliasskindesc_t),
								loadname);
	pheader->skindesc = (byte *)pskindesc - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE) {
			skin+=4;
			skin = Mod_LoadSkin (skin, skinsize, &pskindesc[i].skin, i, 0);
		} else {
			pskintype++;
			pinskingroup = (daliasskingroup_t *)pskintype;
			groupskins = LittleLong (pinskingroup->numskins);

			j = sizeof (maliasskingroup_t) +
					(groupskins - 1) * sizeof (paliasskingroup->skindescs[0]);
			j = (int)&((maliasskingroup_t*)0)->skindescs[groupskins];
			paliasskingroup = Hunk_AllocName (j, loadname);
			paliasskingroup->numskins = groupskins;

			*pskinindex = (byte *)paliasskingroup - (byte *)pheader;

			pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);
			poutskinintervals = Hunk_AllocName (groupskins * sizeof (float),loadname);
			paliasskingroup->intervals = (byte *)poutskinintervals - (byte *)pheader;
			for (i=0 ; i<groupskins ; i++) {
				*poutskinintervals = LittleFloat (pinskinintervals->interval);
				if (*poutskinintervals <= 0)
					Sys_Error ("Mod_LoadAliasSkinGroup: interval<=0");

				poutskinintervals++;
				pinskinintervals++;
			}

			pskintype = (void *)pinskinintervals;
			skin = (byte *)pskintype;

			for (j=0 ; j<groupskins ; j++)
			{
				skin+=4;
				skin = Mod_LoadSkin (skin, skinsize, &paliasskingroup->skindescs[i].skin, i, j);
			}
		}
	}

	return (void *)skin;
}

void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr)
{
}
