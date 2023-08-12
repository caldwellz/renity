# List sources, dependencies, and any extra compiler arguments
client_desktop_srcs = files(['main.cc'])
client_desktop_deps = [dep_sdl]
client_desktop_args = []
asset_install_dir = 'lib' / meson.project_name()


if host_machine.system() == 'windows'
  # Configure and build the Windows resource file
  apprcfile = configure_file(configuration : version_data
    , input : 'res/win32/app.rc.in'
    , output : 'app.rc')
  icofile = import('fs').copyfile('res/win32/app.ico', 'app.ico')
  client_desktop_srcs += import('windows').compile_resources(apprcfile, depends : [icofile])

  # Install asset packages alongside the binary
  asset_install_dir = 'bin'
endif


# Create the executable target
client_desktop_target = executable(
  meson.project_name()
  , client_desktop_srcs
  , dependencies : client_desktop_deps
  , link_with : lib_target
  , include_directories : lib_incdirs
  , install : true
  , c_args : client_desktop_args
  , cpp_args : client_desktop_args
  , win_subsystem: 'windows'
)


# Package assets into zips and include them in the install target
# zipfile command-line interface was apparently added in Python 3.5, but specific-version checking might require the very latest Meson
python3 = import('python').find_installation('python3', required: true, modules: ['zipfile'])
foreach name, deps: asset_packages
  custom_target(
    name + '_pkg'
  , install: true
  , install_dir: asset_install_dir
  , output: name + '.pkg'
  , input: deps
  , command: [python3, '-m', 'zipfile', '-c', '@OUTPUT@', '@SOURCE_ROOT@' / 'assets' / name]
  )
endforeach
