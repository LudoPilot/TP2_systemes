#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) 
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) { 
        /* Processus fils */
        pid_t my_pid = getpid();
        pid_t my_ppid = getppid();
        
        printf("FILS : mon PID = %d, PID de mon père = %d\n", my_pid, my_ppid);
        
        /* Je récupère le dernier chiffre du PID du fils */
        int last_digit = my_pid % 10;
        
        /* On termine le fils avec ce code de retour */
        exit(last_digit);
    }
    else {
        /* Processus père */
        printf("PÈRE : PID du fils = %d\n", pid);

        /* On attend que le fils se termine */
        wait(&status);

        /* Récupération du code de retour du fils s’il s’est terminé correctement */
        if (WIFEXITED(status)) {
            int retour_fils = WEXITSTATUS(status);
            printf("PÈRE : le code de retour du fils est = %d\n", retour_fils);
        } else {
            printf("PÈRE : le processus fils ne s'est pas terminé normalement.\n");
        }
    }

    return 0;
}
