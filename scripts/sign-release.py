#!/usr/bin/env python3
"""Create and sign Kiln's release manifest after all packages have been collected."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('version')
    parser.add_argument('directory', type=Path)
    parser.add_argument('--key', type=Path)
    options = parser.parse_args()
    if not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+', options.version):
        parser.error('release versions must have three numeric components')
    assets = sorted(path for path in options.directory.iterdir() if path.name.startswith('Kiln-'))
    if not assets or any(not path.is_file() or path.is_symlink() or
                         not re.fullmatch(r'Kiln-(?:Installer-)?' + re.escape(options.version) + r'-[A-Za-z0-9.-]+', path.name)
                         for path in assets):
        parser.error('missing assets or invalid asset names')
    lines = []
    for path in assets:
        with path.open('rb') as stream:
            lines.append(f'{hashlib.file_digest(stream, "sha256").hexdigest()}  {path.name}\n')
    checksums = ''.join(lines)
    (options.directory / 'SHA256SUMS').write_text(checksums, encoding='ascii')
    manifest = options.directory / 'update-manifest.txt'
    manifest.write_text(f'kiln-update-v1\nversion {options.version}\n{checksums}', encoding='ascii')
    public = Path(__file__).resolve().parents[1] / 'packaging/update-public.pem'
    with tempfile.TemporaryDirectory(prefix='kiln-sign-') as temporary:
        key = options.key
        if key is None:
            secret = os.environ.get('KILN_UPDATE_SIGNING_KEY', '')
            if not secret:
                parser.error('KILN_UPDATE_SIGNING_KEY is required to publish a release')
            key = Path(temporary) / 'private.pem'
            key.touch(mode=0o600)
            key.write_text(secret)
        signature = options.directory / 'update-manifest.sig'
        subprocess.run(['openssl', 'dgst', '-sha256', '-sign', str(key), '-out', str(signature), str(manifest)], check=True)
        subprocess.run(['openssl', 'dgst', '-sha256', '-verify', str(public), '-signature', str(signature), str(manifest)], check=True)
    print(f'Signed manifest for Kiln {options.version}: {len(assets)} assets')


if __name__ == '__main__':
    main()
