# Dirvana

**An intelligent directory navigation and command augmentation tool for Zsh**

Dirvana (`dv`) streamlines terminal workflow by providing smart directory navigation with fuzzy matching, intelligent autocompletion, and customizable shortcuts. Navigate to deeply nested directories instantly, execute commands with path completion, and create shortcuts for your most-used commands.

## ✨ Key Features

- **🚀 Smart Navigation** - Jump to any directory with partial matching (exact, prefix, suffix, or contains)
- **⚡ Quick-Nav** - Instantly navigate to the best match by pressing Enter
- **🎯 Tab Completion** - Interactive menu-based path completion integrated with Zsh
- **🔗 Custom Shortcuts** - Create command aliases that work with path completion
- **🧠 Learning Algorithm** - Adapts to your usage patterns with frequency-based and recency-based ranking
- **⚙️ Highly Configurable** - Customize matching behavior, exclusions, and result limits
- **💾 Persistent History** - SQLite-backed database remembers your navigation patterns
- **🔄 Auto-Refresh** - Keeps your directory database up-to-date automatically

---

## 📦 Installation

### Automatic Installation (Recommended)

Install Dirvana with a single command:

```sh
curl -fsSL https://jlkendrick.github.io/dirvana/docs/install.sh | bash
```

The installer will:
- Download the latest binary to `~/.local/bin`
- Set up Zsh completion scripts
- Configure your `.zshrc` automatically
- Initialize the directory database

**Note:** Currently supports macOS only. Linux support coming soon.

---

### Manual Installation

If you prefer manual installation or need more control:

#### 1️⃣ Download the Binary

Download or build the `dv-binary` executable and move it to a directory in your `PATH`:

```sh
# Download from the repository
curl -fsSL -o ~/.local/bin/dv-binary https://raw.githubusercontent.com/jlkendrick/dirvana/main/docs/bin/dv-binary

# Make it executable
chmod +x ~/.local/bin/dv-binary

# Ensure ~/.local/bin is in your PATH
export PATH="$HOME/.local/bin:$PATH"
```

#### 2️⃣ Install Zsh Completion Script

Create the completion directory and download the completion script:

```sh
# Create completions directory
mkdir -p ~/.zsh/completions

# Download completion script
curl -fsSL -o ~/.zsh/completions/_dv https://raw.githubusercontent.com/jlkendrick/dirvana/main/docs/scripts/_dv
```

Or manually create `~/.zsh/completions/_dv` with the following content:

```sh
#compdef dv

_dv() {
  local completions
  completions=("${(@f)$(dv-binary --tab "${words[@]}")}")

  compadd -Q -U -V 'Available Options' -- "${completions[@]}"
}
```

#### 3️⃣ Configure Zsh

Add the following to your `~/.zshrc`:

```sh
# Dirvana Zsh completion configuration
fpath=(~/.zsh/completions $fpath)

zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'

setopt menucomplete
setopt autolist

autoload -Uz compinit && compinit -u

# Dirvana command handler
dv() {
  local cmd
  cmd=$(dv-binary --enter dv "$@")

  if [[ -n "$cmd" ]]; then
    eval "$cmd"
  else
    echo "dv-error: No command found for '$*'"
  fi
}

# Auto-refresh database on terminal start
dv-binary --enter dv refresh &> /dev/null & disown
```

#### 4️⃣ Initialize Database

Reload your shell configuration and build the initial database:

```sh
source ~/.zshrc
dv build --root ~
```

---

## 🚀 Usage

### Basic Navigation

Navigate to directories using partial path matching:

```sh
# Tab for completion menu
dv project<Tab>          # Shows all matches for "project"
dv path/to/project       # Navigate to specific path
dv project<Enter>        # Quick-nav to best match

# Navigate with partial matches
dv docs<Enter>           # Jumps to first match containing "docs"
dv proj<Enter>           # Jumps to first match containing "proj"

# No arguments returns to home
dv<Enter>                # cd ~
```

### Command Execution

Prefix any command with `dv` to get path completion:

```sh
# Open directory in VS Code
dv code project<Tab>     # Autocomplete project path
dv code project<Enter>   # Quick-nav and open

# List directory contents
dv ls documents<Enter>   # ls /path/to/documents

# Copy directories
dv cp -r source<Tab>     # Autocomplete source
dv cp -r /path/to/source dest<Tab>  # Then autocomplete destination
```

### Shortcuts System

Create custom shortcuts that integrate with path completion:

#### Add a Shortcut

```sh
dv add <shortcut> <command>

# Examples
dv add code "code"       # Open in VS Code
dv add idea "idea"       # Open in IntelliJ IDEA
dv add term "open -a Terminal"  # Open in new Terminal window
```

#### Use a Shortcut

Shortcuts work with path completion:

```sh
dv code project<Enter>   # Expands to: code /path/to/project
dv idea backend<Enter>   # Expands to: idea /path/to/backend
dv term logs<Tab>        # Autocomplete then open in new terminal
```

#### List All Shortcuts

```sh
dv list
# Output:
# Shortcuts:
# code | code
# idea | idea
# term | open -a Terminal
```

#### Show Specific Shortcut

```sh
dv show code
# Output: Shortcut: code | Command: code
```

#### Delete a Shortcut

```sh
dv delete code
# Output: Shortcut code deleted
```

### Database Management

#### Build Database

Scans directories and creates a fresh database (resets history):

```sh
dv build                 # Build from configured root
dv build --root ~/Code   # Build from specific directory
```

#### Refresh Database

Updates the database with new/deleted directories (preserves history):

```sh
dv refresh               # Refresh from configured root
dv refresh --root ~/Code # Refresh from specific directory
```

**Note:** A refresh runs automatically when you start a new terminal session.

#### Update Dirvana

Install the latest version:

```sh
dv install               # Install latest version
```

### Utility Features

#### Check Version

```sh
dv --version
dv -v
# Output: Dirvana version 1.0.1
```

#### Bypass Dirvana Commands

Use `--` to bypass Dirvana's command interpretation:

```sh
dv -- build              # Navigate to a directory named "build"
dv -- refresh            # Navigate to a directory named "refresh"
```

---

## ⚙️ Configuration

Dirvana's configuration file is located at:
- **macOS:** `~/Library/Application Support/dirvana/config.json`

### Configuration Options

```json
{
  "paths": {
    "init": "/Users/you",
    "db": "/Users/you/Library/Application Support/dirvana/dirvana.db"
  },
  "matching": {
    "max_results": 10,
    "max_history_size": 100,
    "type": "contains",
    "promotion_strategy": "recently_accessed",
    "exclusions": {
      "exact": ["node_modules", "dist", "target", ".git"],
      "prefix": ["."],
      "suffix": ["sdk", "Library"],
      "contains": ["release"]
    }
  }
}
```

### Configuration Reference

#### Paths

| Option | Description | Default |
|--------|-------------|---------|
| `init` | Root directory for database scanning | `~/` |
| `db` | SQLite database location | `~/Library/Application Support/dirvana/dirvana.db` |

#### Matching

| Option | Type | Description | Options/Default |
|--------|------|-------------|-----------------|
| `max_results` | integer | Maximum completions to show | Default: `10` |
| `max_history_size` | integer | Maximum history entries to track | Default: `100` |
| `type` | string | How to match directory names | `exact`, `prefix`, `suffix`, `contains` (default) |
| `promotion_strategy` | string | How to rank results | `recently_accessed` (default), `frequency_based` |

#### Matching Types

- **`exact`** - Only matches directories with the exact name
- **`prefix`** - Matches directories starting with the query
- **`suffix`** - Matches directories ending with the query
- **`contains`** - Matches directories containing the query (substring match)

#### Promotion Strategies

- **`recently_accessed`** - Prioritizes recently visited directories
- **`frequency_based`** - Prioritizes frequently visited directories

#### Exclusions

Specify directories to exclude from the database. Supports four matching patterns:

```json
{
  "exclusions": {
    "exact": ["node_modules", "dist"],        // Exact name match
    "prefix": [".", "tmp_"],                   // Starts with
    "suffix": ["_backup", ".cache"],          // Ends with
    "contains": ["deprecated", "old"]         // Contains substring
  }
}
```

**Default exclusions:** `.git`, `node_modules`, `browser_components`, `dist`, `out`, `target`, `tmp`, `temp`, `cache`, `venv`, `env`, `obj`, `pkg`, `bin`, and directories starting with `.`

---

## 💡 Tips & Best Practices

### Optimal Root Directory

Choose a root directory that:
- Contains all directories you want to navigate to
- Isn't too broad (avoid scanning entire filesystem)
- Excludes system directories

```sh
# Good choices
dv build --root ~/Code
dv build --root ~/Documents

# Less optimal
dv build --root /  # Too many directories, slow performance
```

### Matching Type Selection

- **`contains`** (default) - Most flexible, finds directories anywhere in the tree
- **`prefix`** - Faster, better for consistent naming schemes
- **`exact`** - Most precise, use when you know exact directory names
- **`suffix`** - Useful for projects with naming patterns (e.g., `-api`, `-web`)

### Effective Shortcuts

Create shortcuts for frequently used commands:

```sh
dv add code "code"
dv add open "open"
dv add cd "cd"  # Use shortcuts even for navigation
dv add rm "rm -rf"
dv add mv "mv"
dv add cp "cp -r"
```

### Performance

- Use exclusions to prevent scanning large irrelevant directories
- Run `dv refresh` periodically or let it auto-refresh on terminal start
- Adjust `max_results` based on your screen size
- Set appropriate `max_history_size` (larger = better predictions, more storage)

---

## 🏗️ Building from Source

Requirements:
- CMake 3.16+
- Clang compiler
- C++20 support

```sh
# Clone the repository
git clone https://github.com/jlkendrick/dirvana.git
cd dirvana

# Build
mkdir build && cd build
cmake ..
make

# Binary will be at build/dv-binary
```

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

---

## 📝 License

This project is open source and available under the MIT License.

---

## 🔧 Troubleshooting

### Completion not working

1. Ensure `~/.local/bin` is in your `PATH`
2. Verify `fpath` includes `~/.zsh/completions`
3. Run `compinit` to rebuild completion cache
4. Check that `dv-binary` is executable: `chmod +x ~/.local/bin/dv-binary`

### Database not updating

1. Run `dv refresh` manually
2. Check root directory in config: `cat ~/Library/Application\ Support/dirvana/config.json`
3. Rebuild database: `dv build --root ~/your/root/path`

### Permission errors

1. Ensure proper permissions on config directory:
   ```sh
   chmod 755 ~/Library/Application\ Support/dirvana
   chmod 644 ~/Library/Application\ Support/dirvana/config.json
   ```

### Command not found

1. Verify binary location: `which dv-binary`
2. Add to PATH in `.zshrc`: `export PATH="$HOME/.local/bin:$PATH"`
3. Reload shell: `source ~/.zshrc`

---

**Built with ❤️ for faster terminal navigation**
