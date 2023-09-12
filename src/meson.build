game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

subproject('chemskr')

game3_deps = [
	dependency('gtk4'),
	dependency('gtkmm-4.0'),
	dependency('threads'),
	dependency('eigen3'),
]

game3_sources += gnome.compile_resources('game3-resources',
	'game3.gresource.xml',
	c_name: 'game3',
	source_dir: '../resources'
)

incdir = include_directories('../include')

executable('game3', game3_sources,
	dependencies : game3_deps,
	install : true,
	include_directories : incdir
)
