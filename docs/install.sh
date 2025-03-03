#!/bin/bash

set -e

echo "Installing Dirvana..."

# Determine the platform
OS=$(uname -s)
if [[ "$OS" != "Darwin" ]]; then
	echo "Dirvana is only supported on macOS."
	exit 1
fi
ARCH=$(uname -m)

# Download and install the binary
echo "⏸️ Downloading Dirvana binary..."
BINARY_URL="https://raw.githubusercontent.com/jlkendrick/dirvana/main/docs/bin/dv-binary"
BIN_PATH="/usr/local/bin"
curl -sSL -o dv-binary "$BINARY_URL"
sudo mv dv-binary "$BIN_PATH/dv-binary"
sudo chmod +x "$BIN_PATH/dv-binary"
echo "✅ Dirvana binary installed to $BIN_PATH/dv-binary"

# Copy the tab completion script
echo "⏸️ Installing tab completion..."
COMPLETION_DIR="$HOME/.zsh/completions"
mkdir -p "$COMPLETION_DIR"
curl -fsSL "https://raw.githubusercontent.com/jlkendrick/dirvana/main/docs/scripts/_dv" -o "$COMPLETION_DIR/_dv"
echo "✅ Tab completion script installed to $COMPLETION_DIR/_dv"

# Add required configurations to .zshrc
echo "⏸️ Configuring .zshrc..."
ZSHRC="$HOME/.zshrc"
TEMP_ZSHRC="$HOME/.zshrc.tmp"

# Define the required lines
REQUIRED_LINES=$(cat <<EOF 
fpath=($COMPLETION_DIR \$fpath)

zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'
  
setopt menucomplete
setopt autolist
  
autoload -Uz compinit && compinit -u

EOF
)

# Add missing lines to the top of .zshrc
if ! grep -Fxq "fpath=($COMPLETION_DIR \$fpath)" "$ZSHRC"; then
  (echo "$REQUIRED_LINES"; cat "$ZSHRC") > "$TEMP_ZSHRC"
  mv "$TEMP_ZSHRC" "$ZSHRC"
  echo "✅ Added required configurations to $ZSHRC"
else
  echo "✅ Required configurations already present in $ZSHRC"
fi

# Add enter handler function to .zshrc
if ! grep -q "dv() {" "$ZSHRC"; then
  cat << 'EOF' >> "$ZSHRC"

# Dirvana Enter Handler
dv() {
  local cmd
  cmd=$(dv-binary -enter dv "$@")

  if [[ -n "$cmd" ]]; then
    eval "$cmd"
  else
    echo "dv-error: No command found for '$*'"
  fi
}
EOF
  echo "✅ Added dv() function to $ZSHRC"
fi

# Add automatic refresh on boot
if ! grep -q "dv-binary -enter dv refresh" "$ZSHRC"; then
	echo "dv-binary -enter dv refresh &> /dev/null & disown" >> "$ZSHRC"
	echo "✅ Added automatic refresh on boot to $ZSHRC"
fi

echo "Installation complete! Please restart your terminal or run 'source ~/.zshrc' to apply changes."
exit 0