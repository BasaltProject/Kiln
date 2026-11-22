#!/bin/sh
# Builds one language of a proxy example into <out>/<lang>/proxyevents: build.sh <kiln> <area> <lang> <out>
set -eu
kiln=$1
area=$2
lang=$3
out=$4
here=$(cd "$(dirname "$0")" && pwd)
rm -rf "$out/$lang"
mkdir -p "$out/$lang"
(cd "$out/$lang" && "$kiln" new proxyevents "Proxy events" --lang "$lang" --target proxy >/dev/null)
cd "$out/$lang/proxyevents"
rm -rf src main.go
if [ "$lang" = go ]; then cp "$here/$area/go/src/main.go" main.go; else mkdir src && cp "$here/$area/$lang/src/"* src/; fi
if [ -f "$here/$area/shared/resources/config.toml" ]; then
    cp "$here/$area/shared/resources/config.toml" resources/config.toml
fi
"$kiln" build . --lang "$lang" --target proxy
"$kiln" pack . --target proxy
