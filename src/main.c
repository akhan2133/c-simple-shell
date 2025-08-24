#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "msgs.h"
#include "shell.h"

void handle_sigint(int sig) {
  const char *all = "exit: " EXIT_HELP_MSG "\n"
                    "pwd: " PWD_HELP_MSG "\n"
                    "cd: " CD_HELP_MSG "\n"
                    "help: " HELP_HELP_MSG "\n"
                    "history: " HISTORY_HELP_MSG "\n";
  write(STDOUT_FILENO, "\n", 1);
  write(STDOUT_FILENO, all, strlen(all));

  // Re-display prompt
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd))) {
    write(STDOUT_FILENO, cwd, strlen(cwd));
    write(STDOUT_FILENO, "$ ", 2);
  } else {
    write(STDOUT_FILENO, "$ ", 2);
  }
}

int main(void) {
  char cwd[PATH_MAX];
  char line[PATH_MAX];
  char *argv[MAX_TOKENS];
  int bg;

  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigint");
    exit(1);
  }

  while (1) {
    // reap any exited background children
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }

    // print prompt: cwd
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
      // on error, print to stderr
      write(STDERR_FILENO, "shell: " GETCWD_ERROR_MSG "\n",
            strlen("shell: " GETCWD_ERROR_MSG "\n"));
    } else {
      write(STDOUT_FILENO, cwd, strlen(cwd));
      write(STDOUT_FILENO, "$ ", 2);
    }

    // read one line from stdin
    ssize_t n = read(STDIN_FILENO, line, sizeof(line) - 1);
    if (n <= 0) {
      // EOF or read error => exit shell
      break;
    }
    line[n] = '\0';

    // strip trailing newline so history expand sees "!!" or "!0"
    if (n > 0 && line[n - 1] == '\n') {
      line[n - 1] = '\0';
    }

    // history expansion
    char *expanded = history_expand(line);
    if (line[0] == '!' && expanded == NULL) {
      continue;
    }

    const char *cmd = expanded ? expanded : line;

    if (expanded) {
      write(STDOUT_FILENO, cmd, strlen(cmd));
      write(STDOUT_FILENO, "\n", 1);
    }

    // prep history copy
    size_t len = strlen(cmd);
    char *hist_copy = malloc(len + 2);
    strcpy(hist_copy, cmd);
    if (len == 0 || cmd[len - 1] != '\n') {
      hist_copy[len] = '\n';
      hist_copy[len + 1] = '\0';
    }

    // make a safe copy for parsing
    char *cmd_copy = strdup(cmd);

    // parse
    int argc = parse_line(cmd_copy, argv, &bg);

    if (argc == 0) {
      free(cmd_copy);
      free(hist_copy);
      free(expanded);
      continue;
    }

    // update history and free
    add_history(hist_copy);
    free(hist_copy);
    free(expanded);

    // built in command
    // handle_builtin returns 1 if it recognized and handled it
    if (handle_builtin(argc, argv)) {
      continue;
    }

    // external commands
    pid_t pid = fork();
    if (pid < 0) {
      write(STDERR_FILENO, "shell: " FORK_ERROR_MSG "\n",
            strlen("shell: " FORK_ERROR_MSG "\n"));
    } else if (pid == 0) {

      execvp(argv[0], argv);
      // if exec fails
      write(STDERR_FILENO, "shell: " EXEC_ERROR_MSG "\n",
            strlen("shell: " EXEC_ERROR_MSG "\n"));
      _exit(1);
    } else {
      if (!bg) {
        if (waitpid(pid, NULL, 0) < 0) {
          write(STDERR_FILENO, "shell: " WAIT_ERROR_MSG "\n",
                strlen("shell :" WAIT_ERROR_MSG "\n"));
        }
      }
    }
    free(cmd_copy);
  }
  return 0;
}
