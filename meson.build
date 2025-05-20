project('midirun-config', 'cpp', 'c', version: '0.0.0')

gtk = dependency('gtkmm-4.0', version: '>=4.0.0')
toml = dependency('tomlplusplus')
rtmidi = dependency('rtmidi')
libevdev = dependency('libevdev', required: true)
libudev = dependency('libudev', required: true)

deps = [gtk, toml, rtmidi, libevdev, libudev]

src = [
  'src/main.cpp',
  'src/mainwindow.cpp',
  'include/mainwindow.hpp',
  'include/midilisten.hpp',
  'include/labeledentrybox.hpp',
  'src/labeledentrybox.cpp',
  'include/devicedropdown.hpp',
  'src/devicedropdown.cpp',
  'src/getkeycode.cpp',
  'include/getkeycode.hpp',
  'include/statusbar.hpp',
  'src/statusbar.cpp',
  'include/mappingentry.hpp',
  'include/configentry.hpp',
  'src/mappingentry.cpp',
]

configure_file(
  input: 'data/midirun-config.desktop.in',
  output: 'midirun-config.desktop',
  configuration: {
    'execpath': meson.current_build_dir() + '/midirun-config',
    'datadir': get_option('datadir'),
  },
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications'),
)

executable('midirun-config', src, dependencies: deps, install: true)

# install_data('data/udev/99-midirun-config.rules', install_dir: '/etc/udev/rules.d')

#meson.add_install_script('post_install.sh')

install_data(
  'data/style.css',
  install_dir: join_paths(get_option('datadir'), 'midirun-config'),
)
