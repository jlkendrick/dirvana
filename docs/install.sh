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

# Ensure the completion directory is in fpath
ZSHRC="$HOME/.zshrc"
if ! grep -q "fpath=($COMPLETION_DIR" "$ZSHRC"; then
  echo "fpath=($COMPLETION_DIR \$fpath)" >> "$ZSHRC"
  echo "✅ Added fpath configuration to $ZSHRC"
fi

# Add enter handler function to .zshrc
if ! grep -q "dv() {" "$ZSHRC"; then
  cat << 'EOF' >> "$ZSHRC"

# Dirvana Enter Handler
dv() {
  local cmd
  cmd=\$(dv-binary -enter dv "\$@")

  if [[ -n "\$cmd" ]]; then
    # If the command is not empty, execute it
    eval "\$cmd"
  else
    # If the command is empty, print an error message
    echo "dv-error: No command found for '\$*'"
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