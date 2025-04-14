HASH_DIR=".hash"
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
            mv "$HASH_DIR" ".hash_${STORED_HASH}"
            mkdir -p "$HASH_DIR"
            echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
            ./hashage_command
        fi
    else
        mv "$HASH_DIR" ".hash_unknown"
        mkdir -p "$HASH_DIR"
        echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
        ./hashage_command
    fi
else
    mkdir -p "$HASH_DIR"
    echo "$CURRENT_HASH" > "$VERSION_HASH_FILE"
    ./hashage_command
fi

