#!/bin/bash

SCLP=./sclp
ROOT=testcases

total=0
passed=0
failed=0

for file in $(find "$ROOT" -type f -name "*.c"); do
    ((total++))

    $SCLP --show-tokens "$file" > /dev/null 2>&1
    ret=$?

    if [[ "$file" == *"invalid"* ]]; then
        expected=1
    else
        expected=0
    fi

    if [[ $expected -eq 0 && $ret -eq 0 ]] || [[ $expected -ne 0 && $ret -ne 0 ]]; then
        ((passed++))
        printf "[PASS] %s\n" "$file"
    else
        ((failed++))
        printf "[FAIL] %s (exit=%d)\n" "$file" "$ret"
    fi
done

echo "----------------------"
echo "Total : $total"
echo "Passed: $passed"
echo "Failed: $failed"

# Exit non-zero if anything failed
[ $failed -eq 0 ] || exit 1
