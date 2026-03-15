unity_proj = subproject('unity')
test_dep = unity_proj.get_variable('unity_dep')

test_exe = executable(
  'test_dbg',
  sources: ['test_dbg.c'],
  include_directories: public_headers,
  dependencies: [test_dep],
  link_with: dbg_lib,
)
test('dbg tests', test_exe)
