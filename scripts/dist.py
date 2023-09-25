#!/usr/bin/python3

import os
from pathlib import Path
import shutil
import subprocess

base = Path(os.getenv('MESON_DIST_ROOT'))

# remove Git metadata from tarball
base.joinpath('.gitignore').unlink()
shutil.rmtree(base / '.github')

# build docs and add them to the tarball
subprocess.run(['meson', 'compile', 'html'], check=True)
shutil.copytree('html', base / 'doc/html', symlinks=True)
shutil.rmtree(base / 'doc/html/.doctrees')
