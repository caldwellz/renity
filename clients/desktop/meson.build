# List sources, dependencies, and any extra compiler arguments
client_desktop_srcs = files(['main.cc'])
client_desktop_deps = [dep_sdl]
client_desktop_args = []


# Configure and build the Windows resource file
if host_machine.system() == 'windows'
  apprcfile = configure_file(configuration : version_data
    , input : 'res/win32/app.rc.in'
    , output : 'app.rc')
  icofile = configure_file(copy : true
    , input : 'res/win32/app.ico'
    , output : 'app.ico')
  client_desktop_srcs += import('windows').compile_resources(apprcfile
    , depend_files : [icofile])
endif


# Create the executable target
client_desktop_target = executable(
  meson.project_name()
  , client_desktop_srcs
  , dependencies : client_desktop_deps
  , link_with : lib_target
  , include_directories : lib_incdir
  , install : true
  , c_args : client_desktop_args
  , cpp_args : client_desktop_args
)
