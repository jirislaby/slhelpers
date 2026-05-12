# SPDX-License-Identifier: GPL-2.0-only

foreach py : python_installations
  py.extension_module('slkerncvs',
    'kerncvs.cpp',
    dependencies: [ slkerncvs_lib, pybind11 ],
    install: true,
    include_directories : global_inc,
  )
endforeach

