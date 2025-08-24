# Simple Shell (C)

A Unix-like shell written in **C**, supporting process management, built-in commands, signal handling, and a history feature.  
Implements background execution, custom error handling, and re-running past commands from history.

---

## Features

### Process Management
- Executes external programs in **separate processes** via `fork()` and `exec()`.  
- Foreground execution: parent waits until child finishes.  
- Background execution (`&`): parent does not wait; cleans up zombies with `waitpid()` loop.  
- Errors handled gracefully with custom messages.

### Internal Commands
- `exit` — terminates the shell (with argument checks).  
- `pwd` — prints current directory (error if extra args).  
- `cd` — changes directory; supports:
  - `cd` → home directory  
  - `cd ~` → expand home  
  - `cd -` → previous directory  
- `help` — lists supported internal commands or details on a specific command.  

### Signal Handling
- Custom handler for **SIGINT (Ctrl-C)**:
  - Prevents shell termination.  
  - Displays help info.  
  - Reprints the prompt before continuing.  

### History
- Tracks the **10 most recent commands**, numbered sequentially.  
- `history` — lists recent commands with numbers.  
- `!!` — re-runs the last command.  
- `!n` — re-runs command number `n` if within history window.  
- Handles invalid indices or empty history with error messages.  
- History includes both external and internal commands.  

---

## Skills Demonstrated
- C systems programming with **fork, exec, waitpid, signals**.  
- Process control and background execution.  
- String parsing (`strtok_r`), command dispatching.  
- Building internal commands and stateful shell features.  
- Maintaining command history in memory.  
- Using **CMake** and **Google Test** for structured builds and testing.  

---

## Build & Run
```bash
# Build
mkdir build && cd build
cmake ..
make

# Run the shell
./shell
```

---

## Example Usage
```bash
/home/user$ pwd
/home/user
/home/user$ cd /tmp
/tmp$ ls &
/tmp$ history
3   history
2   ls &
1   cd /tmp
0   pwd
/tmp$ !!
history
3   history
2   ls &
1   cd /tmp
0   pwd
```

---

## Future Improvements
- Add output redirection (>)
- Add pipes (|)
- Expand history beyond 10 commands with file persistence

--- 

## License
This project is licensed under the [MIT License](LICENSE).
