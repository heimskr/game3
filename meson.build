project('game3', 'cpp',
          version: '0.1.0',
    meson_version: '>= 0.62.0',
  default_options: ['warning_level=2', 'werror=false', 'cpp_std=c++23',],
)

vcpkg = find_program('vcpkg', required : true)
run_command(vcpkg, 'install', '--x-wait-for-lock', '--x-manifest-root=' + meson.current_source_dir())

i18n = import('i18n')
gnome = import('gnome')
cc = meson.get_compiler('cpp')

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', 'game3')
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
configure_file(output: 'config.h', configuration: config_h)
add_project_arguments(['-I' + meson.project_build_root()], language: 'cpp')

project_cpp_args = []
test_cpp_args = [
  '-Wall',
  '-Wextra',
]
if get_option('buildtype') != 'plain'
  test_cpp_args += '-fstack-protector-strong'
endif
foreach arg: test_cpp_args
  if cc.has_multi_arguments(arg)
    project_cpp_args += arg
  endif
endforeach
add_project_arguments(project_cpp_args, language: 'cpp')

subdir('data')
subdir('src')
subdir('po')

gnome.post_install(
     glib_compile_schemas: true,
    gtk_update_icon_cache: true,
  update_desktop_database: true,
)