# Mini-Shell Unix

Un interpréteur de commandes (shell) simplifié écrit en C, conçu pour démontrer les principes fondamentaux de la gestion des processus, des signaux et de la programmation système Unix/Linux.

## Table des matières

- [Fonctionnalités](#fonctionnalités)
- [Architecture](#architecture)
- [Installation et Compilation](#installation-et-compilation)
- [Utilisation](#utilisation)
- [Commandes Intégrées](#commandes-intégrées)
- [Gestion des Signaux](#gestion-des-signaux)
- [Structure du Projet](#structure-du-projet)
- [Licence](#licence)

## Fonctionnalités

### Exécution de Commandes
- **Exécution en premier plan** : Exécute les commandes de manière synchrone
- **Exécution en arrière-plan** : Supporte les commandes suffixées avec `&` pour fonctionner en arrière-plan
- **Pipelines** : Enchaîne plusieurs commandes avec des tubes (`|`)
- **Redirection d'entrée/sortie** : Supporte `<`, `>` et `>>`

### Gestion des Jobs
- Liste les travaux actifs (commande `jobs`)
- Suspend les processus (Ctrl+Z / `SIGTSTP`)
- Reprend les travaux en premier plan (`fg`) ou en arrière-plan (`bg`)
- Arrête les processus contrôlés

### Commandes Intégrées
- `cd` : Change de répertoire
- `help` : Affiche l'aide sur les commandes intégrées
- `jobs` : Liste les travaux en cours
- `fg` : Ramène un travail au premier plan
- `bg` : Reprend un travail en arrière-plan
- `exit` / `quit` : Quitte le shell

### Interface Utilisateur
- Invite de commande personnalisée avec affichage colorisé
- Messages d'état avec codes couleur pour meilleure lisibilité
- Gestion correcte des signaux d'interruption (Ctrl+C, Ctrl+Z)

## Architecture

Le projet suit une architecture modulaire basée sur les responsabilités :

- **main.c** : Boucle principale, gestion des signaux
- **shell.c** : Logique centrale, exécution des commandes et gestion des jobs
- **readcmd.c** : Analyse syntaxique des lignes de commande
- **match.c** : Gestion des patterns et des expansions
- **csapp.c** : Wrappers pour les appels système (error handling)

### Gestion des Signaux

Le shell installe des gestionnaires pour :
- **SIGCHLD** : Détecte la fin des processus enfants
- **SIGINT** (Ctrl+C) : Transmet au processus de premier plan
- **SIGTSTP** (Ctrl+Z) : Suspend le processus de premier plan

## Installation et Compilation

### Prérequis
- GCC ou clang
- Make
- Bibliothèques POSIX standard (pthread)

### Compilation

```bash
# Compiler le projet
make

# Nettoyer les fichiers objets
make clean

# Nettoyer tous les fichiers générés
make fclean
```

L'exécutable est généré dans le répertoire `bin/shell`.

## Utilisation

### Lancer le shell

```bash
./bin/shell
```

### Exemples de Commandes

```bash
# Commandes basiques
$ ls -la
$ pwd
$ echo "Hello, World!"

# Exécution en arrière-plan
$ sleep 100 &
$ find / -name "*.txt" &

# Pipelines
$ ls | grep ".c"
$ cat file.txt | wc -l

# Redirection
$ echo "test" > output.txt
$ cat < input.txt > output.txt

# Gestion des jobs
$ jobs
$ fg %1
$ bg %2
$ stop %1
```

## Commandes Intégrées

### cd
Change le répertoire courant.

```bash
$ cd /home/user
$ cd ..
$ cd    # Va au répertoire home
```

### help
Affiche la liste des commandes intégrées disponibles.

```bash
$ help
```

### jobs
Liste tous les travaux (processus) actuellement gérés par le shell.

```bash
$ jobs
[1] Running    ls -la
[2] Stopped    vim file.txt
```

### fg (Foreground)
Ramène un travail au premier plan ou reprend un travail stoppé.

```bash
$ fg %1
$ fg    # Reprend le dernier travail suspendu
```

### bg (Background)
Reprend un travail stoppé en arrière-plan.

```bash
$ bg %1
```

## Gestion des Signaux

| Signal | Combinaison Clavier | Comportement |
|--------|-------------------|-------------|
| SIGINT | Ctrl+C | Interrompt le processus de premier plan |
| SIGTSTP | Ctrl+Z | Suspend le processus de premier plan |
| SIGCHLD | — | Détecte la termination des processus enfants |

## Développement

### Compilation en Mode Debug

```bash
make CFLAGS="-Wall -g -O0"
```

### Exécution sous GDB

```bash
gdb ./bin/shell
```

## Limitations Connues

- Nombre maximal de jobs limité à 10 (`MAXJOBS`)
- Longueur maximale des pipelines : 16 commandes (`MAX_PIPELINE`)
- Pas d'historique de commandes
- Pas de complétion automatique

## Licence

Ce projet est basé sur le code original de Simon Nieuviarts (2002) et est fourni à titre éducatif.

---

**Auteur Original** : Simon Nieuviarts  
**Contexte** : Cours de Programmation Système et Réseau (L3)
