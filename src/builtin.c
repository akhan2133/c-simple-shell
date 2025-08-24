#define _POSIX_C_SOURCE 200809L

#include "msgs.h"
#include "shell.h"
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char last_dir[PATH_MAX];

int handle_builtin(int argc, char **argv) {
  if (strcmp(argv[0], "exit") == 0) {
    if (argc > 1) {
      write(STDERR_FILENO, FORMAT_MSG("exit", TMA_MSG),
            strlen(FORMAT_MSG("exit", TMA_MSG)));
      return 1;
    }
    _exit(0);
  }
  if (strcmp(argv[0], "pwd") == 0) {
    if (argc > 1) {
      write(STDERR_FILENO, FORMAT_MSG("pwd", TMA_MSG),
            strlen(FORMAT_MSG("pwd", TMA_MSG)));
    } else {
      char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) == NULL) {
        write(STDERR_FILENO, FORMAT_MSG("pwd", GETCWD_ERROR_MSG),
              strlen(FORMAT_MSG("pwd", GETCWD_ERROR_MSG)));
      } else {
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "\n", 1);
      }
    }
    return 1;
  }
  if (strcmp(argv[0], "cd") == 0) {
    // Too many args
    if (argc > 2) {
      const char *msg = FORMAT_MSG("cd", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
      return 1;
    }

    // Save current directory so we can swap it on success
    char oldcwd[PATH_MAX] = "";
    if (getcwd(oldcwd, sizeof(oldcwd)) == NULL) {
      oldcwd[0] = '\0';
    }

    const char *target = NULL;
    if (argc == 1) {
      // no arg => $HOME
      struct passwd *pw = getpwuid(getuid());
      target = pw ? pw->pw_dir : NULL;
    } else if (strcmp(argv[1], "-") == 0) {
      // "cd -"
      target = (last_dir[0] ? last_dir : NULL);
    } else if (argv[1][0] == '~') {
      // "~" or "~/..."
      struct passwd *pw = getpwuid(getuid());
      if (pw) {
        static char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s%s", pw->pw_dir,
                 (argv[1][1] == '/' ? argv[1] + 1 : ""));
        target = buf;
      }
    } else {
      // literal path
      target = argv[1];
    }
    if (!target || chdir(target) < 0) {
      const char *err = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
      write(STDERR_FILENO, err, strlen(err));
    } else {
      // Swap in oldcwd for next "cd -"
      if (oldcwd[0]) {
        strncpy(last_dir, oldcwd, PATH_MAX);
        last_dir[PATH_MAX - 1] = '\0';
      }
    }
    return 1;
  }

  if (strcmp(argv[0], "help") == 0) {
    // too many args
    if (argc > 2) {
      const char *msg = FORMAT_MSG("help", TMA_MSG);
      write(STDERR_FILENO, msg, strlen(msg));
    } else if (argc == 2) {
      // help
      const char *cmd = argv[1];
      if (strcmp(cmd, "exit") == 0) {
        const char *exit_msg = FORMAT_MSG("exit", EXIT_HELP_MSG);
        write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
      } else if (strcmp(cmd, "pwd") == 0) {
        const char *pwd_msg = FORMAT_MSG("pwd", PWD_HELP_MSG);
        write(STDOUT_FILENO, pwd_msg, strlen(pwd_msg));
      } else if (strcmp(cmd, "cd") == 0) {
        const char *cd_msg = FORMAT_MSG("cd", CD_HELP_MSG);
        write(STDOUT_FILENO, cd_msg, strlen(cd_msg));
      } else if (strcmp(cmd, "help") == 0) {
        const char *help_msg = FORMAT_MSG("help", HELP_HELP_MSG);
        write(STDOUT_FILENO, help_msg, strlen(help_msg));
      } else if (strcmp(cmd, "history") == 0) {
        const char *history_msg = FORMAT_MSG("history", HISTORY_HELP_MSG);
        write(STDOUT_FILENO, history_msg, strlen(history_msg));
      } else {
        write(STDOUT_FILENO, cmd, strlen(cmd));
        write(STDOUT_FILENO, ": ", 2);
        write(STDOUT_FILENO, EXTERN_HELP_MSG, strlen(EXTERN_HELP_MSG));
        write(STDOUT_FILENO, "\n", 1);
      }
    } else {
      // help with no args => list all
      const char *all =
          FORMAT_MSG("exit", EXIT_HELP_MSG) FORMAT_MSG("pwd", PWD_HELP_MSG)
              FORMAT_MSG("cd", CD_HELP_MSG) FORMAT_MSG("help", HELP_HELP_MSG)
                  FORMAT_MSG("history", HISTORY_HELP_MSG);
      write(STDOUT_FILENO, all, strlen(all));
    }
    return 1;
  }

  if (strcmp(argv[0], "history") == 0) {
    print_history();
    return 1;
  }

  return 0;
}
