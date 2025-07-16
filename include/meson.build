# SPDX-License-Identifier: GPL-2.0-only

public_headers += [
  'helpers/HomeDir.h',
  'helpers/PushD.h',
  'helpers/Ratelimit.h',
  'helpers/String.h',
  'helpers/SSH.h',
]

install_headers(public_headers,
  subdir: 'sl',
  preserve_path: true,
)

name = 'slhelpers++'
pkg.generate(name: name,
  description: name + ': c++ helpers library',
)
