#!/usr/bin/env python
import subprocess

from waflib.extras import autowaf as autowaf
import waflib.Options as Options

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

# Variables for 'waf dist'
APPNAME = 'raul'
VERSION = RAUL_VERSION

# Mandatory variables
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx')
    autowaf.set_options(opt)
    opt.add_option('--test', action='store_true', default=False, dest='build_tests',
                    help="Build unit tests")
    opt.add_option('--log-colour', action='store_true', default=True, dest='log_colour',
                    help="Coloured console/log output")
    opt.add_option('--log-debug', action='store_true', default=False, dest='log_debug',
                    help="Print debugging output")
    opt.add_option('--cpp0x', action='store_true', default=False, dest='cpp0x',
                    help="Use C++0x smart pointers instead of boost")

def configure(conf):
    conf.load('compiler_cxx')
    autowaf.configure(conf)
    conf.line_just = 40
    autowaf.display_header('Raul Configuration')
    autowaf.check_pkg(conf, 'glib-2.0', atleast_version='2.2',
                                      uselib_store='GLIB', mandatory=True)
    autowaf.check_pkg(conf, 'gthread-2.0', atleast_version='2.14.0',
                                      uselib_store='GTHREAD', mandatory=True)

    if Options.platform == 'darwin':
        conf.check(framework_name='CoreServices')
        conf.env.FRAMEWORK_RAUL = ['CoreServices']

    conf.env.BUILD_TESTS = Options.options.build_tests
    if conf.env.BUILD_TESTS:
        conf.check_cxx(lib='gcov',
                       define_name='HAVE_GCOV',
                       mandatory=False)

    if Options.options.log_colour:
        autowaf.define(conf, 'RAUL_LOG_COLOUR', 1)
    if Options.options.log_debug:
        autowaf.define(conf, 'RAUL_LOG_DEBUG', 1)

    if Options.options.cpp0x:
        conf.env.append_value('CXXFLAGS', [ '-std=c++0x' ])
        autowaf.check_header(conf, 'cxx', 'memory')
        autowaf.check_header(conf, 'cxx', 'atomic')
        autowaf.define(conf, 'RAUL_CPP0x', 1)
    else:
        autowaf.check_header(conf, 'cxx', 'boost/shared_ptr.hpp')
        autowaf.check_header(conf, 'cxx', 'boost/weak_ptr.hpp')

    conf.write_config_header('raul_config.h', remove=False)

    autowaf.display_msg(conf, "Unit tests", str(conf.env.BUILD_TESTS))
    print('')

tests = '''
        test/atom_test
        test/atomic_test
        test/list_test
        test/path_test
        test/ptr_test
        test/quantize_test
        test/queue_test
        test/ringbuffer_test
        test/sem_test
        test/smf_test
        test/table_test
        test/thread_test
        test/time_test
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

    lib_source = '''
            src/Configuration.cpp
            src/Maid.cpp
            src/Path.cpp
            src/SMFReader.cpp
            src/SMFWriter.cpp
            src/Symbol.cpp
            src/Thread.cpp
            src/log.cpp
    '''

    framework = ''
    if Options.platform == 'darwin':
        framework = ' CoreServices '

    def set_defines(obj):
        if bld.env.RAUL_CPP0x:
            obj.defines = ['RAUL_CPP0x']

    # Library
    obj = bld(features        = 'cxx cxxshlib',
              export_includes = ['.'],
              source          = lib_source,
              includes        = ['.', './src'],
              name            = 'libraul',
              target          = 'raul',
              uselib          = 'GLIB GTHREAD',
              lib             = ['pthread'],
              framework       = framework,
              install_path    = '${LIBDIR}',
              vnum            = RAUL_LIB_VERSION)
    set_defines(obj);

    if bld.env.BUILD_TESTS:
        test_libs     = []
        test_cxxflags = []
        if bld.is_defined('HAVE_GCOV'):
            test_libs     += ['gcov']
            test_cxxflags += ['-fprofile-arcs', '-ftest-coverage']

        # Static library (for unit test code coverage)
        obj = bld(features     = 'cxx cxxstlib',
                  source       = lib_source,
                  includes     = ['.', './src'],
                  lib          = test_libs,
                  name         = 'libraul_static',
                  target       = 'raul_static',
                  uselib       = 'GLIB GTHREAD',
                  framework    = framework,
                  install_path = '',
                  cxxflags     = test_cxxflags)
        set_defines(obj);

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
            set_defines(obj);

    # Documentation
    autowaf.build_dox(bld, 'RAUL', RAUL_VERSION, top, out)

    bld.add_post_fun(autowaf.run_ldconfig)

def test(ctx):
    autowaf.pre_test(ctx, APPNAME, dirs=['.', 'src', 'test'])
    autowaf.run_tests(ctx, APPNAME, tests.split(), dirs=['.', 'src', 'test'])
    autowaf.post_test(ctx, APPNAME, dirs=['.', 'src', 'test'])

def lint(ctx):
    subprocess.call('cpplint.py --filter=-whitespace/comments,-whitespace/tab,-whitespace/braces,-whitespace/labels,-build/header_guard,-readability/casting,-readability/todo,-build/namespaces,-whitespace/line_length,-runtime/rtti,-runtime/references,-whitespace/blank_line,-runtime/sizeof,-readability/streams,-whitespace/operators,-whitespace/parens,-build/include,-whitespace/comma,-whitespace/newline `find -name *.cpp -or -name *.hpp`', shell=True)
