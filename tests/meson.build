# SPDX-License-Identifier: GPL-2.0-only

crash = executable('crash', 'crash.cpp')

tests = {
  'branches' : { 'libs' : [ slkerncvs_lib ] },
  'curl' : { 'libs' : [ slcurl_lib ] },
  'git' : { 'libs' : [ slgit_lib ] },
  'helpers' : { 'libs' : [ slhelpers_lib ], 'args' : [ crash ] },
  'misc' : {},
  'PatchesAuthors' : { 'libs' : [ slkerncvs_lib ] },
  'ratelimit' : {},
  'sqlite' : { 'libs' : [ slsqlite_lib, sqlite3_lib ] },
  'string' : {},
  'supported_conf' : { 'libs' : [ slkerncvs_lib ] },
  'suse' : {},
}

foreach t : tests.keys()
  test(t, executable('test_' + t, 'test_' + t + '.cpp',
    dependencies: tests[t].get('libs', []),
    include_directories: global_inc,
  ), args : tests[t].get('args', []))
endforeach
