# Copyright 2019-2023 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-or-later

project(
  'raul',
  ['cpp'],
  default_options: [
    'b_ndebug=if-release',
    'buildtype=release',
    'cpp_std=c++17',
  ],
  license: 'GPLv3+',
  meson_version: '>= 0.56.0',
  version: '2.0.1',
)

raul_src_root = meson.current_source_dir()
raul_build_root = meson.current_build_dir()
versioned_name = 'raul-@0@'.format(meson.project_version().split('.')[0])

#######################
# Compilers and Flags #
#######################

# Required tools
pkg = import('pkgconfig')
cpp = meson.get_compiler('cpp')

# Set global warning suppressions
warning_level = get_option('warning_level')
cpp_suppressions = []
if cpp.get_id() in ['clang', 'emscripten']
  if warning_level == 'everything'
    cpp_suppressions += [
      '-Wno-c++17-extensions',
      '-Wno-c++98-compat',
      '-Wno-padded',
      '-Wno-switch-default',
      '-Wno-unsafe-buffer-usage',
      '-Wno-weak-vtables',
    ]

    if not meson.is_cross_build()
      cpp_suppressions += [
        '-Wno-poison-system-directories',
      ]
    endif

    if host_machine.system() == 'windows'
      cpp_suppressions += [
        '-Wno-nonportable-system-include-path',
      ]
    endif
  endif

elif cpp.get_id() == 'gcc'
  if warning_level == 'everything'
    cpp_suppressions += [
      '-Wno-abi-tag',
      '-Wno-effc++',
      '-Wno-inline',
      '-Wno-multiple-inheritance',
      '-Wno-padded',
      '-Wno-suggest-attribute=pure',
      '-Wno-switch-default',
      '-Wno-useless-cast',
    ]
  endif

elif cpp.get_id() == 'msvc'
  cpp_suppressions += [
    '/experimental:external',
    '/external:W0',
    '/external:anglebrackets',
  ]

  if warning_level == 'everything'
    cpp_suppressions += [
      '/wd4514', # unreferenced inline function removed
      '/wd4625', # copy constructor implicitly deleted
      '/wd4626', # assignment operator implicitly deleted
      '/wd4710', # function not inlined
      '/wd4711', # function selected for automatic inline expansion
      '/wd5026', # move constructor implicitly deleted
      '/wd5027', # move assignment operator implicitly deleted
      '/wd5045', # compiler will insert Spectre mitigation
      '/wd5262', # implicit fall-through
    ]
  endif

  if warning_level in ['everything', '3']
    cpp_suppressions += [
      '/wd4706', # assignment within conditional expression
    ]
  endif
endif

cpp_suppressions = cpp.get_supported_arguments(cpp_suppressions)

################
# Dependencies #
################

thread_dep = dependency('threads')

###########
# Library #
###########

headers = files(
  'include/raul/Array.hpp',
  'include/raul/Deletable.hpp',
  'include/raul/DoubleBuffer.hpp',
  'include/raul/Exception.hpp',
  'include/raul/Maid.hpp',
  'include/raul/Noncopyable.hpp',
  'include/raul/Path.hpp',
  'include/raul/Process.hpp',
  'include/raul/RingBuffer.hpp',
  'include/raul/Semaphore.hpp',
  'include/raul/Socket.hpp',
  'include/raul/Symbol.hpp',
)

# Declare dependency for internal meson dependants
raul_dep = declare_dependency(
  include_directories: include_directories('include'),
)

# Generate pkg-config file for external dependants
pkg.generate(
  name: 'Raul',
  description: 'Real-time audio utility library',
  filebase: versioned_name,
  subdirs: [versioned_name],
  version: meson.project_version(),
)

# Override pkg-config dependency for internal meson dependants
meson.override_dependency(versioned_name, raul_dep)

# Install headers to a versioned include directory
install_headers(headers, subdir: versioned_name + '/raul')

#########
# Tests #
#########

if not get_option('tests').disabled()
  subdir('test')
endif
