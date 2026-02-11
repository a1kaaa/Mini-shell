#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "match.h"
#include "stdio.h"

/* Recursive pattern matching supporting '*' wildcard */
static int match(const char *pattern, const char *candidate, int p, int c)
{
    if (pattern[p] == '\0' && candidate[c] == '\0')
        return 1;
    if (pattern[p] == '\0' || candidate[c] == '\0') {
        /* '*' can match empty string */
        if (pattern[p] == '*' && pattern[p + 1] == '\0')
            return 1;
        return 0;
    }
    if (pattern[p] == '*') {
        /* Try matching zero or more characters */
        for (int i = c; i <= (int)strlen(candidate); i++) {
            if (match(pattern, candidate, p + 1, i))
                return 1;
        }
        return 0;
    }
    if (pattern[p] != candidate[c])
        return 0;
    return match(pattern, candidate, p + 1, c + 1);
}

int list_dir(const char *dir, char ***content, int *size)
{
    DIR *d = opendir(dir);
    if (!d)
        return 1;

    struct dirent *entry;
    *content = NULL;
    *size = 0;
    while ((entry = readdir(d))) {
        if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, "."))
            continue;
        char **tmp = realloc(*content, sizeof(char *) * (*size + 1));
        if (!tmp) {
            for (int i = 0; i < *size; i++)
                free((*content)[i]);
            free(*content);
            closedir(d);
            return 1;
        }
        *content = tmp;
        (*content)[*size] = strdup(entry->d_name);
        (*size)++;
    }
    closedir(d);
    return 0;
}

int match_pattern(const char *pattern, char **candidates, int n, char ***selected, int *size)
{
    *selected = NULL;
    *size = 0;

    for (int i = 0; i < n; i++) {
        if (match(pattern, candidates[i], 0, 0)) {
            char **tmp = realloc(*selected, sizeof(char *) * (*size + 1));
            if (!tmp) {
                for (int j = 0; j < *size; j++)
                    free((*selected)[j]);
                free(*selected);
                return 1;
            }
            *selected = tmp;
            (*selected)[*size] = strdup(candidates[i]);
            (*size)++;
        }
    }
    return 0;
}