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

void *Mod_LoadSkin (byte *skin, int skinsize, int *pskinindex, int snum, int gnum)
{
	byte			*pskin;
	unsigned short	*pusskin;
	int				i;

	pskin = Hunk_AllocName (skinsize * r_pixbytes, loadname);
	*pskinindex = (byte *)pskin - (byte *)pheader;

	switch (r_pixbytes) {
	case 1:
		memcpy (pskin, skin, skinsize);
		break;
	case 2:
		pusskin = (unsigned short*)skin;
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
	int		snum, gnum, t;
	int		skinsize;
	byte	*skin;
	int		groupskins;
	daliasskingroup_t		*pinskingroup;
	daliasskininterval_t	*pinskinintervals;
	maliasskindesc_t		*pskindesc;
	maliasskingroup_t		*paliasskingroup;
	float					*poutskinintervals;

	if (numskins < 1 || numskins > MAX_SKINS)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	skinsize = pheader->mdl.skinwidth * pheader->mdl.skinheight;
	pskindesc = Hunk_AllocName (numskins * sizeof (maliasskindesc_t),
								loadname);
	pheader->skindesc = (byte *)pskindesc - (byte *)pheader;

	for (snum=0 ; snum<numskins ; snum++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE) {
			skin = (byte*)(pskintype+1);
			skin = Mod_LoadSkin (skin, skinsize, &pskindesc[snum].skin, snum, 0);
		} else {
			pskintype++;
			pinskingroup = (daliasskingroup_t *)pskintype;
			groupskins = LittleLong (pinskingroup->numskins);

			t = (int)&((maliasskingroup_t*)0)->skindescs[groupskins];
			paliasskingroup = Hunk_AllocName (t, loadname);
			paliasskingroup->numskins = groupskins;

			*pskinindex = (byte *)paliasskingroup - (byte *)pheader;

			pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);
			poutskinintervals = Hunk_AllocName (groupskins * sizeof (float),loadname);
			paliasskingroup->intervals = (byte *)poutskinintervals - (byte *)pheader;
			for (gnum=0 ; gnum<groupskins ; gnum++) {
				*poutskinintervals = LittleFloat (pinskinintervals->interval);
				if (*poutskinintervals <= 0)
					Sys_Error ("Mod_LoadAliasSkinGroup: interval<=0");

				poutskinintervals++;
				pinskinintervals++;
			}

			pskintype = (void *)pinskinintervals;
			skin = (byte *)pskintype;

			for (gnum=0 ; gnum<groupskins ; gnum++)
			{
				skin = Mod_LoadSkin (skin, skinsize, &paliasskingroup->skindescs[snum].skin, snum, gnum);
			}
		}
		pskintype = (daliasskintype_t*)skin;
	}

	return pskintype;
}

void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr)
{
	int i, j;
	stvert_t *pstverts;
	mtriangle_t *ptri;
	int numv = hdr->mdl.numverts;
	int numt = hdr->mdl.numtris;

	pstverts = (stvert_t *)Hunk_AllocName(numv * sizeof(stvert_t),loadname);
	ptri = (mtriangle_t *)Hunk_AllocName(numt * sizeof(mtriangle_t),loadname);

	hdr->stverts = (byte *)pstverts - (byte *)hdr;
	hdr->triangles = (byte *)ptri - (byte *)hdr;

	for (i=0; i<numv; i++) {
		pstverts[i].onseam = stverts[i].onseam;
		pstverts[i].s = stverts[i].s << 16;
		pstverts[i].t = stverts[i].t << 16;
	}

	for (i=0; i<numt; i++) {
		ptri[i].facesfront = triangles[i].facesfront;
		for (j=0; j<3; j++) {
			ptri[i].vertindex[j] = triangles[i].vertindex[j];
		}
	}
}

/*
=================
Mod_LoadAliasFrame
=================
*/
void * Mod_LoadAliasFrame (void * pin, maliasframedesc_t *frame)
{
	trivertx_t		*pframe, *pinframe;
	int				i, j;
	daliasframe_t	*pdaliasframe;

	pdaliasframe = (daliasframe_t *)pin;

	strcpy (frame->name, pdaliasframe->name);

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about
	// endianness
		frame->bboxmin.v[i] = pdaliasframe->bboxmin.v[i];
		frame->bboxmax.v[i] = pdaliasframe->bboxmax.v[i];
	}

	pinframe = (trivertx_t *)(pdaliasframe + 1);
	pframe = Hunk_AllocName (pheader->mdl.numverts * sizeof(*pframe), loadname);

	frame->frame = (byte *)pframe - (byte *)pheader;

	for (j=0 ; j<pheader->mdl.numverts ; j++)
	{
		int		k;

	// these are all byte values, so no need to deal with endianness
		pframe[j].lightnormalindex = pinframe[j].lightnormalindex;

		for (k=0 ; k<3 ; k++)
		{
			pframe[j].v[k] = pinframe[j].v[k];
		}
	}

	pinframe += pheader->mdl.numverts;

	return (void *)pinframe;
}

/*
=================
Mod_LoadAliasGroup
=================
*/
void * Mod_LoadAliasGroup (void * pin, maliasframedesc_t *frame)
{
	daliasgroup_t		*pingroup;
	maliasgroup_t		*paliasgroup;
	int					i, numframes;
	daliasinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;

	pingroup = (daliasgroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	paliasgroup = Hunk_AllocName (sizeof (maliasgroup_t) +
			(numframes - 1) * sizeof (paliasgroup->frames[0]), loadname);

	paliasgroup->numframes = numframes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		frame->bboxmin.v[i] = pingroup->bboxmin.v[i];
		frame->bboxmax.v[i] = pingroup->bboxmax.v[i];
	}

	frame->frame = (byte *)paliasgroup - (byte *)pheader;

	pin_intervals = (daliasinterval_t *)(pingroup + 1);

	poutintervals = Hunk_AllocName (numframes * sizeof (float), loadname);

	paliasgroup->intervals = (byte *)poutintervals - (byte *)pheader;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0)
			Sys_Error ("Mod_LoadAliasGroup: interval<=0");

		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
		ptemp = Mod_LoadAliasFrame (ptemp, (maliasframedesc_t*)&paliasgroup->frames[i]);
	}

	return ptemp;
}
