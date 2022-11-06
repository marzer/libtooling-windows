project(
	'libtooling-windows',
	'cpp',
	version : '2022.11.02' # ??
)

assert(host_machine.system() == 'windows')

cpp = meson.get_compiler('cpp')
is_x64 = host_machine.cpu_family() == 'x86_64'
is_debug = get_option('debug')

libs = [] # appended to in subdir

subdir('lib')

libtooling_windows_dep = declare_dependency(
	dependencies: libs,
	include_directories: [
		include_directories('include'),
		is_x64 ? include_directories('include/win64') : include_directories('include/win32')
	]
)
meson.override_dependency(meson.project_name(), libtooling_windows_dep)
