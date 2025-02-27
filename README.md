# Dirvana
**An intelligent directory navigation and command augmentation tool built for Zsh.**  

Dirvana (`dv`) is designed to streamline working with directories in the terminal by providing autocompletion for directory paths. By default, `dv` wraps the `cd` command but when prefixed before another directory command (e.g., `ls`, `rm`, `mv`, etc.), it suggests relevant directory paths, allowing for quick navigation and command execution. The quick-nav feature further accelerates workflows by instantly selecting and executing the command on the best match. See [Usage](#usage) for more details.

---

## Installation

### **1️⃣ Install the Binary**
Dirvana requires its compiled binary (`dv-binary`, which can be found in the `build/` directory of this repo). Download it or build the project and move it to a directory in your `PATH`, such as `/usr/local/bin`:

```sh
# Example: Move to /usr/local/bin (adjust if needed)
sudo mv dv-binary /usr/local/bin
chmod +x /usr/local/bin/dv-binary
# or, use the script provided at 'scripts/move_binary.sh' (make sure to adjust the path if needed)
source scripts/move_binary.sh
```

### **2️⃣ Copy the Zsh Completion Script**
Dirvana also requires a Zsh completion script (`_dv` which can be found in the `scripts/` directory of this repo) to enable autocompletion. To enable it, copy the script to your Zsh completion directory:

```sh
# Example: Copy to ~/.zsh/completions (adjust if needed)
mkdir -p ~/.zsh/completions
cp _dv ~/.zsh/completions
```

or, manually copy the following function into the file you created (~/.zsh/completions/_dv)

```sh
#compdef dv
_dv() {
  local completions
  completions=("${(@f)$(dv-binary -tab "${words[@]}")}")
  
  compadd -U -V 'Available Options' -- "${completions[@]}"
}
```

### **3️⃣ Enable the Completion Script**
Add the completion script to your Zsh configuration file (e.g., `~/.zshrc`) along with the following configuration options:

```sh
# Use the path where you copied the completion script
fpath=(~/.zsh/completions $fpath)

zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'

setopt menucomplete
setopt autolist

autoload -Uz compinit && compinit
```

### **4️⃣ Add the Command Handler**
Add the following function to your Zsh configuration file (e.g., `~/.zshrc`):

```sh
dv() {
  local cmd
  cmd=$(dv-binary -enter dv "$@")

  if [[ -n "$cmd" ]]; then
    eval "$cmd"
  else
    echo "dv-error: No command found for '$*'"
  fi
}

dv-binary -enter dv refresh &> /dev/null & disown
```
Note: to see a sample Zsh configuration file, refer to the `scripts/sample.zshrc` file in this repo.

### **5️⃣ Reload Zsh Configuration**
Reload your Zsh configuration to apply the changes:

```sh
source ~/.zshrc
```

### **6️⃣ Done!**

---

## Usage

You now have access to the `dv` command, here are some examples of how to use it:
```sh
# --------- Navigating to a directory ---------
dv project + 'Tab' = dv path/to/project # Autocompletes to top match and displays a menu of other matches. Consecutive 'Tab' presses cycle through matches.
# After getting the desired directory, press 'Enter' to navigate to it.
dv another/path/to/project + 'Enter' = # Navigates to 'path2/to/src' (executes 'cd path2/to/src').
#            -- or (quick-nav) --
dv project + 'Enter' = # Tries to 'cd' into 'project' (if it exists) or finds the first match and navigates to it (in this case will 'cd' into 'path/to/project').

# ----------- Running a command ---------
# You can also run commands with 'dv' (e.g., 'ls', 'rm', 'mv', etc.)
dv code project + 'Tab' = dv code path/to/project # Autocompletes to top match and displays a menu of other matches. Consecutive 'Tab' presses cycle through matches.
# After getting the desired directory, press 'Enter' to run the command.
dv code another/path/to/project + 'Enter' = # Runs 'code path/to/project' (opens the directory in VSCode).
#            -- or (quick-nav) --
dv code project + 'Enter' = # Tries to 'code' into 'project' (if it exists) or finds the first match and runs the command on it (in this case will open up 'path/to/project' in VSCode).

# --------- Other utility commands ---------
# If you wish, you can rebuild the database from scratch with 'dv rebuild', however, this will reset all your previous history.
dv rebuild + 'Enter' = # Rebuilds the database

# To refresh the database to account for new/deleted directories, use 'dv refresh'. This will not reset your history.
dv refresh + 'Enter' = # Refreshes the database

# If needed, you can also bypass commands by using '--'. Below is one example but you can use it with any command.
dv -- rebuild + 'Enter' = # Will try to cd into 'rebuild' (if it exists) or find the first match and run the command on it (in this case will 'cd' into 'rebuild').
```

Dirvana will remember the directories you visit and use this information to provide better autocompletion suggestions in the case of similarly named directories.