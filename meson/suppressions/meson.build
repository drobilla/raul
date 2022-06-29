# Copyright 2020-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-or-later

# Project-specific warning suppressions.
#
# This should be used in conjunction with the generic "warnings" sibling that
# enables all reasonable warnings for the compiler.  It lives here just to keep
# the top-level meson.build more readable.

#######
# C++ #
#######

if is_variable('cpp')
  cpp_suppressions = []

  if get_option('strict')
    if cpp.get_id() in ['clang', 'emscripten']
      cpp_suppressions = [
        '-Wno-c++17-extensions',
        '-Wno-padded',
        '-Wno-weak-vtables',
      ]

    elif cpp.get_id() == 'gcc'
      cpp_suppressions = [
        '-Wno-abi-tag',
        '-Wno-inline',
        '-Wno-multiple-inheritance',
        '-Wno-padded',
        '-Wno-suggest-attribute=pure',
        '-Wno-switch-default',
        '-Wno-useless-cast',
      ]

    elif cpp.get_id() == 'msvc'
      cpp_suppressions = [
        '/wd4514', # unreferenced inline function removed
        '/wd4625', # copy constructor implicitly deleted
        '/wd4626', # assignment operator implicitly deleted
        '/wd4706', # assignment within conditional expression
        '/wd4710', # function not inlined
        '/wd4711', # function selected for automatic inline expansion
        '/wd5026', # move constructor implicitly deleted
        '/wd5027', # move assignment operator implicitly deleted
        '/wd5045', # compiler will insert Spectre mitigation
      ]
    endif
  endif

  if cpp.get_id() in ['clang', 'emscripten']
    cpp_suppressions += [
      '-Wno-nullability-extension',
    ]
  endif

  cpp_suppressions = cpp.get_supported_arguments(cpp_suppressions)
endif
