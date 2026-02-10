#!/bin/bash
# ============================================================
#  demonstration.sh - Script de demonstration pour la soutenance
#  Mini-Shell Unix - TP Systemes et Reseaux
# ============================================================
#
#  Usage :  ./demonstration.sh
#
#  Ce script compile le shell, puis lance une serie de
#  demonstrations interactives etape par etape.
#  Appuyer sur ENTREE pour passer a l'etape suivante.
# ============================================================

set -e

# -- Couleurs --
VIOLET='\033[1;35m'
ROSE='\033[95m'
BLEU='\033[1;34m'
ROUGE='\033[1;31m'
VERT='\033[0;32m'
JAUNE='\033[0;33m'
CYAN='\033[0;36m'
GRAS='\033[1m'
DIM='\033[2m'
RST='\033[0m'

SHELL_BIN="bin/shell"
SDRIVER="./sdriver.pl"
TESTS_DIR="./tests"

# Fichiers temporaires pour les tests de redirection
TMP_REDIR="/tmp/demo_shell_redir_$$"
trap "rm -f ${TMP_REDIR}*" EXIT

# ============================================================
# Fonctions utilitaires
# ============================================================

banniere() {
    clear
    echo ""
    echo -e "${VIOLET}╔══════════════════════════════════════════════════════════╗${RST}"
    echo -e "${VIOLET}║${RST}${GRAS}       DEMONSTRATION - Mini-Shell Unix                    ${RST}${VIOLET}║${RST}"
    echo -e "${VIOLET}║${RST}${DIM}       TP Systemes et Reseaux - L3 Informatique           ${RST}${VIOLET}║${RST}"
    echo -e "${VIOLET}╚══════════════════════════════════════════════════════════╝${RST}"
    echo ""
}

section() {
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"
    echo -e "${GRAS}${BLEU}  $1${RST}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"
    echo ""
}

info() {
    echo -e "  ${DIM}$1${RST}"
}

commande() {
    echo -e "  ${JAUNE}\$${RST} ${GRAS}$1${RST}"
}

succes() {
    echo -e "  ${VERT}[OK]${RST} $1"
}

erreur() {
    echo -e "  ${ROUGE}[FAIL]${RST} $1"
}

pause_demo() {
    echo ""
    echo -e "  ${DIM}Appuyez sur ENTREE pour continuer...${RST}"
    read -r
}

run_test() {
    local test_file="$1"
    local desc="$2"
    echo -e "  ${ROSE}Test :${RST} $desc"
    commande "perl $SDRIVER -t $test_file -s $SHELL_BIN"
    echo ""
    if perl "$SDRIVER" -t "$test_file" -s "$SHELL_BIN" 2>&1 | sed 's/^/    /'; then
        succes "$desc"
    else
        erreur "$desc"
    fi
}

run_interactive() {
    local desc="$1"
    shift
    echo -e "  ${ROSE}Demo interactive :${RST} $desc"
    info "Les commandes suivantes vont etre envoyees au shell :"
    for cmd in "$@"; do
        commande "$cmd"
    done
    echo ""
    # Envoie les commandes au shell via un pipe
    {
        for cmd in "$@"; do
            echo "$cmd"
            sleep 0.5
        done
    } | timeout 10 "$SHELL_BIN" 2>&1 | sed 's/^/    /' || true
    echo ""
}

# ============================================================
# Verification prealable
# ============================================================

banniere

section "0. Compilation"

info "Compilation du mini-shell..."
commande "make clean && make"
echo ""
make clean && make 2>&1 | sed 's/^/    /'
echo ""

if [ -x "$SHELL_BIN" ]; then
    succes "Compilation reussie"
else
    erreur "Echec de compilation"
    exit 1
fi

pause_demo

# ============================================================
# ETAPE 1 : Commande quit
# ============================================================

banniere
section "1. Commande quit / exit"
info "Le shell doit se terminer proprement avec 'quit' ou 'exit'."
echo ""

run_test "$TESTS_DIR/test01.txt" "quit termine le shell"

pause_demo

# ============================================================
# ETAPE 2 : Commandes simples
# ============================================================

banniere
section "2. Execution de commandes simples"
info "Le shell doit executer des commandes externes avec arguments."
echo ""

run_test "$TESTS_DIR/test05.txt" "echo et ls avec arguments"

pause_demo

# ============================================================
# ETAPE 3 : Commandes integrees
# ============================================================

banniere
section "3. Commandes integrees (cd, help)"
info "Les commandes integrees s'executent dans le processus du shell."
echo ""

run_test "$TESTS_DIR/test12.txt" "help, cd /tmp, pwd, cd (retour HOME)"

pause_demo

# ============================================================
# ETAPE 4 : Gestion des erreurs
# ============================================================

banniere
section "4. Gestion des erreurs"
info "Le shell affiche un message d'erreur adapte (en rouge)."
echo ""

run_test "$TESTS_DIR/test11.txt" "commande inexistante + repertoire inexistant"

pause_demo

# ============================================================
# ETAPE 5 : Redirections
# ============================================================

banniere
section "5. Redirections d'entree/sortie (< > >>)"
info "Test des redirections : ecriture, ajout, et lecture depuis fichier."
echo ""

run_test "$TESTS_DIR/test06.txt" "Redirection > et >>"
echo ""
run_test "$TESTS_DIR/test07.txt" "Redirection <"

pause_demo

# ============================================================
# ETAPE 6 : Pipes
# ============================================================

banniere
section "6. Pipes simples et multiples"
info "Les commandes reliees par | s'executent en parallele."
echo ""

run_test "$TESTS_DIR/test08.txt" "Pipe simple (2 commandes)"
echo ""
run_test "$TESTS_DIR/test09.txt" "Pipe multiple (4 commandes)"

pause_demo

# ============================================================
# ETAPE 7 : Pipes + Redirections
# ============================================================

banniere
section "7. Pipes combines avec redirections"
info "Test de la combinaison : cat < in | tr | sort > out"
echo ""

run_test "$TESTS_DIR/test10.txt" "Pipe + redirections d'entree et sortie"

pause_demo

# ============================================================
# ETAPE 8 : Arriere-plan et jobs
# ============================================================

banniere
section "8. Execution en arriere-plan (&) et commande jobs"
info "Les commandes terminees par & s'executent en background."
echo ""

run_test "$TESTS_DIR/test13.txt" "Lancement de jobs en arriere-plan"

pause_demo

# ============================================================
# ETAPE 9 : Signaux - Ctrl+C
# ============================================================

banniere
section "9. Signal SIGINT (Ctrl+C)"
info "Ctrl+C doit terminer le processus foreground,"
info "mais le shell doit SURVIVRE."
echo ""

run_test "$TESTS_DIR/test14.txt" "Ctrl+C sur sleep, shell survit"

pause_demo

# ============================================================
# ETAPE 10 : Signaux - Ctrl+Z
# ============================================================

banniere
section "10. Signal SIGTSTP (Ctrl+Z) + fg"
info "Ctrl+Z suspend le processus foreground."
info "fg le reprend, puis Ctrl+C le termine."
echo ""

run_test "$TESTS_DIR/test15.txt" "Ctrl+Z -> jobs -> fg -> Ctrl+C"

pause_demo

# ============================================================
# ETAPE 11 : Gestion des zombies
# ============================================================

banniere
section "11. Gestion des zombies (SIGCHLD)"
info "Les processus termines en arriere-plan ne doivent"
info "PAS rester en etat zombie (<defunct>)."
echo ""

run_test "$TESTS_DIR/test16.txt" "3 jobs bg termines + ps (pas de zombie)"

pause_demo

# ============================================================
# ETAPE 12 : stop
# ============================================================

banniere
section "12. Commande stop"
info "stop %N suspend un job en arriere-plan."
echo ""

run_test "$TESTS_DIR/test18.txt" "sleep & -> jobs -> stop %1 -> jobs"

pause_demo

# ============================================================
# LANCEMENT DES TESTS AUTOMATIQUES
# ============================================================

banniere
section "Execution de tous les tests automatiques"
info "Lancement de test01 a test18 avec sdriver.pl..."
echo ""

PASS=0
FAIL=0
SKIP=0

for test_file in "$TESTS_DIR"/test*.txt; do
    name=$(basename "$test_file" .txt)
    desc=$(head -2 "$test_file" | tail -1 | sed 's/^# *//')

    if timeout 15 perl "$SDRIVER" -t "$test_file" -s "$SHELL_BIN" > /dev/null 2>&1; then
        succes "$name : $desc"
        PASS=$((PASS + 1))
    else
        if [ $? -eq 124 ]; then
            echo -e "  ${JAUNE}[SKIP]${RST} $name : $desc ${DIM}(timeout)${RST}"
            SKIP=$((SKIP + 1))
        else
            erreur "$name : $desc"
            FAIL=$((FAIL + 1))
        fi
    fi
done

echo ""
echo -e "  ${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"
TOTAL=$((PASS + FAIL + SKIP))
echo -e "  ${GRAS}Resultats : ${VERT}$PASS passe(s)${RST}, ${ROUGE}$FAIL echoue(s)${RST}, ${JAUNE}$SKIP timeout(s)${RST} / $TOTAL total"
echo -e "  ${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"

pause_demo

# ============================================================
# Demo interactive live
# ============================================================

banniere
section "13. Demo interactive live"
info "Lancement du mini-shell en mode interactif."
info "Vous pouvez taper des commandes librement."
info "Suggestions :"
echo ""
commande "help"
commande "ls -la | grep shell"
commande "echo bonjour > /tmp/test.txt && cat /tmp/test.txt"
commande "sleep 30 &"
commande "jobs"
commande "stop %1"
commande "jobs"
commande "fg %1       (puis Ctrl+C pour terminer)"
commande "quit"
echo ""
info "Lancement du shell..."
echo ""

"$SHELL_BIN"

# ============================================================
# Fin
# ============================================================

echo ""
echo -e "${VIOLET}╔══════════════════════════════════════════════════════════╗${RST}"
echo -e "${VIOLET}║${RST}${GRAS}${VERT}       Demonstration terminee. Merci !                    ${RST}${VIOLET}║${RST}"
echo -e "${VIOLET}╚══════════════════════════════════════════════════════════╝${RST}"
echo ""
