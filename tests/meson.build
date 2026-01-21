# SPDX-License-Identifier: GPL-2.0-only

crash = executable('crash', 'crash.cpp')

tests = {
  'curl' : { 'libs' : [ slcurl_lib ] },
  'CVE' : { 'libs' : [ slcves_lib ] },
  'git' : { 'libs' : [ slgit_lib ] },
  'helpers' : { 'libs' : [ slhelpers_lib ], 'args' : [ crash ] },
  'kerncvs' : { 'libs' : [ slkerncvs_lib ] },
  'maintainers' : { 'libs' : [ slgit_lib, slkerncvs_lib ] },
  'misc' : {},
  'PCRE2' : { 'libs' : [ slpcre2_lib ] },
  'ratelimit' : {},
  'sqlite' : { 'libs' : [ slsqlite_lib, sqlite3_lib ] },
  'string' : {},
  'suse' : {},
}

foreach t : tests.keys()
  test(t, executable('test_' + t, 'test_' + t + '.cpp',
    dependencies: tests[t].get('libs', []),
    include_directories: global_inc,
  ), args : tests[t].get('args', []))
endforeach
