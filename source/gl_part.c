/*
	r_part.c

	(description)

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
# include <config.h>
#endif
#include "qargs.h"
#include "d_iface.h"
#include "bspfile.h"    // needed by: glquake.h
#include "vid.h"
#include "sys.h"
#include "mathlib.h"    // needed by: protocol.h, render.h, client.h,
#include "host.h"
                        //  modelgen.h, glmodel.h
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "menu.h"
#include "net.h"        // needed by: client.h
#include "protocol.h"   // needed by: client.h
#include "cmd.h"
#include "sbar.h"
#include "render.h"     // needed by: client.h, gl_model.h, glquake.h
#include "client.h"     // need cls in this file
#include "model.h"   // needed by: glquake.h
#include "console.h"
#include "glquake.h"
#include "quakefs.h"

#include <stdlib.h>

#define MAX_PARTICLES			2048	// default max # of particles at one
										//  time
#define MAX_FIRES					128	// rocket flames
#define ABSOLUTE_MIN_PARTICLES	512		// no fewer than this no matter what's
										//  on the command line

int		ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
int		ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
int		ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

particle_t	*active_particles, *free_particles;

particle_t	*particles;
int			r_numparticles;

vec3_t			r_pright, r_pup, r_ppn;

fire_t		r_fires[MAX_FIRES];
extern cvar_t	*gl_fires;
extern qboolean lighthalf;

/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i;

	i = COM_CheckParm ("-particles");

	if (i)
	{
		r_numparticles = (int)(atoi(com_argv[i+1]));
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = MAX_PARTICLES;
	}

	particles = (particle_t *)
			Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");
}


/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;
	
	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<r_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}


void R_ReadPointFile_f (void)
{
	FILE	*f;
	vec3_t	org;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_OSPATH];

// FIXME	sprintf (name,"maps/%s.pts", sv.name);

	COM_FOpenFile (name, &f);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}
	
	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ;; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;
		
		if (!free_particles)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		
		p->die = 99999;
		p->color = (-c)&15;
		p->type = pt_static;
		VectorCopy (vec3_origin, p->vel);
		VectorCopy (org, p->org);
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}
	
/*
===============
R_ParticleExplosion

===============
*/
void R_ParticleExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;

	if (!gl_particles->value)
		return;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 5;
		p->color = ramp1[0];
		p->ramp = rand()&3;
		if (i & 1)
		{
			p->type = pt_explode;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_explode2;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}

/*
===============
R_BlobExplosion

===============
*/
void R_BlobExplosion (vec3_t org)
{
	int			i, j;
	particle_t	*p;
	
	if (!gl_particles->value)
		return;

	for (i=0 ; i<1024 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 1 + (rand()&8)*0.05;

		if (i & 1)
		{
			p->type = pt_blob;
			p->color = 66 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
		else
		{
			p->type = pt_blob2;
			p->color = 150 + rand()%6;
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = org[j] + ((rand()%32)-16);
				p->vel[j] = (rand()%512)-256;
			}
		}
	}
}

/*
===============
R_RunParticleEffect

===============
*/
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int			i, j;
	particle_t	*p;
	int			scale;

	if (!gl_particles->value)
		return;

	if (count > 130)
		scale = 3;
	else if (count > 20)
		scale = 2;
	else
		scale = 1;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die = cl.time + 0.1*(rand()%5);
		p->color = (color&~7) + (rand()&7);
		p->type = pt_grav;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + scale*((rand()&15)-8);
			p->vel[j] = dir[j]*15;// + (rand()%300)-150;
		}
	}
}


/*
===============
R_LavaSplash

===============
*/
void R_LavaSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	if (!gl_particles->value)
		return;

	for (i=-16 ; i<16 ; i++)
		for (j=-16 ; j<16 ; j++)
			for (k=0 ; k<1 ; k++)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;
		
				p->die = cl.time + 2 + (rand()&31) * 0.02;
				p->color = 224 + (rand()&7);
				p->type = pt_grav;
				
				dir[0] = j*8 + (rand()&7);
				dir[1] = i*8 + (rand()&7);
				dir[2] = 256;
	
				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand()&63);
	
				VectorNormalize (dir);						
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}

/*
===============
R_TeleportSplash

===============
*/
void R_TeleportSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	if (!gl_particles->value)
		return;

	for (i=-16 ; i<16 ; i+=4)
		for (j=-16 ; j<16 ; j+=4)
			for (k=-24 ; k<32 ; k+=4)
			{
				if (!free_particles)
					return;
				p = free_particles;
				free_particles = p->next;
				p->next = active_particles;
				active_particles = p;
		
				p->die = cl.time + 0.2 + (rand()&7) * 0.02;
				p->color = 7 + (rand()&7);
				p->type = pt_grav;
				
				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;
	
				p->org[0] = org[0] + i + (rand()&3);
				p->org[1] = org[1] + j + (rand()&3);
				p->org[2] = org[2] + k + (rand()&3);
	
				VectorNormalize (dir);						
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->vel);
			}
}

void R_RocketTrail (vec3_t start, vec3_t end, int type, entity_t *ent)
{
	vec3_t	vec;
	float	len;
	int			j;
	particle_t	*p;

	if (type == 0)
		R_AddFire (start, end, ent);

	if (!gl_particles->value)
		return;
	
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	while (len > 0)
	{
		len -= 3;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		
		VectorCopy (vec3_origin, p->vel);
		p->die = cl.time + 2;

		if (type == 4)
		{	// slight blood
			p->type = pt_slowgrav;
			p->color = 67 + (rand()&3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand()%6)-3);
			len -= 3;
		}
		else if (type == 2)
		{	// blood
			p->type = pt_slowgrav;
			p->color = 67 + (rand()&3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand()%6)-3);
		}
		else if (type == 6)
		{	// voor trail
			p->color = 9*16 + 8 + (rand()&3);
			p->type = pt_static;
			p->die = cl.time + 0.3;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand()&15)-8);
		}
		else if (type == 1)
		{	// smoke smoke
			p->ramp = (rand()&3) + 2;
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand()%6)-3);
		}
		else if (type == 0)
		{	// rocket trail
			p->ramp = (rand()&3);
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand()%6)-3);
		}
		else if (type == 3 || type == 5)
		{	// tracer
			static int tracercount;

			p->die = cl.time + 0.5;
			p->type = pt_static;
			if (type == 3)
				p->color = 52 + ((tracercount&4)<<1);
			else
				p->color = 230 + ((tracercount&4)<<1);
			
			tracercount++;

			VectorCopy (start, p->org);
			if (tracercount & 1)
			{
				p->vel[0] = 30*vec[1];
				p->vel[1] = 30*-vec[0];
			}
			else
			{
				p->vel[0] = 30*-vec[1];
				p->vel[1] = 30*vec[0];
			}
			
		}
		

		VectorAdd (start, vec, start);
	}
}


/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
	particle_t		*p, *kill;
	float			grav;
	int				i;
	float			time2, time3;
	float			time1;
	float			dvel;
	float			frametime;
	unsigned char	*at;
	unsigned char	theAlpha;
	vec3_t			up, right;
	float			scale;
	qboolean		alphaTestEnabled;

	glBindTexture (GL_TEXTURE_2D, particletexture);
	// LordHavoc: particles should not affect zbuffer
	glDepthMask(0);
	alphaTestEnabled = glIsEnabled(GL_ALPHA_TEST);

	if (alphaTestEnabled)
		glDisable(GL_ALPHA_TEST);
	glBegin (GL_TRIANGLES);

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	frametime = host_frametime;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * 800 * 0.05;
	dvel = 4*frametime;
	
	for ( ;; ) 
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
			active_particles = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p=active_particles ; p ; p=p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}

		// hack a scale up to keep particles from disapearing
		scale = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1]
			+ (p->org[2] - r_origin[2])*vpn[2];
		if (scale < 20)
			scale = 1;
		else
			scale = 1 + scale * 0.004;
		at = (byte *)&d_8to24table[(int)p->color];
		if (p->type==pt_fire)
			theAlpha = 255*(6-p->ramp)/6;
//			theAlpha = 192;
//		else if (p->type==pt_explode || p->type==pt_explode2)
//			theAlpha = 255*(8-p->ramp)/8;
		else
			theAlpha = 255;
		if (lighthalf)
			glColor4ub((byte) ((int) at[0] >> 1), (byte) ((int) at[1] >> 1), (byte) ((int) at[2] >> 1), theAlpha);
		else
			glColor4ub(at[0], at[1], at[2], theAlpha);
//		glColor3ubv (at);
//		glColor3ubv ((byte *)&d_8to24table[(int)p->color]);
		glTexCoord2f (0,0);
		glVertex3fv (p->org);
		glTexCoord2f (1,0);
		glVertex3f (p->org[0] + up[0]*scale, p->org[1] + up[1]*scale, p->org[2] + up[2]*scale);
		glTexCoord2f (0,1);
		glVertex3f (p->org[0] + right[0]*scale, p->org[1] + right[1]*scale, p->org[2] + right[2]*scale);

		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;
		
		switch (p->type)
		{
		case pt_static:
			break;
		case pt_fire:
			p->ramp += time1;
			if (p->ramp >= 6)
				p->die = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_explode2:
			p->ramp += time3;
			if (p->ramp >=8)
				p->die = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav;
			break;

		case pt_blob:
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_slowgrav:
		case pt_grav:
			p->vel[2] -= grav;
			break;
		}
	}

	glEnd ();
	if (alphaTestEnabled)
		glEnable(GL_ALPHA_TEST);
	glDepthMask(1);
}

/*
	R_AddFire

	Nifty ball of fire GL effect.  Kinda a meshing of the dlight and
	particle engine code.
*/
float r_firecolor_flame[3]={0.9,0.7,0.3};
float r_firecolor_light[3]={0.9,0.7,0.3};

void
R_AddFire (vec3_t start, vec3_t end, entity_t *ent)
{
	float		len;
	fire_t		*f;
	dlight_t	*dl;
	vec3_t		vec;
	int			key;

	if (!gl_fires->value)
		return;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	key = ent-cl_entities+1;

	if (len)
	{
		f = R_AllocFire (key);
		VectorCopy (end, f->origin);
		VectorCopy (start, f->owner);
		f->size = 20;
		f->die = cl.time + 0.5;
		f->decay = -1;
		f->color=r_firecolor_flame;

		dl = CL_AllocDlight (key);
		VectorCopy (end, dl->origin);
		dl->radius = 200;
		dl->die = cl.time + 0.5;
		dl->color=r_firecolor_light;
	}
}

/*
	R_AllocFire

	Clears out and returns a new fireball
*/
fire_t *
R_AllocFire (int key)
{
	int		i;
	fire_t		*f;
	if (key)	// first try to find/reuse a keyed spot
	{
		f = r_fires;
		for (i = 0; i < MAX_FIRES; i++, f++)
			if (f->key == key)
			{
				memset (f, 0, sizeof(*f));
				f->key = key;
				f->color = f->_color;
				return f;
			}
	}

	f = r_fires;	// no match, look for a free spot
	for (i = 0; i < MAX_FIRES; i++, f++)
	{
		if (f->die < cl.time)
		{
			memset (f, 0, sizeof(*f));
			f->key = key;
			f->color = f->_color;
			return f;
		}
	}

	f = &r_fires[0];
	memset (f, 0, sizeof(*f));
	f->key = key;
	f->color = f->_color;
	return f;	
}

/*
	R_DrawFire

	draws one fireball - probably never need to call this directly
*/
void
R_DrawFire (fire_t *f)
{
	int		i, j;
	vec3_t		vec,vec2;
	float		radius;
	float		*b_sin, *b_cos;

	b_sin = bubble_sintable;
	b_cos = bubble_costable;

	radius = f->size + 0.35;

	// figure out if we're inside the area of effect
	VectorSubtract (f->origin, r_origin, vec);
	if (Length (vec) < radius)
	{
		AddLightBlend (1, 0.5, 0, f->size * 0.0003);	// we are
		return;
	}

	// we're not - draw it
	glBegin (GL_TRIANGLE_FAN);
	if (lighthalf)
		glColor3f(f->color[0]*0.5,f->color[1]*0.5,f->color[2]*0.5);
	else
		glColor3fv(f->color);
	for (i=0 ; i<3 ; i++)
		vec[i] = f->origin[i] - vpn[i] * radius;
	glVertex3fv (vec);
	glColor3f (0.0, 0.0, 0.0);

	// don't panic, this just draws a bubble...
	for (i=16 ; i>=0 ; i--)
	{
		for (j=0 ; j<3 ; j++) {
			vec[j] = f->origin[j] + (*b_cos * vright[j]
				+ vup[j]*(*b_sin)) * radius;
			vec2[j] = f->owner[j] + (*b_cos * vright[j]
				+ vup[j]*(*b_sin)) * radius;
		}
		glVertex3fv (vec);
		glVertex3fv (vec2);

		b_sin+=2;
		b_cos+=2;
	}
	glEnd ();
}

/*
	R_UpdateFires

	Draws each fireball in sequence
*/
void
R_UpdateFires (void)
{
	int		i;
	fire_t	*f;

	if (!gl_fires->value)
		return;

	glDepthMask (0);
	glDisable (GL_TEXTURE_2D);
	glShadeModel (GL_SMOOTH);
	glBlendFunc (GL_ONE, GL_ONE);

	f = r_fires;
	for (i = 0; i < MAX_FIRES; i++, f++)
	{
		if (f->die < cl.time || !f->size)
			continue;
		f->size += f->decay;
		R_DrawFire (f);
	}

	glColor3f (1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (1);
}

void
R_FireColor_f (void)
{
	int i;

	if (Cmd_Argc() == 1) {
		Con_Printf ("r_firecolor %f %f %f\n",
					r_firecolor_flame[0],
					r_firecolor_flame[1],
					r_firecolor_flame[2]);
		return;
	}
	if (Cmd_Argc() == 5 || Cmd_Argc() == 6) {
		Con_Printf ("Warning: obsolete 4th and 5th parameters to r_firecolor ignored\n");
	} else if (Cmd_Argc() !=4) {
		Con_Printf ("Usage r_firecolor R G B\n");
		return;
	}
	for (i=0; i<4; i++) {
		r_firecolor_flame[i]=atof(Cmd_Argv(i+1));
		r_firecolor_light[i]=r_firecolor_flame[i];
	}
}
