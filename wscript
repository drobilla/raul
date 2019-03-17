#!/usr/bin/env python

import os
import subprocess
import sys

from waflib import Options
from waflib.extras import autowaf

# Version of this package (even if built as a child)
RAUL_VERSION = '0.8.10'

# Library version (UNIX style major, minor, micro)
# major increment <=> incompatible changes
# minor increment <=> compatible changes (additions)
# micro increment <=> no interface changes
# Version history:
#   0.4.0 = 0,0,0
#   0.5.0 = 1,0,0 (SVN r1283)
#   0.5.1 = 2,0,0
#   0.6.0 = 3,0,0
#   0.6.1 = 4,0,0 (unreleased)
#   0.6.2 = 5,0,0 (unreleased)
#   0.6.3 = 6,0,0 (unreleased)
#   0.6.4 = 7,0,0 (unreleased)
#   0.6.5 = 8,0,0 (unreleased)
#   0.6.6 = 9,0,0 (unreleased)
#   0.7.0 = 9,0,0
#   0.8.0 = 10,0,0
RAUL_LIB_VERSION = '10.0.0'

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
    autowaf.set_c_lang(conf, 'c99')
    autowaf.set_cxx_lang(conf, 'c++11')

    if conf.env.DEST_OS == 'darwin':
        conf.check(framework_name='CoreServices')
        conf.env.FRAMEWORK_RAUL = ['CoreServices']

    conf.check_cxx(header_name='memory')
    conf.check_cxx(header_name='atomic')

    # TODO: Version includes and use autowaf.set_lib_env() here
    conf.env['INCLUDES_RAUL'] = [os.path.abspath(top) + '/raul']

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
    bld.install_files('${INCLUDEDIR}/raul', bld.path.ant_glob('raul/*.hpp'))
    bld.install_files('${INCLUDEDIR}/raul', bld.path.ant_glob('raul/*.h'))

    # Pkgconfig file
    dict = {'RAUL_PC_LIBS': ' '}
    if bld.env.DEST_OS == 'darwin':
        dict = {'RAUL_PC_LIBS': '-framework CoreServices'}
    autowaf.build_pc(bld, 'RAUL', RAUL_VERSION, '',
                     'GLIB GTHREAD', subst_dict=dict)

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
                      includes     = ['.', './src'],
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
    for t in tests:
        tst([os.path.join('./test', t)])

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
