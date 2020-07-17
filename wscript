#!/usr/bin/env python

import os
import subprocess
import sys

from waflib import Options
from waflib.extras import autowaf

# Library and package version (UNIX style major, minor, micro)
# major increment <=> incompatible changes
# minor increment <=> compatible changes (additions)
# micro increment <=> no interface changes
RAUL_VERSION       = '1.0.0'
RAUL_MAJOR_VERSION = '1'

# Mandatory waf variables
APPNAME = 'raul'        # Package name for waf dist
VERSION = RAUL_VERSION  # Package version for waf dist
top     = '.'           # Source directory
out     = 'build'       # Build directory

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx', cache=True)
    conf.load('autowaf', cache=True)
    autowaf.set_cxx_lang(conf, 'c++11')

    if Options.options.ultra_strict:
        autowaf.add_compiler_flags(conf.env, 'cxx', {
            'clang': [
                '-Wno-padded',
                '-Wno-weak-vtables',
            ],
            'gcc': [
                '-Wno-multiple-inheritance',
                '-Wno-padded',
                '-Wno-useless-cast',
            ],
            'msvc': [
                '/wd4514',  # unreferenced inline function removed
                '/wd4625',  # copy constructor implicitly deleted
                '/wd4626',  # assignment operator implicitly deleted
                '/wd4706',  # assignment within conditional expression
                '/wd5026',  # move constructor implicitly deleted
                '/wd5027',  # move assignment operator implicitly deleted
            ]
        })

    if conf.env.DEST_OS == 'darwin':
        conf.check(framework_name='CoreServices')
        conf.env.FRAMEWORK_RAUL = ['CoreServices']

    conf.check_cxx(header_name='memory')
    conf.check_cxx(header_name='atomic')

    autowaf.set_lib_env(conf, 'raul', RAUL_VERSION, has_objects=False)
    conf.write_config_header('raul_config.h', remove=False)

    autowaf.display_summary(conf,
                            {'Unit tests': bool(conf.env.BUILD_TESTS)})

tests = ['array_test',
         'build_test',
         'double_buffer_test',
         'maid_test',
         'path_test',
         'ringbuffer_test',
         'sem_test',
         'symbol_test',
         'thread_test',
         'time_test']

if sys.platform != 'win32':
    tests += ['socket_test']

def build(bld):
    # Headers
    includedir = '${INCLUDEDIR}/raul-%s/raul' % RAUL_MAJOR_VERSION
    bld.install_files(includedir, bld.path.ant_glob('raul/*.hpp'))
    bld.install_files(includedir, bld.path.ant_glob('raul/*.h'))

    # Pkgconfig file
    pc_libs = '-framework CoreServices' if bld.env.DEST_OS == 'darwin' else ''
    pc_dict = {'RAUL_MAJOR_VERSION': RAUL_MAJOR_VERSION,
               'RAUL_PC_LIBS': pc_libs}
    autowaf.build_pc(bld, 'RAUL', RAUL_VERSION, RAUL_MAJOR_VERSION, [],
                     subst_dict=pc_dict)

    framework = ''
    if bld.env.DEST_OS == 'darwin':
        framework = ' CoreServices '

    if bld.env.BUILD_TESTS:
        test_libs      = []
        test_cxxflags  = []
        test_linkflags = []
        if bld.env.DEST_OS != 'win32':
            test_cxxflags  = ['-pthread']
            test_linkflags = ['-pthread']
        if bld.env.DEST_OS != 'darwin' and bld.env.DEST_OS != 'win32':
            test_libs += ['rt']
        if not bld.env.NO_COVERAGE:
            test_cxxflags  += ['--coverage']
            test_linkflags += ['--coverage']

        # Unit tests
        for i in tests:
            obj = bld(features     = 'cxx cxxprogram',
                      source       = os.path.join('test', i + '.cpp'),
                      includes     = ['.'],
                      lib          = test_libs,
                      use          = 'libraul_static',
                      uselib       = 'GLIB GTHREAD',
                      framework    = framework,
                      target       = os.path.join('test', i),
                      install_path = '',
                      cxxflags     = test_cxxflags,
                      linkflags    = test_linkflags,
                      libs         = test_libs)

    # Documentation
    autowaf.build_dox(bld, 'RAUL', RAUL_VERSION, top, out)

def test(tst):
    with tst.group('Unit') as check:
        for t in tests:
            check([os.path.join('./test', t)])

def lint(ctx):
    "checks code for style issues"
    import subprocess
    cmd = ("clang-tidy -p=. -header-filter=.* -checks=\"*," +
           "-android*," +
           "-clang-analyzer-alpha.*," +
           "-cppcoreguidelines-no-malloc," +
           "-cppcoreguidelines-owning-memory," +
           "-cppcoreguidelines-pro-bounds-array-to-pointer-decay," +
           "-cppcoreguidelines-pro-bounds-pointer-arithmetic," +
           "-cppcoreguidelines-pro-type-const-cast," +
           "-cppcoreguidelines-pro-type-reinterpret-cast," +
           "-cppcoreguidelines-pro-type-vararg," +
           "-cppcoreguidelines-special-member-functions," +
           "-fuchsia-default-arguments," +
           "-fuchsia-overloaded-operator," +
           "-google-runtime-references," +
           "-hicpp-no-array-decay," +
           "-hicpp-no-malloc," +
           "-hicpp-signed-bitwise," +
           "-hicpp-special-member-functions," +
           "-hicpp-vararg," +
           "-llvm-header-guard," +
           "-misc-suspicious-string-compare," +
           "-misc-unused-parameters," +
           "-modernize-make-unique," +
           "-readability-else-after-return," +
           "-readability-implicit-bool-conversion\" " +
           "../raul/*.hpp ../test/*.cpp")
    subprocess.call(cmd, cwd='build', shell=True)

def posts(ctx):
    path = str(ctx.path.abspath())
    autowaf.news_to_posts(
        os.path.join(path, 'NEWS'),
        {'title'        : 'Raul',
         'description'  : autowaf.get_blurb(os.path.join(path, 'README')),
         'dist_pattern' : 'http://download.drobilla.net/raul-%s.tar.bz2'},
        { 'Author' : 'drobilla',
          'Tags'   : 'Hacking, LAD, LV2, RAUL' },
        os.path.join(out, 'posts'))
