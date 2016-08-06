#include "header/dfs.h"

void usage() {
	const char *progname = "dfs";

	printf("usage: %s [-OPTION...]\n"
		"\t--diff\t\t Compare current and save state and print difference\n"
		"\t--update\t Save current state\n"
		"\t--list\t\t List save state\n"
		"\t--friendly\t human readable\n"
		"\t--verbose\t Verbose mode\n"
		"\t--debug\t\t Show dev info\n"
		"\t-c\t\t Specify an alternative configuration file\n"
		"\tex: %s --diff --update\n\n"
        "\tDefault config file:\n\n"
        "\t[general]\n\tdbdir=path to save db csv file (absolute)\n"
        "\t[path to check (absolute)]\n\tattrib=1 or 0\t#check owner, group and right\n"
        "\tcontent=1 or 0\t#check size, date and content\n"
        "\trecurse=1 or 0\t#check subdirectory\n",
		progname, progname);
    exit(0);
}
char *getHashOfFile(char *filename) {
    /* Renvoie le hash MD5 du fichier */
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        char *error = malloc(sizeof(char)*strlen("hashError"));
        strcpy(error, "hashError");
        //printf ("%s can't be opened.\n", filename);
        return error;
    }
    char *szOut = malloc(sizeof(char)*33);
    MD5_Init(&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final(c, &mdContext);
    //for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) snprintf(&(szOut[i*2]), 16*2, "%02x", c[i]);
    //printf ("\n", filename);
    fclose (inFile);
    return szOut;
}
char *getModeProtection(mode_t protection) {
    char *fDroit = malloc(sizeof(char)*10+1);
    strcpy(fDroit, ((S_ISDIR(protection)) ? "d" : ((S_ISLNK(protection)) ? "l" : "-")));
    strcat(fDroit, ((protection & S_IRUSR) ? "r" : "-"));
    strcat(fDroit, ((protection & S_IWUSR) ? "w" : "-"));
    strcat(fDroit, ((protection & S_IXUSR) ? "x" : "-"));
    strcat(fDroit, ((protection & S_IRGRP) ? "r" : "-"));
    strcat(fDroit, ((protection & S_IWGRP) ? "w" : "-"));
    strcat(fDroit, ((protection & S_IXGRP) ? "x" : "-"));
    strcat(fDroit, ((protection & S_IROTH) ? "r" : "-"));
    strcat(fDroit, ((protection & S_IWOTH) ? "w" : "-"));
    strcat(fDroit, ((protection & S_IXOTH) ? "x" : "-"));
    return fDroit;
}

void showHumanDateModifFromStatStruct(char *date) {
    time_t idate = (time_t) atoi(date);
    char time_buf[25];
    strftime(time_buf, sizeof(time_buf), "%d-%b-%Y %H:%M", gmtime(&idate));
    printf("%s", time_buf);
}

void showOwnerNameFromUid(char *owner) {
    int uid = atoi(owner);
    printf("%s", getpwuid(uid)->pw_name);
}

void showGroupNameFromGid(char *group) {
    int gid = atoi(group);
    printf("%s", getgrgid(gid)->gr_name);
}
void showHumanFileSize(char *filesize) {
    double fsize = strtod(filesize, NULL);
    int i = 0;
    /* Octet, Kilo, Mega, Giga, Tera, Peta, Exa, Zetta, Yotta */
    const char units[] = {'B', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
    while (fsize > 1024) {
        fsize /= 1024;
        i++;
    }
    printf("%.*f%c",i , fsize, units[i]);
}

void diffDB(struct conf *pcfg) {
    if (verbose)
        printf("Check difference...\n");
    struct conf *cfg = pcfg;
    char *recurse;
    char *attrib;
    char *content;
    while (cfg != NULL) {
        if (strcmp("general", cfg->section) != 0){
            recurse = getValue(cfg, cfg->section, "recurse");
            attrib = getValue(cfg, cfg->section, "attrib");
            content = getValue(cfg, cfg->section, "content");
            verifDiff(cfg->section, atoi(recurse), atoi(attrib), atoi(content));
            free(recurse);
            free(attrib);
            free(content);
        }
        cfg = cfg->next;
    }
    if (verbose)
        printf("Difference checked!\n");
}
void verifDiff(char *path, int recurse, int attrib, int content) {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(path)) == NULL) {
        if (debug || verbose) printf("cannot open directory: %s\n", path);
        return;
    }
    chdir(path);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* C'est un dossier donc on rentre dedans */
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            /* On entre dans le dossier suivant en relatif grace à chdir(path) */
            if (recurse)
                verifDiff(entry->d_name, 1, attrib, content);
        }
        else {
            /* c'est un fichier donc on récupère les infos et envoie dans fichier csv de sauvegarde */
            /* nom, owner, group, droit, taille, dateModif, hash */
            char strLineToVerif[512];
            char *md5Hash = getHashOfFile(entry->d_name);
            char *droit = getModeProtection(statbuf.st_mode);
            if (attrib && content) // on envoie la totale
                sprintf(strLineToVerif, "%s;%d;%d;%s;%d;%d;%s\n", entry->d_name, (int) statbuf.st_uid, (int) statbuf.st_gid, droit, (int) statbuf.st_size, (int) statbuf.st_mtime, md5Hash);
            else if (attrib && !content) // on envoie la première partie
                sprintf(strLineToVerif, "%s;%d;%d;%s", entry->d_name, (int) statbuf.st_uid, (int) statbuf.st_gid, droit);
            else if (!attrib && content) // on envoie la seconde partie
                sprintf(strLineToVerif, "%d;%d;%s", (int) statbuf.st_size, (int) statbuf.st_mtime, md5Hash);
            int find = findLine(fCsvFile, strLineToVerif, attrib, content);
            sprintf(strLineToVerif, "%s;%d;%d;%s;%d;%d;%s", entry->d_name, (int) statbuf.st_uid, (int) statbuf.st_gid, droit, (int) statbuf.st_size, (int) statbuf.st_mtime, md5Hash);
            free(md5Hash);
            free(droit);
            if (find == 0) // si pas trouvé, on affiche strLineToVerif parce que ça à changé
                printf("CHANGE -> %s\n", strLineToVerif);
            else if (find == -1)
                return;
        }
    }
    chdir("..");
    closedir(dp);
}

void updateDB(struct conf *pcfg) { // --update
    if (verbose)
        printf("update csv database...\n");
    int section = 1;
    char formatFile[25] = "save%d.csv";
    char saveFile[25];
    sprintf(saveFile, formatFile, section);
    char *dbdir = getValue(pcfg, "general", "dbdir");
    if (chdir(dbdir) == -1){
        if (verbose || debug)
            printf("mkdir(%s)...\n", dbdir);
        mkdir(dbdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        chdir(dbdir);
    }
    FILE *f;
    struct conf *cfg = pcfg;
    char *recurse;
    while (cfg != NULL) {
        if (strcmp("general", cfg->section) != 0){
            f = fopen(saveFile, "w");
            if (f == NULL)
                return;
            fprintf(f, "[%s]\n", cfg->section);
            recurse = getValue(cfg, cfg->section, "recurse");
            updateFile(cfg->section, f, atoi(recurse));
            free(recurse);
            fclose(f);
            section++;
            sprintf(saveFile, formatFile, section);
            chdir(dbdir);
        }
        cfg = cfg->next;
    }
    free(dbdir);
    if (verbose)
        printf("csv database updated.\n");
}
void updateFile(char *path, FILE *saveFile, int recurse) {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(path)) == NULL) {
        if (debug || verbose) printf("cannot open directory: %s\n", path);
        return;
    }
    chdir(path);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* C'est un dossier donc on rentre dedans */
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            /* On entre dans le dossier suivant en relatif grace à chdir(path) */
            if (recurse)
                updateFile(entry->d_name, saveFile, 1);
        }
        else {
            /* c'est un fichier donc on récupère les infos et envoie dans fichier csv de sauvegarde */
            /* On sauvegarde tout meme si attrib ou content est à 0, on ignorera dans le --diff */
            /* ordre -> nom, owner, group, droit, taille, dateModif, hash */
            char *md5Hash = getHashOfFile(entry->d_name);
            char *droit = getModeProtection(statbuf.st_mode);
            fprintf(saveFile, "%s;%d;%d;%s;%d;%d;%s\n", entry->d_name, (int) statbuf.st_uid, (int) statbuf.st_gid, droit, (int) statbuf.st_size, (int) statbuf.st_mtime, md5Hash);
            free(md5Hash);
            free(droit);
        }
    }
    chdir("..");
    closedir(dp);
}

void listeDB(struct conf *pcfg)
{
    struct conf *cfg = pcfg;
    int section = 1;
    char formatFile[25] = "save%d.csv";
    char csvFile[25];
    sprintf(csvFile, formatFile, section);
    char *dbdir = getValue(pcfg, "general", "dbdir");
    if (chdir(dbdir) == -1){
        if (debug)
            printf("chdir(%s) FAIL\n", dbdir);
    }
    if (verbose)
        printf("Database: %s\n", dbdir);
    while (cfg->next != NULL){
        read_file(csvFile);
        cfg = cfg->next;
        section++;
        sprintf(csvFile, formatFile, section);
    }
    free(dbdir);
}
void read_file(char const *saveFile) { // --list
    if (verbose)
        printf("Read file %s...\n", saveFile);
    char s[512];
    FILE *f = fopen(saveFile, "r");
    if (f == NULL) {
        if (verbose)
            printf("Can't read file %s\n", saveFile);
        return;
    }
    char *tok;
    int indice = FILE_NAME;
    while (fgets(s, 512-1, f) != NULL) {
        indice = FILE_NAME;
        tok = strtok(s, ";");
        while (tok != NULL) {
            if (friendly){
                switch (indice){
                    case FILE_DMODIF:
                        showHumanDateModifFromStatStruct(tok);
                        break;
                    case FILE_OWNER:
                        showOwnerNameFromUid(tok);
                        break;
                    case FILE_GROUP:
                        showGroupNameFromGid(tok);
                        break;
                    case FILE_SIZE:
                        showHumanFileSize(tok);
                        break;
                    default:
                        printf("%s", tok);
                        break;
                }
                tok = strtok(NULL, ";");
            }
            else {
                printf("%s", tok);
                tok = strtok(NULL, ";");
            }
            if (tok != NULL)
                printf(";");
            indice++;
        }
    }
    free(tok);
    fclose(f);
}

void remplirFile(struct conf *pcfg) {
    struct conf *cfg = pcfg;
    int section = 1;
    char formatFile[25] = "save%d.csv";
    char csvFile[25];
    sprintf(csvFile, formatFile, section);
    char *dbdir = getValue(pcfg, "general", "dbdir");
    if (chdir(dbdir) == -1){
        if (debug)
            printf("chdir(%s) FAIL\n", dbdir);
    }
    while (cfg->next != NULL){
        char s[512];
        FILE *f = fopen(csvFile, "r");
        if (f == NULL)
            return;
        while (fgets(s, 512-1, f) != NULL) {
            enfiler(&fCsvFile, s);
        }
        fclose(f);
        cfg = cfg->next;
        section++;
        sprintf(csvFile, formatFile, section);
    }
    free(dbdir);
}
void enfiler(File *mafile, char *line) {
	File tmp;
	tmp = malloc(sizeof(struct Element));
	tmp->line = malloc(sizeof(char)*strlen(line)+1);
	strcpy(tmp->line, line);
	tmp->suivant = *mafile;
	*mafile=tmp;
}
File defiler(File *mafile) {
	File tmp, tmp2;
	if (*mafile == NULL)
		return NULL;
	else {
		tmp = *mafile;
		if ((*mafile)->suivant == NULL){
			(*mafile) = NULL;
			return tmp;
		}
		else {
			while(tmp->suivant->suivant != NULL)
				tmp = tmp->suivant;
			tmp2 = tmp->suivant;
			tmp->suivant = NULL;
			return tmp2;
		}
	}
}
void afficher(File f) {
	while(f!=NULL) {
        printf("%s",f->line);
        f=f->suivant;
    }
	printf("\n");
}
void detruire(File *f) {
	while(*f != NULL) {
        File sup = defiler(&(*f));
        free(sup->line);
		free(sup);
    }
}
int findLine(File f, char *line, int attrib, int content) {
    int find = 0;
    if (f==NULL){
        if (debug) printf("fCsvFile is NULL\n");
        if (verbose) printf("DB File note found! --diff is not possible\n");
        return -1;
    }
    /* Il y a 3 condition mais les deux dernieres sont pour la clarté,
    ce qu'il faut vérifier est géré par la fonction appelante verifDiff() */
    if (attrib && content) { // on verifie tout
        while(f->suivant!=NULL && find == 0) {
            if (strcmp(f->line, line) == 0)
                find = 1;
            f=f->suivant;
        }
    }
    else if (attrib && !content) { // on ne verifie que OWNER, GROUP, RIGHT
        while(f->suivant!=NULL && find == 0) {
            if (strstr(f->line, line) != NULL)
                find = 1;
            f=f->suivant;
        }
    }
    else if (!attrib && content) { // one ne verifie que SIZE, DMODIF, HASH
        while(f->suivant!=NULL && find == 0) {
            if (strstr(f->line, line) != NULL)
                find = 1;
            f=f->suivant;
        }
    }
    else
        printf("findLine(laFile, %s, %d, %d) problem;\n", line, attrib, content);
    return find;
}
