#!/usr/bin/python3

import os
from pathlib import Path
import shutil
import subprocess

base = Path(os.getenv('MESON_DIST_ROOT'))

# remove Git metadata from tarball
base.joinpath('.gitignore').unlink()
shutil.rmtree(base / '.github')
