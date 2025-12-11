# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
    'cves/CVE.h',
    'cves/CVE2Bugzilla.h',
    'cves/CVEHashMap.h',
]

slcves = library('slcves++', [
    'CVE.cpp',
    'CVE2Bugzilla.cpp',
    'CVEHashMap.cpp',
  ],
  include_directories : global_inc,
  dependencies: [ slgit_lib ],
  install: true,
  version: meson.project_version(),
)

slcves_lib = declare_dependency(
  link_with: slcves,
)

pkg.generate(slcves,
  description: slcves.name() + ': c++ CVEs library',
)
