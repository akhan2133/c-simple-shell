#ifndef SHELL_H
#define SHELL_H

#define MAX_TOKENS 64
#define MAX_LINE 1024
#define MAX_HISTORY 10

// Parse input line into argv[], detect ampersand for background.
int parse_line(char *line, char **argv, int *bg);

// Returns 1 if argv[0] was a built-in (and was handled), 0 otherwise.
int handle_builtin(int argc, char **argv);

// History API: record, print, and expand commands
void add_history(const char *line);
void print_history(void);
char *history_expand(const char *spec);

#endif
