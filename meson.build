project('debug-macros', 'c',
  version: '0.1.0',
  default_options: ['warning_level=3', 'c_std=c11']
)

lib_debug = static_library('debug',
  sources: ['src/debug.c'],
  include_directories: include_directories('include'),
)

install_headers('include/debug.h', subdir: 'debug-macros')

unity_proj = subproject('unity')
test_dep = unity_proj.get_variable('unity_dep')

test_exe = executable('test_debug',
  sources: ['test/test_debug.c'],
  include_directories: include_directories('include'),
  dependencies: test_dep,
  link_with: lib_debug
)
test('debug tests', test_exe)
