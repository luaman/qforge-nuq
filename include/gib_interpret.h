#define GIB_MAXCALLS 2048
#define GIB_MAXSUBARGS 256

extern gib_module_t *gib_currentmod[GIB_MAXCALLS];
extern gib_sub_t *gib_currentsub[GIB_MAXCALLS];
extern int gib_subsp;

extern int gib_argc[GIB_MAXCALLS];
extern char *gib_argv[GIB_MAXCALLS][80];

extern int gib_argofs;

extern char *GIB_SUBARGV[GIB_MAXSUBARGS];
extern int GIB_SUBARGC;

extern char errorline[1024];

char *GIB_Argv(int i);
int GIB_Argc(void);
void GIB_Strip_Arg (char *arg);
int GIB_Execute_Block (char *block, int retflag);
int GIB_Execute_Inst (char *inst);
int GIB_Run_Sub (gib_module_t *mod, gib_sub_t *sub);
