#!/bin/sh
# Builds one language of the proxy interaction example into <out>/<lang>/proxyinteraction: build.sh <kiln> <lang> <out>
set -eu
kiln=$1
lang=$2
out=$3
here=$(cd "$(dirname "$0")" && pwd)
rm -rf "$out/$lang"
mkdir -p "$out/$lang"
(cd "$out/$lang" && "$kiln" new proxyinteraction "Proxy interaction" --lang "$lang" --target proxy >/dev/null)
cd "$out/$lang/proxyinteraction"
rm -rf src main.go
if [ "$lang" = go ]; then cp "$here/go/src/main.go" main.go; else mkdir src && cp "$here/$lang/src/"* src/; fi
"$kiln" build . --lang "$lang" --target proxy
"$kiln" pack . --target proxy
