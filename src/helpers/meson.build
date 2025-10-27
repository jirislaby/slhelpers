# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'helpers/Color.h',
  'helpers/HomeDir.h',
  'helpers/Misc.h',
  'helpers/Process.h',
  'helpers/PushD.h',
  'helpers/Ratelimit.h',
  'helpers/String.h',
  'helpers/SSH.h',
  'helpers/SUSE.h',
  'helpers/Unique.h',
]

slhelpers = library('slhelpers++', [
    'Process.cpp',
  ],
  include_directories : global_inc,
  install: true,
  version: meson.project_version(),
)

slhelpers_lib = declare_dependency(
  link_with: slhelpers,
)

pkg.generate(slhelpers,
  description: slhelpers.name() + ': c++ helpers library',
)
