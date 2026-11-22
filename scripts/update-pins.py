#!/usr/bin/env python3
"""Regenerates src/ToolchainPins.cpp from the upstream release manifests.

Every hash comes from a manifest the publisher serves over HTTPS (or, for npm packages, from downloading the
tarball here), and Kiln re-verifies it on every download. Review the diff before committing.
"""
import hashlib
import json
import re
import sys
import urllib.request

ZIG = "0.17.1"
RUST = "1.100.0"
NODE_MAJOR = "v22."
TINYGO = "0.44.0"
GO = "go1.26.11"
KOTLIN = "2.4.21"
JAVA = "21"
TEAVM = "0.17.0"
ECJ = "3.46.100"
DOTNET = "10.0.404"
WASI_SDK = "29"
ILCOMPILER = "10.0.0-preview.2.25509.1"
ILLINK = "10.0.15"
NPM = [("assemblyscript", "0.28.20"), ("binaryen", "131.0.0-nightly.20260721"), ("long", "5.3.2")]

PLATFORMS = ["linux-x64", "linux-arm64", "macos-x64", "macos-arm64", "windows-x64"]


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "kiln-pins"})) as response:
        return response.read()


def sha256_of(url):
    data = get(url)
    return hashlib.sha256(data).hexdigest(), len(data)


def content_length(url):
    request = urllib.request.Request(url, method="HEAD", headers={"User-Agent": "kiln-pins"})
    with urllib.request.urlopen(request) as response:
        return int(response.headers.get("Content-Length", 0))


rows = []


def row(tool, version, platform, url, sha, size, kind, into="", skip=0, only=""):
    rows.append((tool, version, platform, url, sha, size, kind, into, skip, only))


zig_index = json.loads(get("https://ziglang.org/download/index.json"))[ZIG]
for platform, key in zip(PLATFORMS, ["x86_64-linux", "aarch64-linux", "x86_64-macos", "aarch64-macos", "x86_64-windows"]):
    entry = zig_index[key]
    row("zig", ZIG, platform, entry["tarball"], entry["shasum"], int(entry["size"]),
        "Zip" if entry["tarball"].endswith(".zip") else "TarXz", skip=1)

rust_manifest = get("https://static.rust-lang.org/dist/channel-rust-stable.toml").decode()
rust_hosts = ["x86_64-unknown-linux-gnu", "aarch64-unknown-linux-gnu", "x86_64-apple-darwin", "aarch64-apple-darwin",
              "x86_64-pc-windows-msvc"]


def rust_artifact(package, target):
    section = re.search(r"\[pkg\.%s\.target\.%s\]\n(.*?)\n\n" % (re.escape(package), re.escape(target)),
                        rust_manifest, re.S).group(1)
    return (re.search(r'^url = "([^"]*)"', section, re.M).group(1), re.search(r'^hash = "([^"]*)"', section, re.M).group(1))


assert re.search(r'\[pkg\.rust\]\nversion = "%s ' % re.escape(RUST), rust_manifest), "the stable channel moved on"
for platform, host in zip(PLATFORMS, rust_hosts):
    url, sha = rust_artifact("rustc", host)
    row("rust", RUST, platform, url, sha, 0, "TarGz", skip=1, only="rustc")
    url, sha = rust_artifact("rust-std", "wasm32-unknown-unknown")
    row("rust", RUST, platform, url, sha, 0, "TarGz", skip=1, only="rust-std-wasm32-unknown-unknown")

node_index = [x for x in json.loads(get("https://nodejs.org/dist/index.json")) if x["version"].startswith(NODE_MAJOR) and x["lts"]][0]
node_version = node_index["version"]
sums = dict(reversed(line.split("  ")) for line in get("https://nodejs.org/dist/%s/SHASUMS256.txt" % node_version).decode().split("\n") if line)
for platform, suffix in zip(PLATFORMS, ["linux-x64.tar.gz", "linux-arm64.tar.gz", "darwin-x64.tar.gz", "darwin-arm64.tar.gz", "win-x64.zip"]):
    name = "node-%s-%s" % (node_version, suffix)
    row("node", node_version[1:], platform, "https://nodejs.org/dist/%s/%s" % (node_version, name), sums[name], 0,
        "Zip" if suffix.endswith("zip") else "TarGz", skip=1)

for name, version in NPM:
    url = "https://registry.npmjs.org/%s/-/%s-%s.tgz" % (name, name, version)
    sha, size = sha256_of(url)
    row("assemblyscript", "0.28.20", "any", url, sha, size, "TarGz", into="node_modules/" + name, skip=1)

go_release = [r for r in json.loads(get("https://go.dev/dl/?mode=json&include=all")) if r["version"] == GO][0]
for platform, (os_name, arch) in zip(PLATFORMS, [("linux", "amd64"), ("linux", "arm64"), ("darwin", "amd64"), ("darwin", "arm64"), ("windows", "amd64")]):
    entry = [f for f in go_release["files"] if f["os"] == os_name and f["arch"] == arch and f["kind"] == "archive"][0]
    row("go", GO[2:], platform, "https://dl.google.com/go/" + entry["filename"], entry["sha256"], entry["size"],
        "Zip" if entry["filename"].endswith(".zip") else "TarGz", skip=1)

tinygo_release = json.loads(get("https://api.github.com/repos/tinygo-org/tinygo/releases/tags/v" + TINYGO))
for platform, suffix in zip(PLATFORMS, ["linux-amd64.tar.gz", "linux-arm64.tar.gz", "darwin-amd64.tar.gz", "darwin-arm64.tar.gz", "windows-amd64.zip"]):
    asset = [a for a in tinygo_release["assets"] if a["name"] == "tinygo%s.%s" % (TINYGO, suffix)][0]
    row("tinygo", TINYGO, platform, asset["browser_download_url"], asset["digest"].split(":")[1], asset["size"],
        "Zip" if suffix.endswith("zip") else "TarGz", skip=1)

kotlin_url = "https://github.com/JetBrains/kotlin/releases/download/v%s/kotlin-compiler-%s.zip" % (KOTLIN, KOTLIN)
kotlin_sha = get(kotlin_url + ".sha256").decode().split()[0]
for platform in PLATFORMS:
    row("kotlin", KOTLIN, platform, kotlin_url, kotlin_sha, 0, "Zip", skip=1)

TEAVM_JARS = ["https://repo.maven.apache.org/maven2/org/teavm/cli/%s/cli-%s-all.jar" % (TEAVM, TEAVM),
              "https://repo.maven.apache.org/maven2/org/eclipse/jdt/ecj/%s/ecj-%s.jar" % (ECJ, ECJ)]
for url in TEAVM_JARS:
    sha, size = sha256_of(url)
    row("teavm", TEAVM, "any", url, sha, size, "Raw")

for platform, (os_name, arch, archive) in zip(PLATFORMS, [("linux", "x64", "tar.gz"), ("linux", "aarch64", "tar.gz"), ("macos", "x64", "tar.gz"), ("macos", "aarch64", "tar.gz"), ("windows", "x64", "zip")]):
    listing = json.loads(get("https://api.azul.com/metadata/v1/zulu/packages/?java_version=%s&os=%s&arch=%s&archive_type=%s&java_package_type=jre&javafx_bundled=false&crac_supported=false&latest=true&release_status=ga&availability_types=CA&page=1&page_size=1" % (JAVA, os_name, arch, archive)))[0]
    detail = json.loads(get("https://api.azul.com/metadata/v1/zulu/packages/" + listing["package_uuid"]))
    row("jre", ".".join(map(str, listing["java_version"])), platform, detail["download_url"], detail["sha256_hash"],
        detail.get("size", 0), "Zip" if archive == "zip" else "TarGz", skip=1)

dotnet_sdk = [f for f in json.loads(get("https://builds.dotnet.microsoft.com/dotnet/release-metadata/10.0/releases.json"))["releases"]
              if f["sdk"]["version"] == DOTNET][0]["sdk"]["files"]
dotnet_file = [f for f in dotnet_sdk if f["rid"] == "linux-x64" and f["url"].endswith(".tar.gz")][0]
dotnet_data = get(dotnet_file["url"])
assert hashlib.sha512(dotnet_data).hexdigest() == dotnet_file["hash"].lower(), "the dotnet sdk does not match the manifest"
sha, size = hashlib.sha256(dotnet_data).hexdigest(), len(dotnet_data)
row("dotnet", DOTNET, "linux-x64", dotnet_file["url"], sha, size, "TarGz")
EXPERIMENTAL = "https://pkgs.dev.azure.com/dnceng/9ee6d478-d288-47f7-aacc-f6e6d082ae6d/_packaging/fe324174-469a-4296-a9cb-1fd885bb6f2b/nuget/v3/flat2"
for name, version, feed in [("microsoft.dotnet.ilcompiler.llvm", ILCOMPILER, EXPERIMENTAL),
                            ("runtime.linux-x64.microsoft.dotnet.ilcompiler.llvm", ILCOMPILER, EXPERIMENTAL),
                            ("runtime.wasi-wasm.microsoft.dotnet.ilcompiler.llvm", ILCOMPILER, EXPERIMENTAL),
                            ("microsoft.net.illink.tasks", ILLINK, "https://api.nuget.org/v3-flatcontainer")]:
    url = "%s/%s/%s/%s.%s.nupkg" % (feed, name, version, name, version)
    sha, size = sha256_of(url)
    row("dotnet", DOTNET, "linux-x64", url, sha, size, "Zip", into="nuget/%s/%s" % (name, version))
wasi_asset = [a for a in json.loads(get("https://api.github.com/repos/WebAssembly/wasi-sdk/releases/tags/wasi-sdk-" + WASI_SDK))["assets"]
              if a["name"] == "wasi-sdk-%s.0-x86_64-linux.tar.gz" % WASI_SDK][0]
row("wasisdk", WASI_SDK + ".0", "linux-x64", wasi_asset["browser_download_url"], wasi_asset["digest"].split(":")[1],
    wasi_asset["size"], "TarGz", skip=1)

out = ['// Generated by scripts/update-pins.py; the hashes are checked again on every download.',
       '#include "ToolchainPins.hpp"', '', 'namespace basalt::kiln {', '',
       'const std::vector<Pin> kPins{']
for tool, version, platform, url, sha, size, kind, into, skip, only in rows:
    size = size or content_length(url)
    out.append('    {"%s", "%s", "%s",\n     "%s",\n     "%s", %d, PinKind::%s, "%s", %d, "%s"},' % (tool, version, platform, url, sha, size, kind, into, skip, only))
out += ['};', '', '} // namespace basalt::kiln', '']
open("src/ToolchainPins.cpp", "w").write("\n".join(out))
print(len(rows), "pins", file=sys.stderr)
