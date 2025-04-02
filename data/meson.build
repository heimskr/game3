# Generate and install the desktop file with translations
desktop_file = i18n.merge_file(
    input: 'gay.heimskr.Game3.desktop.in',
    output: 'gay.heimskr.Game3.desktop',
    type: 'desktop',
    po_dir: '../po',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
)

# Validate the desktop file if the utility is available
desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
    test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

# Generate and install the AppStream metadata file with translations
appstream_file = i18n.merge_file(
    input: 'gay.heimskr.Game3.appdata.xml.in',
    output: 'gay.heimskr.Game3.appdata.xml',
    po_dir: '../po',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'appdata')
)

# Validate the AppStream file if the utility is available
appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
    test('Validate appstream file', appstream_util,
        args: ['validate', appstream_file])
endif

# Install the GSettings schema file
install_data('gay.heimskr.Game3.gschema.xml',
    install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas')
)

# Validate the schema file if the utility is available
compile_schemas = find_program('glib-compile-schemas', required: false)
if compile_schemas.found()
    test('Validate schema file', compile_schemas,
        args: ['--strict', '--dry-run', meson.current_source_dir()])
endif

# Handle icons in a subdirectory
subdir('icons')

# Install sound files for the game
sound_files = ['sound1.ogg', 'sound2.ogg']
install_data(sound_files,
    install_dir: join_paths(get_option('datadir'), 'gay.heimskr.Game3', 'sounds')
)

# Install image asset files for the game
image_files = ['image1.png', 'image2.png']
install_data(image_files,
    install_dir: join_paths(get_option('datadir'), 'gay.heimskr.Game3', 'images')
)

# Install a default configuration file
install_data('config.ini',
    install_dir: join_paths(get_option('datadir'), 'gay.heimskr.Game3')
)

# Install the man page for game documentation
install_man('gay.heimskr.Game3.6')
