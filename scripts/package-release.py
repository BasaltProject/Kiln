#!/usr/bin/env python3
"""Package the current native build alongside its portable release binary."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
parser.add_argument('destination', type=Path)
parser.add_argument('--version', required=True)
parser.add_argument('--platform', required=True)
options = parser.parse_args()
options.destination.mkdir(parents=True, exist_ok=True)
output = options.build.resolve() / 'packages'
if sys.platform == 'win32':
    generators = 'WIX'
elif sys.platform == 'darwin':
    generators = 'productbuild'
elif sys.platform.startswith('freebsd'):
    generators = 'FREEBSD'
else:
    generators = 'DEB;RPM'
subprocess.run(['cpack', '--config', str(options.build.resolve() / 'CPackConfig.cmake'), '-G', generators,
                '-B', str(output)], check=True)
packages = [path for path in output.iterdir() if path.suffix in ('.msi', '.pkg', '.deb', '.rpm', '.txz')]
if not packages:
    raise SystemExit('CPack produced no installable packages')
for package in packages:
    if sys.platform == 'darwin':
        with tempfile.TemporaryDirectory(prefix='pkg-check-', dir=output) as temporary:
            expanded = Path(temporary) / 'expanded'
            subprocess.run(['pkgutil', '--expand', str(package), str(expanded)], check=True)
            distribution = ET.parse(expanded / 'Distribution').getroot()
            references = {element.get('id') for element in distribution.findall('pkg-ref')
                          if element.text and element.text.strip()}
            choices = {element.get('id') for element in distribution.findall('./choice/pkg-ref')}
            if not choices or not choices.issubset(references):
                raise SystemExit(f'{package.name}: distribution has no valid component package choices')
    suffix = '.pkg' if sys.platform.startswith('freebsd') else package.suffix
    name = f'Kiln-Installer-{options.version}-{options.platform}{suffix}'
    shutil.copy2(package, options.destination / name)
if sys.platform != 'win32':
    installer = options.destination / f'Kiln-Installer-{options.version}-{options.platform}.sh'
    shutil.copy2(options.build / 'install.sh', installer)
    installer.chmod(0o755)
