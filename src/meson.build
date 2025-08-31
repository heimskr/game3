game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

quasi_msys2 = get_option('quasi_msys2')
sorta_windows = build_machine.system() == 'windows' or quasi_msys2 != ''
actually_linux = build_machine.system() == 'linux' and quasi_msys2 == ''

if get_option('discord_rich_presence') == true
	game3_sources += run_command('discord-grabber.sh', check: true).stdout().strip().split('\n')
endif

cmake = import('cmake')

chemskr = subproject('chemskr')
namegen = subproject('namegen')
sqlitecpp = subproject('sqlitecpp', default_options: 'default_library=static')
miniaudio = subproject('miniaudio')
clip = cmake.subproject('clip')

zipios_options = cmake.subproject_options()
curlpp_options = cmake.subproject_options()

zlib_path = get_option('zlib_path')
if zlib_path != ''
	curlpp_options.add_cmake_defines({'ZLIB_LIBRARY:FILEPATH': zlib_path})
	zipios_options.add_cmake_defines({'ZLIB_LIBRARY:FILEPATH': zlib_path})
endif

if quasi_msys2 != ''
	zipios_options.append_compile_args('cpp', '-I' + quasi_msys2 + '/include')
endif

zipios_options.add_cmake_defines({'BUILD_SHARED_LIBS': false, 'BUILD_DOCUMENTATION': false, 'BUILD_ZIPIOS_TESTS': false})
zipios_options.set_override_option('cpp_std', 'c++23')
zipios = cmake.subproject('Zipios', options: zipios_options)

curlpp = cmake.subproject('curlpp', options: curlpp_options)

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
	curlpp.dependency('curlpp_static'),
	dependency('sigc++-3.0'),
	dependency('libpng'),
	dependency('libcurl'),
	dependency('leveldb'),
	zipios.dependency('zipios'),
	clip.dependency('clip'),
	chemskr.get_variable('chemskr'),
	namegen.get_variable('namegen'),
	miniaudio.get_variable('miniaudio'),
	sqlitecpp.get_variable('sqlitecpp_dep'),
]

if get_option('use_unwind')
	game3_deps += dependency('libunwind')
endif

if actually_linux
	game3_deps += dependency('xcb')
endif

inc_dirs = [
	include_directories('..' / 'include'),
	include_directories('..' / 'voronoi' / 'src'),
	include_directories('..' / 'subprojects' / 'fixed_string' / 'include'),
	include_directories('..' / 'subprojects' / 'backward-cpp'),
]

link_with = [chemskr.get_variable('chemskr_lib'), namegen.get_variable('namegen_lib')]

link_args = [
	'-lbfd',
	# '-lstdc++exp', # If using std::stacktrace
]

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

if actually_linux
	game3_deps += dependency('glu')
	link_args += '-lX11'
endif

if get_option('external_fastnoise2')
	if sorta_windows
		link_args += '../subprojects/FastNoise2/build_windows/src/libFastNoise.a'
	else
		link_args += '../subprojects/FastNoise2/build/Release/lib/libFastNoise.a'
	endif
else
	game3_deps += dependency('FastNoise2', modules: ['FastNoise2::FastNoise'])
endif

if get_option('vcpkg_triplet') != ''
	vcpkg_root = 'vcpkg_installed' / get_option('vcpkg_triplet')
	inc_dirs += include_directories('..' / vcpkg_root / 'include')
	link_args += '../' + vcpkg_root + '/lib/libnoise-static.a'
elif sorta_windows
	link_args += '../subprojects/libnoise/build/src/libnoise-static.a'
	link_args += '-lws2_32'
	link_args += '-lopengl32'
	link_args += '-lwsock32'
	link_args += '-lstdc++exp'
	link_args += '-lcrypt32'
	# link_args += '-lgcc'
	link_args += '-lglew32'
	link_args += '-lglu32'
	link_args += '-ldbghelp'
	inc_dirs += include_directories('..' / 'subprojects' / 'libnoise' / 'src')
elif actually_linux
	link_args += '-lnoise'
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
