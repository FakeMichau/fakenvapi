dll = shared_library(
    'nvapi'+target_suffix,
    ['main.cpp', 'fakenvapi.cpp', 'util.cpp', 'log.cpp'],
    name_prefix : '',
    dependencies : [ lib_dxgi ],
    include_directories: [ spdlog_headers ],
    install: true
)
