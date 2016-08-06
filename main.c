#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "header/dfs.h"

#define CFGFILE "dfs.conf"

int update=0, diff=0, liste=0, friendly=0, verbose=0, debug=0, help=0;

int main(int argc, char *argv[])
{
    int idxopt, opt;
	char *configfile = NULL;
	struct option t_opt[] =
	{
        {"update",     	no_argument,       			&update,  1 },
        {"diff",   		no_argument, 				&diff,    2 },
        {"list",   		no_argument,       			&liste,   3 },
        {"friendly",   	no_argument,       			&friendly,4 },
        {"verbose",   	no_argument,       			&verbose, 5 },
        {"debug",   	no_argument,       			&debug,   6 },
        {"help",   	    no_argument,       			&help,    7 },
        //{"c",   		required_argument,     		NULL,  7 },
        {0,           	0,                 			NULL,     0 }
    };

	// OPTION LONGUE (-- de la structure) et les courtes dans la chaine (c:)
	while ((opt = getopt_long(argc, argv, "c:", t_opt, &idxopt )) != -1) {
		switch (opt){
			case 'c':
				configfile = optarg;
				break;
			default:
				break;
		}

    }
    if (help)
        usage();
    else {
        fCsvFile = NULL; // la file contenant les sauvegardes csv
        struct conf *cfg = NULL;
        /* Si -c, utiliser loadConf avec le configfile renseigné, sinon utilisé celui du define */
        int bLoadSuccess;
        if (configfile != NULL)
            bLoadSuccess = loadConf(configfile, &cfg);
        else
            bLoadSuccess = loadConf(CFGFILE, &cfg);
        /* Si le fichier de config n'a pas été chargé, alors quitter ! */
        if (!bLoadSuccess) {
            printf("\nConfig file loading error!\n");
            exit(0);
        }

        remplirFile(cfg);
        if (debug)
            afficheConf(cfg);

        if (diff)
            diffDB(cfg);
        if (update)
            updateDB(cfg);
        if (liste)
            listeDB(cfg);
        /* si friendly -> active juste la traduction dans la fonction listDB */
        /* si verbose -> active juste des infos utilisateurs dans les fonctions appelé */
        /* si debug -> active juste des infos développeurs dans les fonctions appelé (fonction+arguments) -> pour un debugage plus simple si besoin */
        detruire(&fCsvFile);
        free(cfg);
    }
    //printf("%d|%d|%d|%d|%d|%d|%s\n", update, diff, list, friendly, verbose, debug, configfile);

    return 0;
}
