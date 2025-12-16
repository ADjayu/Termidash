# Termidash

A modern, cross-platform command-line shell written in C++17 with support for Windows, Linux, and macOS.

## Features

### 🖥️ Cross-Platform Support
- **Windows**: Full support with native Windows API integration
- **Linux/macOS**: Unix-compatible implementation

### 📝 Interactive Shell
- **Command History**: Persistent history saved to `~/.termidash_history`
- **Tab Completion**: Dynamic completion with fuzzy matching (LCS)
- **Startup Configuration**: Load `.termidashrc` from home directory
- **Safe Mode**: Run with `--safe-mode` to block dangerous commands

### 🔒 Security Features
- Input sanitization (removes control characters)
- Path traversal detection
- Password masking in history
- Safe mode blocks: `rm`, `del`, `format`, `sudo`, etc.

### 📊 Logging
Logs are stored in OS-standard locations:
- **Windows**: `%APPDATA%\Termidash\logs\`
- **macOS**: `~/Library/Logs/Termidash/`
- **Linux**: `~/.local/share/termidash/logs/`

### 🔄 Control Flow
```bash
if test condition
  echo "true"
else
  echo "false"
end

while test condition
  echo "looping"
end

for item in apple banana cherry
  echo $item
end
```

### 📦 Pipelines & Operators
- `cmd1 | cmd2` - Standard pipes
- `cmd1 |> cmd2` - Trim pipe (removes whitespace)
- `;` `&&` `||` - Command chaining

### 📁 I/O Redirection
```bash
cmd < input.txt       # Input
cmd > output.txt      # Output (overwrite)
cmd >> output.txt     # Output (append)
cmd 2> error.txt      # Stderr
cmd &> all.txt        # Both stdout and stderr
```

### ⚙️ Background Jobs
```bash
long_task &           # Run in background
jobs                  # List jobs
fg %1                 # Foreground
bg %1                 # Background
```

### 🏷️ Aliases & Variables
```bash
alias ll='ls -la'
NAME=value && echo $NAME
unset NAME
```

### 🔢 Arithmetic & Functions
```bash
echo $((5 + 3))       # Output: 8

function greet
  echo "Hello, $1!"
end
greet World
```

## Building

### Requirements
- CMake 3.16+
- C++17 compiler (MSVC, GCC, Clang)
- Git (for dependency fetching)

### Automated Build (Recommended)

**Windows (PowerShell):**
```powershell
.\build.ps1 build     # Build project
.\build.ps1 test      # Run tests
.\build.ps1 package   # Create ZIP package
.\build.ps1 all       # Full pipeline
.\build.ps1 clean     # Clean build
```

**macOS / Linux:**
```bash
chmod +x build.sh     # Make executable (first time)
./build.sh build      # Build project
./build.sh test       # Run tests
./build.sh package    # Create package (DMG/DEB/RPM)
./build.sh all        # Full pipeline
./build.sh clean      # Clean build
```

**Debug builds:**
```powershell
.\build.ps1 build -BuildType Debug    # Windows
```
```bash
BUILD_TYPE=Debug ./build.sh build     # macOS/Linux
```

### Manual Build
```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build --config Release
```

### Creating Packages
```bash
cd build
cpack -G ZIP      # Windows
cpack -G DragNDrop # macOS (DMG)
cpack -G DEB      # Linux (Debian)
cpack -G RPM      # Linux (Red Hat)
```

## Command Line Options

```
Usage: termidash [options] [script_file]

Options:
  -c <command>    Execute a single command and exit
  --safe-mode     Run in safe mode (blocks dangerous commands)
  --help, -h      Show help message
  --version, -v   Show version information
```

## Testing

**Test Coverage**: 223 unit tests covering:
- Expression evaluator, Variable/Alias/Function managers
- Parser, Completion engine, Control flow handler
- Command substitution, Brace/Glob expansion
- Prompt engine, Security utilities

```bash
.\build.ps1 test              # Windows
./build.sh test               # macOS/Linux
```

## Configuration

### `.termidashrc`
```bash
# ~/.termidashrc
alias ll='dir /w'
NAME=Termidash
echo "Welcome to $NAME!"
```

### Command History
Saved to `~/.termidash_history` and persists across sessions.

## Built-in Commands

| Command | Description |
|---------|-------------|
| `help` | Display available commands |
| `clear` / `cls` | Clear the screen |
| `exit` | Exit the shell |
| `pwd` / `cd` | Working directory |
| `echo` / `cat` | Output text/files |
| `history` | Command history |
| `alias` / `unalias` | Manage aliases |

See `help` command for full list.

## License

MIT License - see [LICENSE](LICENSE) file.

## Version

Termidash Shell Version 1.0.0
