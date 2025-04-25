#!/bin/bash

HASH_DIR="/home/$USER/.hash"
TEMP_DIR="$HASH_DIR/temp"
VERSION_HASH_FILE="$HASH_DIR/system_version_hash.txt"
VERSION_FILE="/var/lib/dpkg/status"

calculate_hash() {
    sha256sum "$VERSION_FILE" | awk '{print $1}'
}

CURRENT_HASH=$(calculate_hash)

if [ -d "$HASH_DIR" ]; then
    if [ -f "$VERSION_HASH_FILE" ]; then
        STORED_HASH=$(cat "$VERSION_HASH_FILE")
        if [ "$CURRENT_HASH" != "$STORED_HASH" ]; then
            mv "$HASH_DIR" "/home/$USER/.hash_${STORED_HASH}"
            mkdir -p "$TEMP_DIR"
            echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
            ./hashage_command
        fi
    else
        mv "$HASH_DIR" "/home/$USER/.hash_unknown"
        mkdir -p "$TEMP_DIR"
        echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
        ./hashage_command
    fi
else
    mkdir -p "$TEMP_DIR"
    echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
    ./hashage_command
fi
