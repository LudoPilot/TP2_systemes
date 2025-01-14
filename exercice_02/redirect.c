#include <stdio.h>   
#include <stdlib.h>    
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/wait.h> 
#include <fcntl.h>     
#include <errno.h>     
#include <string.h>    

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <programme_a_executer>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 
     * argv[1] = nom du programme à lancer (ex: "./affiche")
     * Pour le tester, nous passerons un mot "AU REVOIR" en paramètre à ce programme.
     */

    /* Fork pour créer un processus fils */
    pid_t pidFils = fork();
    if (pidFils < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pidFils == 0) {
        /* ---------------- PROCESSUS FILS ---------------- */
        pid_t pidFilsCourant = getpid();
        printf("[FILS] Mon PID = %d\n", pidFilsCourant);

        /*
         * Étape 2 : Fermer le descripteur 1 (stdout)
         * Étape 3 : Pour fermer le descripteur 2 (stderr) à la place, commentez la ligne ci-dessous et décommentez la suivante.
         */
        if (close(STDOUT_FILENO) < 0) {
            fprintf(stderr, "[FILS] Erreur close(STDOUT_FILENO) : %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }
        // if (close(STDERR_FILENO) < 0) {
        //     fprintf(stderr, "[FILS] Erreur close(STDERR_FILENO) : %s\n", strerror(errno));
        //     _exit(EXIT_FAILURE);
        // }

        /*
         * Ouvrir un fichier temporaire dans /tmp/
         * mkstemp nécessite un template contenant 'XXXXXX'
         */
        char templateFichier[] = "/tmp/proc-exercise-XXXXXX";
        int fdTmp = mkstemp(templateFichier);
        if (fdTmp < 0) {
            fprintf(stderr, "[FILS] Erreur mkstemp : %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        /* 
         * On peut supprimer le fichier du répertoire pour éviter de le laisser trainer 
         * (le descripteur fdTmp reste ouvert malgré tout).
         * Cela permet de travailler avec un fichier temporaire anonyme.
         */
        // unlink(templateFichier); // optionnel

        /* 
         * fdTmp est le nouveau descripteur du fichier temporaire.  
         * Redirigeons STDOUT_FILENO (1) ou STDERR_FILENO (2) vers fdTmp.
         * Dans ce code, on redirige stdout (descripteur n°1).
         * Pour rediriger stderr (descripteur n°2), remplacez STDOUT_FILENO par STDERR_FILENO ci-dessous.
         */
        if (dup2(fdTmp, STDOUT_FILENO) < 0) {
            fprintf(stderr, "[FILS] Erreur dup2 : %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        /* Plus besoin de fdTmp si dup2 a fonctionné (optionnel de le fermer) */
        // close(fdTmp);

        /* Afficher le numéro du descripteur ouvert (pour debug) */
        fprintf(stderr, "[FILS] Descripteur fichier temporaire = %d\n", fdTmp);

        /*
         * Exécuter le programme passé en argument (argv[1]) et lui fournir un mot supplémentaire,
         * par exemple "AU REVOIR".
         * Remarquez que si on a fermé stdout et qu’on a redirigé dessus, les messages envoyés
         * sur stdout par ce programme seront écrits dans le fichier /tmp/proc-exercise-XXXXXX.
         */
        execlp(argv[1], argv[1], "AU REVOIR", NULL);

        /* Si execlp échoue, on gère l'erreur et on quitte */
        fprintf(stderr, "[FILS] Erreur execlp : %s\n", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    else {
        /* ---------------- PROCESSUS PÈRE ---------------- */
        pid_t pidPere = getpid();
        printf("[PERE] Mon PID = %d\n", pidPere);
        printf("[PERE] PID de mon fils = %d\n", pidFils);

        /* On attend la fin du processus fils */
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            int codeRetourFils = WEXITSTATUS(status);
            printf("[PERE] Fils terminé avec code de retour = %d\n", codeRetourFils);
        } else if (WIFSIGNALED(status)) {
            int signalFils = WTERMSIG(status);
            printf("[PERE] Fils tué par le signal %d\n", signalFils);
        } else {
            printf("[PERE] Fils terminé anormalement.\n");
        }

        printf("[PERE] That's All Folks !\n");
    }

    return EXIT_SUCCESS;
}
