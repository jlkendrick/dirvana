#!/bin/bash
# move_binary.sh - Copy the new binary to /usr/local/bin and remove the old one

# Set the source path to the new binary.
SOURCE="../../build/dv-binary"

# Set the destination path in /usr/local/bin.
DEST="/usr/local/bin/dv-binary"

echo "Installing new binary from $SOURCE to $DEST..."

# Check if the old binary exists, then remove it.
if [ -f "$DEST" ]; then
  echo "Removing old binary at $DEST..."
  sudo rm -f "$DEST"
fi

# Copy the new binary to /usr/local/bin
sudo cp "$SOURCE" "$DEST"

# Ensure the new binary is executable.
sudo chmod +x "$DEST"

echo "Installation complete. New binary is now in /usr/local/bin."