#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variables.h"

/* fichier de config */
int loadConf(char *filename, struct conf **pcfg); // charge la config à partir du fichier et rempli la liste chainée
char* getValue(struct conf *pcfg, char *section, char *key); // retourne la valeur de la clée pour la section renseignée
void afficheConf(struct conf *pcfg); // affiche la config à partir de la liste chainée

/* gestion des listes chainées */
void ajouterSection(struct conf **pcfg, char *section); // fait un append (a la fin) de la section sur la liste chainée
void ajouterVariable(struct conf **pcfg, char *key, char *value); // fait un append de la paire (clef/value) sur la liste chainée de variable
