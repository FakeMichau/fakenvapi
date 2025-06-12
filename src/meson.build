src = files([
    'main.cpp',
    'fakenvapi.cpp',
    'low_latency.cpp',
    'low_latency_d3d.cpp',
    'low_latency_vk.cpp',
    'low_latency_tech/ll_antilag2.cpp',
    'low_latency_tech/ll_antilag_vk.cpp',
    'low_latency_tech/ll_xell.cpp',
    'low_latency_tech/ll_latencyflex.cpp',
    'vulkan_hooks.cpp',
    'util.cpp',
    'log.cpp'
])

fakenvapi_version_dep = declare_dependency(sources: fakenvapi_version)

shared_library(
    'nvapi' + target_suffix,
    src,
    rc,
    name_prefix: '',
    dependencies: [fakenvapi_version_dep, lib_dxgi, lib_detours],
    include_directories: [spdlog_headers, xell_headers, detours_headers, vulkan_headers],
    install: true,
)
