#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h> // pour la date
#include <pwd.h> // pour les infos utilisateur
#include <grp.h> // pour les infos groupe
#include <openssl/md5.h> // pour le hash du fichier
#include "conf.h"

/*struct stat {
    dev_t     st_dev;      // ID du périphérique contenant le fichier
    ino_t     st_ino;      // Numéro inœud
    mode_t    st_mode;     // Protection
    nlink_t   st_nlink;    // Nb liens matériels
    uid_t     st_uid;      // UID propriétaire
    gid_t     st_gid;      // GID propriétaire
    dev_t     st_rdev;     // ID périphérique (si fichier spécial)
    off_t     st_size;     // Taille totale en octets
    blksize_t st_blksize;  // Taille de bloc pour E/S
    blkcnt_t  st_blocks;   // Nombre de blocs alloués
    time_t    st_atime;    // Heure dernier accès
    time_t    st_mtime;    // Heure dernière modification
    time_t    st_ctime;    // Heure dernier changement état
};*/
File fCsvFile;
/* Les fonctions utiles des options */
void usage(); // (--hepl) affiche l'aide de la fonction dfs
char *getHashOfFile(char *filename);
char *getModeProtection(mode_t protection);

/* Pour le mode friendly */
void showHumanDateModifFromStatStruct(char *date);
void showOwnerNameFromUid(char *owner);
void showGroupNameFromGid(char *group);
void showHumanFileSize(char *fsize);

/* Gestion lecture et maj des fichier de sauvegarde */
void diffDB(struct conf *pcfg); // (--diff)
void verifDiff(char *path, int recurse, int attrib, int content);

void updateDB(struct conf *pcfg); // met à jour la bdd de fichier csv (--update)
void updateFile(char *path, FILE *saveFile, int recurse); // met à jour le fichier demander par la fonction updateDB

void listeDB(struct conf *pcfg); // (--list) affiche le fichier de sauvegarde, donc l'état sauvegardé
void read_file(char const *saveFile); // est appelé par listeDB()

/* gestion de la file pour la comparaison */
void remplirFile(struct conf *pcfg);
void enfiler(File *mafile, char *line);
File defiler(File *mafile);
void afficher(File f);
void detruire(File *f);
int findLine(File f, char *line, int attrib, int content);
