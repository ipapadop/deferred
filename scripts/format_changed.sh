#!/bin/bash

# Find changed files (Added, Copied, Modified, Renamed) in the working tree and staged area
FILES=$(git diff --name-only --diff-filter=ACMR HEAD | grep -E "\.(cpp|hpp)$")

if [ -z "$FILES" ]; then
  echo "No changed source files to format."
  exit 0
fi

echo "Formatting changed files:"
for file in $FILES; do
  if [ -f "$file" ]; then
    echo "  $file"
    clang-format -i "$file"
  fi
done
