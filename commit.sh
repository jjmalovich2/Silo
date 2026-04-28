#!/bin/bash

set -e

echo "Staging changes..."
git add -A

echo ""
echo "Enter commit message:"
read -r msg

echo "Committing..."
git commit -m "$msg"

echo "Pushing to origin main..."
git push origin main

echo "Done!"
