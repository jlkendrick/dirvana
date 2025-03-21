# Dirvana
**An intelligent directory navigation and command augmentation tool built for Zsh.**  

Dirvana (`dv`) is designed to streamline working with directories in the terminal by providing autocompletion for directory paths. By default, `dv` wraps the `cd` command but can also provide completions when prefixed before another directory command (e.g., `ls`, `rm`, `mv`, etc.), allowing for quick navigation and command execution. The quick-nav feature instantly executes the current command with the current best match. See [Usage](#usage) for more details.

---

## Installation

### Automatic Installation (recommended)
To install Dirvana, run the following command in your terminal:

```sh
curl -fsSL https://jlkendrick.github.io/dirvana/docs/install.sh | bash
```

<center>---- or ----</center>

\
If you prefer to install Dirvana manually, follow these steps:
### Manual Installation
#### **1️⃣ Install the Binary**
Dirvana requires its compiled binary (`dv-binary`, which can be found in the `build/` directory of this repo). Download it or build the project and move it to a directory in your `PATH`, such as `/usr/local/bin`:

```sh
# Example: Move to /usr/local/bin (adjust if needed)
sudo mv dv-binary /usr/local/bin
chmod +x /usr/local/bin/dv-binary
# or, use the script provided at 'scripts/move_binary.sh' (make sure to adjust the path if needed)
source scripts/move_binary.sh
```

#### **2️⃣ Copy the Zsh Completion Script**
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

#### **3️⃣ Enable the Completion Script**
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

#### **4️⃣ Add the Command Handler**
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

#### **5️⃣ Reload Zsh Configuration**
Reload your Zsh configuration to apply the changes:

```sh
source ~/.zshrc
```
---

## Usage

You now have access to the `dv` command, here are some examples of how to use it:

Note: The examples below show results assuming the matching type is set to `exact`, but you can change it to the options seen in the [Configuration](#configuration) section.
```sh
# --------- Navigating to a directory ---------
dv project + 'Tab' = dv path/to/project # Autocompletes to top match and displays a menu of other matches. Consecutive 'Tab' presses cycle through matches.
# After getting the desired directory, press 'Enter' to navigate to it.
dv another/path/to/project + 'Enter' = # Navigates to 'another/path/to/project' (executes 'cd another/path/to/project').
#            -- or (quick-nav) --
dv project + 'Enter' = # Finds the first match and navigates to it (in this case will 'cd' into 'path/to/project').

# ----------- Running a command ---------
# You can also run commands with 'dv' (e.g., 'ls', 'rm', 'mv', etc.)
dv code project + 'Tab' = dv code path/to/project # Autocompletes to top match and displays a menu of other matches. Consecutive 'Tab' presses cycle through matches.
# After getting the desired directory, press 'Enter' to run the command.
dv code another/path/to/project + 'Enter' = # Runs 'code path/to/project' (opens the directory in VSCode).
#            -- or (quick-nav) --
dv code project + 'Enter' = # Finds the first match and runs the command on it (in this case will open up 'path/to/project' in VSCode).

# Also try chaining commands:
dv cp -r to_copy + 'Tab' = dv cp -r path/to/dir/to_copy # Use this as an intermediate step to then do
dv cp -r path/to/dir/to_copy target_dir + 'Tab' = dv cp -r path/to/dir/to_copy path/to/target_dir # This can then be run with 'Enter' to copy the directory.

# --------- Other utility commands ---------
# If you wish, you can rebuild the database from scratch with 'dv rebuild', however, this will reset all your previous history.
dv rebuild + 'Enter' = # Rebuilds the database

# To refresh the database to account for new/deleted directories, use 'dv refresh'. This will not reset your history.
dv refresh + 'Enter' = # Refreshes the database

# If needed, you can also bypass commands by using '--'. Below is one example but you can use it with any command.
dv -- rebuild + 'Enter' = # Will try to cd into 'rebuild' (if it exists) or find the first match and run the command on it (in this case will 'cd' into 'rebuild').
```

Dirvana will remember the directories you visit and use this information to provide better autocompletion suggestions in the case of similarly named directories.

---
## Configuration
Dirvana can be configured to suit your needs through the configuration JSON file. The configuration file is located at `~/.config/dirvana/config.json`. Here are the available options:

```ts
{
  "paths": {
    "init": "/some/path", // Dirvana will have knowledge of all the directories that branch off of this path. 
                          // Keep in mind that this path will be used to build the database, so it should be a 
                          // directory that contains all the directories you want to track, but not too many to 
                          // avoid performance issues.
  },
  "matching": {
    "max_results": 10, // The maximum number of results to display when using the 'Tab' key for autocompletion.
    "promotion_strategy": "strategy_name", // Controls how Dirvana adjusts the order of the results based on your 
                                           // usage patterns. Available strategies include:
                                           // 1. "recently_accessed" - Promotes the directory you most recently 
                                           // accessed to the top of the list.
                                           // 2. "frequency_based" - Sorts the results based on how frequently you
                                           // have accessed each directory.
    "type": "type_name", // Controls how Dirvana matches the results. Available types include:
                         // 1. "exact" - Only matches directories whose names exactly match the search term.
                         // 2. "prefix" - Matches directories whose names start with the search term.
                         // 3. "suffix" - Matches directories whose names end with the search term.
                         // 4. "contains" - Matches directories whose names contain the search term (substring matching).
    "exclusions": { // Lists of directories to exclude from the database along with the patterns to match them.
                    // will not track these directories or their subdirectories.
      "exact": ["/path/to/exclude1", "/path/to/exclude2"], // Will exclude these exact directories and their subdirectories.
      "prefix": ["/path/to/prefix_exclude1", "/path/to/prefix_exclude2"], // Will exclude direcotories that start with these prefixes and their subdirectories.
      "suffix": ["/path/to/suffix_exclude1", "/path/to/suffix_exclude2"], // Will exclude directories that end with these suffixes and their subdirectories.
      "contains": ["/path/to/contains_exclude1", "/path/to/contains_exclude2"] // Will exclude directories that contain these substrings and their subdirectories.
    }
}
```