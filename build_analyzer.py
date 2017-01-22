#!/usr/bin/env python2

from __future__ import print_function
import glob
import os
import platform
import subprocess
import sys
import time

cxx = os.environ.get('CXX')
if not cxx:
    cxx = "g++"
ld = cxx

#find out if we're running on mac or linux and set the dynamic library extension
dylib_ext = ""
if platform.system().lower() == "darwin":
    target = "macOS"
    dylib_ext = "dylib"
elif platform.system().lower() == "linux":
    if "mingw" in cxx:
        target = "Windows"
        dylib_ext = "dll"
    else:
        target = "Linux"
        dylib_ext = "so"
else:
    print("Unsupported build environment", file=sys.stderr)
    sys.exit(1)
    
print("Running on " + platform.system())
print("Building for: {}".format(target))

#make sure the release folder exists, and clean out any .o/.so file if there are any
if not os.path.exists("release"):
    os.makedirs("release")

os.chdir("release")
o_files = glob.glob("*.o")
o_files.extend(glob.glob("*." + dylib_ext))
for o_file in o_files:
    os.remove(o_file)
os.chdir("..")


#make sure the debug folder exists, and clean out any .o/.so files if there are any
if not os.path.exists("debug"):
    os.makedirs("debug")

os.chdir("debug")
o_files = glob.glob("*.o");
o_files.extend(glob.glob("*." + dylib_ext))
for o_file in o_files:
    os.remove(o_file)
os.chdir("..")

#find all the cpp files in /source.  We'll compile all of them
os.chdir("source")
cpp_files = glob.glob("*.cpp");
os.chdir("..")

#specify the search paths/dependencies/options for gcc
include_paths = ["include"]
link_paths = ["lib"]

# bit hacky but works for GCC and Clang
target64 = False
for line in subprocess.check_output([cxx, '-v'], stderr=subprocess.STDOUT).splitlines():
    if line.startswith('Target: '):
        target64 = '64' in line

if target64 and not platform.system().lower() == "darwin":
    link_dependencies = ["Analyzer64"] #refers to libAnalyzer.dylib or libAnalyzer.so
else:
    link_dependencies = ["Analyzer"] #refers to libAnalyzer.dylib or libAnalyzer.so


debug_compile_flags = ['-O0', '-g']
release_compile_flags = ['-O3']

cxxflags = ['-I{}'.format(p) for p in include_paths]
cxxflags.extend(['-w', '-c', '-fpic'])

ldflags = ['-L{}'.format(p) for p in link_paths]
ldflags.extend(['-l{}'.format(l) for l in link_dependencies])

if dylib_ext == "dylib":
    ldflags.append("-dynamiclib")
else:
    ldflags.append("-shared")

def call_shell(cmd):
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    while True:
        time.sleep(0.1)
        ret = p.poll()
        print(p.stdout.read(), end='')
        print(p.stderr.read(), file=sys.stderr, end='')
        if ret != None:
            break
    return ret

def compile(cpp_file, obj_file, flags=[]):
    cmd = [cxx]
    cmd.extend(cxxflags)
    cmd.extend(flags)
    cmd.extend(['-o', obj_file])
    cmd.append(cpp_file)
    print(" ".join(cmd))
    return call_shell(cmd) == 0

def link(objs, out_file, flags=[]):
    cmd = [ld]
    cmd.extend(ldflags)
    cmd.extend(flags)
    cmd.extend(['-o', out_file])
    cmd.extend(objs)
    print(" ".join(cmd))
    return call_shell(cmd) == 0

result = 0

release_objs = []
debug_objs = []

#loop through all the cpp files, build up the gcc command line, and attempt to compile each cpp file
for cpp_file in cpp_files:
    obj = os.path.join('release', cpp_file.replace( ".cpp", ".o" ))
    release_objs.append(obj)
    if not compile(os.path.join('source', cpp_file), obj, release_compile_flags):
        result = 1
    obj = os.path.join('debug', cpp_file.replace( ".cpp", ".o" ))
    debug_objs.append(obj)
    if not compile(os.path.join('source', cpp_file), obj, debug_compile_flags):
        result = 1

if result == 0:

    #figgure out what the name of this analyzer is
    analyzer_name = ""
    for cpp_file in cpp_files:
        if cpp_file.endswith("Analyzer.cpp"):
            analyzer_name = cpp_file.replace("Analyzer.cpp", "")
            break

    lib_file = 'lib{}Analyzer.{}'.format(analyzer_name, dylib_ext)

    if not link(release_objs, os.path.join('release', lib_file)):
        result = 1
    if not link(debug_objs, os.path.join('debug', lib_file)):
        result = 1

if result != 0:
    print('')
    print('************************************************')
    print('Errors were generated during build!!!')
    print('************************************************')
    print('')
sys.exit(result)
