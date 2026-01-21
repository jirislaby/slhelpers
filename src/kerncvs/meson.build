# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'kerncvs/Branches.h',
  'kerncvs/CollectConfigs.h',
  'kerncvs/Maintainers.h',
  'kerncvs/PatchesAuthors.h',
  'kerncvs/Pattern.h',
  'kerncvs/Person.h',
  'kerncvs/RPMConfig.h',
  'kerncvs/Stanza.h',
  'kerncvs/SupportedConf.h',
]

slkerncvs = library('slkerncvs++', [
    'Branches.cpp',
    'CollectConfigs.cpp',
    'Maintainers.cpp',
    'PatchesAuthors.cpp',
    'Pattern.cpp',
    'Person.cpp',
    'SupportedConf.cpp',
  ],
  include_directories : global_inc,
  dependencies: [ slcurl_lib, slgit_lib, slpcre2_lib ],
  install: true,
  version: meson.project_version(),
)

slkerncvs_lib = declare_dependency(
  link_with: slkerncvs,
)

pkg.generate(slkerncvs,
  description: slkerncvs.name() + ': kerncvs (SUSE) c++ library',
)
