# SPDX-License-Identifier: GPL-2.0-only

crash = executable('crash', 'crash.cpp')

tests = {
  'branches' : { 'libs' : [ slkerncvs_lib ] },
  'curl' : { 'libs' : [ slcurl_lib ] },
  'git' : { 'libs' : [ slgit_lib ] },
  'ratelimit' : {},
  'process' : { 'libs' : [ slhelpers_lib ], 'args' : [ crash ] },
  'string' : {},
  'supported_conf' : { 'libs' : [ slkerncvs_lib ] },
}

foreach t : tests.keys()
  test(t, executable('test_' + t, 'test_' + t + '.cpp',
    dependencies: tests[t].get('libs', []),
    include_directories: global_inc,
  ), args : tests[t].get('args', []))
endforeach
