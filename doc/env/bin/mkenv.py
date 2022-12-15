#!/usr/bin/python3

from pathlib import Path
import subprocess
import sys
import venv

base_dir = Path(sys.argv[1])
env_dir = Path(sys.argv[2])

venv.create(env_dir, with_pip=True)
subprocess.run(
    [
        env_dir / 'bin/pip',
        'install',
        '--force-reinstall',
        '-r', base_dir / 'doc/requirements.txt'
    ],
    check=True
)
