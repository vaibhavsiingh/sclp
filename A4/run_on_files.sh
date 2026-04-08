#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

dir="$1"

if [[ ! -d "$dir" ]]; then
    echo "Error: '$dir' is not a directory."
    exit 1
fi

shopt -s nullglob

exit_code=0

for file in "$dir"/*.c; do
    echo "=== $file ==="
    if ! ./compiler --show-rtl "$file"; then
        exit_code=1
        echo "Command failed for: $file" >&2
    fi
    if ! diff -bw "$file.ans.rtl" "$file.rtl"; then
        echo "Diff found for: $file"
        exit_code=1
    fi
    echo

done

exit "$exit_code"
