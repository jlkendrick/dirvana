# Dirvana shell integration
# Source this file from your ~/.zshrc:
#   source "$(brew --prefix)/etc/dirvana.zsh"

dv() {
  local cmd
  cmd=$(dv-binary --enter dv "$@")
  if [[ -n "$cmd" ]]; then
    eval "$cmd"
  else
    echo "dv-error: No command found for '$*'"
  fi
}

dv-binary --enter dv refresh &> /dev/null & disown
