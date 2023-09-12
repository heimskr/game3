game3_sources = run_command('grabber.sh', check: true).stdout().strip().split('\n')

game3_deps = [
  dependency('gtk4'),
]

game3_sources += gnome.compile_resources('game3-resources',
  'game3.gresource.xml',
  c_name: 'game3'
)

executable('game3', game3_sources,
  dependencies: game3_deps,
       install: true,
)
