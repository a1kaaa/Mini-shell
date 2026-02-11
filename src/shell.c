/*
 * Mini-shell Unix - Architecture modulaire
 * Copyright (C) 2002, Simon Nieuviarts (code original)
 */

#include "shell.h"
#include "csapp.h"

/* ============================================ */
/* ========== Variables globales (jobs) ========== */
/* ============================================ */
job_t jobs[MAXJOBS];
static int next_job_id = 1;


/* ============================================ */
/* ========== Table des commandes intégrées ========== */
/* ============================================ */

// Commandes intégrées : tout ce qui modifie le Shell (cd y est mais pas mkdir par exemple)
builtin_cmd_t builtin_commands[] = {
    {"quit", builtin_quit, "Quitte le shell"},
    {"exit", builtin_exit, "Quitte le shell"},
    {"cd", builtin_cd, "Change de répertoire"},
    {"help", builtin_help, "Affiche l'aide"},
    {"jobs", builtin_jobs, "Liste les travaux en cours"},
    {"fg", builtin_fg, "Met un travail au premier plan"},
    {"bg", builtin_bg, "Relance un travail en arrière-plan"},
    {"stop", builtin_stop, "Arrête un travail"},
    {NULL, NULL, NULL}  // Sentinel
};



/* ============================================ */
/* ========== Implémentation des commandes intégrées (base) ========== */
/* ============================================ */
int builtin_quit(char **args) {
    printf(COL_VIOLET "À bientôt !" COL_RESET "\n");
    exit(0);
    return 0;
}

int builtin_exit(char **args) {
    return builtin_quit(args);
}

int builtin_cd(char **args) {
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home && chdir(home) != 0) {
            perror("cd");
            return 1;
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
    }
    return 0;
}

int builtin_help(char **args) {
    printf(COL_VIOLET "Mini-shell" COL_RESET " - Commandes intégrées disponibles :\n");
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        printf("  " COL_ROSE "%-10s" COL_RESET " - %s\n", builtin_commands[i].name, builtin_commands[i].description);
    }

    printf("Les autres commandes ne sont pas intégrées\n");
    return 0;
}



/* ============================================ */
/* ========== Gestion des commandes intégrées ========== */
/* ============================================ */
int try_execute_builtin(char **cmd) {
    if (cmd == NULL || cmd[0] == NULL) {
        return -1;  // Pas une commande intégrée
    }

    // Parcourir la table des commandes intégrées
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(cmd[0], builtin_commands[i].name) == 0) {
            return builtin_commands[i].func(cmd);
        }
    }

    return -1;  // Pas une commande intégrée
}


/* ============================================ */
/* ========== Utilitaires ========== */
/* ============================================ */
void command_error(const char *cmd) {
    fprintf(stderr, COL_ROUGE "%s: command not found" COL_RESET "\n", cmd);
}

int count_commands(char ***seq) {
    int count = 0;
    while (seq[count] != NULL) {
        count++;
    }
    return count;
}

// Enlève les espaces au début et à la fin d'une chaîne (en place)
// Gère les espaces ASCII normaux ET les espaces insécables UTF-8 (0xC2 0xA0)
char *trim_whitespace(char *str) {
    if (str == NULL) {
        return NULL;
    }

    // Enlever les espaces au début (ASCII et UTF-8 non-breaking space)
    while (*str == ' ' || *str == '\t' ||
           ((unsigned char)*str == 0xC2 && (unsigned char)*(str+1) == 0xA0)) {
        if ((unsigned char)*str == 0xC2) {
            str += 2;  // Sauter les 2 octets de l'espace insécable UTF-8
        } else {
            str++;
        }
    }

    // Si la chaîne ne contient que des espaces
    if (*str == '\0') {
        return str;
    }

    // Enlever les espaces à la fin
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' ||
           (end > str && (unsigned char)*(end-1) == 0xC2 && (unsigned char)*end == 0xA0))) {
        if ((unsigned char)*end == 0xA0 && end > str && (unsigned char)*(end-1) == 0xC2) {
            end -= 2;  // Reculer de 2 octets pour l'espace insécable UTF-8
        } else {
            end--;
        }
    }
    *(end + 1) = '\0';

    return str;
}

// Construit la chaîne de commande pour l'affichage des jobs
void build_cmdline_str(struct cmdline *l, char *buf, size_t bufsize) {
    buf[0] = '\0';
    for (int i = 0; l->seq[i] != NULL; i++) {
        if (i > 0) strncat(buf, " | ", bufsize - strlen(buf) - 1);
        for (int j = 0; l->seq[i][j] != NULL; j++) {
            if (j > 0) strncat(buf, " ", bufsize - strlen(buf) - 1);
            strncat(buf, l->seq[i][j], bufsize - strlen(buf) - 1);
        }
    }
}


/* ============================================ */
/* ========== Gestion des jobs ========== */
/* ============================================ */
void init_jobs(void) {
    for (int i = 0; i < MAXJOBS; i++) {
        jobs[i].id = 0;
        jobs[i].pgid = 0;
        jobs[i].num_procs = 0;
        jobs[i].num_done = 0;
        jobs[i].state = JOB_DONE;
        jobs[i].bg = 0;
        jobs[i].cmdline[0] = '\0';
    }
    next_job_id = 1;
}

int add_job(pid_t pgid, pid_t *pids, int num_procs, job_state_t state, int bg, const char *cmdline) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid == 0) {
            // Seuls les jobs en arrière-plan reçoivent un numéro visible
            // Les jobs de premier plan reçoivent id=0 (invisible dans 'jobs')
            // et recevront un id si stoppés par Ctrl+Z
            jobs[i].id = bg ? next_job_id++ : 0;
            jobs[i].pgid = pgid;
            for (int j = 0; j < num_procs && j < MAX_PIPELINE; j++) {
                jobs[i].pids[j] = pids[j];
            }
            jobs[i].num_procs = num_procs;
            jobs[i].num_done = 0;
            jobs[i].state = state;
            jobs[i].bg = bg;
            strncpy(jobs[i].cmdline, cmdline, sizeof(jobs[i].cmdline) - 1);
            jobs[i].cmdline[sizeof(jobs[i].cmdline) - 1] = '\0';
            return jobs[i].id;
        }
    }
    fprintf(stderr, COL_ROUGE "Erreur: trop de jobs" COL_RESET "\n");
    return -1;
}

void remove_job(pid_t pgid) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid == pgid) {
            jobs[i].id = 0;
            jobs[i].pgid = 0;
            jobs[i].num_procs = 0;
            jobs[i].num_done = 0;
            jobs[i].state = JOB_DONE;
            jobs[i].bg = 0;
            jobs[i].cmdline[0] = '\0';
            return;
        }
    }
}

job_t *find_job_by_pid(pid_t pgid) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid == pgid) return &jobs[i];
    }
    return NULL;
}

job_t *find_job_by_id(int id) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].id == id && jobs[i].pgid != 0) return &jobs[i];
    }
    return NULL;
}

// Trouve le job de premier plan actif
job_t *get_fg_job(void) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid != 0 && !jobs[i].bg && jobs[i].state == JOB_RUNNING) {
            return &jobs[i];
        }
    }
    return NULL;
}

// Parse une référence de job : %N pour le numéro, ou PID directement
job_t *parse_job_ref(const char *ref) {
    if (ref == NULL) {
        // Sans argument : dernier job (plus grand id)
        job_t *last = NULL;
        for (int i = 0; i < MAXJOBS; i++) {
            if (jobs[i].pgid != 0) {
                if (last == NULL || jobs[i].id > last->id) {
                    last = &jobs[i];
                }
            }
        }
        return last;
    }
    if (ref[0] == '%') {
        int id = atoi(ref + 1);
        return find_job_by_id(id);
    } else {
        pid_t pid = atoi(ref);
        return find_job_by_pid(pid);
    }
}

const char *job_state_str(job_state_t state) {
    switch (state) {
        case JOB_RUNNING: return "Running";
        case JOB_STOPPED: return "Stopped";
        case JOB_DONE:    return "Done";
        default: return "Unknown";
    }
}

// Vérifie et affiche les jobs en arrière-plan terminés
void check_completed_bg_jobs(void) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid != 0 && jobs[i].id > 0 && jobs[i].bg && jobs[i].state == JOB_DONE) {
            printf(COL_CYAN "[%d]" COL_RESET " " COL_VERT "Done" COL_RESET "\t\t" COL_ROSE "%s" COL_RESET "\n", jobs[i].id, jobs[i].cmdline);
            remove_job(jobs[i].pgid);
        }
        // Nettoyer silencieusement les jobs fg terminés (id == 0)
        if (jobs[i].pgid != 0 && jobs[i].id == 0 && jobs[i].state == JOB_DONE) {
            remove_job(jobs[i].pgid);
        }
    }
}


/* ============================================ */
/* ========== Traitants de signaux ========== */
/* ============================================ */

// SIGCHLD : ramasser les processus fils terminés ou stoppés
void sigchld_handler(int sig) {
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        // Trouver le job correspondant à ce PID
        for (int i = 0; i < MAXJOBS; i++) {
            if (jobs[i].pgid == 0) continue;
            int found = 0;
            for (int j = 0; j < jobs[i].num_procs; j++) {
                if (jobs[i].pids[j] == pid) {
                    if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        // Processus terminé (normalement ou par signal)
                        jobs[i].pids[j] = 0;
                        jobs[i].num_done++;
                        if (jobs[i].num_done >= jobs[i].num_procs) {
                            jobs[i].state = JOB_DONE;
                        }
                    } else if (WIFSTOPPED(status)) {
                        // Processus stoppé (Ctrl+Z / SIGTSTP)
                        jobs[i].state = JOB_STOPPED;
                        jobs[i].bg = 1;  // Passe en arrière-plan
                        // Attribuer un numéro de job s'il n'en avait pas (fg stoppé)
                        if (jobs[i].id == 0) {
                            jobs[i].id = next_job_id++;
                        }
                    }
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
    }

    errno = saved_errno;
}

// SIGINT (Ctrl+C) : transmettre au groupe de processus du premier plan
void sigint_handler(int sig) {
    job_t *fg = get_fg_job();
    if (fg != NULL) {
        kill(-fg->pgid, SIGINT);
    }
}

// SIGTSTP (Ctrl+Z) : transmettre au groupe de processus du premier plan
void sigtstp_handler(int sig) {
    job_t *fg = get_fg_job();
    if (fg != NULL) {
        kill(-fg->pgid, SIGTSTP);
    }
}


/* ============================================ */
/* ========== Commandes intégrées (jobs) ========== */
/* ============================================ */

// jobs : liste tous les travaux en cours
int builtin_jobs(char **args) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgid != 0 && jobs[i].id > 0) {
            const char *state_col = jobs[i].state == JOB_STOPPED ? COL_JAUNE :
                                    jobs[i].state == JOB_RUNNING ? COL_VERT : COL_VERT;
            printf(COL_CYAN "[%d]" COL_RESET " %d %s%s" COL_RESET "\t" COL_ROSE "%s" COL_RESET "\n",
                   jobs[i].id, jobs[i].pgid, state_col,
                   job_state_str(jobs[i].state), jobs[i].cmdline);
            if (jobs[i].state == JOB_DONE) {
                remove_job(jobs[i].pgid);
            }
        }
    }
    return 0;
}

// fg : mettre un travail au premier plan
int builtin_fg(char **args) {
    job_t *j = parse_job_ref(args[1]);
    if (j == NULL) {
        fprintf(stderr, COL_ROUGE "fg: aucun travail correspondant" COL_RESET "\n");
        return 1;
    }

    printf(COL_ROSE "%s" COL_RESET "\n", j->cmdline);
    j->bg = 0;
    j->state = JOB_RUNNING;
    kill(-j->pgid, SIGCONT);

    // Attendre le job au premier plan
    wait_for_fg_job(j);
    return 0;
}

// bg : relancer un travail stoppé en arrière-plan
int builtin_bg(char **args) {
    job_t *j = parse_job_ref(args[1]);
    if (j == NULL) {
        fprintf(stderr, COL_ROUGE "bg: aucun travail correspondant" COL_RESET "\n");
        return 1;
    }

    j->bg = 1;
    j->state = JOB_RUNNING;
    printf(COL_CYAN "[%d]" COL_RESET " " COL_ROSE "%s" COL_RESET " &\n", j->id, j->cmdline);
    kill(-j->pgid, SIGCONT);

    return 0;
}

// stop : arrêter un travail
int builtin_stop(char **args) {
    job_t *j = parse_job_ref(args[1]);
    if (j == NULL) {
        fprintf(stderr, COL_ROUGE "stop: aucun travail correspondant" COL_RESET "\n");
        return 1;
    }

    kill(-j->pgid, SIGSTOP);
    return 0;
}


/* ============================================ */
/* ========== Gestion des redirections ========== */
/* ============================================ */
void setup_redirections(char *input_file, char *output_file, int out_append) {
    // Redirection d'entrée
    if (input_file != NULL) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror(input_file);
            exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    // Redirection de sortie (> ou >>)
    if (output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= out_append ? O_APPEND : O_TRUNC;
        int fd_out = open(output_file, flags, 0644);
        if (fd_out < 0) {
            perror(output_file);
            exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}



/* ============================================ */
/* ========== Exécution de commandes ========== */
/* ============================================ */

// Commande simple sans gestion des jobs (conservée pour compatibilité)
void execute_simple_command(char **cmd, char *input_file, char *output_file, int out_append) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        setup_redirections(input_file, output_file, out_append);

        for (int i = 0; cmd[i] != NULL; i++) {
            cmd[i] = trim_whitespace(cmd[i]);
        }

        execvp(cmd[0], cmd);
        command_error(cmd[0]);
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

// Attendre un job de premier plan (boucle avec sleep comme recommandé par le TP)
// sleep(1) est interrompu par SIGCHLD, donc le réveil est quasi-immédiat
void wait_for_fg_job(job_t *j) {
    while (j->state == JOB_RUNNING) {
        sleep(1);
    }

    if (j->state == JOB_DONE) {
        remove_job(j->pgid);
    } else if (j->state == JOB_STOPPED) {
        printf(COL_CYAN "[%d]" COL_RESET " " COL_JAUNE "Stopped" COL_RESET "\t\t" COL_ROSE "%s" COL_RESET "\n", j->id, j->cmdline);
    }
}


// Exécution d'un pipeline (commande simple ou multiple) avec gestion des jobs
void execute_pipeline(struct cmdline *l) {
    int num_cmds = count_commands(l->seq);
    int bg = l->bg;

    // Construire la chaîne de commande pour l'affichage
    char cmdline_str[256];
    build_cmdline_str(l, cmdline_str, sizeof(cmdline_str));

    // Créer les pipes
    int num_pipes = num_cmds - 1;
    int pipes[num_pipes > 0 ? num_pipes : 1][2];
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return;
        }
    }

    // Bloquer SIGCHLD pendant la mise en place des processus et du job
    sigset_t mask_chld, prev_mask;
    sigemptyset(&mask_chld);
    sigaddset(&mask_chld, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask_chld, &prev_mask);

    // Créer tous les processus (exécution PARALLÈLE)
    pid_t pids[num_cmds];
    pid_t pgid = 0;

    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            return;
        }

        if (pids[i] == 0) {
            // ===== PROCESSUS FILS =====

            // Rétablir les traitants de signaux par défaut
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);

            // Débloquer SIGCHLD dans le fils
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);

            // Groupe de processus : tous dans le même groupe (pgid du 1er fils)
            setpgid(0, pgid);

            // Redirection d'entrée
            if (i == 0 && l->in) {
                int fd = open(l->in, O_RDONLY);
                if (fd < 0) { perror(l->in); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            } else if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // Redirection de sortie
            if (i == num_cmds - 1 && l->out) {
                int flags = O_WRONLY | O_CREAT;
                flags |= l->out_append ? O_APPEND : O_TRUNC;
                int fd = open(l->out, flags, 0644);
                if (fd < 0) { perror(l->out); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (i < num_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Fermer TOUS les pipes dans le fils
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Nettoyer les arguments (enlever espaces parasites)
            for (int j = 0; l->seq[i][j] != NULL; j++) {
                l->seq[i][j] = trim_whitespace(l->seq[i][j]);
            }

            // Exécuter la commande
            execvp(l->seq[i][0], l->seq[i]);

            command_error(l->seq[i][0]);
            exit(1);
        }

        // ===== PROCESSUS PARENT =====
        if (i == 0) pgid = pids[0];
        setpgid(pids[i], pgid);  // Aussi dans le parent pour éviter la race condition
    }

    // Fermer tous les pipes dans le parent
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Ajouter le job à la table
    int job_id = add_job(pgid, pids, num_cmds, JOB_RUNNING, bg, cmdline_str);

    // Débloquer SIGCHLD
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    if (bg) {
        // Arrière-plan : afficher le numéro de job et le pgid
        printf(COL_CYAN "[%d]" COL_RESET " %d\n", job_id, pgid);
    } else {
        // Premier plan : attendre la fin du job
        job_t *j = find_job_by_pid(pgid);
        if (j != NULL) {
            wait_for_fg_job(j);
        }
    }
}

void execute_cmdline(struct cmdline *l) {
    // Vérifier si c'est une commande vide
    if (l->seq == NULL || l->seq[0] == NULL || l->seq[0][0] == NULL) {
        return;
    }

    // Vérifier si c'est une commande intégrée (seulement sans pipe et sans redirection)
    if (count_commands(l->seq) == 1 && l->in == NULL && l->out == NULL) {
        int result = try_execute_builtin(l->seq[0]);
        if (result >= 0) {
            return;
        }
    }

    // Sinon, exécuter la commande ou le pipeline
    execute_pipeline(l);
}


char *get_prompt() {
    char path[MAXBUF];
    if(!getcwd(path, MAXBUF))
    {
        fprintf(stderr, "Erreur: impossible de resoudre le repetoire courant\n");
        exit(1);
    }
    int size_path = strlen(path);
    char *user = getenv("USER");
    if(!user)
    {
        fprintf(stderr, "Erreur: impossible d'obtenir l'usager\n");
        exit(1);
    }
    int size_user = strlen(user);
    char host[MAXBUF];
    if(gethostname(host, MAXBUF))
    {
        perror("hostname");
        exit(1);
    }
    int size_host = strlen(host);

    char *prompt = malloc(size_path + size_user + size_host + 5);
    int i = 1;
    prompt[0] ='[';
    for(int k = 0; k < size_user; k++)
    {
        prompt[i++] = user[k];
    }
    prompt[i++] = '@';
    for(int k = 0; k < size_host; k++)
    {
        prompt[i++] = host[k];
    }
    prompt[i++] = ']';
    for(int k = 0; k < size_path; k++)
    {
        prompt[i++] = path[k];
    }
    prompt[i++] = '$';
    prompt[i] = '\0';

    return prompt;
}

