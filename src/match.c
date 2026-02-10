#include "match.h"
#include "stdlib.h"
#include "stdio.h"
#include <dirent.h>


int list_dir(const char *dir, char ***content, int *size) 
{
    DIR *d = opendir(dir);
    struct dirent *dir_content;
    *size = 0;
    while((dir_content = readdir(d)))
    {
        if(!(*size)) 
        {
            *content = malloc(sizeof(char *));
            if(!(*content)) {
                fprintf(stderr, "Erreur d'allocation: list dir impossible");
                return 1;
            }
            *content[(*size)++] = dir_content->d_name;
        } else 
        {
            char **tmp = realloc(*content, (*size) + 1);
            if(!tmp) {
                free(*content);
                fprintf(stderr, "Erreur d'allocation: list dir impossible");
                return 1;
            }
            *content = tmp;
            *content[*(size)++] = dir_content->d_name;
        }
    }
    return 0;
}

int match(const char *pattern, const char *candidate, int p, int c) 
{
    if (pattern[p] == '\0') 
    {
        return candidate[c] == '\0';
    } else if (pattern[p] == '*') {
        for (; candidate[c] != '\0'; c++) 
        {
            if (match(pattern, candidate, p+1, c))
                return 1;
        }
        return match(pattern, candidate, p+1, c);
    } else if (pattern[p] != '?' && pattern[p] != candidate[c]) 
    {
        return 0;
    }  else 
    {
        return match(pattern, candidate, p+1, c+1);
    }
}

int match_pattern(const char *pattern, char **candidates, int n, char ***selected, int *size) 
{
    *selected = malloc(sizeof(char*)*n);
    if(!selected) 
    {
        fprintf(stderr, "Erreur d'allocation: matching patern impossible");
        return 1;
    }
    *size = 0;
    for(int i = 0; i < n; i++) 
    {
        if(match(pattern, candidates[i], 0, 0))
            *selected[(*size)++] = candidates[i];
    }
    return 0;
}
