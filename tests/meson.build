# SPDX-License-Identifier: GPL-2.0-only

tests = {
  'branches' : [ slkerncvs_lib ],
  'ratelimit' : [],
  'string' : [],
  'supported_conf' : [ slkerncvs_lib ],
}

foreach t : tests.keys()
  test(t, executable('test_' + t, 'test_' + t + '.cpp',
    dependencies: tests[t],
    include_directories: global_inc,
  ))
endforeach
