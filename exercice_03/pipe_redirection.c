#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int main(void) {
    int pipe_fd[2]; // Je crée un tableau pour stocker les descripteurs du pipe
    pid_t pid_ps, pid_grep; // PID pour les processus ps et grep

    // Création du pipe
    if (pipe(pipe_fd) < 0) {
        perror("Erreur lors de la création du pipe");
        return EXIT_FAILURE;
    }

    // 1er fork pour lancer "ps eaux"
    pid_ps = fork();
    if (pid_ps < 0) {
        perror("Erreur lors du fork pour ps");
        return EXIT_FAILURE;
    }

    if (pid_ps == 0) {
        // Fils pour exécuter la commande "ps eaux"
        
        // Redirection de la sortie standard stdout vers le côté écriture du pipe
        close(pipe_fd[0]); 
        if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
            perror("Erreur lors de dup2 pour ps");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]); // fermeture de l'écriture après la redirection
        
        // Commande "ps eaux"
        execlp("ps", "ps", "eaux", NULL);
        perror("Erreur lors de l'exécution de la commande ps");
        exit(EXIT_FAILURE);
    }

    // 2ème fork pour exécuter la commande "grep ^root"
    pid_grep = fork();
    if (pid_grep < 0) {
        perror("Erreur lors du fork pour grep");
        return EXIT_FAILURE;
    }

    if (pid_grep == 0) {
        // Fils pour exécuter "grep ^root"

        // Redirection de l'entrée standard (stdin) vers le côté lecture du pipe
        close(pipe_fd[1]); 
        if (dup2(pipe_fd[0], STDIN_FILENO) < 0) {
            perror("Erreur lors de dup2 pour grep");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[0]); // fermeture de l'écriture après redirection

        // Redirection de la sortie standard stdout vers /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null < 0) {
            perror("Erreur lors de l'ouverture de /dev/null");
            exit(EXIT_FAILURE);
        }
        if (dup2(dev_null, STDOUT_FILENO) < 0) {
            perror("Erreur lors de dup2 pour /dev/null");
            exit(EXIT_FAILURE);
        }
        close(dev_null); // fermeture de /dev/null après la redirection

        // On lance "grep ^root"
        execlp("grep", "grep", "^root", NULL);
        perror("Erreur lors de l'exécution de grep");
        exit(EXIT_FAILURE);
    }

    // Le père ferme les descripteurs du pipe
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    // Attente des deux processus fils : on attend la fin de "ps" et la fin de "grep"
    int status_ps, status_grep;
    waitpid(pid_ps, &status_ps, 0);
    waitpid(pid_grep, &status_grep, 0);

    // Vérification du statut de grep
    if (WIFEXITED(status_grep) && WEXITSTATUS(status_grep) == 0) {
        // Si grep s'est terminé avec succès, on affiche "root est connecté"
        const char *message = "root est connecté\n";
        write(STDOUT_FILENO, message, strlen(message));
    }

    return EXIT_SUCCESS;
}
