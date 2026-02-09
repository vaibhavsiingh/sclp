#!/bin/bash

# ============================
# Usage check
# ============================
if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_directory>"
    exit 1
fi

INPUT_DIR="$1"

if [ ! -d "$INPUT_DIR" ]; then
    echo "Error: '$INPUT_DIR' is not a directory"
    exit 1
fi

# ============================
# Temp directories
# ============================
TMP1=$(mktemp -d)
TMP2=$(mktemp -d)

cleanup() {
    rm -rf "$TMP1" "$TMP2"
}
trap cleanup EXIT

# ============================
# Processing files
# ============================
for file in "$INPUT_DIR"/*; do
    [ -f "$file" ] || continue
    fname=$(basename "$file")

    echo "======================================"
    echo "Testing file: $fname"

    ./sclp --show-tokens "$file" >"$TMP1/$fname.toks"
    ./sclp --show-tokens "$file" >"$TMP2/$fname.toks"

    if diff -u "$TMP1/$fname.toks" "$TMP2/$fname.toks" >/dev/null; then
        echo "✅ Token output MATCHES"
    else
        echo "❌ Token output DIFFERENT"
        diff -u "$TMP1/$fname.toks" "$TMP2/$fname.toks"
    fi
done


