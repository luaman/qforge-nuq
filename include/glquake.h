/*
	glquake.h

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

#ifndef __glquake_h
#define __glquake_h

#ifndef __GNUC__
// disable data conversion warnings
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
#endif
  
#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "qtypes.h"
#include "model.h"
#include "d_iface.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);


#ifdef _WIN32
// Function prototypes for the Texture Object Extension routines
typedef GLboolean (APIENTRY *ARETEXRESFUNCPTR)(GLsizei, const GLuint *,
                    const GLboolean *);
typedef void (APIENTRY *BINDTEXFUNCPTR)(GLenum, GLuint);
typedef void (APIENTRY *DELTEXFUNCPTR)(GLsizei, const GLuint *);
typedef void (APIENTRY *GENTEXFUNCPTR)(GLsizei, GLuint *);
typedef GLboolean (APIENTRY *ISTEXFUNCPTR)(GLuint);
typedef void (APIENTRY *PRIORTEXFUNCPTR)(GLsizei, const GLuint *,
                    const GLclampf *);
typedef void (APIENTRY *TEXSUBIMAGEPTR)(int, int, int, int, int, int, int, int, void *);

extern	BINDTEXFUNCPTR bindTexFunc;
extern	DELTEXFUNCPTR delTexFunc;
extern	TEXSUBIMAGEPTR TexSubImage2DFunc;
#endif

extern	int texture_extension_number;
extern	int		texture_mode;
extern int gl_mtex_enum;

extern	float	gldepthmin, gldepthmax;

void GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha);
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel);
int GL_FindTexture (char *identifier);

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t glv;

extern	int glx, gly, glwidth, glheight;

#ifdef _WIN32
extern	PROC glArrayElementEXT;
extern	PROC glColorPointerEXT;
extern	PROC glTexturePointerEXT;
extern	PROC glVertexPointerEXT;
#endif

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle
#define	MAX_LBM_HEIGHT		480

#define	MAX_GLTEXTURES		2048

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01


void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
texture_t *R_TextureAnimation (texture_t *base);

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned			width;
	unsigned			height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;

//====================================================


extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	qboolean	envmap;
extern	int	currenttexture;
extern	int	cnttextures[2];
extern	int	particletexture;
extern	int	netgraphtexture;
extern	int	playertextures;

extern	int	skytexturenum;		// index in cl.loadmodel, not gl texture object

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_drawviewmodel;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_waterwarp;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_lightmap;
extern	cvar_t	*r_shadows;
extern	cvar_t	*r_mirroralpha;
extern	cvar_t	*r_wateralpha;
extern	cvar_t	*r_waterripple;
extern	cvar_t	*r_dynamic;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_netgraph;

extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_poly;
extern	cvar_t	*gl_texsort;
extern	cvar_t	*gl_smoothmodels;
extern	cvar_t	*gl_affinemodels;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl_keeptjunctions;
extern	cvar_t	*gl_reporttjunctions;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_nocolors;
extern	cvar_t	*gl_doubleeyes;

extern	cvar_t	*gl_ztrick;
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_subdivide_size;
extern	cvar_t	*gl_particles;
extern	cvar_t	*gl_fires;
extern	cvar_t	*gl_fb_models;
extern	cvar_t	*gl_fb_bmodels;

extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;

extern	cvar_t	*gl_max_size;
extern	cvar_t	*gl_playermip;

extern	cvar_t	*r_skyname;
extern	cvar_t	*gl_skymultipass;

extern	int			mirrortexturenum;	// quake texturenum, not gltexturenum
extern	qboolean	mirror;
extern	qboolean	lighthalf;
extern	mplane_t	*mirror_plane;

extern	float	r_world_matrix[16];

extern float bubble_sintable[], bubble_costable[];

extern	const char *gl_vendor;
extern	const char *gl_renderer;
extern	const char *gl_version;
extern	const char *gl_extensions;

void R_TranslatePlayerSkin (int playernum);
void GL_Bind (int texnum);

// Multitexture
#define    TEXTURE0_SGIS				0x835E
#define    TEXTURE1_SGIS				0x835F

#ifndef _WIN32
#define APIENTRY /* */
#endif

typedef void (APIENTRY *lpMTexFUNC) (GLenum, GLfloat, GLfloat);
typedef void (APIENTRY *lpSelTexFUNC) (GLenum);
extern lpMTexFUNC qglMTexCoord2fSGIS;
extern lpSelTexFUNC qglSelectTextureSGIS;
extern lpMTexFUNC qglMTexCoord2f;
extern lpSelTexFUNC qglSelectTexture;

extern qboolean gl_mtexable;

void GL_SubdivideSurface (msurface_t *fa);

void GL_DisableMultitexture(void);
void GL_EnableMultitexture(void);
void GL_BuildLightmaps (void);
void GL_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha) ;
void GL_Set2D (void);
void GL_CheckGamma (unsigned char *pal);

void EmitWaterPolys (msurface_t *fa);
void EmitSkyPolys (msurface_t *fa);
void EmitBothSkyLayers (msurface_t *fa);
void R_DrawSkyChain (msurface_t *s);
void R_LoadSkys (char *);
void R_DrawSky (void);

void R_RotateForEntity (entity_t *e);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);

void AddLightBlend (float, float, float, float);

typedef struct {
	int		key;			// allows reusability
	vec3_t	origin, owner;
	float	size;
	float	die, decay;		// duration settings
	float	minlight;		// lighting threshold
	float	_color[3];		// RGBA
	float	*color;
} fire_t;

void R_AddFire (vec3_t, vec3_t, entity_t *ent);
fire_t *R_AllocFire (int);
void R_DrawFire (fire_t *);
void R_UpdateFires (void);


#endif // __glquake_h
