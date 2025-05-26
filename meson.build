# SPDX-License-Identifier: GPL-2.0-only

project('slsqlite++', 'cpp',
  version: '1.0',
  default_options: [
    'optimization=2',
    'warning_level=2',
    'b_lto=true',
    'b_pie=true',
    'cpp_std=c++17',
  ])

cpp_compiler = meson.get_compiler('cpp')
sqlite3_lib = cpp_compiler.find_library('sqlite3')

public_headers = [
    'SQLConn.h',
    'SQLiteSmart.h',
]

sqlite = library(meson.project_name(), [
    'SQLiteSmart.cpp',
    'SQLConn.cpp',
    public_headers,
  ],
  dependencies: sqlite3_lib,
  install: true,
  version: meson.project_version(),
)

install_headers(public_headers,
  subdir: meson.project_name(),
)

pkg = import('pkgconfig')
pkg.generate(sqlite,
  description: meson.project_name() + ': c++ sqlite wrapper library',
)

#subdir('tests')
