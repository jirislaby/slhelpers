# SPDX-License-Identifier: GPL-2.0-only

curl_lib = cpp_compiler.find_library('curl')

public_headers += [
    'curl/Curl.h',
]

slcurl = library('slcurl++', [
    'Curl.cpp',
  ],
  include_directories : global_inc,
  dependencies: curl_lib,
  install: true,
  version: meson.project_version(),
)

slcurl_lib = declare_dependency(
  link_with: slcurl,
)

pkg.generate(slcurl,
  description: slcurl.name() + ': c++ curl wrapper library',
)
