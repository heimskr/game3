project('game3', 'cpp', version: '0.41.0',
	meson_version: '>= 0.62.0',
	default_options: ['warning_level=2', 'werror=false', 'cpp_std=c++23'])

cmake = import('cmake')
cxx = meson.get_compiler('cpp')

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', 'game3')
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
source_root = meson.project_source_root()
add_project_arguments([
	'-I' + meson.project_build_root(),
	'-I' + source_root + '/subprojects',
	'-I' + source_root + '/subprojects/libnoise/src',
	'-I' + source_root + '/subprojects/FastNoise2/include',
], language: 'cpp')

project_cpp_args = ['-DV8_COMPRESS_POINTERS', '-DV8_ENABLE_SANDBOX']
if build_machine.system() == 'windows'
	project_cpp_args += '-D_POSIX_C_SOURCE'
endif

test_cpp_args = [
	'-Wall',
	'-Wextra',
	'-Wno-volatile'
]

if get_option('enable_zip8')
	config_h.set('ENABLE_ZIP8', '1')
endif

if get_option('is_flatpak')
	config_h.set('IS_FLATPAK', '1')
endif

if get_option('vcpkg_triplet') != ''
	config_h.set('USING_VCPKG', '1')
endif

if get_option('discord_rich_presence')
	config_h.set('DISCORD_RICH_PRESENCE', '1')
endif

if get_option('enable_scripting')
	config_h.set('GAME3_ENABLE_SCRIPTING', '1')
endif

if get_option('buildtype') != 'plain'
	test_cpp_args += '-fstack-protector-strong'
endif

configure_file(output: 'config.h', configuration: config_h)

foreach arg: test_cpp_args
	if cxx.has_multi_arguments(arg)
		project_cpp_args += arg
	endif
endforeach

add_project_arguments(project_cpp_args, language: 'cpp')

subdir('src')
