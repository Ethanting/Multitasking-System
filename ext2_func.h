/******************** func.h file ***************************/
#ifndef EXT2_FUNC_H
#define EXT2_FUNC_H
#include"type.h"

int chdir(char *pathname);

int rpwd(MINODE *wd);

void pwd(MINODE *wd);

int ls();

int kmkdir(MINODE *pmip,char *basename,int pino);

int mkdir_d(char *pathname);

int kcreate(MINODE *pmip,char *basename,int pino);

int create(char *pathname);

int rmdir(char *pathname);

int rm(char *pathname);

int quit();

int ext2_menu();
#endif //FUNC_H
