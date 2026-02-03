# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'helpers/Color.h',
  'helpers/Exception.h',
  'helpers/HomeDir.h',
  'helpers/LastError.h',
  'helpers/Misc.h',
  'helpers/Process.h',
  'helpers/PtrStore.h',
  'helpers/PushD.h',
  'helpers/Ratelimit.h',
  'helpers/String.h',
  'helpers/SSH.h',
  'helpers/SUSE.h',
  'helpers/Unique.h',
  'helpers/Views.h',
]

ssh_lib = dependency('libssh')
slhelpers = library('slhelpers++', [
    'Process.cpp',
    'SSH.cpp',
  ],
  include_directories : global_inc,
  dependencies: ssh_lib,
  install: true,
  version: meson.project_version(),
)

slhelpers_lib = declare_dependency(
  link_with: slhelpers,
)

pkg.generate(slhelpers,
  description: slhelpers.name() + ': c++ helpers library',
)
