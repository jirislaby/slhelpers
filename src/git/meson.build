# SPDX-License-Identifier: GPL-2.0-only

git2_lib = dependency('libgit2')
# libgit's *_INIT produce those
cpp_args = [ '-Wno-missing-field-initializers' ]

if cpp_compiler.has_member('git_remote_callbacks', 'update_refs',
    prefix : '#include <git2.h>')
  cpp_args += '-DLIBGIT_HAS_UPDATE_REFS'
endif

public_headers += [
    'git/Git.h',
]

slgit = library('slgit++', [
    'Git.cpp',
  ],
  cpp_args: cpp_args,
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
  requires: git2_lib,
  description: slgit.name() + ': c++ git wrapper library',
)
