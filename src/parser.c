#define _POSIX_C_SOURCE 200809L // for strtok_r
#include "shell.h"
#include <ctype.h>
#include <string.h>

int parse_line(char *line, char **argv, int *bg) {
  char *p;
  size_t len;
  int argc = 0;

  // strip trailing newline if any
  p = strchr(line, '\n');
  if (p != NULL) {
    *p = '\0';
  }

  // default: no background
  *bg = 0;

  // trim trailing whitespace
  len = strlen(line);
  while (len > 0 && isspace((unsigned char)line[len - 1])) {
    line[--len] = '\0';
  }

  // detect ampersand at end => background
  if (len > 0 && line[len - 1] == '&') {
    *bg = 1;
    line[--len] = '\0';

    // trim spaces before ampersand
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
      line[--len] = '\0';
    }
  }

  // tokenize on ASCII whitespace
  char *saveptr = NULL;
  char *tok = strtok_r(line, " \t\r\n", &saveptr);
  while (tok != NULL && argc < MAX_TOKENS - 1) {
    argv[argc++] = tok;
    tok = strtok_r(NULL, " \t\r\n", &saveptr);
  }
  argv[argc] = NULL;

  return argc;
}
