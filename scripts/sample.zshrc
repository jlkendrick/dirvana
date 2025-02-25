fpath=(~/.zsh/completions $fpath)

zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'

setopt menucomplete
setopt autolist

autoload -Uz compinit && compinit

# The other content of your existing .zshrc file #
# ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃ ⌃  #
# | | | | | | | | | | | | | | | | | | | | | | |  #
# | | | | | | | | | | | | | | | | | | | | | | |  #
# ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄ ⌄  #
# The other content of your existing .zshrc file #

dv() {
  # Unrecognized command
  if [[ $# -eq 0 ]]; then
    echo "Usage: dv <directory> + ('tab' or 'enter') | dv rebuild + 'enter' | dv -- <directory> + 'enter'"
    return 1
  fi

  # Check if command bypasser was used
  if [[ "$1" == "--" ]]; then
    if [[ -z "$2" ]]; then
      echo "Usage: dv -- <directory>"
      return 1
    fi
    cd "$2" && dv-binary update "$2"
    return $?
  fi

  # Handle commands
  case "$1" in
    rebuild)
      # Rebuild
      dv-binary rebuild
      ;;
    *)
      # Otherwise, try to change directory
      if cd "$1" 2>/dev/null; then
        dv-binary update "$1"
      else
        # If the directory doesn't exist, try to match
        local matches
        matches=($(dv-binary "$1"))

        # If there is a match, cd into the first match
        if [[ ${#matches[@]} -gt 0 ]]; then
          cd "${matches[1]}" && dv-binary update "${matches[1]}"
        else
          # If there are no matches, print an error message
          echo "dv-error: Could not change directory to '$1'"
          return 1
        fi
      fi
      ;;
  esac
}

dv-binary refresh &> /dev/null & disown