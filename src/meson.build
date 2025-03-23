game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

if get_option('discord_rich_presence') == true
	game3_sources += run_command('discord-grabber.sh', check: true).stdout().strip().split('\n')
endif

cmake = import('cmake')

chemskr = subproject('chemskr')
namegen = subproject('namegen')
sqlitecpp = subproject('sqlitecpp', default_options: 'default_library=static')
miniaudio = subproject('miniaudio')
# fastnoise2 = cmake.subproject('FastNoise2')

game3_deps = [
	dependency('glibmm-2.68'),
	dependency('threads'),
	dependency('openssl'),
	dependency('freetype2'),
	dependency('libzstd'),
	dependency('gl'),
	dependency('glfw3'),
	dependency('asio'),
	dependency('liblz4'),
	dependency('boost', modules: ['json']),
	# fastnoise2.dependency('FastNoise2', modules: ['FastNoise2::FastNoise']),
	# dependency('FastNoise2', modules: ['FastNoise2::FastNoise']),
	dependency('sigc++-3.0'),
	dependency('libpng'),
	chemskr.get_variable('chemskr'),
	namegen.get_variable('namegen'),
	miniaudio.get_variable('miniaudio'),
	sqlitecpp.get_variable('sqlitecpp_dep'),
]

inc_dirs = [
	include_directories('..' / 'include'),
	include_directories('..' / 'voronoi' / 'src'),
	include_directories('..' / 'subprojects' / 'fixed_string' / 'include'),
]

link_with = [chemskr.get_variable('chemskr_lib'), namegen.get_variable('namegen_lib')]

link_args = []

if get_option('enable_zip8')
	zip8 = subproject('zip8')
	game3_deps += zip8.get_variable('zip8')
	inc_dirs += zip8.get_variable('inc_dir')
	link_with += zip8.get_variable('zip8_object')
endif

if get_option('enable_scripting')
	game3_deps += [
		dependency('v8'),
		dependency('v8_libplatform'),
	]
endif

if build_machine.system() == 'linux'
	game3_deps += dependency('glu')
	link_args += '-lX11'
endif

if get_option('vcpkg_triplet') != ''
	vcpkg_root = 'vcpkg_installed' / get_option('vcpkg_triplet')
	inc_dirs += include_directories('..' / vcpkg_root / 'include')
	link_args += '../' + vcpkg_root + '/lib/libnoise-static.a'
elif build_machine.system() == 'linux'
	link_args += '-lnoise'
elif build_machine.system() == 'cygwin'
	link_args += '../subprojects/FastNoise2/build/lib/libFastNoise.a'
	link_args += '../subprojects/libnoise/build/src/libnoise-static.a'
	inc_dirs += include_directories('..' / 'subprojects' / 'libnoise' / 'src')
endif

if build_machine.system() == 'darwin'
	link_args += ['-L/opt/homebrew/Cellar/llvm/19.1.5/lib/c++']
else
	game3_deps += cxx.find_library('m')
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
	link_with: link_with,
	link_args: link_args,
	install: true,
	include_directories: [inc_dirs])
