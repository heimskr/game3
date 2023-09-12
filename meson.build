project('game3', 'cpp',
          version: '0.1.0',
    meson_version: '>= 0.62.0',
  default_options: ['warning_level=2', 'werror=false', 'cpp_std=c++23',],
)

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
  '-Wcast-align',
  '-Wdeclaration-after-statement',
  '-Werror=address',
  '-Werror=array-bounds',
  '-Werror=empty-body',
  '-Werror=implicit',
  '-Werror=implicit-function-declaration',
  '-Werror=incompatible-pointer-types',
  '-Werror=init-self',
  '-Werror=int-conversion',
  '-Werror=int-to-pointer-cast',
  '-Werror=main',
  '-Werror=misleading-indentation',
  '-Werror=missing-braces',
  '-Werror=missing-include-dirs',
  '-Werror=nonnull',
  '-Werror=overflow',
  '-Werror=parenthesis',
  '-Werror=pointer-arith',
  '-Werror=pointer-to-int-cast',
  '-Werror=redundant-decls',
  '-Werror=return-type',
  '-Werror=sequence-point',
  '-Werror=shadow',
  '-Werror=strict-prototypes',
  '-Werror=trigraphs',
  '-Werror=undef',
  '-Werror=write-strings',
  '-Wformat-nonliteral',
  '-Wignored-qualifiers',
  '-Wimplicit-function-declaration',
  '-Wlogical-op',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-cast-function-type',
  '-Wno-dangling-pointer',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wundef',
  '-Wuninitialized',
  '-Wunused',
  '-fno-strict-aliasing',
  ['-Werror=format-security', '-Werror=format=2'],
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
