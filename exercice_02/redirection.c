#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // On vérifie les arguments
    if (argc < 2) {
        fprintf(stderr, "Usage (père)  : %s <mot> [stdout|stderr]\n", argv[0]);
        fprintf(stderr, "Usage (child) : %s <mot> child\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Détection du mode "child"
    if ((argc >= 3) && (strcmp(argv[2], "child") == 0)) {
        // Mode fils/affiche
        printf("Message depuis le mode child : %s\n", argv[1]);
        return EXIT_SUCCESS;
    }

    // Mode père : vérification de l'argument stdout/stderr
    int descripteurAFermer = STDOUT_FILENO; // Par défaut, on ferme stdout
    if (argc >= 3) {
        if (strcmp(argv[2], "stderr") == 0) {
            descripteurAFermer = STDERR_FILENO; // Si "stderr" est précisé, on ferme stderr
        } else if (strcmp(argv[2], "stdout") != 0) {
            fprintf(stderr, "Erreur : argument invalide '%s'. Utilisez 'stdout' ou 'stderr'.\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    // On crée un processus fils
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Mode fils
        printf("[FILS] PID = %d\n", getpid());

        // Fermeture du descripteur
        if (close(descripteurAFermer) < 0) {
            fprintf(stderr, "[FILS] Erreur close(descripteur=%d) : %s\n", descripteurAFermer, strerror(errno));
            _exit(EXIT_FAILURE);
        }

        // Création d'un fichier temporaire
        char templateFichier[] = "/tmp/proc-exercise-XXXXXX";
        int fdTmp = mkstemp(templateFichier);
        if (fdTmp < 0) {
            fprintf(stderr, "[FILS] Erreur mkstemp : %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        // Redirection du flux fermé vers le fichier temporaire
        if (dup2(fdTmp, descripteurAFermer) < 0) {
            fprintf(stderr, "[FILS] Erreur dup2 : %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        // Affichage du fichier temporaire utilisé
        fprintf(stderr, "[FILS] Fichier temporaire : %s (descripteur %d)\n", templateFichier, fdTmp);

        // Exécution du même programme en mode "child"
        execlp(argv[0], argv[0], argv[1], "child", NULL);

        // Si on arrive ici, exec a échoué
        fprintf(stderr, "[FILS] Erreur execlp : %s\n", strerror(errno));
        _exit(EXIT_FAILURE);
    } else {
        // Mode père
        printf("[PERE] PID = %d, PID fils = %d\n", getpid(), pid);

        // Attente de la fin du fils
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            int codeFils = WEXITSTATUS(status);
            printf("[PERE] Fils terminé, code = %d\n", codeFils);
        } else if (WIFSIGNALED(status)) {
            printf("[PERE] Fils tué par le signal %d\n", WTERMSIG(status));
        } else {
            printf("[PERE] Fin anormale du fils.\n");
        }

        printf("[PERE] That's All Folks !\n");
    }

    return EXIT_SUCCESS;
}
