windows = import('windows')

rc_file = configure_file(
  input: 'resource.rc.in',
  output: 'resource.rc',
  configuration: {
    'FILEVERSION': '1,0,0,0',
  }
)

rc = windows.compile_resources(rc_file)

dll = shared_library(
    'nvapi'+target_suffix,
    ['main.cpp', 'fakenvapi.cpp', 'util.cpp', 'log.cpp'],
    rc,
    name_prefix : '',
    dependencies : [ lib_dxgi ],
    include_directories: [ spdlog_headers ],
    install: true
)
