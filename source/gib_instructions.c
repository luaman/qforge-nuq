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
#include "gib_error.h"

static gib_inst_t *gibinstructions;

char *gib_subret;

void GIB_AddInstruction (char *name, gib_func_t func)
{
	gib_inst_t *new;
	new = malloc(sizeof(gib_inst_t));
	new->name = malloc(strlen(name) + 1);
	new->func = func;
	strcpy(new->name, name);
	new->next = gibinstructions;
	gibinstructions = new;
}

gib_inst_t *GIB_Find_Instruction (char *name)
{
	gib_inst_t *inst;
	if (!(gibinstructions))
		return 0;
	for (inst = gibinstructions; strcmp(inst->name, name); inst = inst->next)
		if (!(inst->next))
			return 0;
	return inst;
}

void GIB_Init_Instructions (void)
{
	GIB_AddInstruction("echo", GIB_Echo_f);
	GIB_AddInstruction("call", GIB_Call_f);
	GIB_AddInstruction("varprint", GIB_VarPrint_f);
	GIB_AddInstruction("return", GIB_Return_f);
}

int GIB_Echo_f (void)
{
	Con_Printf("%s\n",GIB_Argv(1));
	return 0;
}

int GIB_Call_f (void)
{
	gib_module_t *mod;
	gib_sub_t *sub;
	int i, ret;

	mod = GIB_Get_ModSub_Mod (GIB_Argv(1));
	if (!mod)
		return GIB_E_NOSUB;
	sub = GIB_Get_ModSub_Sub (GIB_Argv(1));
	if (!sub)
		return GIB_E_NOSUB;
	GIB_SUBARGC = GIB_Argc() - 1;
	GIB_SUBARGV[0] = sub->name;
	for (i = 1; i <= GIB_SUBARGC; i++)
		GIB_SUBARGV[i] = GIB_Argv(i + 1);
	ret = GIB_Run_Sub (mod, sub);
	if (gib_subret)
		GIB_Var_Set("retval", gib_subret);
	gib_subret = 0;
	return ret;
}

int GIB_VarPrint_f (void)
{
	gib_var_t *var;
	int i;
	for (i = 1; i <= GIB_Argc(); i++)
	{
		var = GIB_Var_FindLocal(GIB_Argv(i));
		if (!var)
			return GIB_E_NOVAR;
		Con_Printf("%s", var->value);
	}
	Con_Printf ("\n");
	return 0;
}

int GIB_Return_f (void)
{
	if (GIB_Argc() != 1)
		return GIB_E_NUMARGS;
	gib_subret = malloc(strlen(GIB_Argv(1)) + 1);
	strcpy(gib_subret, GIB_Argv(1));
	return GIB_E_RETURN; // Signal to block executor to return immediately
}
