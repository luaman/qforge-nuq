#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
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

gib_module_t *gib_currentmod[GIB_MAXCALLS];
gib_sub_t *gib_currentsub[GIB_MAXCALLS];
int gib_subsp = 0;

char *GIB_SUBARGV[GIB_MAXSUBARGS];
int GIB_SUBARGC;

int gib_argc[GIB_MAXCALLS];
char *gib_argv[GIB_MAXCALLS][80];

int gib_argofs = 0;

char errorline[1024];

char *GIB_Argv(int i)
{
	return gib_argv[gib_subsp + gib_argofs][i];
}

int GIB_Argc(void)
{
	return gib_argc[gib_subsp + gib_argofs];
}

void GIB_Strip_Arg (char *arg)
{
	if (arg[0] == '{' || arg[0] == '\'' || arg[0] == '\"')
	{
		arg[strlen(arg) - 1] = 0;
		memmove(arg, arg + 1, strlen(arg));
	}
}

int GIB_Execute_Block (char *block, int retflag)
{
	int len, i, ret;
	char *code;
	
	i = 0;

	while ((len = GIB_Get_Inst(block + i)) > 0)
	{
		code = malloc(len + 1);
		strncpy(code, block + i, len);
		code[len] = 0;
		if ((ret = GIB_Execute_Inst(code)))
		{
			if (retflag && ret == GIB_E_RETURN)
				return 0;
			else
			{
				strcpy(errorline, code);
				free (code);
				return ret;
			}
		}
		free (code);
		i += len + 1;
	}
	return 0;
}

int GIB_Execute_Inst (char *inst)
{
	char *buffer;
	char *buffer2;
	char *buffer3;
	int i, n, len, ret;
	gib_inst_t *ginst;


	buffer = malloc(strlen(inst) + 1);
	i = 0;

	while (isspace(inst[i]))
		i++;
	
	for (n = 0; i <= strlen(inst); i++)
	{
		if (inst[i] == '\n' || inst[i] == '\t')
			buffer[n] = ' ';
		else
			buffer[n] = inst[i];
		n++;
	}


	buffer2 = malloc(2048);
	buffer3 = malloc(2048);
	GIB_ExpandVars (buffer, buffer2, 2048);
	GIB_ExpandBackticks (buffer2, buffer3, 2048);
	
	gib_argc[gib_subsp + gib_argofs] = 0;
	for (i = 0; buffer3[i] != ' '; i++);
	gib_argv[gib_subsp + gib_argofs][0] = malloc(i + 1);
	strncpy(gib_argv[gib_subsp + gib_argofs][0], buffer3, i);
	gib_argv[gib_subsp + gib_argofs][0][i] = 0;
	for (n = 0;;n++)
	{
		for (;isspace(buffer3[i]); i++);
		if (buffer3[i] == 0)
			break;
		if ((len = GIB_Get_Arg(buffer3 + i)) < 0) // Parse error
			return GIB_E_PARSE;
		else
		{
			gib_argv[gib_subsp + gib_argofs][n + 1] = malloc(len + 1);
			strncpy(gib_argv[gib_subsp + gib_argofs][n + 1], buffer3 + i, len);
			gib_argv[gib_subsp + gib_argofs][n + 1][len] = 0;
			GIB_ExpandEscapes (gib_argv[gib_subsp + gib_argofs][n + 1]);
			i += len;
		}
	}
	gib_argc[gib_subsp + gib_argofs] = n;
	
	free(buffer);
	free(buffer2);
	free(buffer3);
	
	for (i = 1; i <= n; i++)
		GIB_Strip_Arg (gib_argv[gib_subsp + gib_argofs][i]);
	if (!(ginst = GIB_Find_Instruction(gib_argv[gib_subsp + gib_argofs][0])))
		return 1;	
	ret = ginst->func ();

	for (i = 0; i <= n; i++)
		free(gib_argv[gib_subsp + gib_argofs][i]);
	return ret;
}

int GIB_Run_Sub (gib_module_t *mod, gib_sub_t *sub)
{
	int ret, i;
	char buf[256];
	if (++gib_subsp >= GIB_MAXCALLS)
		return 3;
	gib_currentmod[gib_subsp] = mod;
	gib_currentsub[gib_subsp] = sub;
	gib_locals[gib_subsp] = 0;

	for (i = 0; i <= GIB_SUBARGC; i++)
	{
		sprintf(buf, "arg%i", i);
		GIB_Var_Set (buf, GIB_SUBARGV[i]);
	}

	ret = GIB_Execute_Block(sub->code, 1);
	if (gib_locals[gib_subsp])
		GIB_Var_FreeAll(gib_locals[gib_subsp]);
	gib_subsp--;
	return ret;
}