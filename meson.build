project('renity', ['c', 'cpp']
  , version : '0.2.0'
  , license: 'MPL-2.0'
  , meson_version : '>=0.56.0' # For win_subsystem
  , default_options : ['buildtype=debug', 'warning_level=3', 'cpp_eh=s', 'cpp_rtti=true', 'c_std=c11', 'cpp_std=c++11', 'prefix=/usr/local'])


# Add DEBUG/NDEBUG defines
if get_option('buildtype').to_lower() == 'debug'
  add_project_arguments('-DRENITY_DEBUG', language: ['c', 'cpp'])
else
  add_project_arguments('-DRENITY_NDEBUG', language: ['c', 'cpp'])
endif


# Run subdirectory scripts
subdir('include')
subdir('src')
if get_option('RENITY_BUILD_DOCS')
  subdir('docs')
endif
if get_option('RENITY_BUILD_TESTS')
  subdir('tests')
endif
if get_option('RENITY_BUILD_SERVER')
  subdir('server')
endif
subdir('clients')
