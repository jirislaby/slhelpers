# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'kerncvs/Branches.h',
  'kerncvs/SupportedConf.h',
]

slkerncvs = library('slkerncvs++', [
    'Branches.cpp',
    'SupportedConf.cpp',
  ],
  include_directories : global_inc,
  dependencies: slcurl_lib,
  install: true,
  version: meson.project_version(),
)

slkerncvs_lib = declare_dependency(
  link_with: slkerncvs,
)

pkg.generate(slkerncvs,
  description: slkerncvs.name() + ': kerncvs (SUSE) c++ library',
)
