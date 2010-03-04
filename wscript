#!/usr/bin/env python
import autowaf
import Options
import os

# Version of this package (even if built as a child)
RAUL_VERSION = '0.6.5'

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
RAUL_LIB_VERSION = '8.0.0'

# Variables for 'waf dist'
APPNAME = 'raul'
VERSION = RAUL_VERSION

# Mandatory variables
srcdir = '.'
blddir = 'build'

def set_options(opt):
	autowaf.set_options(opt)
	opt.add_option('--test', action='store_true', default=False, dest='build_tests',
			help="Build unit tests")
	opt.add_option('--log-colour', action='store_true', default=True, dest='log_colour',
			help="Coloured console/log output")
	opt.add_option('--log-debug', action='store_true', default=False, dest='log_debug',
			help="Print debugging output")

def configure(conf):
	autowaf.configure(conf)
	autowaf.display_header('Raul Configuration')
	conf.check_tool('compiler_cxx')
	autowaf.check_pkg(conf, 'glib-2.0', atleast_version='2.2',
			uselib_store='GLIB', mandatory=True)
	autowaf.check_pkg(conf, 'gthread-2.0', atleast_version='2.14.0',
			uselib_store='GTHREAD', mandatory=True)
	
	conf.env['BUILD_TESTS'] = Options.options.build_tests

	if Options.options.log_colour:
		conf.define('LOG_COLOUR', 1)
	if Options.options.log_debug:
		conf.define('LOG_DEBUG', 1)
	
	conf.write_config_header('raul-config.h')

	# Boost headers
	autowaf.check_header(conf, 'boost/shared_ptr.hpp', mandatory=True)
	autowaf.check_header(conf, 'boost/weak_ptr.hpp', mandatory=True)
	autowaf.check_header(conf, 'boost/utility.hpp', mandatory=True)
	
	autowaf.print_summary(conf)
	autowaf.display_msg(conf, "Unit tests", str(conf.env['BUILD_TESTS']))
	print
	
tests = '''
	test/atom_test
	test/atomic_test
	test/list_test
	test/midi_ringbuffer_test
	test/path_test
	test/quantize_test
	test/queue_test
	test/ringbuffer_test
	test/smf_test
	test/table_test
	test/thread_test
	test/time_test
'''

def build(bld):
	# Headers
	bld.install_files('${INCLUDEDIR}/raul', 'raul/*.hpp')
	bld.install_files('${INCLUDEDIR}/raul', 'raul/*.h')

	# Pkgconfig file
	autowaf.build_pc(bld, 'RAUL', RAUL_VERSION, 'GLIBMM GTHREAD')

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

	# Library
	obj = bld.new_task_gen('cxx', 'shlib')
	obj.export_incdirs = ['.']
	obj.source       = lib_source
	obj.includes     = ['.', './src']
	obj.name         = 'libraul'
	obj.target       = 'raul'
	obj.uselib       = 'GLIBMM GTHREAD'
	obj.install_path = '${LIBDIR}'
	obj.vnum         = RAUL_LIB_VERSION
	
	if bld.env['BUILD_TESTS']:
		# Static library (for unit test code coverage)
		obj = bld.new_task_gen('cxx', 'staticlib')
		obj.source       = lib_source
		obj.includes     = ['.', './src']
		obj.name         = 'libraul_static'
		obj.target       = 'raul_static'
		obj.uselib       = 'GLIBMM GTHREAD'
		obj.install_path = ''
		obj.cxxflags      = [ '-fprofile-arcs',  '-ftest-coverage' ]

		# Unit tests
		for i in tests.split():
			obj = bld.new_task_gen('cxx', 'program')
			obj.source       = i + '.cpp'
			obj.includes     = ['.', './src']
			obj.uselib_local = 'libraul_static'
			obj.uselib       = 'GLIB GLIBMM'
			obj.libs         = 'gcov'
			obj.target       = i
			obj.install_path = ''
			obj.cxxflags      = [ '-fprofile-arcs',  '-ftest-coverage' ]

	# Documentation
	autowaf.build_dox(bld, 'RAUL', RAUL_VERSION, srcdir, blddir)
	bld.install_files('${HTMLDIR}', blddir + '/default/doc/html/*')

def test(ctx):
	autowaf.run_tests(ctx, APPNAME, tests.split())

def shutdown():
	autowaf.shutdown()

