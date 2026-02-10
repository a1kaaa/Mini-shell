#ifndef __MATCH_H__
#define __MATCH_H__

int match_pattern(const char *pattern, char **candidates, int n, char ***selected, int *size);
int list_dir(const char *dir, char ***content, int *size);

#endif