#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <input_file> [output_file]"
    exit 1
fi

input_file="$1"
output_file="${2:-}"

if [[ ! -f "$input_file" ]]; then
    echo "Error: '$input_file' is not a file."
    exit 1
fi

# Remove everything from the first occurrence of ';;' to the end of line.
if [[ -n "$output_file" ]]; then
    sed 's/;;.*$//' "$input_file" > "$output_file"
else
    sed 's/;;.*$//' "$input_file"
fi
