project('DshotAnalyzer', 'cpp',
  default_options: ['cpp_std=c++17'])

cxx = meson.get_compiler('cpp')

if host_machine.cpu_family() == 'x86_64' and host_machine.system() != 'darwin'
  analyzer_name = 'Analyzer64'
else
  analyzer_name = 'Analyzer'
endif

sdk_lib = cxx.find_library(analyzer_name, dirs: meson.current_source_dir() + '/sdk/lib')

source = [
  'source/DshotAnalyzer.cpp',
  'source/DshotAnalyzerResults.cpp',
  'source/DshotAnalyzerSettings.cpp',
  'source/DshotSimulationDataGenerator.cpp',
]
includes = include_directories('source', 'sdk/include')
libs = 'sdk/lib'
shared_library('DShotAnalyzer', source, include_directories: includes, dependencies: sdk_lib)

