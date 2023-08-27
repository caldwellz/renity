lib_incdirs += [
  include_directories('imgui')
  # Add the generated GLES3/gl3.h using the build path
, include_directories('.')
]


# Use generic KHR/khrplatform.h unless configured otherwise (safe default because many platforms either won't have one, or will use a generic one anyway)
if get_option('RENITY_USE_GENERIC_KHR_HEADERS')
  lib_incdirs += [include_directories('EGL-Registry/api')]
endif


lib_args += ['-DIMGUI_IMPL_OPENGL_ES3']
if get_option('buildtype').to_lower() == 'debug'
  lib_args += ['-DIMGUI_IMPL_OPENGL_DEBUG']
endif


# Generate API loader(s) from flext template(s)
python3 = import('python').find_installation('python3', required: true, modules: ['wheezy.template'])
flext_gles3 = custom_target(
  'flext_gles30'
, output: 'flextGLES3.c'
, input: ['flextgl/flextGLgen.py', 'flext_templates/gles30/gles30_profile.txt', 'flext_templates/gles30/gl3.h.template', 'flext_templates/gles30/flextGLES3.c.template']
, command: [python3, '@INPUT0@', '-D', '@OUTDIR@', '-t', '../src/3rdparty/flext_templates/gles30', '@INPUT1@']
)
# Do it again since there's no easy way to copy it to "GLES3/gl3.h", except by regenerating with a custom output path
flext_gl3_h = custom_target(
  'gl3_h'
, depends: [flext_gles3]
, output: 'gl3.h'
, input: ['flextgl/flextGLgen.py', 'flext_templates/gles30/gles30_profile.txt', 'flext_templates/gles30/gl3.h.template', 'flext_templates/gles30/flextGLES3.c.template']
, command: [python3, '@INPUT0@', '-D', 'src/3rdparty/GLES3', '-t', '../src/3rdparty/flext_templates/gles30', '@INPUT1@']
)

lib_srcs += [flext_gles3, flext_gl3_h]
lib_srcs += files([
    'duktape/duktape.cc'
  , 'imgui/backends/imgui_impl_opengl3.cpp'
  , 'imgui/backends/imgui_impl_sdl3.cpp'
  , 'imgui/imgui_demo.cpp'
  , 'imgui/imgui_draw.cpp'
  , 'imgui/imgui_tables.cpp'
  , 'imgui/imgui_widgets.cpp'
  , 'imgui/imgui.cpp'
])
