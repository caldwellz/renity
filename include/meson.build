# Add the current directory to the include path
lib_incdirs = [include_directories('.')]

# Fill in config.h with the build options
conf_data = configuration_data()
conf_data.set('RENITY_BUILD_SHARED', get_option('default_library').contains('shared'))
conf_data.set('RENITY_USE_EXCEPTIONS', not get_option('cpp_eh').contains('none'))
conf_data.set('RENITY_USE_RTTI', get_option('cpp_rtti'))
conf_data.set('RENITY_USE_STL', get_option('RENITY_USE_STL'))
conffile = configure_file(configuration : conf_data
  , input : 'config.h.in'
  , output : 'config.h')

lib_headers = [
    conffile
  , 'types.h'
  , 'version.h'
  , 'Action.h'
  , 'ActionHandler.h'
  , 'ActionManager.h'
  , 'Dictionary.h'
  , 'Dimension2D.h'
#  , 'EntityManager.h'
  , 'GL_PointRenderer.h'
  , 'GL_TileRenderer.h'
  , 'HashTable.h'
  , 'InputMapper.h'
  , 'Point2D.h'
  , 'Rect2D.h'
  , 'Resource.h'
  , 'ResourceManager.h'
  , 'Sprite.h'
  , 'Window.h'
]

# Install headers - add others to lib_headers above
base_include_subdir = meson.project_name()
install_headers(lib_headers, subdir : meson.project_name())

# Process subdirectories
subdir('resources')
subdir('utils')
