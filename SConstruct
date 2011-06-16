# Build system copied/modified from
# http://www.scons.org/cgi-bin/wiki/AdvancedBuildExample
# Original by zedshaw@zedshaw.com
import os
import sys
from build_support import *
from build_config import *

variables = Variables('custom.py')
variables.Add(EnumVariable('debug', 'Build with debug symbols', 'no',
                    ['yes','no']))
env = Environment(options=variables) #  = variables, tools=['mingw','qt4'])
# env = Environment()
env['QT4DIR'] = qt_dir
env['ENV']['PKG_CONFIG_PATH'] = qt_pkg_config_path
env.Tool('mingw');
env.Tool('qt4');

Help(variables.GenerateHelpText(env))

mode = "Release"
env.Append(CPPFLAGS = flgs)
print "env debug: " + env.get('debug')

if (env.get('debug') == 'yes'):
    print "Debug build"
    mode = "Debug"
    env.Append(CPPFLAGS = ['-g', '-O0'])
    env.Append(CPPDEFINES = 'DEBUG')
else:
    env.Append(CPPFLAGS = ['-O3'])
    env.Append(CPPDEFINES = 'NDEBUG')

# Build_config can use variables to denote the platform and
# build variant
env['BUILD_VARIANT']  = mode
env['BUILD_PLATFORM'] = sys.platform
#env['QT4DIR'] = qt_dir
#env['ENV']['PKG_CONFIG_PATH'] = qt_pkg_config_path

# Qt4 setup
env.EnableQt4Modules(['QtGui', 
                      'QtCore'
                     ])
                     
# Construct target directories and names. Since the
# build specific SConscript file is one level above
# the build dir, some trickery is required.
target_dir_base = '#' + SelectBuildDir(build_base_dir)
target_dir      = os.sep.join([target_dir_base, mode])
target_name     = os.sep.join([mode, target_name])

## Get the sources
sources_raw = DirGlob(source_base_dir, '*.cpp')
## Now must make a pure version relative to the build directory
## in Release/Debug (without the source_base_dir at start)
sources = []
for source in sources_raw:
    sources.append(os.sep.join([target_dir] + source.split(os.sep)[1:]))

# Set resource
sources.append(target_dir + os.sep + "resource.o") # os.sep.join([target_dir, "resource.o"]))
# resource = ""
resource = "#src/resource.rc"

# setup the include paths
env.Append(CPPPATH=include_search_path)
env.Append(LIBS=libs)
env.Append(LIBPATH=lib_search_path)
env.Append(CPPDEFINES=defs)

# start the build

# variables the sub build directories need
Export('env', 'sources', 'mode', 'target_name', 'resource', 'libs')
env.BuildDir(target_dir, source_base_dir, duplicate=0)
env.SConscript(target_dir_base + os.sep + 'SConscript')

