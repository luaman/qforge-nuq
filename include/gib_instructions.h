void GIB_AddInstruction (char *name, gib_func_t func);
gib_inst_t *GIB_Find_Instruction (char *name);
void GIB_Init_Instructions (void);
int GIB_Echo_f (void);
int GIB_Call_f (void);
int GIB_VarPrint_f (void);
int GIB_Return_f (void);
