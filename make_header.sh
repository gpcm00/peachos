#!/usr/bin/env bash

set -e

path="$1"

dir="$(dirname "$path")"
file="$(basename "$path")"
name="${file%.h}"
guard="$(echo "${name}_H" | tr '[:lower:]' '[:upper:]')"

mkdir -p "$dir"

cat > "$path" <<EOF
#ifndef $guard
#define $guard

#endif
EOF
