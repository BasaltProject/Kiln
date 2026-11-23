#!/usr/bin/env bash
# Usage: publish-release.sh <version> <sha> <artifact dir>
# Publishes portable binaries, installers and the signed update manifest as the latest release.
set -euo pipefail

version="${1:?version}"
sha="${2:?sha}"
artifacts="${3:?artifact dir}"
out="$(mktemp -d)"

cp "$artifacts"/*/* "$out"/
python3 scripts/sign-release.py "$version" "$out"

gh release create "v$version" "$out"/* --target "$sha" --title "$version" --generate-notes --latest
