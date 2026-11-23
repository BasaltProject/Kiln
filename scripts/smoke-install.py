#!/usr/bin/env python3
"""Check native installation, the CLI update controls, and uninstallation on CI."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile

def run_msi(command, log):
    try:
        parts = []
        for argument in command:
            if argument.startswith('INSTALL_ROOT='):
                name, value = argument.split('=', 1)
                parts.append(f'{name}="{value}"')
            elif any(character.isspace() for character in argument):
                parts.append(f'"{argument}"')
            else:
                parts.append(argument)
        result = subprocess.run(' '.join(parts), timeout=300)
    except subprocess.TimeoutExpired:
        result = None
    if result is None or result.returncode not in (0, 3010):
        if log.exists():
            data = log.read_bytes()
            encoding = 'utf-16' if data.startswith(b'\xff\xfe') else 'utf-8'
            print(data.decode(encoding, errors='replace'), flush=True)
        if result is None:
            raise SystemExit('Windows Installer did not finish within 300 seconds')
        raise SystemExit(result.returncode)


parser = argparse.ArgumentParser()
parser.add_argument('directory', type=Path)
parser.add_argument('--version', required=True)
options = parser.parse_args()
assets = options.directory.resolve()
with tempfile.TemporaryDirectory(prefix='kiln-install-') as temporary:
    root = Path(temporary)
    environment = dict(os.environ, KILN_CONFIG_HOME=str(root / 'config'), KILN_HOME=str(root / 'cache'))
    if sys.platform == 'win32':
        package = next(assets.glob('*.msi'))
        prefix = root / 'Kiln Install'
        log = root / 'msi.log'
        command = ['msiexec.exe', '/i', str(package), '/qn', '/norestart', f'INSTALL_ROOT={prefix}', '/l*v', str(log)]
        print('Installing MSI into ' + str(prefix), flush=True)
        run_msi(command, log)
        binary = prefix / 'bin/kiln.exe'
        uninstall = ['msiexec.exe', '/x', str(package), '/qn', '/norestart']
    elif sys.platform == 'darwin':
        package = next(assets.glob('*.pkg'))
        subprocess.run(['sudo', 'installer', '-pkg', str(package), '-target', '/'], check=True)
        binary = Path('/usr/local/bin/kiln')
        uninstall = None
    elif sys.platform.startswith('freebsd'):
        package = next(assets.glob('*.pkg'))
        subprocess.run(['pkg', 'add', str(package)], check=True)
        binary = Path('/usr/local/bin/kiln')
        uninstall = ['pkg', 'delete', '-y', 'basalt-kiln']
    else:
        package = next(assets.glob('*.deb'))
        subprocess.run(['sudo', 'dpkg', '-i', str(package)], check=True)
        binary = Path('/usr/bin/kiln')
        uninstall = ['sudo', 'dpkg', '-r', 'basalt-kiln']
        subprocess.run(['rpm', '-qpl', str(next(assets.glob('*.rpm')))], check=True)
    try:
        print('Checking installed executable: ' + str(binary), flush=True)
        actual = subprocess.check_output([str(binary), '--version'], text=True, timeout=30).strip()
        if actual != f'Kiln {options.version}':
            raise SystemExit(f'wrong installed version: {actual}')
        if not (binary.parent / 'kiln.installation').is_file():
            raise SystemExit('missing package ownership marker')
        for setting in ('on', 'status', 'off'):
            print('Checking update setting: ' + setting, flush=True)
            subprocess.run([str(binary), '--offline', 'update', '--auto', setting], env=environment, check=True, timeout=30)
        if subprocess.run([str(binary), '--offline', 'update'], env=environment, timeout=30).returncode != 1:
            raise SystemExit('offline updates must fail before downloading')
    finally:
        if uninstall:
            if sys.platform == 'win32':
                log = root / 'uninstall.log'
                run_msi(uninstall + ['/l*v', str(log)], log)
            else:
                subprocess.run(uninstall, check=True)
        else:
            subprocess.run(['sudo', 'rm', '-f', str(binary), str(binary.parent / 'kiln.installation')], check=True)
            subprocess.run(['sudo', 'pkgutil', '--forget', 'org.basaltmc.kiln.Runtime'], check=False)
