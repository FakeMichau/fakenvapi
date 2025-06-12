dll = shared_library(
    'nvapi'+target_suffix,
    ['main.cpp', 'fakenvapi.cpp', 'low_latency.cpp', 'low_latency_tech.cpp', 'll_antilag2.cpp', 'll_antilag_vk.cpp', 'll_xell.cpp', 'll_latencyflex.cpp', 'vulkan_hooks.cpp', 'util.cpp', 'log.cpp', fakenvapi_version],
    rc,
    name_prefix : '',
    dependencies : [ lib_dxgi, lib_detours ],
    include_directories: [ spdlog_headers, xell_headers, detours_headers, vulkan_headers ],
    install: true
)
