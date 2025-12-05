# Termidash

A modern, cross-platform command-line shell written in C++17 with support for Windows and Linux/macOS.

## Features

### 🖥️ Cross-Platform Support
- **Windows**: Full support with native Windows API integration
- **Linux/macOS**: Unix-compatible implementation

### 📝 Interactive Shell
- **Command History**: Persistent history saved to `~/.termidash_history`
  - Navigate with Up/Down arrow keys
  - History command to view past commands
- **Tab Completion**: Dynamic completion with intelligent ranking
  - Built-in commands
  - Executables in PATH
  - Files and directories in current path
  - Fuzzy matching using LCS (Longest Common Subsequence)
- **Startup Configuration**: Load `.termidashrc` from home directory on startup

### 🔄 Control Flow
```bash
# If-Else statements
if test condition
  echo "condition true"
else
  echo "condition false"
end

# While loops
while test condition
  echo "looping..."
end

# For loops
for item in apple banana cherry
  echo $item
end
```

### 📦 Pipelines & Operators
- **Standard Pipes**: `cmd1 | cmd2`
- **Trim Pipe**: `cmd1 |> cmd2` - Trims whitespace from output before passing
- **Command Chaining**:
  - `;` - Sequential execution
  - `&&` - Execute next only if previous succeeds
  - `||` - Execute next only if previous fails

### 📁 I/O Redirection
```bash
# Input/Output redirection
cmd < input.txt          # Input redirection
cmd > output.txt         # Output redirection (overwrite)
cmd >> output.txt        # Output redirection (append)
cmd 2> error.txt         # Stderr redirection
cmd 2>> error.txt        # Stderr append
cmd &> all.txt           # Redirect both stdout and stderr
cmd &>> all.txt          # Append both stdout and stderr

# Here-documents
cmd << EOF
multi-line
input
EOF
```

### ⚙️ Background Jobs
```bash
long_running_cmd &       # Run in background
jobs                     # List all background jobs
fg %1                    # Bring job 1 to foreground
bg %1                    # Continue job 1 in background
```

### 🏷️ Aliases
```bash
alias                    # List all aliases
alias ll='dir /w'        # Create an alias
unalias ll               # Remove an alias
```

### 📊 Variables
```bash
NAME=value               # Set a variable
echo $NAME               # Use a variable
unset NAME               # Remove a variable
```

Variables integrate with environment variables - if a shell variable is not found, environment variables are checked.

### 🔢 Arithmetic Expressions
```bash
# Arithmetic expansion
echo $((5 + 3))          # Output: 8
echo $((10 * 2))         # Output: 20
echo $((a + b))          # Variables in expressions

# Arithmetic commands
((count = count + 1))    # Inline arithmetic

# Supported operators: +, -, *, /, ==, !=, <, >, <=, >=
```

### 📜 User-Defined Functions
```bash
# Function definition (method 1)
function greet
  echo "Hello, $1!"
end

# Function definition (method 2)
greet() {
  echo "Hello, $1!"
}

# Function call
greet World              # Output: Hello, World!
```
Functions support positional arguments (`$1`, `$2`, etc.) with proper scoping.

### 📝 Script Execution
```bash
# Run a script file
termidash script.sh

# Run a single command
termidash -c "echo hello"
```
Scripts support comments with `#` at the beginning of lines.

## Built-in Commands

### Common Commands (Cross-Platform)
| Command | Description |
|---------|-------------|
| `help` | Display available commands |
| `clear` | Clear the screen |
| `exit` | Exit the shell |
| `version` | Display shell version |
| `pwd` | Print working directory |
| `cd <dir>` | Change directory |
| `echo <text>` | Print text to output |
| `cat <file>` | Display file contents |
| `touch <file>` | Create a file or update timestamp |
| `rm <file>` | Remove a file |
| `history` | Display command history |
| `alias` | Manage command aliases |
| `unalias <name>` | Remove an alias |
| `unset <var>` | Unset a variable |

### Text Processing
| Command | Description |
|---------|-------------|
| `grep <pattern> <file>` | Search for pattern in file |
| `sort <file>` | Sort lines in a file |
| `head <file>` | Display first 10 lines |
| `tail <file>` | Display last 10 lines |

### Windows-Specific Commands
| Command | Description |
|---------|-------------|
| `cls` | Clear screen |
| `ver` | Show Windows version |
| `dir` | List directory contents |
| `type <file>` | Display file contents |
| `copy <src> <dst>` | Copy a file |
| `del <file>` | Delete a file |
| `mkdir <dir>` | Create a directory |
| `rmdir <dir>` | Remove a directory |
| `drives` | List available drives |
| `cwd` | Display current working directory |
| `getenv <var>` | Get environment variable |
| `setenv <var> <val>` | Set environment variable |
| `tasklist` | List running processes |
| `taskkill <pid/name>` | Kill a process |
| `ping <host>` | Ping a host |
| `ipconfig` | Show network configuration |
| `whoami` | Display current user |
| `hostname` | Display computer name |
| `systeminfo` | Display system information |
| `netstat` | Show network statistics |
| `attrib <file>` | Show file attributes |
| `assoc` | Display file associations |
| `time` | Display current time |
| `date` | Display current date |
| `pause` | Pause and wait for keypress |

## Building

### Requirements
- CMake 3.16 or higher
- C++17 compatible compiler (MSVC, GCC, Clang)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .
```

### Platform-Specific Notes
- **Windows**: Uses native Windows API for terminal, process management, and job control
- **Linux/macOS**: Uses POSIX APIs for Unix compatibility

## Configuration

### `.termidashrc`
Create a `.termidashrc` file in your home directory to run commands on shell startup:

```bash
# ~/.termidashrc example
alias ll='dir /w'
alias cls='clear'
NAME=Termidash
echo "Welcome to $NAME!"
```

### Command History
History is automatically saved to `~/.termidash_history` and persists across sessions.

## Usage Examples

```bash
# Interactive mode
> echo "Hello, World!"
Hello, World!

# Pipelines
> dir | grep ".txt"

# Background jobs
> long_task &
[1] long_task
> jobs
[1] 12345 Running long_task
> fg %1

# Variables and arithmetic
> count=5
> echo $((count * 2))
10

# Control flow
> for f in *.txt
>>   echo "Processing $f"
>> end

# Functions
> function backup
>>   copy $1 $1.bak
>> end
> backup important.txt
```

## License

This project is provided as-is for educational and personal use.

## Version

Termidash Shell Version 1.0.0
