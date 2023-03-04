# Get compiler details - used in version info and finding dependencies
cppcomp = meson.get_compiler('cpp')


# These arguments are only used to build the library
# and not the executables that use the library.
lib_args = ['-DBUILDING_RENITY']


# Find dependencies
# - SDL3
dep_sdl = dependency('sdl3', required : false)
if not dep_sdl.found()
    dep_sdl = cppcomp.find_library('SDL3', has_headers : ['SDL3/SDL.h'])
endif

# - SDL3_image
dep_sdlimage = dependency('SDL3_image', required : false)
if not dep_sdlimage.found()
    dep_sdlimage = cppcomp.find_library('SDL3_image', has_headers : ['SDL3/SDL_image.h'])
endif

# - PhysFS
dep_physfs = dependency('physfs', required : false)
if not dep_physfs.found()
    dep_physfs = cppcomp.find_library('physfs', has_headers : ['physfs.h'])
endif

# - C math library (usually only needed on Linux)
dep_m = cppcomp.find_library('m', required : false)
lib_deps = [dep_sdl, dep_sdlimage, dep_physfs, dep_m]

# List sources and process subdirectories - version file comes later
lib_srcs = files([
  'Sprite.cc'
, 'Texture.cc'
, 'Window.cc'
])
subdir('utils')


# Obtain version info from Meson and git (or use defaults if git data is not found)
version_str = meson.project_version()
version_parts = version_str.split('.')
revision_num = 0
revision_commit = get_option('buildtype').to_upper()
git = find_program('git', required : false)
if git.found()
  tagres = run_command(git, 'rev-parse', '--short', 'HEAD'
    , capture : true
    , check : false)
  if tagres.returncode() == 0
    revision_commit = tagres.stdout().strip()
    revision_num = run_command(git, 'rev-list', '--count', 'HEAD', capture : true, check : false).stdout().strip()
  endif
endif


# Configure version file
version_data = configuration_data()
version_data.set('publisher', get_option('PUBLISHER_NAME'))
version_data.set('description', get_option('PRODUCT_DESCRIPTION'))
version_data.set('name', meson.project_name())
version_data.set('version', version_str)
version_data.set('version_major', version_parts[0])
version_data.set('version_minor', version_parts[1])
version_data.set('version_patch', version_parts[2])
version_data.set('revision_num', revision_num)
version_data.set('revision_commit', revision_commit)
version_data.set('build_type', get_option('buildtype').to_lower())
version_data.set('compiler', cppcomp.get_id())
version_data.set('compiler_version', cppcomp.version())
versionfile = configure_file(configuration : version_data
  , input : 'version.c.in'
  , output : 'version.c')


# Create the library target
lib_target = library(
  meson.project_name()
  , [lib_srcs, versionfile]
  , dependencies : lib_deps
  , include_directories : lib_incdir
  , install : true
  , c_args : lib_args
  , cpp_args : lib_args
  , gnu_symbol_visibility : 'hidden'
)
declare_dependency(sources : versionfile, link_with : lib_target)
declare_dependency(include_directories : lib_incdir, link_with : lib_target)


# Generate pkgconfig info
pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : meson.project_name(),
  filebase : meson.project_name(),
  description : '',
  subdirs : meson.project_name(),
  libraries : lib_target,
  version : meson.project_version()
)
