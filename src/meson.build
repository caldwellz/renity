# Get compiler details - used in version info and finding dependencies
cppcomp = meson.get_compiler('cpp')


# These arguments are only used to build the library
# and not the executables that use the library.
lib_args = ['-DBUILDING_RENITY']


# Find dependencies
# - SDL2
dep_sdl = dependency('sdl2', required : false)
if not dep_sdl.found()
    dep_sdl = cppcomp.find_library('SDL2', has_headers : ['SDL2/SDL.h'])
endif

# - SDL2_image
dep_sdlimage = dependency('SDL2_image', required : false)
if not dep_sdlimage.found()
    dep_sdlimage = cppcomp.find_library('SDL2_image', has_headers : ['SDL_image.h'])
endif

# - PhysFS
dep_physfs = dependency('physfs', required : false)
if not dep_physfs.found()
    dep_physfs = cppcomp.find_library('physfs', has_headers : ['physfs.h'])
endif
lib_deps = [dep_sdl, dep_sdlimage, dep_physfs]


# List sources and process subdirectories - version file comes later
lib_srcs = files([
  'Window.cc'
])
subdir('utils')


# Configure version info
version_data = configuration_data()
version_data.set('name', meson.project_name())
version_data.set('version', meson.project_version())
version_data.set('vcs_tag_placeholder', '@VCS_TAG@')
version_data.set('compiler', cppcomp.get_id())
version_data.set('compiler_version', cppcomp.version())
versionpre = configure_file(configuration : version_data
  , input : 'version.c.in.pre'
  , output : 'version.c.in')
git = find_program('git', required : false)
versionfile = vcs_tag(command : ['git', 'rev-parse', '--short', 'HEAD'], fallback : get_option('buildtype').to_upper()
  , input : versionpre
  , output : 'version.c')
# To get commit number instead of ID: ['git', 'rev-list', '--count', 'HEAD']


# Create the library target
lib_target = library(meson.project_name(), [lib_srcs, versionfile[0]], dependencies : lib_deps, include_directories : lib_incdir, install : true, c_args : lib_args, cpp_args : lib_args, gnu_symbol_visibility : 'hidden')
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
