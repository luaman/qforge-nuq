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

gib_var_t *gib_locals[GIB_MAXCALLS];

gib_var_t *GIB_Var_FindLocal (char *key)
{
	gib_var_t *var;
	if (!(gib_locals[gib_subsp]))
		return 0;
	for (var = gib_locals[gib_subsp]; strcmp(key, var->key); var = var->next)
		if (!(var->next))
			return 0;
	return var;
}
gib_var_t *GIB_Var_FindGlobal (char *key)
{
	gib_var_t *var;
	if (!(gib_currentmod[gib_subsp]->vars))
		return 0;
	for (var = gib_currentmod[gib_subsp]->vars; strcmp(key, var->key); var = var->next)
		if (!(var->next))
			return 0;
	return var;
}


void GIB_Var_Set (char *key, char *value)
{
	gib_var_t *var;
	if ((var = GIB_Var_FindLocal(key)))
	{
		Con_Printf("Value already found.\n");
		free(var->value);
	}
	else
	{
		var = malloc(sizeof(gib_var_t));
		var->key = malloc(strlen(key) + 1);
		strcpy(var->key, key);
		var->next = gib_locals[gib_subsp];
		gib_locals[gib_subsp] = var;
	}
	var->value = malloc(strlen(value) + 1);
	strcpy(var->value, value);
}

void GIB_Var_FreeAll (gib_var_t *var)
{
	gib_var_t *temp;

	for (;var; var = temp)
	{
		temp = var->next;
		free(var->key);
		free(var->value);
		free(var);
	}
}
