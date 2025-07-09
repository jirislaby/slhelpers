tests = {
  'branches' : [ slkerncvs_lib ],
  'ratelimit' : [],
  'string' : [],
}

foreach t : tests.keys()
  test(t, executable('test_' + t, 'test_' + t + '.cpp',
    dependencies: tests[t],
    include_directories: global_inc,
  ))
endforeach
