#define _POSIX_C_SOURCE 200809L
#include "msgs.h"
#include "shell.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char hist_buf[MAX_HISTORY][MAX_LINE];
static int hist_count = 0; // total commands seen
static int hist_next = 0;  // next slot to overwrite (0 to 9)

void add_history(const char *line) {
  if (line == NULL || *line == '\0') {
    return;
  }

  // Make a local copy to modify
  char temp[MAX_LINE];
  strncpy(temp, line, MAX_LINE - 1);
  temp[MAX_LINE - 1] = '\0';

  // Strip trailing newline
  size_t len = strlen(temp);
  if (len > 0 && temp[len - 1] == '\n') {
    temp[len - 1] = '\0';
  }

  // Copy cleaned line into ring buffer
  strncpy(hist_buf[hist_next], temp, MAX_LINE - 1);
  hist_buf[hist_next][MAX_LINE - 1] = '\0';

  hist_next = (hist_next + 1) % MAX_HISTORY;
  hist_count++;
}

void print_history(void) {
  int start = hist_count > MAX_HISTORY ? hist_count - MAX_HISTORY : 0;
  int end = hist_count - 1;
  char buf[MAX_LINE + 32];
  for (int num = end; num >= start; num--) {
    int idx = num % MAX_HISTORY;
    int len = snprintf(buf, sizeof(buf), "%d\t%s\n", num, hist_buf[idx]);
    write(STDOUT_FILENO, buf, len);
  }
}

char *history_expand(const char *spec) {
  if (!spec || spec[0] != '!') {
    return NULL;
  }

  // !! => last command
  if (spec[1] == '!' && spec[2] == '\0') {
    if (hist_count == 0) {
      const char *err = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);
      write(STDERR_FILENO, err, strlen(err));
      return NULL;
    }
    int last = hist_count - 1;
    int idx = last % MAX_HISTORY;
    return strdup(hist_buf[idx]);
  }

  // !n => specific history #
  char *endptr;
  long n = strtol(spec + 1, &endptr, 10);
  if (*endptr != '\0' || n < 0 || n >= hist_count ||
      n < hist_count - MAX_HISTORY) {
    const char *err = FORMAT_MSG("history", HISTORY_INVALID_MSG);
    write(STDERR_FILENO, err, strlen(err));
    return NULL;
  }
  int idx = n % MAX_HISTORY;
  return strdup(hist_buf[idx]);
}
