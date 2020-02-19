# Build system copied/modified from
# http://www.scons.org/cgi-bin/wiki/AdvancedBuildExample
# Original by zedshaw@zedshaw.com
import os
import sys
from build_support import *
from build_config import *

opts = Variables('custom.py')
opts.Add(EnumVariable('debug', 'Build with debug symbols', 'no',
                    allowed_values=('yes','no')))
opts.Add(EnumVariable('win32', 'Build for win32 target', 'no',
                    allowed_values=('yes','no')))
env = Environment(variables = opts) # , tools=['mingw', 'qt4'])
# env = Environment()
Help(opts.GenerateHelpText(env))

platform = None
env['BUILD_PLATFORM'] = sys.platform
env['ENV']['PKG_CONFIG_PATH'] = qt_pkg_config_path

if (sys.platform == 'win32'):
    env.Tool('mingw')
    env['QT4DIR'] = qt_dir
elif (env.get('win32') == 'yes'):
    env.Tool('mingw')
    env['CC'] = 'i686-w64-mingw32-gcc'
    env['CXX'] = 'i686-w64-mingw32-g++'
    env['AR'] = 'i686-w64-mingw32-ar'
    env['RANLIB'] = 'i686-w64-mingw32-ranlib'
    env['BUILD_PLATFORM'] = 'win32'
    env['QT4DIR'] = qt_dir
    platform = 'win32'
else:
    env.Tool('gcc')
    env['QT4DIR'] = ''

env.Tool('qt4')

mode = "Release"
env.Append(CPPFLAGS = cflags)
env.Append(LINKFLAGS = lflags)
print("env debug: " + env.get('debug'))
print("env win32: " + env.get('win32'))

if (env.get('debug') == 'yes'):
    print("Debug build")
    mode = "Debug"
    env.Append(CPPFLAGS = ['-g', '-O0'])
    env.Append(CPPDEFINES = 'DEBUG')
else:
    env.Append(CPPFLAGS = ['-O3'])
    env.Append(CPPDEFINES = 'NDEBUG')
    env.Append(LINKFLAGS = ['-s'])

# Build_config can use variables to denote the platform and
# build variant
env['BUILD_VARIANT']  = mode
#env['BUILD_PLATFORM'] = sys.platform
#env['QT4DIR'] = qt_dir
#env['ENV']['PKG_CONFIG_PATH'] = qt_pkg_config_path

# Qt4 setup
env.EnableQt4Modules(['QtGui', 
                      'QtCore'
                     ],
                     crosscompiling = (platform == 'win32'))
                     
# Construct target directories and names. Since the
# build specific SConscript file is one level above
# the build dir, some trickery is required.
target_dir_base = '#' + SelectBuildDir(build_base_dir, platform)
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
env.VariantDir(target_dir, source_base_dir, duplicate=0)
env.SConscript(target_dir_base + os.sep + 'SConscript')

