#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include "cvar.h"
#include "console.h"
#include "qargs.h"
#include "cmd.h"
#include "zone.h"
#include "quakefs.h"
#include "gib.h"
#include "gib_instructions.h"
#include "gib_interpret.h"
#include "gib_modules.h"
#include "gib_parse.h"
#include "gib_vars.h"



//static char *gib_args;

// Standard cvars

void GIB_Init (void)
{
	Cmd_AddCommand("gibload", GIB_Load_f);
	Cmd_AddCommand("gibstats", GIB_Stats_f);
	Cmd_AddCommand("gib", GIB_Gib_f);
	GIB_Init_Instructions ();
}

void GIB_Gib_f (void)
{
	gib_sub_t *sub;
	gib_module_t *mod;
	int i, ret;
	if (!(mod = GIB_Get_ModSub_Mod(Cmd_Argv(1))))
	{
		Con_Printf("Module not found!\n");
		return;
	}
	if (!(sub = GIB_Get_ModSub_Sub(Cmd_Argv(1))))
		Con_Printf("Subroutine not found!\n");
	else
	{
	GIB_SUBARGC = Cmd_Argc() - 1;
	GIB_SUBARGV[0] = sub->name;
	for (i = 1; i <= GIB_SUBARGC; i++)
		GIB_SUBARGV[i] = Cmd_Argv(i + 1);
	ret = GIB_Run_Sub(mod, sub);
	if (ret != 0)
		Con_Printf("Error in execution of %s!\nError code: %i\n\nLine at fault: %s\n", Cmd_Argv(1), ret, errorline);
	}
}

void GIB_Load_f (void)
{
	char filename[256];
	FILE *f;

	sprintf(filename, "%s/%s.gib", com_gamedir, Cmd_Argv(1));
	f = fopen(filename, "r");
	if (f)
	{
		GIB_Module_Load(Cmd_Argv(1), f);
		fclose(f);
	}
	else
		Con_Printf("gibload: File not found.\n");
}









