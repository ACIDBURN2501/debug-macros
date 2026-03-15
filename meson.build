project(
  'debug-macros',
  'c',
  version: '1.1.0',
  default_options: ['warning_level=3', 'c_std=c11'],
)

# ── Version header ─────────────────────────────────────────────────────────────

ver = meson.project_version().split('.')

major = ver[0].to_int()
minor = ver[1].to_int()
patch = ver[2].to_int()

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set('MAJOR', major)
conf.set('MINOR', minor)
conf.set('PATCH', patch)

configure_file(
  input: 'config' / 'dbg_version.h.in',
  output: 'dbg_version.h',
  configuration: conf,
  install: true,
  install_dir: get_option('includedir') / 'dbg',
)

# ── Include directories ────────────────────────────────────────────────────────

public_headers = include_directories('include')

# ── Application core library ───────────────────────────────────────────────────

library_sources = ['src/dbg.c']

dbg_lib = static_library(
  'dbg',
  library_sources,
  include_directories: public_headers,
  install: true,
  install_dir: get_option('libdir'),
)

# Install public headers
install_headers('include/dbg.h', subdir: 'dbg')

dbg_dep = declare_dependency(
  include_directories: public_headers,
  link_with: dbg_lib,
)

# ── Host unit tests ────────────────────────────────────────────────────────────

if get_option('build_tests')
  subdir('tests')
endif
