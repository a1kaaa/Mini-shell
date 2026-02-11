#include "shell.h"
#include "csapp.h"
#include "readcmd.h"
/* ============================================ */
/* ========== MAIN ========== */
/* ============================================ */
int main() {
    // Initialiser la table des jobs
    init_jobs();

    // Installer les traitants de signaux avec sigaction
    struct sigaction sa;

    // SIGCHLD : ramasser les processus terminés/stoppés
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Redémarrer les appels système interrompus
    sigaction(SIGCHLD, &sa, NULL);

    // SIGINT (Ctrl+C) : transmettre au processus de premier plan
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    // SIGTSTP (Ctrl+Z) : transmettre au processus de premier plan
    sa.sa_handler = sigtstp_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa, NULL);

    while (1) {
        struct cmdline *l;

        // Vérifier les jobs terminés en arrière-plan
        check_completed_bg_jobs();
        char *prompt = get_prompt();
        printf(COL_VIOLET "%s" COL_RESET, prompt);
        
        fflush(stdout);
        free(prompt);
        l = readcmd();

        // EOF (Ctrl+D)
        if (!l) {
            printf("\n" COL_VIOLET "exit" COL_RESET "\n");
            exit(0);
        }

        // Erreur de syntaxe
        if (l->err) {
            fprintf(stderr, COL_ROUGE "error: %s" COL_RESET "\n", l->err);
            continue;
        }

        // Exécuter la ligne de commande
        execute_cmdline(l);
    }

    return 0;
}
