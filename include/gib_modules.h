void GIB_Module_Load (char *name, FILE *f);
gib_module_t *GIB_Create_Module (char *name);
gib_sub_t *GIB_Create_Sub (gib_module_t *mod, char *name);
void GIB_Read_Sub (gib_sub_t *sub, FILE *f);
gib_module_t *GIB_Find_Module (char *name);
gib_sub_t *GIB_Find_Sub (gib_module_t *mod, char *name);
void GIB_Stats_f (void);
