#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L 60

extern int diff, verbose, friendly, debug;

/* nom, owner, group, droit, taille, dateModif, hash */
enum { FILE_NAME = 0, FILE_OWNER, FILE_GROUP, FILE_RIGHT, FILE_SIZE, FILE_DMODIF, FILE_HASH }; 

struct variable
{
    char *key, *value;
    struct variable *next;
};
struct conf
{
    char *section;
    struct variable *variables;
    struct conf *next;
};

struct Element
{
	char *line;
	struct Element * suivant;
};typedef struct Element * File;
