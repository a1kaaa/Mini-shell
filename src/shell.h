#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "readcmd.h"

/* ========== Couleurs ANSI ========== */
#define COL_RESET   "\033[0m"
#define COL_VIOLET  "\033[1;35m"   // Prompt
#define COL_ROSE    "\033[95m"     // Commandes
#define COL_BLEU    "\033[1;34m"   // Arguments/options
#define COL_ROUGE   "\033[1;31m"   // Erreurs
#define COL_VERT    "\033[0;32m"   // Running / Done
#define COL_JAUNE   "\033[0;33m"   // Stopped
#define COL_CYAN    "\033[0;36m"   // Numéros de jobs

/* ========== Commandes intégrées ========== */
typedef struct {
    char *name;                    // Nom de la commande
    int (*func)(char **args);      // Fonction à exécuter (retourne 0 si ça réussit)
    char *description;             // Description de la commande
} builtin_cmd_t;

/* Vérifier si c'est une commande intégrée et l'exécuter */
int try_execute_builtin(char **cmd);

/* Commandes intégrées */
int builtin_quit(char **args);
int builtin_exit(char **args);
int builtin_cd(char **args);
int builtin_help(char **args);
int builtin_jobs(char **args);
int builtin_fg(char **args);
int builtin_bg(char **args);
int builtin_stop(char **args);

/* ========== Gestion des jobs ========== */
#define MAXJOBS 10
#define MAX_PIPELINE 16

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_state_t;

typedef struct {
    int id;                        // Numéro du job (1-based, 0 = slot libre)
    pid_t pgid;                    // Process Group ID (0 = slot libre)
    pid_t pids[MAX_PIPELINE];      // PIDs des processus du pipeline
    int num_procs;                 // Nombre de processus dans le pipeline
    int num_done;                  // Nombre de processus terminés
    job_state_t state;             // État courant
    int bg;                        // 1 = arrière-plan, 0 = premier plan
    char cmdline[256];             // Texte de la commande pour affichage
} job_t;

extern job_t jobs[MAXJOBS];

void init_jobs(void);
int add_job(pid_t pgid, pid_t *pids, int num_procs, job_state_t state, int bg, const char *cmdline);
void remove_job(pid_t pgid);
job_t *find_job_by_pid(pid_t pgid);
job_t *find_job_by_id(int id);
job_t *get_fg_job(void);
job_t *parse_job_ref(const char *ref);
const char *job_state_str(job_state_t state);
void check_completed_bg_jobs(void);

/* ========== Traitants de signaux ========== */
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

/* ========== Exécution ========== */
void execute_cmdline(struct cmdline *l);
void execute_simple_command(char **cmd, char *input_file, char *output_file, int out_append);
void execute_pipeline(struct cmdline *l);
void wait_for_fg_job(job_t *j);

/* ========== Gestion des redirections ========== */
void setup_redirections(char *input_file, char *output_file, int out_append);

/* ========== Utilitaires ========== */
void command_error(const char *cmd);
int count_commands(char ***seq);
char *trim_whitespace(char *str);
void build_cmdline_str(struct cmdline *l, char *buf, size_t bufsize);

#endif /* __SHELL_H__ */
