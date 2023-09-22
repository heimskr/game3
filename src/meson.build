game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

chemskr = subproject('chemskr')
sqlitecpp = subproject('sqlitecpp', default_options: 'default_library=static')

zig = find_program('zig', required: true)

resgen_gen = custom_target('resgen-gen',
	input: 'resgen.zig',
	output: 'resgen',
	command: [zig, 'build-exe', '-O', 'ReleaseSmall', '--main-pkg-path', '..', '@INPUT@'])

resources_h = custom_target('resources-header-gen',
	output: 'resources.h',
	capture: true,
	command: ['../src/resgen', '-h'])

message(meson.current_build_dir())

resources_o = custom_target('resources-obj-gen',
	input: 'resources.zig',
	output: 'resources.o',
	command: ['../hack.sh', '@BUILD_ROOT@', meson.project_source_root()])

resources_dep = declare_dependency(sources: resources_o[0])

game3_deps = [
	dependency('gtk4'),
	dependency('gtkmm-4.0'),
	dependency('threads'),
	dependency('eigen3'),
	dependency('libevent'),
	dependency('libevent_openssl'),
	dependency('libevent_pthreads'),
	dependency('openssl'),
	dependency('freetype2'),
	dependency('libzstd'),
	dependency('sqlite3'),
	dependency('gl'),
	dependency('glfw3'),
	dependency('x11'),
	chemskr.get_variable('chemskr'),
	sqlitecpp.get_variable('sqlitecpp_dep'),
	resources_dep
]

if build_machine.system() != 'darwin'
	game3_deps += dependency('glu')
endif

game3_sources += gnome.compile_resources('game3-resources',
	'game3.gresource.xml',
	c_name: 'game3',
	source_dir: '..' / 'resources')

game3_sources += resources_h[0]

inc_dirs = [include_directories('..' / 'include')]

link_args = []

if get_option('vcpkg_triplet') != ''
	vcpkg_root = 'vcpkg_installed' / get_option('vcpkg_triplet')
	inc_dirs += include_directories('..' / vcpkg_root / 'include')
	link_args += '-L../' + vcpkg_root + '/lib/libnoise-static.a'
else
	link_args += '-lnoise'
endif

executable('game3', game3_sources,
	dependencies: game3_deps,
	link_with: [chemskr.get_variable('chemskr_lib')],
	link_args: link_args,
	install: true,
	include_directories: [inc_dirs])
