sphinx_build = custom_target(
  'sphinx-build',
  command : [files('mkenv.py'), '@SOURCE_ROOT@', '@BUILD_ROOT@/doc/env'],
  input : '../../requirements.txt',
  output : 'sphinx-build',
)
