#!/bin/bash

set -e

echo "Building Silo..."
g++ *.cpp -o silo
echo "Compiled successfully"

echo "Installing system libraries to ~/.silo/lib/..."
mkdir -p ~/.silo/lib
cp lib/*.sl ~/.silo/lib/
echo "Libraries installed"

echo "Installing silo binary to /usr/local/bin..."
sudo cp silo /usr/local/bin/silo
echo "✅ Done! Run: silo yourfile.sl"