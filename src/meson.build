game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

chemskr = subproject('chemskr')


game3_deps = [
	dependency('gtk4'),
	dependency('gtkmm-4.0'),
	dependency('threads'),
	dependency('eigen3'),
	dependency('libevent'),
	dependency('libevent_openssl'),
	dependency('openssl'),
	dependency('freetype2'),
	dependency('libzstd'),
	chemskr.get_variable('chemskr'),
]

game3_sources += gnome.compile_resources('game3-resources',
	'game3.gresource.xml',
	c_name: 'game3',
	source_dir: '../resources'
)

inc_dirs = [
	include_directories('../include'),
	include_directories('../vcpkg_installed/x64-linux/include'),
]

# install_headers('subprojects/chemskr/parser.h', subdir: 'chemskr')

executable('game3', game3_sources,
	dependencies : game3_deps,
	install : true,
	include_directories : [inc_dirs]
)
