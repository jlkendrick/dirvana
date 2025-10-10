#!/bin/bash

set -e

VERSION="main"

echo "Installing Dirvana version $VERSION"

# Determine the platform
OS=$(uname -s)
if [[ "$OS" != "Darwin" ]]; then
  echo "Dirvana is only supported on macOS."
  exit 1
fi
ARCH=$(uname -m)
# Download the binary with backup handling
echo "⏸️ Downloading Dirvana binary..."
BINARY_URL="https://raw.githubusercontent.com/jlkendrick/dirvana/$VERSION/docs/bin/dv-binary"
BINARY_PATH="$HOME/.local/bin"
BINARY_FILE="$BINARY_PATH/dv-binary"

# Create directory if it doesn't exist
mkdir -p "$BINARY_PATH"

# Back up existing binary if it exists
if [ -f "$BINARY_FILE" ]; then
  mv "$BINARY_FILE" "${BINARY_FILE}.backup"
  echo "📦 Existing binary backed up to ${BINARY_FILE}.backup"
fi

# Download new binary
if curl -sSL -o "$BINARY_FILE" "$BINARY_URL"; then
  chmod +x "$BINARY_FILE"
  echo "✅ Dirvana binary installed to $BINARY_FILE"
  # Remove backup if download was successful
  if [ -f "${BINARY_FILE}.backup" ]; then
    rm "${BINARY_FILE}.backup"
    echo "🗑️ Removed backup file ${BINARY_FILE}.backup"
  fi
else
  echo "❌ Failed to download binary"
  # Restore backup if available
  if [ -f "${BINARY_FILE}.backup" ]; then
    mv "${BINARY_FILE}.backup" "$BINARY_FILE"
    echo "🔄 Restored previous binary from backup"
  fi
  exit 1
fi

# Similar approach for tab completion script
echo "⏸️ Installing tab completion..."
TAB_URL="https://raw.githubusercontent.com/jlkendrick/dirvana/main/docs/scripts/_dv"
TAB_PATH="$HOME/.zsh/completions"
TAB_FILE="$TAB_PATH/_dv"

mkdir -p "$TAB_PATH"

# Back up existing completion script if it exists
if [ -f "$TAB_FILE" ]; then
  mv "$TAB_FILE" "${TAB_FILE}.backup"
  echo "📦 Existing completion script backed up to ${TAB_FILE}.backup"
fi

# Download new completion script
if curl -fsSL -o "$TAB_FILE" "$TAB_URL"; then
  chmod +x "$TAB_FILE"
  echo "✅ Tab completion script installed to $TAB_FILE"
  # Remove backup if download was successful
  if [ -f "${TAB_FILE}.backup" ]; then
    rm "${TAB_FILE}.backup"
    echo "🗑️ Removed backup file ${TAB_FILE}.backup"
  fi
else
  echo "❌ Failed to download tab completion script"
  # Restore backup if available
  if [ -f "${TAB_FILE}.backup" ]; then
    mv "${TAB_FILE}.backup" "$TAB_FILE"
    echo "🔄 Restored previous completion script from backup"
  fi
fi


# Add required configurations to .zshrc
echo "⏸️ Configuring .zshrc..."
ZSHRC="$HOME/.zshrc"
TEMP_ZSHRC="$HOME/.zshrc.tmp"

# Define the required lines
REQUIRED_LINES=$(cat <<EOF
# Begin Dirvana Zsh completion configuration
fpath=($TAB_PATH \$fpath)

zstyle ':completion:*' list-grouped yes
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list '' 'r:|=*'
  
setopt menucomplete
setopt autolist
  
autoload -Uz compinit && compinit -u
# End Dirvana Zsh completion configuration

EOF
)

# Add missing lines to the top of .zshrc
if ! grep -q "# Begin Dirvana Zsh completion configuration" "$ZSHRC"; then
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
  cmd=$(dv-binary --enter dv "$@")

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
if ! grep -q "# Dirvana automatic refresh on boot" "$ZSHRC"; then
  {
    echo ""
    echo "# Dirvana automatic refresh on boot"
    echo "dv-binary --enter dv refresh &> /dev/null & disown"
  } >> "$ZSHRC"
  echo "✅ Added automatic refresh on boot to $ZSHRC"
fi

# Add PATH to .zshrc
if ! grep -q "# Add ~/.local/bin to PATH" "$HOME/.zshrc"; then
  {
    echo ""
    echo "# Add ~/.local/bin to PATH"
    echo 'export PATH="$HOME/.local/bin:$PATH"'
  } >> "$HOME/.zshrc"
  echo "✅ Added ~/.local/bin to PATH in .zshrc"
fi

# Preemptively create the necessary directories
mkdir -p "$HOME/Library/Application Support/dirvana"
echo "✅ Created necessary directory for Dirvana"

# Run rebuild command to initialize the database
echo "Please specify a root directory to initialize Dirvana from."
echo "You can use a relative path, absolute path, or ~ for your home directory."
read -p "Root directory (default: $HOME): " root_dir < /dev/tty

# If no directory is provided, default to the home directory
if [ -z "$root_dir" ]; then
    root_dir="$HOME"
fi

# Convert to absolute path, expanding ~
root_dir_expanded="${root_dir/#\~/$HOME}"

# Handle relative paths by prepending $PWD if the path doesn't start with /
if [[ ! "$root_dir_expanded" = /* ]]; then
    root_dir_expanded="$PWD/$root_dir_expanded"
fi

# Check if directory exists
if [ ! -d "$root_dir_expanded" ]; then
    echo "Error: Directory '$root_dir_expanded' not found."
    echo "Note: If using a relative path, it should be relative to: $PWD"
    exit 1
fi

# Get clean absolute path (resolves symlinks and removes any trailing slash)
abs_root_dir=$(cd "$root_dir_expanded" && pwd)

echo "⏸️ Initializing Dirvana database from '$abs_root_dir'..."
if ! $BINARY_FILE --enter dv build --root "$abs_root_dir" &> /dev/null; then
  echo "❌ Failed to initialize Dirvana database for root '$abs_root_dir'"
  exit 1
fi
echo "✅ Dirvana database initialized successfully"

echo "Installation complete! Please restart your terminal to apply the changes."
exit 0