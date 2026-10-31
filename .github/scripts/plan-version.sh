#!/usr/bin/env bash
# Usage: plan-version.sh <release|ci> <CMakeLists.txt>
# Prints the version to stamp (also to $GITHUB_OUTPUT). Releases are <minecraft>.<n>, one per commit to main.
set -euo pipefail

mode="${1:?mode}"
project="${2:?CMakeLists.txt}"
mc="$(sed -n 's/^ *VERSION \([0-9][0-9]*\.[0-9][0-9]*\)\..*/\1/p' "$project" | head -1)"
[ -n "$mc" ] || { echo "no Minecraft version in $project" >&2; exit 1; }

latest=0
while read -r tag; do
    patch="${tag#v"$mc".}"
    if [ "$patch" != "$tag" ] && [[ "$patch" =~ ^[0-9]+$ ]] && [ "$patch" -gt "$latest" ]; then
        latest="$patch"
    fi
done < <(gh release list --exclude-pre-releases --limit 100 --json tagName --jq '.[].tagName')

case "$mode" in
release) version="$mc.$((latest + 1))" ;;
*) version="$mc.$latest-ci${GITHUB_RUN_NUMBER:-0}" ;;
esac

echo "$version"
[ -z "${GITHUB_OUTPUT:-}" ] || echo "version=$version" >>"$GITHUB_OUTPUT"
