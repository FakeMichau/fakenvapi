project('nvapi-dummy', 
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
    '-D_CRT_SECURE_NO_WARNINGS'
]

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(link_args), language: 'cpp')

dxvk_cpu_family = target_machine.cpu_family()
if dxvk_cpu_family == 'x86_64'
    target_suffix = '64'
else
    target_suffix = ''
endif

subdir('src')
