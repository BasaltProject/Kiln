#!/bin/sh
# Builds one language of the proxy network example into <out>/<lang>/netdemo: build.sh <kiln> <lang> <out>
set -eu
kiln=$1
lang=$2
out=$3
here=$(cd "$(dirname "$0")" && pwd)
rm -rf "$out/$lang"
mkdir -p "$out/$lang"
(cd "$out/$lang" && "$kiln" new netdemo "Network demo" --lang "$lang" --target proxy >/dev/null)
cd "$out/$lang/netdemo"
rm -rf src main.go
if [ "$lang" = go ]; then cp "$here/go/src/main.go" main.go; else mkdir src && cp "$here/$lang/src/"* src/; fi
sed -i 's/^players_data = true$/network = ["proxy"]\npackets = true/' plugin.toml
cp "$here/shared/resources/config.toml" resources/config.toml
"$kiln" build . --lang "$lang" --target proxy
"$kiln" pack . --target proxy
