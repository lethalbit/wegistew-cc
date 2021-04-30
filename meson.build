project(
	'wegistew',
	'cpp',
	default_options: [
		'cpp_std=c++17',
		'warning_level=3',
	],
	version: '0.0.1',
	license: [
		'BSD-3-Clause',
	],
	meson_version: '>=0.57.0',
)

cxx = meson.get_compiler('cpp')

if not meson.is_subproject()
	install_headers('wegistew.hh')
endif

wegistew_dep = declare_dependency(
	include_directories: [
		include_directories('.'),
	],
	version: meson.project_version()
)
