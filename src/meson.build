game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

if get_option('discord_rich_presence') == true
	game3_sources += run_command('discord-grabber.sh', check: true).stdout().strip().split('\n')
endif

chemskr = subproject('chemskr')
namegen = subproject('namegen')
sqlitecpp = subproject('sqlitecpp', default_options: 'default_library=static')
miniaudio = subproject('miniaudio')

game3_deps = [
	dependency('gtk4'),
	dependency('gtkmm-4.0'),
	dependency('threads'),
	dependency('eigen3'),
	dependency('openssl'),
	dependency('freetype2'),
	dependency('libzstd'),
	dependency('sqlite3'),
	dependency('gl'),
	dependency('glfw3'),
	dependency('asio'),
	dependency('liblz4'),
	dependency('nlohmann_json'),
	dependency('glm'),
	dependency('boost'),
	dependency('FastNoise2', modules: ['FastNoise2::FastNoise']),
	dependency('vte-2.91-gtk4'),
	chemskr.get_variable('chemskr'),
	namegen.get_variable('namegen'),
	miniaudio.get_variable('miniaudio'),
	sqlitecpp.get_variable('sqlitecpp_dep'),
]

if get_option('enable_scripting')
	game3_deps += [
		dependency('v8'),
		dependency('v8_libplatform'),
	]
endif

link_args = []

if build_machine.system() == 'linux'
	game3_deps += dependency('glu')
	link_args += '-lX11'
endif

game3_sources += gnome.compile_resources('game3-resources',
	'game3.gresource.xml',
	c_name: 'game3',
	source_dir: '..' / 'resources')

inc_dirs = [
	include_directories('..' / 'include'),
	include_directories('..' / 'voronoi' / 'src'),
]

if get_option('vcpkg_triplet') != ''
	vcpkg_root = 'vcpkg_installed' / get_option('vcpkg_triplet')
	inc_dirs += include_directories('..' / vcpkg_root / 'include')
	link_args += '../' + vcpkg_root + '/lib/libnoise-static.a'
else
	link_args += '-lnoise'
endif

if get_option('discord_rich_presence') == true
	if get_option('is_flatpak') == true
		game3_deps += cxx.find_library('discord_game_sdk', required: true, dirs: ['/app/lib'])
	else
		game3_deps += cxx.find_library('discord_game_sdk', required: true)
	endif
	inc_dirs += include_directories('..' / 'discord')
endif

executable('game3', game3_sources,
	dependencies: game3_deps,
	link_with: [chemskr.get_variable('chemskr_lib'), namegen.get_variable('namegen_lib')],
	link_args: link_args,
	install: true,
	include_directories: [inc_dirs])
