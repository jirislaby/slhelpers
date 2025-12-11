# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'pcre2/PCRE2.h',
]

pcre2_lib = dependency('libpcre2-8')
slpcre2_lib = declare_dependency(
  compile_args: '-DPCRE2_CODE_UNIT_WIDTH=8',
  dependencies: pcre2_lib,
)

name = 'slpcre2++'
pkg.generate(name: name,
  description: name + ': c++ PCRE2 wrapper library',
  libraries: slpcre2_lib,
  requires: pcre2_lib,
)
