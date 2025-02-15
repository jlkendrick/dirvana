# Dirvana
**A simple directory navigation tool with Zsh-powered autocompletion.**  

Dirvana enhances `cd` by integrating **custom autocompletion** into Zsh's completion system, enabling fast and efficient movement across large filesystems.

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
  if [[ $# -eq 0 ]]; then
    echo "Usage: dv <directory> + ('tab' or 'enter') | dv rebuild + 'enter' | dv -- <directory> + 'enter'"
    return 1
  fi

  if [[ "$1" == "--" ]]; then
    if [[ -z "$2" ]]; then
      echo "Usage: dv -- <directory>"
      return 1
    fi
    cd "$2" && dv-binary update "$2"
    return $?
  fi

  case "$1" in
    rebuild)
      dv-binary rebuild
      ;;
    *)
      if cd "$1"; then
        dv-binary update "$1"
      else
        echo "Error: Could not change directory to '$1'"
        return 1
      fi
      ;;
  esac
}

dv-binary refresh &> /dev/null & disown
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

You can now use `dv` how you would normally use `cd`, with the added benefit of improved autocompletion using `Tab`. For example:

```sh
# Navigate to a directory
dv src + 'Tab' = dv path1/to/src # Autocompletes to top match and displays a menu of other matches. Consecutive 'Tab' presses cycle through matches.

# After getting the desired directory, press 'Enter' to navigate to it
dv path2/to/src + 'Enter' = # Navigates to 'path2/to/src'

# If you make/delete directories, you can rebuild the database with 'dv rebuild'
dv rebuild + 'Enter' = # Rebuilds the database

# If needed, you can also bypass commands by using '--'
dv -- rebuild + 'Enter' = # Navigates to 'rebuild'
```

Dirvana will remember the directories you visit and use this information to provide better autocompletion suggestions in the case of similarly named directories.