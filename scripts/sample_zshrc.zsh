# ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
# | | | | | | | | | | | | | | | | | | | | | | |
# The other content of your existing .zshrc file.


# Add the following somewhere in your .zshrc file:
zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'

setopt menucomplete
setopt autolist

autoload -Uz compinit && compinit -u

# --------------- TAB completion for dv ---------------
_dv() {
  local partial
  partial="${words[CURRENT]}"

  local completions
  completions=("${(@f)$(dv-binary "$partial")}")
  
  compadd -U -V 'Available Options' -- "${completions[@]}"
}
compdef _dv dv

# ---------------- ENTER handler for dv ---------------
dv() {
  if [[ $# -eq 0 ]]; then
    echo "Usage: dv <directory> + ('tab' and/or 'enter') | dv rebuild + 'enter' | dv -- <directory> + 'enter'"
    return 1
  fi

  # If '--' is given as the first argument, treat the second argument as a directory and bypass special handling.
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