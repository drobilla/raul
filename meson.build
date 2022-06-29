# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-or-later

project('raul', ['cpp'],
        version: '1.1.1',
        license: 'GPLv3+',
        meson_version : '>= 0.49.2',
        default_options: [
          'b_ndebug=if-release',
          'buildtype=release',
          'cpp_std=c++17',
        ])

raul_src_root = meson.current_source_dir()
versioned_name = 'raul-@0@'.format(meson.project_version().split('.')[0])

#######################
# Compilers and Flags #
#######################

# Required tools
pkg = import('pkgconfig')
cpp = meson.get_compiler('cpp')

# Set global warning flags
if get_option('strict') and not meson.is_subproject()
  subdir('meson/warnings')
endif
subdir('meson/suppressions')

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

# Generage pkg-config file for external dependants
pkg.generate(
  name: 'Raul',
  description: 'Real-time audio utility library',
  filebase: versioned_name,
  subdirs: [versioned_name],
  version: meson.project_version(),
)

# Install headers to a versioned include directory
install_headers(headers, subdir: versioned_name + '/raul')

#########
# Tests #
#########

if not get_option('tests').disabled()
  subdir('test')
endif
