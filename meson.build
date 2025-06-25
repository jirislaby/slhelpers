# SPDX-License-Identifier: GPL-2.0-only

project('slhelpers', 'cpp',
  version: '2.0',
  default_options: [
    'optimization=2',
    'warning_level=2',
    'b_lto=true',
    'b_pie=true',
    'cpp_std=c++17',
  ])

cpp_compiler = meson.get_compiler('cpp')
pkg = import('pkgconfig')
global_inc = include_directories('include')
public_headers = []

subdir('src')

subdir('include')

#subdir('tests')
