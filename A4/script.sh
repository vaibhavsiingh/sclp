#!/bin/bash

# Script to download Level-4 valid test programs from IITB SCLP web

BASE_URL="https://www.cse.iitb.ac.in/~uday/sclp-web/test_files/Level-4-invalid-programs/"
DOWNLOAD_DIR="testcases/L4-invalid"

# Create the download directory if it doesn't exist
mkdir -p "$DOWNLOAD_DIR"

echo "Downloading Level-4 valid test programs..."
echo "Base URL: $BASE_URL"
echo "Download directory: $DOWNLOAD_DIR"
echo ""

# Download files from l4-exmp1.c to l4-exmp14.c
for i in {1..5}; do
    filename="l4-exmp${i}-invalid.c"
    url="${BASE_URL}/${filename}"
    filepath="${DOWNLOAD_DIR}/${filename}"
    
    echo -n "Downloading $filename... "
    
    # Use curl with silent mode and redirect to file
    if curl -s -f -o "$filepath" "$url"; then
        echo "✓ Done"
    else
        echo "✗ Failed (file may not exist)"
        # Remove the file if curl created an empty one
        rm -f "$filepath"
    fi
done

echo ""
echo "Download complete!"
echo "Files saved to: $(pwd)/$DOWNLOAD_DIR"
ls -la "$DOWNLOAD_DIR"
