#!/bin/sh
# Builds one language of the admin example into <out>/<lang>/proxyadmin: build.sh <kiln> <lang> <out>
set -eu
kiln=$1
lang=$2
out=$3
here=$(cd "$(dirname "$0")" && pwd)
rm -rf "$out/$lang"
mkdir -p "$out/$lang"
(cd "$out/$lang" && "$kiln" new proxyadmin "Proxy admin" --lang "$lang" --target proxy >/dev/null)
cd "$out/$lang/proxyadmin"
rm -rf src main.go
if [ "$lang" = go ]; then cp "$here/go/src/main.go" main.go; else mkdir src && cp "$here/$lang/src/"* src/; fi
cp "$here/shared/resources/config.toml" resources/config.toml
sed -i 's/^players_data = true$/storage = true\nplayers_data = true\ncommands = true/' plugin.toml
"$kiln" build . --lang "$lang" --target proxy
"$kiln" pack . --target proxy
