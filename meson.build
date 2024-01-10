project('game3', 'cpp', version: '0.32.0',
	meson_version: '>= 0.62.0',
	default_options: ['warning_level=2', 'werror=false', 'cpp_std=c++20'])

i18n = import('i18n')
gnome = import('gnome')
cxx = meson.get_compiler('cpp')

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', 'game3')
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
add_project_arguments(['-I' + meson.project_build_root()], language: 'cpp')

project_cpp_args = []
test_cpp_args = [
	'-Wall',
	'-Wextra',
	'-Wno-volatile'
]

if get_option('is_flatpak')
	config_h.set('IS_FLATPAK', '1')
endif

if get_option('vcpkg_triplet') != ''
	config_h.set('USING_VCPKG', '1')
endif

if get_option('discord_rich_presence')
	config_h.set('DISCORD_RICH_PRESENCE', '1')
endif

if get_option('buildtype') != 'plain'
	test_cpp_args += '-fstack-protector-strong'
endif

if build_machine.system() == 'darwin'
	config_h.set('GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED', '1')
endif

configure_file(output: 'config.h', configuration: config_h)

foreach arg: test_cpp_args
	if cxx.has_multi_arguments(arg)
		project_cpp_args += arg
	endif
endforeach

add_project_arguments(project_cpp_args, language: 'cpp')

subdir('src')

if build_machine.system() == 'linux'
	subdir('data')
	subdir('po')
	gnome.post_install(
		glib_compile_schemas: true,
		gtk_update_icon_cache: true,
		update_desktop_database: true,
	)
endif
