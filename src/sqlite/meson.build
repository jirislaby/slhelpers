# SPDX-License-Identifier: GPL-2.0-only

sqlite3_lib = cpp_compiler.find_library('sqlite3')

public_headers += [
    'sqlite/SQLConn.h',
    'sqlite/SQLiteSmart.h',
]

slsqlite = library('slsqlite++', [
    'SQLiteSmart.cpp',
    'SQLConn.cpp',
  ],
  include_directories : global_inc,
  dependencies: sqlite3_lib,
  install: true,
  version: meson.project_version(),
)

pkg.generate(slsqlite,
  description: slsqlite.name() + ': c++ sqlite wrapper library',
)
