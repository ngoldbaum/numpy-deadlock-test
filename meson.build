project(
  'numpy-deadlock-test',
  'c',
)

py_mod = import('python')
py = py_mod.find_installation()

incdir_numpy = run_command(py,
  [
    '-c',
    'import numpy; print(numpy.get_include())'
  ],
  check: true
).stdout().strip()

includes = include_directories(
  [
    incdir_numpy,
  ]
)

srcs = [
  'numpy_deadlock_test/numpy_deadlock_test.c'
]

py.extension_module(
  'numpy_deadlock_test',
  srcs,
  install: true,
  include_directories: includes
)