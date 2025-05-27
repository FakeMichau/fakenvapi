project('fakenvapi', 
    'cpp',
    default_options: [
        'cpp_std=c++20',
        'warning_level=2'
    ])

cpp = meson.get_compiler('cpp')

lib_dxgi = cpp.find_library('dxgi')

link_args = [
    '-static',
    '-static-libgcc',
    '-static-libstdc++',
    '-Wl,--file-alignment=4096'
]

compiler_args = [
    '-Wno-unused-parameter',
    '-D_CRT_SECURE_NO_WARNINGS',
    '-D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR'
]

spdlog_headers = include_directories('./external/spdlog/include')
xell_headers = include_directories('./external/xell/inc')

lib_xell = declare_dependency(
    include_directories: xell_headers,
    link_args: [meson.current_source_dir() / 'external/xell/lib/libxell.lib']
)

windows = import('windows')

rc_file = configure_file(
  input: 'resource.rc.in',
  output: 'resource.rc',
  configuration: {
    'FILEVERSION': '1,3,0,0',
  }
)

rc = windows.compile_resources(rc_file)

fakenvapi_version = vcs_tag(
    command: ['git', 'describe', '--always', '--tags', '--dirty=-dirty'],
    input:  'version.h.in',
    output: 'version.h'
)

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(link_args), language: 'cpp')

dxvk_cpu_family = target_machine.cpu_family()
if dxvk_cpu_family == 'x86_64'
    target_suffix = '64'
else
    target_suffix = ''
endif

subdir('src')
