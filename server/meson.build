# List sources, dependencies, and any extra compiler arguments
server_srcs = files(['main.cc'])
server_deps = [dep_sdl]
server_args = []


# Configure and build the Windows resource file
if host_machine.system() == 'windows'
  apprcfile = configure_file(configuration : version_data
    , input : 'res/win32/app.rc.in'
    , output : 'app.rc')
  icofile = configure_file(copy : true
    , input : 'res/win32/app.ico'
    , output : 'app.ico')
  server_srcs += import('windows').compile_resources(apprcfile
    , depend_files : [icofile])
endif


# Create the executable target
server_target = executable(
  meson.project_name() + '_server'
  , server_srcs
  , dependencies : server_deps
  , link_with : lib_target
  , include_directories : lib_incdir
  , install : true
  , c_args : server_args
  , cpp_args : server_args
)
