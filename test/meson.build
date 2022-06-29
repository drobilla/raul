# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-or-later

if not get_option('tests').disabled() and not meson.is_subproject()
  # Check release metadata
  autoship = find_program('autoship', required: get_option('tests'))
  if autoship.found()
    test(
      'autoship', autoship,
      args: ['test', raul_src_root],
      suite: 'data',
    )
  endif
endif

###################
# Header Warnings #
###################

header_args = []
if cpp.get_id() == 'clang'
  header_args += [
    '-Wno-c++17-extensions',
    '-Wno-padded',
    '-Wno-weak-vtables',
  ]
elif cpp.get_id() == 'gcc'
  header_args += [
    '-Wno-abi-tag',
    '-Wno-multiple-inheritance',
    '-Wno-padded',
    '-Wno-switch-default',
    '-Wno-useless-cast',
  ]
elif cpp.get_id() == 'msvc'
  header_args += [
    '/wd4626', # assignment operator implicitly deleted
    '/wd5027', # move assignment operator implicitly deleted
  ]
endif

# Test that headers have (almost) no warnings (ignoring usual suppressions)
test(
  'test_headers',
  executable(
    'test_headers',
    files('headers/test_headers.cpp'),
    cpp_args: header_args,
    dependencies: raul_dep,
  ),
)

##############
# Unit Tests #
##############

tests = [
  'array_test',
  'build_test',
  'double_buffer_test',
  'maid_test',
  'path_test',
  'ringbuffer_test',
  'sem_test',
  'symbol_test',
  'thread_test',
]

if host_machine.system() != 'windows'
  tests += [
    'socket_test',
  ]
endif

foreach test : tests
  test(
    test,
    executable(
      test,
      '@0@.cpp'.format(test),
      cpp_args: cpp_suppressions,
      dependencies: [raul_dep, thread_dep],
    ),
    suite: 'unit',
  )
endforeach
