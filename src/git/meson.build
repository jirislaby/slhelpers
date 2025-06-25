# SPDX-License-Identifier: GPL-2.0-only

git2_lib = cpp_compiler.find_library('git2')

public_headers += [
    'git/Git.h',
]

slgit = library('slgit++', [
    'Git.cpp',
  ],
  include_directories : global_inc,
  dependencies: git2_lib,
  install: true,
  version: meson.project_version(),
)

slgit_lib = declare_dependency(
  link_with: slgit,
  dependencies: git2_lib,
)

pkg.generate(slgit,
  description: slgit.name() + ': c++ git wrapper library',
)
