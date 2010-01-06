#!/usr/bin/env python
import autowaf
import Options

# Version of this package (even if built as a child)
RAUL_VERSION = '0.6.0'

# Library version (UNIX style major, minor, micro)
# major increment <=> incompatible changes
# minor increment <=> compatible changes (additions)
# micro increment <=> no interface changes
# Version history:
#   0.4.0 = 0,0,0
#   0.5.0 = 1,0,0 (SVN r1283)
#   0.5.1 = 2,0,0
#   0.6.0 = 3,0,0
RAUL_LIB_VERSION = '3.0.0'

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

def build(bld):
	# Headers
	bld.install_files('${INCLUDEDIR}/raul', 'raul/*.hpp')
	bld.install_files('${INCLUDEDIR}/raul', 'raul/*.h')

	# Pkgconfig file
	autowaf.build_pc(bld, 'RAUL', RAUL_VERSION, 'GLIBMM GTHREAD')

	# Library
	obj = bld.new_task_gen('cxx', 'shlib')
	obj.source = '''
		src/Configuration.cpp
		src/Maid.cpp
		src/Path.cpp
		src/SMFReader.cpp
		src/SMFWriter.cpp
		src/Symbol.cpp
		src/Thread.cpp
		src/log.cpp
	'''
	obj.export_incdirs = ['.']
	obj.includes     = ['.', './src']
	obj.name         = 'libraul'
	obj.target       = 'raul'
	obj.uselib       = 'GLIBMM GTHREAD'
	obj.install_path = '${LIBDIR}'
	obj.vnum         = RAUL_LIB_VERSION

	# Unit tests
	bld.add_subdirs('tests')

	# Documentation
	autowaf.build_dox(bld, 'RAUL', RAUL_VERSION, srcdir, blddir)
	bld.install_files('${HTMLDIR}', blddir + '/default/doc/html/*')

def shutdown():
	autowaf.shutdown()

