#!/bin/bash
# move_binary.sh - Copy the new binary to /usr/local/bin and remove the old one

# Set the source path to the new binary.
SOURCE="../../build/dv-binary"
# SOURCE="../scripts/_dv"

# Set the destination path in /usr/local/bin.
DEST="$HOME/.local/bin/"
# DEST="/Users/jameskendrick/.local/bin/_dv"

echo "Installing new binary from $SOURCE to $DEST..."

# Check if the old binary exists, then remove it.
if [ -f "$DEST/dv-binary" ]; then
  echo "Removing old binary at $DEST/dv-binary..."
  sudo rm -f "$DEST/dv-binary"
fi

# Copy the new binary to /usr/local/bin
sudo cp "$SOURCE" "$DEST"

# Ensure the new binary is executable.
sudo chmod +x "$DEST/dv-binary"

echo "Installation complete. New binary is now in $DEST/dv-binary."