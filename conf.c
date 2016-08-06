#include "header/conf.h"

int loadConf(char *filename, struct conf **pcfg) {
    if (verbose)
        printf("Load config...\n");
    FILE *in;
    char s[L];

    in = fopen(filename, "r");
    if (in == NULL) {
        if (debug)
            printf("Error load ->[%s]", filename);
        //perror(filename);
        return 0;
    }
    //fseek(conffile, 0, SEEK_SET);
    while (fgets(s, L-1, in) != NULL) {
        if (s[0] != '\n') {
            //strtok(s, "\n");
            if (s[strlen(s)-1] == '\n')
                s[strlen(s)-1] = '\0';
            if (strchr(s, '#') != NULL)
                *strchr(s, '#') = '\0';
            // si c'est un debut de section
            if (s[0] == '[') {
                s[strlen(s)-1] = '\0';
                ajouterSection(pcfg, &(s[1])); // &(s[1]) pour ne pas avoir le premier crochet
            }
            // si on est dans les variables de la section
            else {
                char *key, *value;
                strtok(s, "=");
                key = malloc(sizeof(char)*strlen(s)+1);
                strcpy(key, s);

                strcpy(s, strtok(NULL, "="));
                value = malloc(sizeof(char)*strlen(s)+1);
                strcpy(value, s);
                ajouterVariable(pcfg, key, value);
                free(key);
                free(value);
            }
        }
    }
    if (verbose)
        printf("Config loaded.\n");
    return 1;
}

char* getValue(struct conf *pcfg, char *section, char *key) {
    struct conf *cfg = pcfg;
    struct variable *var;
    while (cfg != NULL && strcmp(cfg->section, section) != 0 ){
        cfg = cfg->next;
    }
    var = cfg->variables;
    while (var != NULL && strcmp(var->key, key) != 0 ){
        var = var->next;
    }
    char *szValueOut = malloc(sizeof(char)*strlen(var->value)+1);
    strcpy(szValueOut, var->value);
    return szValueOut;
}

void afficheConf(struct conf *pcfg) {
    struct conf *cfg = pcfg;
    struct variable *var;
    while (cfg != NULL){
        var = cfg->variables;
        printf("\n[%s]\n", cfg->section);
        while (var != NULL) {
            printf("%s->%s\n", var->key, var->value);
            var = var->next;
        }
        cfg = cfg->next;
    }
}

void ajouterSection(struct conf **pcfg, char *section) {
    struct conf *newSection = malloc(sizeof(struct conf));
	newSection->section = malloc(sizeof(char)*strlen(section)+1);
	strcpy(newSection->section, section);
	newSection->variables = NULL;
	newSection->next = NULL;
	if (*pcfg == NULL)
		*pcfg = newSection;
	else{
		struct conf *tmp = *pcfg;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = newSection;
	}
}
void ajouterVariable(struct conf **pcfg, char *key, char *value) {
    struct conf *tmpS = *pcfg;
	while (tmpS->next != NULL){
		tmpS = tmpS->next;
	}
    struct variable *newVar = (struct variable*) malloc(sizeof(struct variable));
    newVar->key = malloc(sizeof(char)*strlen(key)+1);
	strcpy(newVar->key, key);
	newVar->value = malloc(sizeof(char)*strlen(value)+1);
	strcpy(newVar->value, value);
	newVar->next = NULL;
	if (tmpS->variables == NULL)
		tmpS->variables = newVar;
	else{
		struct variable *tmpV = tmpS->variables;
		while(tmpV->next != NULL)
			tmpV = tmpV->next;
		tmpV->next = newVar;
	}
}
