# SPDX-License-Identifier: GPL-2.0-only

project('slhelpers', 'cpp',
  version: '2.0',
  default_options: [
    'optimization=2',
    'warning_level=2',
    'b_lto=true',
    'b_pie=true',
    'cpp_std=c++17',
  ])

cpp_compiler = meson.get_compiler('cpp')
pkg = import('pkgconfig')
global_inc = include_directories('include')
public_headers = []

subdir('src')

subdir('include')

subdir('tests')

doxygen = find_program('doxygen', required: false)
if doxygen.found()
    doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: {
      'DOXYGEN_INPUT_DIR': meson.project_source_root(),
      'DOXYGEN_OUTPUT_DIR': meson.project_build_root() / 'docs',
    }
  )

  custom_target('api-docs',
    output: 'api-docs',
    input: doxyfile,
    command: [ doxygen, '@INPUT@' ],
  )

  install_subdir(
    meson.project_build_root() / 'docs' / 'html',
    install_dir: get_option('datadir') / 'doc' / meson.project_name(),
  )
endif
