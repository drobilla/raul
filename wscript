#!/usr/bin/env python
import os
import subprocess
import waflib.Options as Options
import waflib.extras.autowaf as autowaf

# Version of this package (even if built as a child)
RAUL_VERSION = '0.8.5'

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
    autowaf.set_options(opt)
    opt.add_option('--test', action='store_true', dest='build_tests',
                   help="Build unit tests")

def configure(conf):
    conf.line_just = 40
    conf.load('compiler_cxx')
    autowaf.configure(conf)
    autowaf.display_header('Raul Configuration')

    conf.check_cxx(cxxflags=["-std=c++0x"])
    conf.env.append_unique('CXXFLAGS', ['-std=c++0x'])

    if Options.platform == 'darwin':
        conf.check(framework_name='CoreServices')
        conf.env.FRAMEWORK_RAUL = ['CoreServices']

    conf.env.BUILD_TESTS = Options.options.build_tests
    if conf.env.BUILD_TESTS:
        conf.check_cxx(lib='gcov',
                       define_name='HAVE_GCOV',
                       mandatory=False)

    conf.check_cxx(header_name='memory')
    conf.check_cxx(header_name='atomic')

    # TODO: Version includes and use autowaf.set_lib_env() here
    conf.env['INCLUDES_RAUL'] = [os.path.abspath(top) + '/raul']

    autowaf.define(conf, 'RAUL_VERSION', RAUL_VERSION)
    conf.write_config_header('raul_config.h', remove=False)

    autowaf.display_msg(conf, "Unit tests", bool(conf.env.BUILD_TESTS))
    print('')

tests = '''
        test/array_test
        test/build_test
        test/double_buffer_test
        test/maid_test
        test/path_test
        test/queue_test
        test/ringbuffer_test
        test/sem_test
        test/socket_test
        test/symbol_test
        test/thread_test
        test/time_test
        test/uri_test
'''

def build(bld):
    # Headers
    bld.install_files('${INCLUDEDIR}/raul', bld.path.ant_glob('raul/*.hpp'))
    bld.install_files('${INCLUDEDIR}/raul', bld.path.ant_glob('raul/*.h'))

    # Pkgconfig file
    dict = {'RAUL_PC_LIBS': ' '}
    if Options.platform == 'darwin':
        dict = {'RAUL_PC_LIBS': '-framework CoreServices'}
    autowaf.build_pc(bld, 'RAUL', RAUL_VERSION, '',
                     'GLIB GTHREAD', subst_dict=dict)

    framework = ''
    if Options.platform == 'darwin':
        framework = ' CoreServices '

    if bld.env.BUILD_TESTS:
        test_libs     = ['pthread']
        test_cxxflags = []
        if bld.env.DEST_OS != 'darwin' and bld.env.DEST_OS != 'win32':
            test_libs += ['rt']
        if bld.is_defined('HAVE_GCOV'):
            test_libs     += ['gcov']
            test_cxxflags += ['-fprofile-arcs', '-ftest-coverage']

        # Unit tests
        for i in tests.split():
            obj = bld(features     = 'cxx cxxprogram',
                      source       = i + '.cpp',
                      includes     = ['.', './src'],
                      lib          = test_libs,
                      use          = 'libraul_static',
                      uselib       = 'GLIB GTHREAD',
                      framework    = framework,
                      target       = i,
                      install_path = '',
                      cxxflags     = test_cxxflags)

    # Documentation
    autowaf.build_dox(bld, 'RAUL', RAUL_VERSION, top, out)

def test(ctx):
    autowaf.pre_test(ctx, APPNAME, dirs=['.', 'src', 'test'])
    autowaf.run_tests(ctx, APPNAME, tests.split(), dirs=['.', 'src', 'test'])
    autowaf.post_test(ctx, APPNAME, dirs=['.', 'src', 'test'])

def lint(ctx):
    subprocess.call('cpplint.py --filter=-whitespace/comments,-whitespace/tab,-whitespace/braces,-whitespace/labels,-build/header_guard,-readability/casting,-whitespace/line_length,-runtime/references,-readability/streams,-build/include_order raul/*', shell=True)
